/* The file is part of Snowman decompiler. */
/* Copyright Markus Gothe <nietzsche@lysator.liu.se> 2015 */
/* See doc/licenses.asciidoc for the licensing information. */

#include "BfdParser.h"

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */
#include <QFile>
#include <QDebug>

#include <nc/common/Foreach.h>
#include <nc/common/LogToken.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Reader.h>
#include <nc/core/image/Relocation.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/ParseError.h>
#include <nc/core/input/Utils.h>

#define PACKAGE 1 /* Work-around for bfd.h */
#define PACKAGE_VERSION 1 /* Work-around for bfd.h */

#include "bfd.h"

namespace nc {
namespace input {
namespace bfdparser {

namespace {

using nc::core::input::read;
using nc::core::input::getAsciizString;
using nc::core::input::ParseError;

class BfdParserImpl {
    Q_DECLARE_TR_FUNCTIONS(BfdParserImpl)

    QIODevice *source_;
    core::image::Image *image_;
    const LogToken &log_;
    bfd *abfd, *oldbfd = nullptr;

    ByteOrder byteOrder_;
    bool isarchive = false;
    std::vector<std::unique_ptr<core::image::Section>> sections_;
    std::vector<const core::image::Symbol *> symbols_;

  public:
    BfdParserImpl(QIODevice *source, core::image::Image *image, const LogToken &log):
        source_(source), image_(image), log_(log), byteOrder_(ByteOrder::Current) {
    }

