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

#include "PeParser.h"

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */
#include <QIODevice>

#include <nc/common/ByteOrder.h>
#include <nc/common/Foreach.h>
#include <nc/common/LogToken.h>
#include <nc/common/StringToInt.h>
#include <nc/common/Types.h>
#include <nc/common/make_unique.h>

#include <nc/core/image/Image.h>
#include <nc/core/image/Reader.h>
#include <nc/core/image/Relocation.h>
#include <nc/core/image/Section.h>
#include <nc/core/input/ParseError.h>
#include <nc/core/input/Utils.h>

#include "pe.h"

namespace nc {
namespace input {
namespace pe {

namespace {

using nc::core::input::read;
using nc::core::input::getAsciizString;
using nc::core::input::ParseError;

const ByteOrder peByteOrder = ByteOrder::LittleEndian;

bool seekFileHeader(QIODevice *source) {
    IMAGE_DOS_HEADER dosHeader;

    if (!read(source, dosHeader)) {
        return false;
    }

    peByteOrder.convertFrom(dosHeader.e_magic);
    peByteOrder.convertFrom(dosHeader.e_lfanew);

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        return false;
    }

    if (!source->seek(dosHeader.e_lfanew)) {
        return false;
    }

    uint32_t signature;
    if (!read(source, signature)) {
        return false;
    }
    peByteOrder.convertFrom(signature);

    if (signature != IMAGE_NT_SIGNATURE) {
        return false;
    }

    return true;
}

union IMPORT_LOOKUP_TABLE_ENTRY32 {
    struct {
        uint32_t Name : 31;
        uint32_t IsOrdinal : 1;
    };
    uint32_t RawValue;
};

union IMPORT_LOOKUP_TABLE_ENTRY64 {
    struct {
        uint64_t Name : 63;
        uint64_t IsOrdinal : 1;
    };
    uint64_t RawValue;
};

static_assert(sizeof(IMPORT_LOOKUP_TABLE_ENTRY32) == sizeof(uint32_t), "");
static_assert(sizeof(IMPORT_LOOKUP_TABLE_ENTRY64) == sizeof(uint64_t), "");

template<class IMAGE_OPTIONAL_HEADER, class IMPORT_LOOKUP_TABLE_ENTRY>
class PeParserImpl {
    Q_DECLARE_TR_FUNCTIONS(PeParserImpl)

    QIODevice *source_;
    core::image::Image *image_;
    const LogToken &log_;

    ByteAddr optionalHeaderOffset_;
    IMAGE_FILE_HEADER &fileHeader_;
    IMAGE_OPTIONAL_HEADER optionalHeader_;

public:
    PeParserImpl(QIODevice *source, core::image::Image *image, const LogToken &log, IMAGE_FILE_HEADER &fileHeader):
        source_(source), image_(image), log_(log), fileHeader_(fileHeader)
    {}

    void parse() {
        optionalHeaderOffset_ = source_->pos() - sizeof(optionalHeader_.Magic);

        parseFileHeader();
        parseOptionalHeader();
        parseSections();
        parseSymbols();
        parseImports();
        parseBaseRelocs();
        parseExports();
        image_->setEntryPoint(optionalHeader_.ImageBase + optionalHeader_.AddressOfEntryPoint);
    }

private:
    void parseFileHeader() {
        /*
         * fileHeader_.Characteristics contains endianness flags:
         * IMAGE_FILE_BYTES_REVERSED_LO for little endian,
         * IMAGE_FILE_BYTES_REVERSED_HI for big endian.
         * However, it is unclear how to get these flags without
         * knowing the endianness in which Characteristics are stored...
         */
        peByteOrder.convertFrom(fileHeader_.PointerToSymbolTable);
        peByteOrder.convertFrom(fileHeader_.NumberOfSymbols);
        peByteOrder.convertFrom(fileHeader_.NumberOfSections);
    }

    void parseOptionalHeader() {
        if (!source_->seek(optionalHeaderOffset_)) {
            throw ParseError(tr("Cannot seek to the optional header."));
        }

        if (!read(source_, optionalHeader_)) {
            throw ParseError(tr("Cannot read the optional header."));
        }

        peByteOrder.convertFrom(optionalHeader_.ImageBase);
        peByteOrder.convertFrom(optionalHeader_.AddressOfEntryPoint);

        foreach (auto &entry, optionalHeader_.DataDirectory) {
            peByteOrder.convertFrom(entry.VirtualAddress);
            peByteOrder.convertFrom(entry.Size);
        }
    }

    void parseSections() {
        if (!source_->seek(optionalHeaderOffset_ + fileHeader_.SizeOfOptionalHeader)) {
            log_.warning(tr("Cannot seek to the section header table."));
            return;
        }

        for (std::size_t i = 0; i < fileHeader_.NumberOfSections; ++i) {
            IMAGE_SECTION_HEADER sectionHeader;
            if (!read(source_, sectionHeader)) {
                log_.warning(tr("Cannot read the section header number %1.").arg(i));
                return;
            }

            peByteOrder.convertFrom(sectionHeader.VirtualAddress);
            peByteOrder.convertFrom(sectionHeader.SizeOfRawData);
            peByteOrder.convertFrom(sectionHeader.Characteristics);
            peByteOrder.convertFrom(sectionHeader.PointerToRawData);

            auto section = std::make_unique<core::image::Section>(
                getAsciizString(sectionHeader.Name), sectionHeader.VirtualAddress + optionalHeader_.ImageBase,
                sectionHeader.SizeOfRawData);

            section->setAllocated((sectionHeader.Characteristics & IMAGE_SCN_MEM_DISCARDABLE) == 0);
            section->setReadable(sectionHeader.Characteristics & IMAGE_SCN_MEM_READ);
            section->setWritable(sectionHeader.Characteristics & IMAGE_SCN_MEM_WRITE);
            section->setExecutable(sectionHeader.Characteristics & IMAGE_SCN_MEM_EXECUTE);

            section->setCode(sectionHeader.Characteristics & IMAGE_SCN_CNT_CODE);
            section->setData(sectionHeader.Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA);
            section->setBss(sectionHeader.Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA);

            if (sectionHeader.SizeOfRawData == 0) {
                log_.debug(tr("Section %1 has no raw data.").arg(section->name()));
            } else {
                log_.debug(tr("Reading contents of section %1 (size of raw data = 0x%2).").arg(section->name()).arg(sectionHeader.SizeOfRawData));

                QByteArray bytes;

                auto pos = source_->pos();
                if (source_->seek(sectionHeader.PointerToRawData)) {
                    bytes = source_->read(sectionHeader.SizeOfRawData);
                } else {
                    log_.warning(tr("Could not seek to the data of section %1.").arg(section->name()));
                }
                source_->seek(pos);

                if (static_cast<DWORD>(bytes.size()) != sectionHeader.SizeOfRawData) {
                    log_.warning(tr("Could read only 0x%1 bytes of section %2, although its raw size is 0x%3.")
                                     .arg(bytes.size(), 0, 16)
                                     .arg(section->name())
                                     .arg(sectionHeader.SizeOfRawData));
                }

                section->setContent(std::move(bytes));
            }

            image_->addSection(std::move(section));
        }
    }

    void parseSymbols() {
        if (!fileHeader_.PointerToSymbolTable || !fileHeader_.NumberOfSymbols) {
            return;
        }

        if (!source_->seek(fileHeader_.PointerToSymbolTable)) {
            log_.warning(tr("Cannot seek to the symbol table."));
            return;
        }

        /*
         * http://www.delorie.com/djgpp/doc/coff/symtab.html
         */
        std::vector<IMAGE_SYMBOL> symbols(fileHeader_.NumberOfSymbols);

        if (!read(source_, *symbols.data(), fileHeader_.NumberOfSymbols)) {
            log_.warning(tr("Cannot read the symbol table."));
            return;
        }

        uint32_t stringTableSize;
        if (!read(source_, stringTableSize)) {
            log_.warning(tr("Cannot read the size of the string table."));
            return;
        }

        QByteArray stringTable(4, 0);
        stringTable.resize(stringTableSize);
        if (!read(source_, stringTable.data()[4], stringTableSize - 4)) {
            log_.warning(tr("Cannot read the string table."));
            return;
        }

        foreach (IMAGE_SYMBOL &symbol, symbols) {
            peByteOrder.convertFrom(symbol.Type);
            peByteOrder.convertFrom(symbol.Value);
            peByteOrder.convertFrom(symbol.SectionNumber);

            using core::image::Symbol;
            using core::image::SymbolType;

            SymbolType type;
            /*
             * Microsoft tools set this field to 0x20 (function) or 0x0 (not a function).
             * -- http://www.kishorekumar.net/pecoff_v8.1.htm
             */
            if (symbol.Type == 0x20) {
                type = SymbolType::FUNCTION;
            } else {
                type = SymbolType::NOTYPE;
            }

            QString name;
            if (symbol.N.Name.Short) {
                name = getAsciizString(symbol.N.ShortName);
            } else {
                name = getAsciizString(stringTable, symbol.N.Name.Long);
            }

            auto value = symbol.Value;
            const core::image::Section *section = nullptr;

            std::size_t sectionNumber = symbol.SectionNumber - 1;
            if (sectionNumber < image_->sections().size()) {
                section = image_->sections()[sectionNumber];
                value += section->addr();
            }

            image_->addSymbol(std::make_unique<Symbol>(type, name, value, section));
        }

        foreach (auto section, image_->sections()) {
            if (section->name().startsWith('/')) {
                if (auto offset = stringToInt<uint32_t>(section->name().mid(1))) {
                    QString newName = getAsciizString(stringTable, *offset);
                    if (!newName.isEmpty()) {
                        section->setName(newName);
                    }
                }
            }
        }
    }

    void parseImports() {
        if (optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0) {
            return;
        }

        auto reader = core::image::Reader(image_);

        IMAGE_IMPORT_DESCRIPTOR descriptor;
        for (auto descriptorAddress = optionalHeader_.ImageBase + optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
             image_->readBytes(descriptorAddress, reinterpret_cast<char *>(&descriptor), sizeof(descriptor)) == sizeof(descriptor);
             descriptorAddress += sizeof(descriptor))
        {
            if (descriptor.Characteristics == 0) {
                break;
            }

            peByteOrder.convertFrom(descriptor.Characteristics);
            peByteOrder.convertFrom(descriptor.Name);
            peByteOrder.convertFrom(descriptor.FirstThunk);

            auto dllName = reader.readAsciizString(descriptor.Name + optionalHeader_.ImageBase, 1024);
            log_.debug(tr("Found imports from DLL: %1").arg(dllName));

            parseImportAddressTable(dllName, descriptor.FirstThunk + optionalHeader_.ImageBase);
        }
    }

    void parseImportAddressTable(const QString &dllName, ByteAddr virtualAddress) {
        auto reader = core::image::Reader(image_);

        IMPORT_LOOKUP_TABLE_ENTRY entry;
        for (auto entryAddress = virtualAddress;
             image_->readBytes(entryAddress, reinterpret_cast<char *>(&entry), sizeof(entry)) == sizeof(entry);
             entryAddress += sizeof(entry))
        {
            if (entry.RawValue == 0) {
                break;
            }

            peByteOrder.convertFrom(entry);

            if (entry.IsOrdinal) {
                log_.debug(tr("Found an import by ordinal value: %1").arg(entry.Name));

                image_->addRelocation(std::make_unique<core::image::Relocation>(
                    entryAddress,
                    image_->addSymbol(std::make_unique<core::image::Symbol>(
                        core::image::SymbolType::FUNCTION, tr("%1:%2").arg(dllName).arg(entry.Name), boost::none)),
                    sizeof(IMPORT_LOOKUP_TABLE_ENTRY)));
            } else {
                auto name = reader.readAsciizString(
                    optionalHeader_.ImageBase + entry.Name + sizeof(IMAGE_IMPORT_BY_NAME().Hint), 1024);

                log_.debug(tr("Found an import by name: %1").arg(name));

                image_->addRelocation(std::make_unique<core::image::Relocation>(
                    entryAddress, image_->addSymbol(std::make_unique<core::image::Symbol>(
                                      core::image::SymbolType::FUNCTION, std::move(name), boost::none)),
                    sizeof(IMPORT_LOOKUP_TABLE_ENTRY)));
            }
        }
    }
    void parseExports() {
        if (optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress == 0) {
            return;
        }

        auto reader = core::image::Reader(image_);

        IMAGE_EXPORT_DIRECTORY directory;
        auto directoryAddress =
            optionalHeader_.ImageBase + optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;

        if (image_->readBytes(directoryAddress, reinterpret_cast<char *>(&directory), sizeof(directory)) != sizeof(directory)) {
            log_.warning(tr("Cannot read the image export directory."));
            return;
        }

        peByteOrder.convertFrom(directory.Characteristics);
        peByteOrder.convertFrom(directory.Name);
        peByteOrder.convertFrom(directory.NumberOfFunctions);
        peByteOrder.convertFrom(directory.NumberOfNames);
        peByteOrder.convertFrom(directory.AddressOfFunctions);
        peByteOrder.convertFrom(directory.AddressOfNames);
        peByteOrder.convertFrom(directory.AddressOfNameOrdinal);

        for (DWORD i = 0; i < directory.NumberOfNames; i++) {
            WORD ordinal;
            DWORD nameRVA;
            if (image_->readBytes(optionalHeader_.ImageBase + directory.AddressOfNames + i * sizeof(nameRVA),
                                  reinterpret_cast<char *>(&nameRVA), sizeof(nameRVA)) != sizeof(nameRVA)) {
                log_.warning(tr("Cannot read the address value of the export directory item number %1.").arg(i));
                return;
            }
            if (image_->readBytes(optionalHeader_.ImageBase + directory.AddressOfNameOrdinal + i * sizeof(ordinal),
                                  reinterpret_cast<char *>(&ordinal), sizeof(ordinal)) != sizeof(ordinal)) {
                log_.warning(tr("Cannot read the ordinal value of the export directory item number %1.").arg(i));
                return;
            }
            peByteOrder.convertFrom(nameRVA);
            peByteOrder.convertFrom(ordinal);

            DWORD entry;
            if (image_->readBytes(optionalHeader_.ImageBase + directory.AddressOfFunctions + ordinal * sizeof(entry),
                                  reinterpret_cast<char *>(&entry), sizeof(entry)) != sizeof(entry)) {
                log_.warning(
                    tr("Cannot read the function address value of the export directory item number %1.").arg(i));
                return;
            }

            peByteOrder.convertFrom(entry);

            if (entry >= directoryAddress &&
                entry < directoryAddress + optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
                // entries with the export section are forwarded to another dll
                continue;
            }

            auto name = reader.readAsciizString(optionalHeader_.ImageBase + nameRVA, 1024);
            image_->addSymbol(std::make_unique<core::image::Symbol>(core::image::SymbolType::FUNCTION, std::move(name),
                                                                    optionalHeader_.ImageBase + entry));
        }
    }

    void parseBaseRelocs() {
        if (optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress == 0) {
            return;
        }

        auto reader = core::image::Reader(image_);

        auto headerAddress =
            optionalHeader_.ImageBase + optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;

        auto end = headerAddress + optionalHeader_.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
        const core::image::Section *section = image_->getSectionByName(".reloc");
        if (!section)
            return;

        const core::image::Symbol *baseSymbol = image_->addSymbol(std::make_unique<core::image::Symbol>(
            core::image::SymbolType::NOTYPE, "(image base)", optionalHeader_.ImageBase));

        while (headerAddress < end) {
            IMAGE_BASE_RELOC_BLOCK_HEADER header;
            if (section->readBytes(headerAddress, reinterpret_cast<char *>(&header), sizeof(header)) != sizeof(header)) {
                log_.warning(tr("Cannot read the image base reloc header."));
                return;
            }
            peByteOrder.convertFrom(header.PageRVA);
            peByteOrder.convertFrom(header.Size);

            int num = (header.Size - 8) / 2;
            for (int i = 0; i < num; i++ ) {
                WORD reloc;
                if (section->readBytes(headerAddress + 8 + i * sizeof(reloc),
                                       reinterpret_cast<char *>(&reloc), sizeof(reloc)) != sizeof(reloc)) {
                    log_.warning(tr("Cannot read the base reloc number %1.").arg(i));
                    return;
                }
                peByteOrder.convertFrom(reloc);
                WORD type = reloc >> 12;
                WORD offset = reloc & ((1<<12) - 1);
                DWORD address = header.PageRVA + offset + optionalHeader_.ImageBase;
                ByteSize addend;
                int size;

                switch (type) {
                    case IMAGE_REL_BASED_HIGHLOW:
                        {
                            size = 4;
                            DWORD currentValue;
                            if (image_->readBytes(address,
                                                  reinterpret_cast<char *>(&currentValue), sizeof(currentValue)) != sizeof(currentValue)) {
                                log_.warning(tr("Cannot read reloc at %1.").arg(address));
                                continue;
                            }
                            addend = currentValue - optionalHeader_.ImageBase;
                            break;
                        }
                    case IMAGE_REL_BASED_DIR64:
                        {
                            size = 8;
                            ByteAddr currentValue;
                            if (image_->readBytes(address,
                                                  reinterpret_cast<char *>(&currentValue), sizeof(currentValue)) != sizeof(currentValue)) {
                                log_.warning(tr("Cannot read reloc at %1.").arg(address));
                                continue;
                            }
                            addend = currentValue - optionalHeader_.ImageBase;
                            break;
                        }
                    case IMAGE_REL_BASED_ABSOLUTE:
                        continue;
                    default:
                        log_.warning(tr("Unknown base reloc type %1.").arg(type));
                        continue;
                }

                image_->addRelocation(std::make_unique<core::image::Relocation>(address, baseSymbol, size, addend));
            }
            headerAddress += header.Size;
        }
    }
};

} // anonymous namespace

PeParser::PeParser():
    core::input::Parser(QLatin1String("PE"))
{}

bool PeParser::doCanParse(QIODevice *source) const {
    return seekFileHeader(source);
}

void PeParser::doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const {
    if (!seekFileHeader(source)) {
        throw ParseError(tr("PE signature doesn't match."));
    }

    IMAGE_FILE_HEADER fileHeader;
    if (!read(source, fileHeader)) {
        throw ParseError(tr("Cannot read the file header."));
    }

    peByteOrder.convertFrom(fileHeader.Machine);
    switch (fileHeader.Machine) {
        case IMAGE_FILE_MACHINE_I386:
            image->platform().setArchitecture(QLatin1String("i386"));
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            image->platform().setArchitecture(QLatin1String("x86-64"));
            break;
        case IMAGE_FILE_MACHINE_ARM: /* FALLTHROUGH */
        case IMAGE_FILE_MACHINE_THUMB:
            image->platform().setArchitecture(QLatin1String("arm-le"));
            break;
        default:
            throw ParseError(tr("Unknown machine id: 0x%1.").arg(fileHeader.Machine, 0, 16));
    }

    /* Just a guess. */
    image->platform().setOperatingSystem(core::image::Platform::Windows);

    WORD optionalHeaderMagic;
    if (!read(source, optionalHeaderMagic)) {
        throw ParseError(tr("Cannot read magic of the optional header."));
    }

    peByteOrder.convertFrom(optionalHeaderMagic);
    switch (optionalHeaderMagic) {
        case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
            log.debug(tr("Parsing as a PE32 file."));
            PeParserImpl<IMAGE_OPTIONAL_HEADER32, IMPORT_LOOKUP_TABLE_ENTRY32>(source, image, log, fileHeader).parse();
            break;
        case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
            log.debug(("Parsing as a PE32+ file."));
            PeParserImpl<IMAGE_OPTIONAL_HEADER64, IMPORT_LOOKUP_TABLE_ENTRY64>(source, image, log, fileHeader).parse();
            break;
        default:
            throw ParseError(tr("Unknown optional header magic: 0x%1").arg(optionalHeaderMagic, 0, 16));
    }
}

} // namespace pe
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
