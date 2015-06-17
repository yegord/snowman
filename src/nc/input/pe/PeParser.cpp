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

#include "pe.h"

namespace nc {
namespace input {
namespace pe {

namespace {

const ByteOrder peByteOrder = ByteOrder::LittleEndian;

bool seekFileHeader(QIODevice *source) {
    IMAGE_DOS_HEADER dosHeader;

    if (source->read(reinterpret_cast<char *>(&dosHeader), sizeof(dosHeader)) != sizeof(dosHeader)) {
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
    if (source->read(reinterpret_cast<char *>(&signature), sizeof(signature)) != sizeof(signature)) {
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
    Q_DECLARE_TR_FUNCTIONS(PeParserPrivate)

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
            throw core::input::ParseError(tr("Cannot seek to the optional header."));
        }

        if (source_->read(reinterpret_cast<char *>(&optionalHeader_), sizeof(optionalHeader_)) != sizeof(optionalHeader_)) {
            throw core::input::ParseError(tr("Cannot read the optional header."));
        }

        peByteOrder.convertFrom(optionalHeader_.ImageBase);

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
            if (source_->read(reinterpret_cast<char *>(&sectionHeader), sizeof(sectionHeader)) != sizeof(sectionHeader)) {
                log_.warning(tr("Cannot read the section header number %1.").arg(i));
                return;
            }

            peByteOrder.convertFrom(sectionHeader.VirtualAddress);
            peByteOrder.convertFrom(sectionHeader.SizeOfRawData);
            peByteOrder.convertFrom(sectionHeader.Characteristics);
            peByteOrder.convertFrom(sectionHeader.PointerToRawData);
            peByteOrder.convertFrom(sectionHeader.SizeOfRawData);

            auto section = std::make_unique<core::image::Section>(
                getString(sectionHeader.Name), sectionHeader.VirtualAddress + optionalHeader_.ImageBase, sectionHeader.SizeOfRawData
            );

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

        if (source_->read(reinterpret_cast<char *>(&symbols[0]), sizeof(IMAGE_SYMBOL) * fileHeader_.NumberOfSymbols) !=
            static_cast<qint64>(sizeof(IMAGE_SYMBOL) * fileHeader_.NumberOfSymbols))
        {
            log_.warning(tr("Cannot read the symbol table."));
            return;
        }

        uint32_t stringTableSize;
        if (source_->read(reinterpret_cast<char *>(&stringTableSize), sizeof(stringTableSize)) != sizeof(stringTableSize)) {
            log_.warning(tr("Cannot read the size of the string table."));
            return;
        }

        std::unique_ptr<char[]> stringTable(new char[stringTableSize]);
        memset(stringTable.get(), 0, 4);
        if (source_->read(stringTable.get() + 4, stringTableSize - 4) != stringTableSize - 4) {
            log_.warning(tr("Cannot read the string table."));
            return;
        }

        auto getStringFromTable = [&](uint32_t offset) -> QString {
            if (offset < stringTableSize) {
                return QString::fromLatin1(
                    stringTable.get() + offset, qstrnlen(stringTable.get() + offset, stringTableSize - offset));
            } else {
                return QString();
            }
        };

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
                name = getString(symbol.N.ShortName);
            } else {
                name = getStringFromTable(symbol.N.Name.Long);
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
                    QString newName = getStringFromTable(*offset);
                    if (!newName.isEmpty()) {
                        section->setName(newName);
                    }
                }
            }
        }
    }

    template<std::size_t size>
    QString getString(const char (&buffer)[size]) const {
        return QString::fromLatin1(buffer, qstrnlen(buffer, size));
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
                        core::image::SymbolType::FUNCTION, tr("%1:%2").arg(dllName).arg(entry.Name), boost::none))));
            } else {
                auto name = reader.readAsciizString(
                    optionalHeader_.ImageBase + entry.Name + sizeof(IMAGE_IMPORT_BY_NAME().Hint), 1024);

                log_.debug(tr("Found an import by name: %1").arg(name));

                image_->addRelocation(std::make_unique<core::image::Relocation>(
                    entryAddress, image_->addSymbol(std::make_unique<core::image::Symbol>(
                                      core::image::SymbolType::FUNCTION, std::move(name), boost::none))));
            }
        }
    }
};

} // anonymous namespace

PeParser::PeParser():
    core::input::Parser("PE")
{}

bool PeParser::doCanParse(QIODevice *source) const {
    return seekFileHeader(source);
}

void PeParser::doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const {
    if (!seekFileHeader(source)) {
        throw core::input::ParseError(tr("PE signature doesn't match."));
    }

    IMAGE_FILE_HEADER fileHeader;
    if (source->read(reinterpret_cast<char *>(&fileHeader), sizeof(fileHeader)) != sizeof(fileHeader)) {
        throw core::input::ParseError(tr("Cannot read the file header."));
    }

    peByteOrder.convertFrom(fileHeader.Machine);
    switch (fileHeader.Machine) {
        case IMAGE_FILE_MACHINE_I386:
            image->setArchitecture(QLatin1String("i386"));
            break;
        case IMAGE_FILE_MACHINE_AMD64:
            image->setArchitecture(QLatin1String("x86-64"));
            break;
        case IMAGE_FILE_MACHINE_ARM: /* FALLTHROUGH */
        case IMAGE_FILE_MACHINE_THUMB:
            image->setArchitecture(QLatin1String("arm-le"));
            break;
        default:
            throw core::input::ParseError(tr("Unknown machine id: 0x%1.").arg(fileHeader.Machine, 0, 16));
    }

    WORD optionalHeaderMagic;
    if (source->read(reinterpret_cast<char *>(&optionalHeaderMagic), sizeof(optionalHeaderMagic)) != sizeof(optionalHeaderMagic)) {
        throw core::input::ParseError(tr("Cannot read magic of the optional header."));
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
            throw core::input::ParseError(tr("Unknown optional header magic: 0x%1").arg(optionalHeaderMagic, 0, 16));
    }

    image->setDemangler("msvc");
}

} // namespace pe
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