    void parse() {
        QFile *file = static_cast<QFile *>(source_);
        QString filename = file->fileName();
        source_->seek(0); /* Convention */

        /* Read filename */
        if((abfd = bfd_openr(filename.toLatin1().data(), nullptr)) == nullptr) {
            throw ParseError(tr("Could not open file: %1").arg(filename));
        }

        /* Here we try to figure out the target. */
        if (!bfd_check_format(abfd, bfd_object)) {
            if (!bfd_check_format(abfd, bfd_archive)) {
                bfd_close(abfd);
                throw ParseError(tr("Could not open file: %1").arg(filename));
            } else {
				isarchive = true;
            }
        }

		if(isarchive){
			bool isnested = false;
			bfd *tmpbfd = abfd;
			size_t depth = 0;
			
			oldbfd = abfd;
			abfd = bfd_openr_next_archived_file(oldbfd, nullptr);
			while(abfd || isnested){
				if(abfd == nullptr){
					isnested = false;
					oldbfd = tmpbfd;
					/* Traverse to end of nested archive */
					abfd = bfd_openr_next_archived_file(oldbfd, nullptr);
					for(size_t i = 0; i < depth;){
						abfd = bfd_openr_next_archived_file(oldbfd, abfd);
						if(abfd == nullptr){
							break; /* Nothing more to see. */
						}
						if(bfd_check_format(abfd, bfd_archive)){
							i++; /* Found an archive, increment. */
						} else if (!bfd_check_format(abfd, bfd_object)) {
							bfd_close(abfd);
							bfd_close(oldbfd);
			    			throw ParseError(tr("Error unkown format in archive."));
						}
					}
					continue;
				}

				/* Regular parsing goes here. */
				if (!bfd_check_format(abfd, bfd_object)) {
					if(!bfd_check_format(abfd, bfd_archive)){
						bfd_close(abfd);
						bfd_close(oldbfd);
			    		throw ParseError(tr("Error unkown format in archive."));
					} else { /* Nested archive found */
						oldbfd = abfd;
						abfd = bfd_openr_next_archived_file(abfd, nullptr);
						isnested = true;
						depth++;
					}
				} 
	
				log_.debug(tr("Found file: %1 size: %2").arg(getAsciizString(abfd->filename)).arg(bfd_get_arch_size(abfd)));
				abfd = bfd_openr_next_archived_file(oldbfd, abfd);
			}
			
			/* FIXME: Select file here. */
			abfd = bfd_openr_next_archived_file(tmpbfd, nullptr); 
			if (!bfd_check_format(abfd, bfd_object)){
					bfd_close(abfd);
					bfd_close(oldbfd);
		    		throw ParseError(tr("Error nested archive not supported yet."));
			}
		}

        bfd_architecture arch = bfd_get_arch(abfd);
        byteOrder_ = bfd_big_endian(abfd) ? ByteOrder::BigEndian : ByteOrder::LittleEndian;

        switch (arch) {
        case bfd_arch_i386:
            if(bfd_arch_bits_per_address(abfd) == 32) {
                image_->platform().setArchitecture(QLatin1String("i386"));
            } else {
                image_->platform().setArchitecture(QLatin1String("x86-64"));
            }
            break;
        case bfd_arch_arm:
            if (byteOrder_ == ByteOrder::LittleEndian) {
                image_->platform().setArchitecture(QLatin1String("arm-le"));
            } else {
                image_->platform().setArchitecture(QLatin1String("arm-be"));
            }
            break;
        case bfd_arch_mips:
        	if(bfd_get_flavour(abfd) == bfd_target_elf_flavour && bfd_get_section_by_name(abfd, ".rodata.sceModuleInfo") != nullptr){ /* Assume this is a PSP / PRX ELF file. */
                    image_->platform().setArchitecture(QLatin1String("allegrex"));
            } else if (byteOrder_ == ByteOrder::LittleEndian) {
                if(bfd_arch_bits_per_address(abfd) == 32) {
                    image_->platform().setArchitecture(QLatin1String("mips-le"));
                } else {
                    image_->platform().setArchitecture(QLatin1String("mips64-le"));
                }
            } else if(bfd_arch_bits_per_address(abfd) == 32) {
                image_->platform().setArchitecture(QLatin1String("mips-be"));
            } else {
                image_->platform().setArchitecture(QLatin1String("mips64-be"));
            }
            break;
        default:
            const char *id = bfd_printable_name(abfd);
            bfd_close(abfd);
            throw ParseError(tr("Unknown machine id: %1.").arg(getAsciizString(id)));
        }

        parseSections();

        if (!(bfd_get_file_flags(abfd) & HAS_SYMS)) {
            log_.warning(tr("Cannot find any symbols."));
        } else {
            parseSymbols(FALSE); /* Slurping static symtab like it is cum. */
            parseSymbols(TRUE);  /* Slurping dynamic symtab like it is a bukakke party. */
        }

        parseRelocations();
        parseDynamicRelocations();

        foreach (auto &section, sections_) {
            image_->addSection(std::move(section));
        }
        
        /* Add entry point, IFF found, to functions */
        auto start_address = bfd_get_start_address(abfd);
        if(start_address != 0){
			image_->addSymbol(std::make_unique<core::image::Symbol>(core::image::SymbolType::FUNCTION, "_start", start_address, image_->getSectionContainingAddress(start_address)));
        }
        
        bfd_close(abfd);
        if(isarchive){
        	bfd_close(oldbfd);
        }
        return;
    }

  private:

    void parseSections() {
        /* Make a callback to dump_sections_headers() */
        asection *p;

        for (p = abfd->sections; p != NULL; p = p->next) {
            unsigned int opb = bfd_octets_per_byte(abfd);

            /* Ignore linker created section.  See elfNN_ia64_object_p in bfd/elfxx-ia64.c.  */
            if(p->flags & SEC_LINKER_CREATED) {
                log_.warning(tr("Ignoring linker created section."));
                continue;
            }

            auto section = std::make_unique<core::image::Section>(getAsciizString(bfd_get_section_name(abfd, p)), bfd_get_section_vma(abfd, p), static_cast<unsigned long>(bfd_section_size(abfd, p) / opb));

            section->setAllocated(p->flags & SEC_ALLOC);
            section->setReadable();
            section->setWritable(!(p->flags & SEC_READONLY));
            section->setExecutable(p->flags & SEC_CODE);

            section->setCode(p->flags & SEC_CODE);
            section->setData(p->flags & SEC_DATA);
            section->setBss(!(p->flags & SEC_LOAD) && (p->flags & SEC_ALLOC)); /* If load is cleared and the section is allocated then treat it as a bss-section. */
            section->setName(getAsciizString(bfd_section_name(abfd, p)));

            bfd_size_type strsize = bfd_section_size(abfd, p);
            bfd_byte *content = (bfd_byte *)malloc(strsize);

            if (!bfd_get_section_contents(abfd, p, content, 0, strsize)) {
                free(content);
                bfd_close(abfd);
                throw ParseError(tr("Could not parse content in sections."));
            }

            //qDebug()  << getAsciizString(bfd_section_name(abfd, p)) << "is:"  << strsize;

            QByteArray bytes;
            bytes.resize(strsize);
            memcpy(bytes.data(), reinterpret_cast<const void *>(content), strsize);

            section->setContent(std::move(bytes));
            sections_.push_back(std::move(section));
            free(content);
            content = nullptr;
        }
        return;
    }


