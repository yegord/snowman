/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "ElfParser.h"

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */
#include <QIODevice>

#include <nc/common/Foreach.h>
#include <nc/common/LogToken.h>
#include <nc/common/Range.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Reader.h>
#include <nc/core/image/Relocation.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/ParseError.h>

#include "elf32.h"
#include "elf64.h"

namespace nc {
namespace input {
namespace elf {

namespace {

class Elf32 {
public:
    static const unsigned char elfclass = ELFCLASS32;
    typedef Elf32_Ehdr Ehdr;
    typedef Elf32_Shdr Shdr;
    typedef Elf32_Sym Sym;
    typedef Elf32_Rel Rel;
    typedef Elf32_Rela Rela;

    static unsigned char st_type(unsigned char info) { return ELF32_ST_TYPE(info); }
    static Elf32_Addr r_sym(Elf32_Addr offset) { return ELF32_R_SYM(offset); }
};

class Elf64 {
public:
    static const unsigned char elfclass = ELFCLASS64;
    typedef Elf64_Ehdr Ehdr;
    typedef Elf64_Shdr Shdr;
    typedef Elf64_Sym Sym;
    typedef Elf64_Rel Rel;
    typedef Elf64_Rela Rela;

    static unsigned char st_type(unsigned char info) { return ELF64_ST_TYPE(info); }
    static Elf64_Addr r_sym(Elf64_Addr offset) { return ELF64_R_SYM(offset); }
};

template<class Elf>
class RelWithoutAddend {
public:
    typedef typename Elf::Rel Rel;
    static void convertFrom(const ByteOrder &byteOrder, Rel &rel) {
        byteOrder.convertFrom(rel.r_offset);
        byteOrder.convertFrom(rel.r_info);
    }
    static ByteSize addend(const Rel &) { return 0; }
};

template<class Elf>
class RelWithAddend {
public:
    typedef typename Elf::Rela Rel;
    static void convertFrom(const ByteOrder &byteOrder, Rel &rel) {
        byteOrder.convertFrom(rel.r_offset);
        byteOrder.convertFrom(rel.r_info);
        byteOrder.convertFrom(rel.r_addend);
    }
    static ByteSize addend(const Rel &rel) { return rel.r_addend; }
};

template<class Elf>
class ElfParserImpl {
    Q_DECLARE_TR_FUNCTIONS(ElfParserPrivate)

    QIODevice *source_;
    core::image::Image *image_;
    const LogToken &log_;

    typename Elf::Ehdr ehdr_;
    ByteOrder byteOrder_;
    std::vector<typename Elf::Shdr> shdrs_;
    std::vector<std::unique_ptr<core::image::Section>> sections_;
    boost::unordered_map<std::size_t, std::vector<std::unique_ptr<core::image::Symbol>>> symbolTables_;
    boost::unordered_map<std::size_t, std::vector<std::unique_ptr<core::image::Relocation>>> relocationTables_;

public:
    ElfParserImpl(QIODevice *source, core::image::Image *image, const LogToken &log):
        source_(source), image_(image), log_(log), byteOrder_(ByteOrder::Current)
    {}

    void parse() {
        parseElfHeader();
        parseSections();
        parseSymbols();
        parseRelocations();

        foreach (auto &section, sections_) {
            image_->addSection(std::move(section));
        }
        foreach (auto &indexAndTable, symbolTables_) {
            foreach (auto &symbol, indexAndTable.second) {
                image_->addSymbol(std::move(symbol));
            }
        }
        foreach (auto &indexAndTable, relocationTables_) {
            foreach (auto &relocation, indexAndTable.second) {
                image_->addRelocation(std::move(relocation));
            }
        }
    }

private:
    void parseElfHeader() {
        source_->seek(0);

        if (source_->read(reinterpret_cast<char *>(&ehdr_), sizeof(ehdr_)) != sizeof(ehdr_)) {
            throw core::input::ParseError(tr("Could not read ELF header."));
        }

        if (ehdr_.e_ident[EI_CLASS] != Elf::elfclass) {
            throw core::input::ParseError(tr("The instantiation of the parser class does not match the ELF class."));
        }

        if (ehdr_.e_ident[EI_DATA] == ELFDATA2LSB) {
            byteOrder_ = ByteOrder::LittleEndian;
        } else if (ehdr_.e_ident[EI_DATA] == ELFDATA2MSB) {
            byteOrder_ = ByteOrder::BigEndian;
        } else {
            log_.warning(tr("Invalid byte order in ELF file: %1. Assuming host byte order.").arg(ehdr_.e_ident[EI_DATA]));
        }

        byteOrder_.convertFrom(ehdr_.e_machine);
        byteOrder_.convertFrom(ehdr_.e_shoff);
        byteOrder_.convertFrom(ehdr_.e_shnum);
        byteOrder_.convertFrom(ehdr_.e_shstrndx);

        switch (ehdr_.e_machine) {
            case EM_386:
                image_->setArchitecture(QLatin1String("i386"));
                break;
            case EM_X86_64:
                image_->setArchitecture(QLatin1String("x86-64"));
                break;
            case EM_ARM:
                if (byteOrder_ == ByteOrder::LittleEndian) {
                    image_->setArchitecture(QLatin1String("arm-le"));
                } else {
                    image_->setArchitecture(QLatin1String("arm-be"));
                }
                break;
            default:
                throw core::input::ParseError(tr("Unknown machine id: %1.").arg(ehdr_.e_machine));
        }
    }

    void parseSections() {
        source_->seek(ehdr_.e_shoff);

        /*
         * Read section headers.
         */
        shdrs_.resize(ehdr_.e_shnum);
        if (source_->read(reinterpret_cast<char *>(&shdrs_[0]), sizeof(typename Elf::Shdr) * shdrs_.size())
            != static_cast<qint64>(sizeof(typename Elf::Shdr) * shdrs_.size()))
        {
            throw core::input::ParseError(tr("Cannot read section headers."));
        }

        /*
         * Read section contents.
         */
        sections_.reserve(shdrs_.size());

        foreach (typename Elf::Shdr &shdr, shdrs_) {
            byteOrder_.convertFrom(shdr.sh_addr);
            byteOrder_.convertFrom(shdr.sh_size);
            byteOrder_.convertFrom(shdr.sh_flags);
            byteOrder_.convertFrom(shdr.sh_type);
            byteOrder_.convertFrom(shdr.sh_offset);
            byteOrder_.convertFrom(shdr.sh_link);

            auto section = std::make_unique<core::image::Section>(QString(), shdr.sh_addr, shdr.sh_size);

            section->setAllocated(shdr.sh_flags & SHF_ALLOC);
            section->setReadable();
            section->setWritable(shdr.sh_flags & SHF_WRITE);
            section->setExecutable(shdr.sh_flags & SHF_EXECINSTR);

            section->setCode(shdr.sh_type == SHT_PROGBITS && section->isExecutable());
            section->setBss(shdr.sh_type == SHT_NOBITS);
            section->setData(section->isAllocated() && !section->isCode() && !section->isBss());

            if (!section->isBss()) {
                if (source_->seek(shdr.sh_offset)) {
                    auto bytes = source_->read(shdr.sh_size);

                    if (bytes.size() != static_cast<int>(shdr.sh_size)) {
                        log_.warning(tr("Could read only 0x%1 bytes of section %2, although its size is 0x%3.")
                                         .arg(bytes.size(), 0, 16)
                                         .arg(section->name())
                                         .arg(shdr.sh_size));
                    }

                    section->setContent(std::move(bytes));
                } else {
                    log_.warning(tr("Could not seek to the data of section %1.").arg(section->name()));
                }
            }

            sections_.push_back(std::move(section));
        }

        /*
         * Read names of the sections.
         */
        if (ehdr_.e_shstrndx < shdrs_.size()) {
            auto shstrtab = sections_[ehdr_.e_shstrndx].get();
            core::image::Reader reader(shstrtab);

            for (std::size_t i = 0; i < shdrs_.size(); ++i) {
                sections_[i]->setName(reader.readAsciizString(shdrs_[i].sh_name, shstrtab->size()));
            }
        }
    }

    void parseSymbols() {
        for (std::size_t i = 0; i < sections_.size(); ++i) {
            if (shdrs_[i].sh_type == SHT_SYMTAB || shdrs_[i].sh_type == SHT_DYNSYM) {
                parseSymbols(i);
            }
        }
    }

    void parseSymbols(std::size_t symtabIndex) {
        assert(symtabIndex < sections_.size());

        auto strtabIndex = shdrs_[symtabIndex].sh_link;
        if (strtabIndex >= shdrs_.size()) {
            log_.warning(tr("Symbol table (section number %1) has invalid string table section number %2.").arg(symtabIndex).arg(strtabIndex));
            return;
        }

        auto symtab = sections_[symtabIndex].get();
        auto strtab = sections_[strtabIndex].get();

        core::image::Reader strtabReader(strtab);

        auto &result = symbolTables_[symtabIndex];

        typename Elf::Sym sym;
        for (ByteAddr addr = symtab->addr(); addr < symtab->endAddr(); addr += sizeof(sym)) {
            if (symtab->readBytes(addr, &sym, sizeof(sym)) != sizeof(sym)) {
                break;
            }

            byteOrder_.convertFrom(sym.st_name);
            byteOrder_.convertFrom(sym.st_value);
            byteOrder_.convertFrom(sym.st_info);
            byteOrder_.convertFrom(sym.st_shndx);

            using core::image::Symbol;
            using core::image::SymbolType;

            SymbolType type;
            switch (Elf::st_type(sym.st_info)) {
                case STT_OBJECT:
                    type = SymbolType::OBJECT;
                    break;
                case STT_FUNC:
                    type = SymbolType::FUNCTION;
                    break;
                case STT_SECTION:
                    type = SymbolType::SECTION;
                    break;
                default:
                    type = SymbolType::NOTYPE;
                    break;
            }

            const core::image::Section *section = nullptr;
            if (sym.st_shndx < sections_.size() && sym.st_shndx != SHN_UNDEF) {
                section = sections_[sym.st_shndx].get();
            }

            auto name = strtabReader.readAsciizString(strtab->addr() + sym.st_name, strtab->size());
            auto symbol = std::make_unique<Symbol>(type, std::move(name), sym.st_value, section);

            result.push_back(std::move(symbol));
        }
    }

    void parseRelocations() {
        for (std::size_t i = 0; i < shdrs_.size(); ++i) {
            if (shdrs_[i].sh_type == SHT_RELA) {
                parseRelocations<RelWithAddend<Elf>>(i);
            } else if (shdrs_[i].sh_type == SHT_REL) {
                parseRelocations<RelWithoutAddend<Elf>>(i);
            }
        }
    }

    template<class Relocation>
    void parseRelocations(std::size_t reltabIndex) {
        assert(reltabIndex < sections_.size());

        auto reltab = sections_[reltabIndex].get();

        std::size_t symIndex = shdrs_[reltabIndex].sh_link;
        if (symIndex >= sections_.size()) {
            log_.warning(tr("Relocations table %1 uses invalid symbol table %2.").arg(reltabIndex).arg(symIndex));
            return;
        }

        if (!nc::contains(symbolTables_, symIndex)) {
            parseSymbols(symIndex);
        }

        const auto &symbolTable = nc::find(symbolTables_, symIndex);

        auto &result = relocationTables_[reltabIndex];

        typename Relocation::Rel rel;

        for (ByteAddr addr = reltab->addr(); addr < reltab->endAddr(); addr += sizeof(rel)) {
            if (reltab->readBytes(addr, &rel, sizeof(rel)) != sizeof(rel)) {
                break;
            }

            Relocation::convertFrom(byteOrder_, rel);

            auto symbolIndex = Elf::r_sym(rel.r_info);

            if (symbolIndex < symbolTable.size()) {
                result.push_back(std::make_unique<core::image::Relocation>(
                    rel.r_offset, symbolTable[symbolIndex].get(), Relocation::addend(rel)));
            } else {
                log_.warning(tr("Symbol index %1 is out of range: symbol table has only %2 elements.").arg(symbolIndex).arg(symbolTable.size()));
            }
        }
    }
};

} // anonymous namespace

ElfParser::ElfParser():
    core::input::Parser("ELF")
{}

bool ElfParser::doCanParse(QIODevice *source) const {
    Elf32_Ehdr ehdr;
    if (source->read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr)) != sizeof(ehdr)) {
        return false;
    }
    return IS_ELF(ehdr);
}

void ElfParser::doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const {
    Elf32_Ehdr ehdr;

    std::size_t bytesRead = source->read(reinterpret_cast<char *>(&ehdr), sizeof(ehdr));
    if (bytesRead < sizeof(ehdr.e_ident) || !IS_ELF(ehdr)) {
        throw core::input::ParseError(tr("ELF signature doesn't match."));
    }

    switch (ehdr.e_ident[EI_CLASS]) {
        case ELFCLASS32: {
            ElfParserImpl<Elf32>(source, image, log).parse();
            break;
        }
        case ELFCLASS64: {
            ElfParserImpl<Elf64>(source, image, log).parse();
            break;
        }
        default: {
            throw core::input::ParseError(tr("Unknown ELF class: %1.").arg(ehdr.e_ident[EI_CLASS]));
        }
    }
}

} // namespace elf
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
