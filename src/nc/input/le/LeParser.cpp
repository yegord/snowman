#include "LeParser.h"
#include <nc/core/input/ParseError.h>
#include <nc/core/input/Utils.h>
#include <nc/core/image/Section.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Relocation.h>
#include <nc/common/make_unique.h>
#include <nc/common/LogToken.h>
#include <nc/common/ByteOrder.h>
#include <nc/common/Foreach.h>

namespace nc {
namespace input {
namespace le {

namespace {

struct header_pos {
    qint64 mz;
    qint64 le;
};

using nc::core::input::read;
using nc::core::input::ParseError;

const ByteOrder bo = ByteOrder::LittleEndian;

header_pos find_header_pos(QIODevice *in) {
    char a;
    while (in->getChar(&a)) {
    again:
        if (a != 'M')
            continue;
        char b;
        if (!in->getChar(&b))
            break;
        if (b != 'Z') {
            a = b;
            goto again;
        }
        qint64 mz = in->pos() - 2;
        if (!in->seek(mz + 0x3c))
            break;
        std::uint32_t le_off;
        if (!read(in, le_off))
            break;
        bo.convertFrom(le_off);
        if (!in->seek(mz + le_off))
        {
        rewind:
            in->seek(mz + 2);
            continue;
        }
        char le_sig[2];
        if (!read(in, le_sig))
            goto rewind;
        if (memcmp(le_sig, "LE", 2))
            goto rewind;
        header_pos hpos = { .mz = mz, .le = mz + le_off };
        return hpos;
    }
    return { -1, -1 };
}

#define LE_HEADER \
    FIELD(std::uint16_t, signature) \
    FIELD(std::uint8_t,  byte_order) \
    FIELD(std::uint8_t,  word_order) \
    FIELD(std::uint32_t, unused) \
    FIELD(std::uint16_t, cpu) \
    FIELD(std::uint16_t, os) \
    FIELD(std::uint32_t, unused2) \
    FIELD(std::uint32_t, unused3) \
    FIELD(std::uint32_t, pages) \
    FIELD(std::uint32_t, initial_object_CS_number) \
    FIELD(std::uint32_t, initial_EIP) \
    FIELD(std::uint32_t, initial_object_SS_number) \
    FIELD(std::uint32_t, initial_ESP) \
    FIELD(std::uint32_t, memory_page_size) \
    FIELD(std::uint32_t, bytes_on_last_page) \
    FIELD(std::uint32_t, fixup_section_size) \
    FIELD(std::uint32_t, fixup_section_checksum) \
    FIELD(std::uint32_t, loader_section_size) \
    FIELD(std::uint32_t, loader_section_checksum) \
    FIELD(std::uint32_t, offset_of_object_table) \
    FIELD(std::uint32_t, object_table_entries) \
    FIELD(std::uint32_t, object_page_map_offset) \
    FIELD(std::uint32_t, object_iterate_data_map_offset) \
    FIELD(std::uint32_t, resource_table_offset) \
    FIELD(std::uint32_t, resource_table_entries) \
    FIELD(std::uint32_t, resident_names_table_offset) \
    FIELD(std::uint32_t, entry_table_offset) \
    FIELD(std::uint32_t, module_directive_table_offset) \
    FIELD(std::uint32_t, module_directive_entries) \
    FIELD(std::uint32_t, fixup_page_table_offset) \
    FIELD(std::uint32_t, fixup_record_table_offset) \
    FIELD(std::uint32_t, imported_modules_name_table_offset) \
    FIELD(std::uint32_t, inported_modules_count) \
    FIELD(std::uint32_t, imported_procedure_name_table_offset) \
    FIELD(std::uint32_t, per_page_checksum_table_offset) \
    FIELD(std::uint32_t, data_pages_offset_from_top_of_file) \

enum {
    OBJECT_READABLE     = 1 << 0,
    OBJECT_WRITABLE     = 1 << 1,
    OBJECT_EXECUTABLE   = 1 << 2,
    OBJECT_DISCARDABLE  = 1 << 4,
};

#define OBJ_HEADER \
    FIELD(std::uint32_t, virtual_segment_size) \
    FIELD(std::uint32_t, relocation_base_address) \
    FIELD(std::uint32_t, object_flags) \
    FIELD(std::uint32_t, page_map_index) \
    FIELD(std::uint32_t, page_map_entries) \
    FIELD(std::uint32_t, unused) \

#define FIXUP_HEADER \
    FIELD(std::uint8_t, src) \
    FIELD(std::uint8_t, flags) \
    FIELD(std::int16_t, srcoff) \

#define EMIT_ALL \
    EMIT(LE_HEADER, le_header) \
    EMIT(OBJ_HEADER, obj_header) \
    EMIT(FIXUP_HEADER, fixup_header) \

// emitting structs
#define EMIT(fields, name) struct name { fields } __attribute__((packed));
#define FIELD(t, n) t n;
EMIT_ALL
#undef FIELD
#undef EMIT

static_assert(sizeof(le_header) == 132, "le_header has incorrect size");
static_assert(sizeof(obj_header) == 24, "obj_header has incorrect size");
static_assert(sizeof(fixup_header) == 4, "fixup_header has incorrect size");

// -----------------------------------------------------------------------------
#define EMIT(fields, name) void fix_byte_order(name &h) { fields }
#define FIELD(t, n) bo.convert(&h.n, sizeof(h.n), bo, ByteOrder::Current);
EMIT_ALL
#undef FIELD
#undef EMIT

template <typename T>
void fix_byte_order(T &v) {
    bo.convertFrom(v);
}

// -----------------------------------------------------------------------------

#define EMIT(fields, name) \
QString toQString(const name &h) __attribute__((unused)); \
QString toQString(const name &h) { \
    QString s = QString::fromLatin1(#name ":"); \
    fields \
    return s; \
}
#define FIELD(t, n) s.append(QString::fromLatin1("\n" #n " %1").arg(h.n, 1, 16));
EMIT_ALL
#undef EMIT
#undef FIELD

// -----------------------------------------------------------------------------

template <typename V, typename E>
void checked_read(QIODevice *in, V &val, const E &err) {
    if (!read(in, val))
        err();
    fix_byte_order(val);
}

} // namespace

LeParser::LeParser():
    core::input::Parser(QLatin1String("LE"))
{}

bool LeParser::doCanParse(QIODevice *in) const {
    return find_header_pos(in).le != -1;
}

void LeParser::doParse(QIODevice *in, core::image::Image *image, const LogToken &log) const {
    header_pos hpos = find_header_pos(in);
    le_header h;
    if (!in->seek(hpos.le) || !read(in, h)) {
        throw ParseError(tr("Truncated LE header"));
    }
    if (h.byte_order) {
        throw ParseError(tr("Big endian byte order in LE is unsupported"));
    }
    if (h.word_order) {
        throw ParseError(tr("Big endian word order in LE is unsupported"));
    }
    fix_byte_order(h);
    log.debug(toQString(h));

    image->platform().setArchitecture(QLatin1String("i386"));
    image->platform().setOperatingSystem(core::image::Platform::DOS);

    // = loading sections =

    std::vector<obj_header> sec_headers;
    for (std::uint32_t oi = 0; oi < h.object_table_entries; ++oi) {
        qint64 pos = hpos.le + h.offset_of_object_table + 24 * oi;
        obj_header oh;
        if (!in->seek(pos) || !read(in, oh)) {
            throw ParseError(tr("Truncated object entry %1").arg(oi));
        }
        fix_byte_order(oh);
        sec_headers.push_back(oh);
        auto section = std::make_unique<core::image::Section>(
                            QString(QLatin1String(".seg%1")).arg(oi),
                            ByteAddr(oh.relocation_base_address),
                            ByteAddr(oh.page_map_entries * h.memory_page_size));
        section->setAllocated((oh.object_flags & OBJECT_DISCARDABLE) == 0);
        section->setReadable(oh.object_flags & OBJECT_READABLE);
        section->setWritable(oh.object_flags & OBJECT_WRITABLE);
        if (oh.object_flags & OBJECT_EXECUTABLE) {
            section->setExecutable(true);
            section->setCode(true);
        } else {
            section->setData(true);
        }
        qint64 off = hpos.mz + h.data_pages_offset_from_top_of_file + (oh.page_map_index - 1) * h.memory_page_size;
        qint64 len = oh.page_map_entries * h.memory_page_size;
        if (oi == h.object_table_entries - 1) { // last object has last page
            len -= h.memory_page_size - h.bytes_on_last_page;
        }
        QByteArray bytes;
        if (!in->seek(off) || (bytes = in->read(len), bytes.size() != len)) {
            throw ParseError(tr("Truncated object body at 0x%1:0x%2 for object %3").arg(off, 1, 16).arg(len, 1, 16).arg(oi));
        }
        section->setContent(std::move(bytes));
        log.debug(tr("Adding section %1 at 0x%2:0x%3").arg(section->name()).arg(off, 1, 16).arg(len, 1, 16));
        image->addSection(std::move(section));
        if (h.initial_object_CS_number - 1 == oi) {
            image->setEntryPoint(oh.relocation_base_address + h.initial_EIP);
            log.debug(tr("Entry point set to 0x%1").arg(oh.relocation_base_address + h.initial_EIP, 1, 16));
        }

        core::image::Section *s = image->sections()[oi];
        image->addSymbol(std::make_unique<core::image::Symbol>(
                    core::image::SymbolType::NOTYPE, s->name(), ByteAddr(oh.relocation_base_address), s));
    }

    // = loading fixups =

    qint64 fpt_off = hpos.le + h.fixup_page_table_offset;
    std::vector<std::uint32_t> fixup_page_table(h.pages + 1);
    qint64 fpt_size = fixup_page_table.size() * 4;
    if (!in->seek(fpt_off) || in->read((char *) &fixup_page_table[0], fpt_size) != fpt_size) {
        throw ParseError(tr("Truncated fixup page table at 0x%1:0x%2").arg(fpt_off, 1, 16).arg(fpt_size, 1, 16));
    }
    foreach(auto &s, fixup_page_table) {
        bo.convertFrom(s);
    }

    for (std::size_t npage = 0; npage < h.pages; ++npage) {
        qint64 start = hpos.le + h.fixup_record_table_offset + fixup_page_table[npage];
        qint64 end = hpos.le + h.fixup_record_table_offset + fixup_page_table[npage + 1];
        if (!in->seek(start)) {
            throw ParseError(tr("Truncated fixup block for page %1 at 0x%2").arg(npage).arg(start, 1, 16));
        }

        std::uint32_t seci;
        for (seci = 0; seci < sec_headers.size(); ++seci) {
            std::size_t b = sec_headers[seci].page_map_index - 1;
            if (npage >= b && npage < b + sec_headers[seci].page_map_entries)
                break;
        }
        if (seci == sec_headers.size()) {
            throw ParseError(tr("No section corresponds to page %1").arg(npage));
        }
        ByteAddr page_virt_addr = image->sections()[seci]->addr() + (npage + 1 - sec_headers[seci].page_map_index) * h.memory_page_size;

        // see Fixup Record Table in http://www.textfiles.com/programming/FORMATS/lxexe.txt
        enum {
            FIXUP_SRC_16BIT_SELECTOR = 2,
            FIXUP_SRC_16BIT = 5,
            FIXUP_SRC_32BIT = 7,
            FIXUP_DST_FLAG_32BIT_TARGET_OFFSET = 0x10,
            FIXUP_DST_FLAG_16BIT_OBJECT_ID = 0x40,
        };
        while (in->pos() < end) {
            qint64 start_mark = in->pos();
            auto throw_truncated = [npage, start_mark]() {
                throw ParseError(tr("Truncated fixup for page %1 at 0x%2").arg(npage).arg(start_mark, 1, 16));
            };
            fixup_header fh;
            checked_read(in, fh, throw_truncated);

            std::uint16_t dstobj;
            if (fh.flags & FIXUP_DST_FLAG_16BIT_OBJECT_ID) {
                checked_read(in, dstobj, throw_truncated);
            } else {
                std::uint8_t dstobj8;
                checked_read(in, dstobj8, throw_truncated);
                dstobj = dstobj8;
            }
            if (dstobj > sec_headers.size()) {
                throw ParseError(tr("Fixup at 0x%1 mentions object %2, but binary has only %3 objects")
                                    .arg(start_mark, 1, 16).arg(dstobj).arg(sec_headers.size()));
            }

            if (fh.src == FIXUP_SRC_16BIT_SELECTOR) {
                continue; // skipping rare segment selector relocations TODO
            }

            std::uint32_t dst;
            std::uint16_t dst16;
            ByteSize width = 4;
            if (fh.src == FIXUP_SRC_16BIT) {
                checked_read(in, dst16, throw_truncated);
                dst = dst16;
                width = 2;
            } else {
                if (fh.src != FIXUP_SRC_32BIT) {
                    throw ParseError(tr("Fixup at 0x%1 has unsupported src %2").arg(start_mark, 1, 16).arg(fh.src));
                }
                if (fh.flags == FIXUP_DST_FLAG_32BIT_TARGET_OFFSET) {
                    checked_read(in, dst, throw_truncated);
                } else if (fh.flags == 0) {
                    checked_read(in, dst16, throw_truncated);
                    dst = dst16;
                } else {
                    throw ParseError(tr("Fixup at 0x%1 has unsupported flags 0x%2").arg(start_mark, 1, 16).arg(fh.flags, 1, 16));
                }
            }
            // skipping fixups that cross page lower boundary, they're accounted for by the previous page
            if (fh.srcoff < 0) {
                continue;
            }
            image->addRelocation(std::make_unique<core::image::Relocation>(
                    page_virt_addr + fh.srcoff,   // relocation virtual address
                    image->symbols()[dstobj - 1], // section alias as base
                    width,                        // 4 byte relocations only
                    dst));                        // offset from section start
        }
    }
}

} // namespace le
} // namespace input
} // namespace nc