    void parseRelocations() {
        /* Make a callback to dump_sections_headers() */
        asection *s;

        for (s = abfd->sections; s != NULL; s = s->next) {
            //unsigned int opb = bfd_octets_per_byte(abfd);

            /* Ignore linker created section.  See elfNN_ia64_object_p in bfd/elfxx-ia64.c.  */
            if(s->flags & SEC_LINKER_CREATED) {
                log_.warning(tr("Ignoring linker created section."));
                continue;
            }

            if (bfd_is_abs_section(s) || bfd_is_und_section(s) || bfd_is_com_section(s) || ((s->flags & SEC_RELOC) == 0)) {
                continue;
            }

            arelent **relpp, **p;
            asymbol **syms;
            long relcount;
            unsigned int symsize;
            long symcount;
            long relsize = bfd_get_reloc_upper_bound(abfd, s);

            if (relsize == 0) {
                log_.warning(tr("Cannot find any relocs."));
                return;
            }

            if(relsize < 0) {
                bfd_close(abfd);
                throw ParseError(tr("Could not parse relocations: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
            }

            symcount = bfd_read_minisymbols(abfd, FALSE, (void **) &syms, &symsize);

            if (symcount == 0) {
                free(syms);
                return;
            }

            if (symcount < 0) {
                free(syms);
                bfd_close(abfd);
                throw ParseError(tr("bfd_canonicalize_symtab: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
            }

            relpp = (arelent **)malloc(relsize);
            relcount = bfd_canonicalize_reloc(abfd, s, relpp, syms);

            if (relcount == 0) {
                log_.warning(tr("Cannot find any relocations."));
                free(syms);
                free(relpp);
                bfd_close(abfd);
                return;
            }

            if (relcount < 0) {
                free(syms);
                free(relpp);
                bfd_close(abfd);
                throw ParseError(tr("Failed to read relocations: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
            }

            for (p = relpp; relcount && *p != NULL; p++, relcount--) {
                arelent *q = *p;

                if (q->sym_ptr_ptr && *q->sym_ptr_ptr) {
                    asymbol *asym = *(q->sym_ptr_ptr);
                    const char *sym_name = bfd_asymbol_name(asym);
                    QString name = getAsciizString(sym_name);
                    bfd_signed_vma addend = 0;
					reloc_howto_type *fixupinfo = q->howto;
                	//(void)bfd_simple_get_relocated_section_contents(abfd, (*q->sym_ptr_ptr)->section, NULL, q->sym_ptr_ptr);
    
               		if (q->addend) {
                	 	/* TODO: Use fixupinfo here */
                        addend = q->addend;
                   	}

                    for (std::size_t m = 0; m < symbols_.size(); m++) {
                        auto tmpsym = symbols_[m];
                        if(tmpsym->name() == name) {
                            log_.debug(tr("Found relocation for %1.").arg(name));
                            auto relocation = std::make_unique<core::image::Relocation>(q->address, tmpsym, addend);
                            image_->addRelocation(std::move(relocation));
                            break;
                        }
                    }
                    //qDebug() << "name: " << name << "address: " << q->address  << " addend: " << q->addend;
                }
            }


            free(syms);
            syms = nullptr;
            free(relpp);
            relpp = nullptr;
        }
        return;
    }

    void parseDynamicRelocations() {
        arelent **relpp, **p;
        asymbol **dynsyms;
        long relcount;
        unsigned int dynsymsize;
        long dynsymcount;

        long relsize = bfd_get_dynamic_reloc_upper_bound(abfd);

        if (relsize == 0) {
            return;
        }

        if(relsize < 0) {
            if (bfd_get_file_flags (abfd) & DYNAMIC) {
                bfd_close(abfd);
                throw ParseError(tr("Could not parse relocations: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
            } else {
                return;
            }
        }

        dynsymcount = bfd_read_minisymbols(abfd, TRUE, (void **) &dynsyms, &dynsymsize);

        if (dynsymcount == 0) {
            free(dynsyms);
            return;
        }

        if (dynsymcount < 0) {
            free(dynsyms);
            bfd_close(abfd);
            throw ParseError(tr("bfd_canonicalize_symtab: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
        }

        relpp = (arelent **)malloc(relsize);
        relcount = bfd_canonicalize_dynamic_reloc(abfd, relpp, dynsyms);

        if (relcount == 0) {
			if (bfd_get_file_flags (abfd) & DYNAMIC) {
            	log_.warning(tr("Cannot find any dynamic relocations."));
            	bfd_close(abfd);
			}
            free(dynsyms);
            free(relpp);	
            return;
        }

        if (relcount < 0) {
            free(dynsyms);
            free(relpp);
            bfd_close(abfd);
            throw ParseError(tr("Failed to read relocations: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
        }

        for (p = relpp; relcount && *p != NULL; p++, relcount--) {
            arelent *q = *p;

            if (q->sym_ptr_ptr && *q->sym_ptr_ptr) {
                asymbol *asym = *(q->sym_ptr_ptr);
                const char *sym_name = bfd_asymbol_name(asym);
                QString name = getAsciizString(sym_name);
                bfd_signed_vma addend = 0;
				reloc_howto_type *fixupinfo = q->howto;
				//(void)bfd_simple_get_relocated_section_contents(abfd, (*q->sym_ptr_ptr)->section, NULL, q->sym_ptr_ptr);
    
                if (q->addend) {
                	 /* TODO: Use fixupinfo here */
                    addend = q->addend;
                }

                for (std::size_t m = 0; m < symbols_.size(); m++) {
                    auto tmpsym = symbols_[m];
                    if(tmpsym->name() == name) {
                        log_.debug(tr("Found dynamic relocation for %1.").arg(name));
                        auto relocation = std::make_unique<core::image::Relocation>(q->address, tmpsym, addend);
                        image_->addRelocation(std::move(relocation));
                        break;
                    }
                }
                //qDebug() << "name: " << name << "address: " << q->address  << " addend: " << q->addend;
            }
        }

        free(dynsyms);
        dynsyms = nullptr;
        free(relpp);
        relpp = nullptr;
        return;
    }


    void parseSymbols(bfd_boolean isdynamic) {
        unsigned int symsize;
        long symcount;
        asymbol **syms;

        symcount = bfd_read_minisymbols(abfd, isdynamic, (void **) &syms, &symsize);
        if (symcount == 0) {
            return;
        }

        if (symcount < 0) {
            if (!(bfd_get_file_flags (abfd) & DYNAMIC) && isdynamic == TRUE) {
                return;
            } else {
                bfd_close(abfd);
                throw ParseError(tr("bfd_canonicalize_symtab: %1.").arg(getAsciizString(bfd_errmsg(bfd_get_error()))));
            }
        }

        for (int i = 0; i < symcount; i++) {
            using core::image::Symbol;
            using core::image::SymbolType;
            asymbol *asym = syms[i];
            const char *sym_name = bfd_asymbol_name(asym);
            char symclass = bfd_decode_symclass(asym);
            int sym_value = bfd_asymbol_value(asym);

            QString name = getAsciizString(sym_name);
            boost::optional<ConstantValue> value = static_cast<long>(sym_value);

            const core::image::Section *section = nullptr;

            for (std::size_t m = 0; m < sections_.size(); m++) {
                auto tmp = sections_[m].get();
                if((tmp->containsAddress(sym_value) && tmp->isAllocated()) || ((tmp->addr() == sym_value) && (tmp->name() == name))) {
                    section = tmp;
                    break;
                }
            }

            //qDebug()  << name << "is:"  << symclass;

            SymbolType type = SymbolType::NOTYPE;

            /* First inspect flags - Helps parsing ELF files withouth heuristics */
            if(asym->flags & BSF_FUNCTION) {
                type = SymbolType::FUNCTION;
            } else if (asym->flags & BSF_SECTION_SYM) {
                type = SymbolType::SECTION;
            } else if (asym->flags & BSF_OBJECT) {
                type = SymbolType::OBJECT;
            } else if ((asym->flags & BSF_FILE) || (asym->flags & BSF_DEBUGGING)) {
                type = SymbolType::DEBUG;
            }

            if(type == SymbolType::NOTYPE) {
                switch (symclass) {
                case 'A':
                case 'D':
                case 'G':
                case 'O':
                case 'B': /* Object */
                case 'S': /* Small object */
                    type = SymbolType::OBJECT;
                    break;
                case 'F':
                case 'I':
                case 'T': /* .text */
                case 'U':
                case 'V':
                case 'W':
                    type = SymbolType::FUNCTION;
                    break;
                case 'a': /* abs section */
                case 'b': /* .bss */
                case 'C': /* Constructor???? */
                case 'c': /* .scommon */
                case 'd': /* .data */
                case 'e': /* .eata */
                case 'g': /* .sdata */
                case 'i': /* .idata */
                case 't': /* .text */
                case 'r': /* .rdata */
                case 'n': /* .comment */
                case 'N': /* .debug */
                case 'p': /* .pdata */
                case 's': { /* .sbss */
                    if(section != nullptr && (section->isBss() || section->isData()) && !((section != nullptr && (section->addr() == sym_value)) || (section != nullptr && (section->name() == name)))) {
                        type = SymbolType::OBJECT;
                        break;
                    } else if(section != nullptr && section->isCode() && !((section != nullptr && (section->addr() == sym_value)) || (section != nullptr && (section->name() == name)))) {
                        type = SymbolType::FUNCTION;
                        break;
                    } else if((section != nullptr && (section->addr() == sym_value)) || (section != nullptr && (section->name() == name))) {
                        type = SymbolType::SECTION;
                        break;
                    }
                }
                case 'v':
                case 'w': /* weak */
                default: {
                    if(section != nullptr && (section->isBss() || section->isData())) {
                        type = SymbolType::OBJECT;
                        break;
                    } else if(section != nullptr && section->isCode()) {
                        type = SymbolType::FUNCTION;
                        break;
                    } else {
                        type = SymbolType::NOTYPE;
                        break;
                    }
                }
                }
            }

            auto sym = std::make_unique<Symbol>(type, name, value, section);
            symbols_.push_back(sym.get());
            image_->addSymbol(std::move(sym));
        }

        free(syms);
        syms = nullptr;
        return;
    }
};

} // anonymous namespace

BfdParser::BfdParser():
    core::input::Parser(QLatin1String("BFD")) {
}

bool BfdParser::doCanParse(QIODevice *source) const {
    bfd *abfd = nullptr;
    QFile *file = static_cast<QFile *>(source);
    QString filename = file->fileName();

    bfd_init();
    abfd = bfd_openr(filename.toLatin1().data(), nullptr);
    if(abfd == nullptr) {
        return false;
    }
    if (!(bfd_check_format(abfd, bfd_object) || bfd_check_format(abfd, bfd_archive))) {
        bfd_close(abfd);
        return false;
    }
    bfd_architecture arch = bfd_get_arch(abfd);
    if(((arch == bfd_arch_unknown) || (arch == bfd_arch_obscure)) && !bfd_check_format(abfd, bfd_archive)) {
        bfd_close(abfd);
        return false;
    }
    bfd_close(abfd);
    return true;
}

void BfdParser::doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const {
    BfdParserImpl(source, image, log).parse();
}

} // namespace bfdparser
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */

