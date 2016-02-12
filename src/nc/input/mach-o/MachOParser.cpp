/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#include "MachOParser.h"

#include <boost/optional.hpp>

#include <nc/common/ByteOrder.h>
#include <nc/common/CheckedCast.h>
#include <nc/common/Foreach.h>
#include <nc/common/LogToken.h>
#include <nc/common/Unreachable.h>
#include <nc/common/make_unique.h>
#include <nc/common/Range.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/ParseError.h>
#include <nc/core/input/Utils.h>

#include "mach-o.h"

namespace nc {
namespace input {
namespace mach_o {

namespace {

using nc::core::input::read;
using nc::core::input::getAsciizString;
using nc::core::input::ParseError;

boost::optional<std::pair<SmallBitSize, ByteOrder>>
getBitnessAndByteOrder(uint32_t magic) {
    static const ByteOrder byteOrders[] = {
        ByteOrder::BigEndian,
        ByteOrder::LittleEndian
    };

    foreach (const auto &byteOrder, byteOrders) {
        auto m = magic;
        byteOrder.convertFrom(m);

        if (m == MH_MAGIC) {
            return std::make_pair(32, byteOrder);
        } else if (m == MH_MAGIC_64) {
            return std::make_pair(64, byteOrder);
        }
    }
    return boost::none;
}

class MachO32 {
public:
    typedef mach_header MachHeader;
    typedef nlist Nlist;
    static const uint32_t magic = MH_MAGIC;
};

class MachO64 {
public:
    typedef mach_header_64 MachHeader;
    typedef nlist_64 Nlist;
    static const uint32_t magic = MH_MAGIC_64;
};

struct IndirectSection {
    uint32_t stride;
    uint32_t size;
    ByteAddr addr;
    uint32_t index;
    const core::image::Section *section;
};

class MachOParserImpl {
    Q_DECLARE_TR_FUNCTIONS(MachOParserImpl)

    QIODevice *source_;
    core::image::Image *image_;
    const LogToken &log_;

    ByteOrder byteOrder_;
    boost::unordered_map<const core::image::Section *, uint64_t> section2foff_;
    std::vector<const core::image::Section *> sections_;
    std::vector<const core::image::Symbol *> symbols_;
    std::vector<IndirectSection> indirectSections_;

public:
    MachOParserImpl(QIODevice *source, core::image::Image *image, const LogToken &log):
        source_(source), image_(image), log_(log), byteOrder_(ByteOrder::Current)
    {}

    template<class Mach>
    void parse() {
        source_->seek(0);

        typename Mach::MachHeader header;
        if (!read(source_, header)) {
            throw ParseError(tr("Could not read Mach-O header."));
        }

        auto bitnessAndByteOrder = getBitnessAndByteOrder(header.magic);
        if (!bitnessAndByteOrder) {
            throw ParseError(tr("Mach-O magic does not match."));
        }

        byteOrder_ = bitnessAndByteOrder->second;

        byteOrder_.convertFrom(header.magic);
        byteOrder_.convertFrom(header.cputype);
        byteOrder_.convertFrom(header.ncmds);

        if (header.magic != Mach::magic) {
            throw ParseError(tr("The instantiation of the method does not match the Mach-O class."));
        }

        switch (header.cputype) {
            case CPU_TYPE_I386:
                image_->platform().setArchitecture(QLatin1String("i386"));
                break;
            case CPU_TYPE_X86_64:
                image_->platform().setArchitecture(QLatin1String("x86-64"));
                break;
            case CPU_TYPE_ARM:
                image_->platform().setArchitecture(QLatin1String(byteOrder_ == ByteOrder::LittleEndian ? "arm-le" : "arm-be"));
                break;
            default:
                throw ParseError(tr("Unknown CPU type: %1.").arg(header.cputype));
        }

        parseLoadCommands<Mach>(header.ncmds);
    }

private:
    template<class Mach>
    void parseLoadCommands(uint32_t ncmds) {
        log_.debug(tr("Parsing load commands, %1 of them.").arg(ncmds));

        for (uint32_t i = 0; i < ncmds; ++i) {
            log_.debug(tr("Parsing load command number %1.").arg(i));

            auto pos = source_->pos();

            load_command loadCommand;
            if (!read(source_, loadCommand)) {
                throw ParseError(tr("Could not read the load command."));
            }
            byteOrder_.convertFrom(loadCommand.cmd);
            byteOrder_.convertFrom(loadCommand.cmdsize);

            log_.debug(tr("Read load command 0x%1 of size %2.").arg(loadCommand.cmd, 0, 16).arg(loadCommand.cmdsize));

            if (!source_->seek(pos)) {
                throw ParseError(tr("Could not reseek to the load command."));
            }

            switch (loadCommand.cmd & ~LC_REQ_DYLD) {
                case LC_SEGMENT: {
                    parseSegmentCommand<segment_command, section>();
                    break;
                }
                case LC_SEGMENT_64: {
                    parseSegmentCommand<segment_command_64, section_64>();
                    break;
                }
                case LC_SYMTAB: {
                    parseSymtabCommand<Mach>();
                    break;
                }
                case LC_DYSYMTAB: {
                    parseDySymtabCommand<Mach>();
                    break;
                }
                case LC_MAIN: {
                    parseMainCommand<Mach>();
                    break;
                }
            }

            if (!source_->seek(pos + loadCommand.cmdsize)) {
                throw ParseError(tr("Could not seek to the next load command."));
            }
        }
    }

    template<class SegmentCommand, class Section>
    void parseSegmentCommand() {
        SegmentCommand command;
        if (!read(source_, command)) {
            throw ParseError(tr("Could not read segment command."));
        }
        byteOrder_.convertFrom(command.nsects);
        byteOrder_.convertFrom(command.initprot);

        log_.debug(tr("Found segment '%1' with %2 sections.").arg(getAsciizString(command.segname)).arg(command.nsects));

        for (uint32_t i = 0; i < command.nsects; ++i) {
            log_.debug(tr("Parsing section number %1.").arg(i));
            parseSection<Section>(command.initprot);
        }
    }

    template<class Section>
    void parseSection(vm_prot_t protection) {
        Section section;
        if (!read(source_, section)) {
            throw ParseError(tr("Could not read section."));
        }
        byteOrder_.convertFrom(section.addr);
        byteOrder_.convertFrom(section.size);
        byteOrder_.convertFrom(section.offset);
        byteOrder_.convertFrom(section.flags);
        byteOrder_.convertFrom(section.reserved1);
        byteOrder_.convertFrom(section.reserved2);


        auto sectionName = getAsciizString(section.sectname);
        auto segmentName = getAsciizString(section.segname);

        log_.debug(tr("Found section '%1' in segment '%2', addr = 0x%3, size = 0x%4.")
                       .arg(sectionName)
                       .arg(segmentName)
                       .arg(section.addr, 0, 16)
                       .arg(section.size, 0, 16));

        auto imageSection = std::make_unique<core::image::Section>(tr("%1,%2").arg(segmentName).arg(sectionName),
                                                                   section.addr, section.size);

        uint32_t section_type = section.flags & SECTION_TYPE;
        if (section_type == S_NON_LAZY_SYMBOL_POINTERS ||
            section_type == S_LAZY_SYMBOL_POINTERS ||
            section_type == S_LAZY_DYLIB_SYMBOL_POINTERS ||
            section_type == S_THREAD_LOCAL_VARIABLE_POINTERS ||
            section_type == S_SYMBOL_STUBS)
        {
            IndirectSection indirectSection;
            if (section_type == S_SYMBOL_STUBS)
                indirectSection.stride = section.reserved2;
            else
                indirectSection.stride = 8;
            indirectSection.index = section.reserved1;
            indirectSection.size = section.size;
            indirectSection.addr = section.addr;
            indirectSection.section = imageSection.get();
            indirectSections_.push_back(indirectSection);
        }

        imageSection->setAllocated(protection);
        imageSection->setReadable(protection & VM_PROT_READ);
        imageSection->setWritable(protection & VM_PROT_WRITE);
        imageSection->setExecutable(protection & VM_PROT_EXECUTE);

        imageSection->setCode(section.flags & (S_ATTR_SOME_INSTRUCTIONS | S_ATTR_PURE_INSTRUCTIONS));
        imageSection->setData(!imageSection->isCode());
        imageSection->setBss((section.flags & SECTION_TYPE) == S_ZEROFILL);

        if (!imageSection->isBss()) {
            auto pos = source_->pos();
            if (!source_->seek(section.offset)) {
                throw ParseError("Could not seek to the beginning of the section's content.");
            }
            auto bytes = source_->read(section.size);
            if (checked_cast<uint32_t>(bytes.size()) != section.size) {
                log_.warning("Could not read all the section's content.");
            } else {
                imageSection->setContent(std::move(bytes));
            }
            source_->seek(pos);
        }

        sections_.push_back(imageSection.get());
        section2foff_[imageSection.get()]= section.offset;
        image_->addSection(std::move(imageSection));
    }

    template<class Mach>
    void parseSymtabCommand() {
        symtab_command command;
        if (!read(source_, command)) {
            throw ParseError(tr("Could not read symtab command."));
        }
        byteOrder_.convertFrom(command.symoff);
        byteOrder_.convertFrom(command.nsyms);
        byteOrder_.convertFrom(command.stroff);
        byteOrder_.convertFrom(command.strsize);

        log_.debug(tr("Found a symbol table with %1 entries.").arg(command.nsyms));

        if (!source_->seek(command.stroff)) {
            throw ParseError(tr("Could not seek to the string table."));
        }

        auto stringTable = source_->read(command.strsize);
        if (checked_cast<uint32_t>(stringTable.size()) != command.strsize) {
            throw ParseError(tr("Could not read string table."));
        }

        if (!source_->seek(command.symoff)) {
            throw ParseError(tr("Could not seek to the symbol table."));
        }

        for (uint32_t i = 0; i < command.nsyms; ++i) {
            using core::image::Symbol;
            using core::image::SymbolType;

            typename Mach::Nlist symbol;
            if (!read(source_, symbol)) {
                throw ParseError(tr("Could not read symbol number %1.").arg(i));
            }
            byteOrder_.convertFrom(symbol.n_strx);
            byteOrder_.convertFrom(symbol.n_type);
            byteOrder_.convertFrom(symbol.n_sect);
            byteOrder_.convertFrom(symbol.n_value);

            QString name = getAsciizString(stringTable, symbol.n_strx);

            boost::optional<ConstantValue> value;
            if ((symbol.n_type & N_TYPE) != N_UNDF) {
                value = symbol.n_value;
            }

            const core::image::Section *section = nullptr;
            if (symbol.n_sect != NO_SECT && symbol.n_sect <= sections_.size()) {
                section = sections_[symbol.n_sect - 1];
            }

            /* Mach-O does not tell us the type. Let's do some guessing. */
            auto type = SymbolType::NOTYPE;
            if (section) {
                if (section->isCode()) {
                    type = SymbolType::FUNCTION;
                } else if (section->isData()) {
                    type = SymbolType::OBJECT;
                }
            }

            auto sym = std::make_unique<core::image::Symbol>(type, name, value, section);
            symbols_.push_back(sym.get());
            image_->addSymbol(std::move(sym));
        }
    }

    template<class Mach>
    void parseDySymtabCommand() {
        dysymtab_command command;
        if (!read(source_, command)) {
            throw ParseError(tr("Could not read dsymtab command."));
        }
        byteOrder_.convertFrom(command.ilocalsym);
        byteOrder_.convertFrom(command.nlocalsym);
        byteOrder_.convertFrom(command.iextdefsym);
        byteOrder_.convertFrom(command.nextdefsym);
        byteOrder_.convertFrom(command.iundefsym);
        byteOrder_.convertFrom(command.nundefsym);
        byteOrder_.convertFrom(command.indirectsymoff);
        byteOrder_.convertFrom(command.nindirectsyms);

        if (!source_->seek(command.indirectsymoff)) {
            throw ParseError(tr("Could not seek to the string table."));
        }

        foreach (const auto &indirectSection, indirectSections_) {
            if (!source_->seek(command.indirectsymoff + indirectSection.index * sizeof(uint32_t))) {
                throw ParseError(tr("Could not seek to the string table."));
            }

            for (uint32_t i = 0; i < indirectSection.size; i += indirectSection.stride) {
                using core::image::Symbol;
                using core::image::SymbolType;

                uint32_t symbolIndex;

                if (!read(source_, symbolIndex)) {
                    throw ParseError(tr("Could not read symbol number %1.").arg(i));
                }
                byteOrder_.convertFrom(symbolIndex);

                if (! (symbolIndex & INDIRECT_SYMBOL_LOCAL || symbolIndex & INDIRECT_SYMBOL_ABS)) {
                    if (symbolIndex >= symbols_.size()) {
                        throw ParseError(tr("Symbol index %1 is too large.").arg(symbolIndex));
                    }
                    const core::image::Symbol * sym = symbols_[symbolIndex];
                    image_->addSymbol(std::make_unique<core::image::Symbol>(sym->type(), sym->name(), indirectSection.addr + i, indirectSection.section));
                }
            }
        }
    }
    template<class Mach>
    void parseMainCommand() {
        entry_point_command command;
        if (!read(source_, command)) {
            throw ParseError(tr("Could not read entry point command."));
        }
        byteOrder_.convertFrom(command.entryoff);

        log_.debug(tr("Found an entry point offset %1.").arg(command.entryoff));
        foreach(const auto &section, sections_) {
            auto offset = nc::find(section2foff_, section);
            assert(offset);

            if (offset <= command.entryoff && command.entryoff < offset+section->size()) {
                auto entrypoint = command.entryoff - offset + section->addr();

                log_.debug(tr("Entry point = 0x%1.").arg(entrypoint,8, 16));
                image_->setEntryPoint(entrypoint);
                break;
            }
        }
    }
};

} // anonymous namespace

MachOParser::MachOParser():
    core::input::Parser("Mach-O")
{}

bool MachOParser::doCanParse(QIODevice *source) const {
    uint32_t magic;
    return read(source, magic) && getBitnessAndByteOrder(magic);
}

void MachOParser::doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const {
    uint32_t magic;
    if (!read(source, magic)) {
        throw ParseError(tr("Could not read Mach-O magic."));
    }

    auto bitnessAndByteOrder = getBitnessAndByteOrder(magic);
    if (!bitnessAndByteOrder) {
        throw ParseError(tr("Mach-O magic does not match."));
    }

    switch (bitnessAndByteOrder->first) {
        case 32:
            MachOParserImpl(source, image, log).parse<MachO32>();
            break;
        case 64:
            MachOParserImpl(source, image, log).parse<MachO64>();
            break;
        default:
            unreachable();
    }
}

} // namespace mach_o
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
