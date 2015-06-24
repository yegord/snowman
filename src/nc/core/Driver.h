/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/common/Types.h>

#include <QCoreApplication> /* For Q_DECLARE_TR_FUNCTIONS. */

namespace nc {
namespace core {

namespace image {
    class Section;
    class ByteSource;
}

class Context;

/**
 * Relatively high-level interface for running analyses in the right order.
 */
class Driver {
    Q_DECLARE_TR_FUNCTIONS(Driver)

public:
    /**
     * Parses a file by the first suitable parser.
     *
     * \param context Context.
     * \param filename Name of the file to parse.
     */
    static void parse(Context &context, const QString &filename);

    /**
     * Disassembles all code sections.
     *
     * \param context Context.
     */
    static void disassemble(Context &context);

    /**
     * Disassembles an image section.
     *
     * \param context Context.
     * \param section Valid pointer to the image section.
     */
    static void disassemble(Context &context, const image::Section *section);

    /*
     * Disassembles all instructions in the given range of addresses.
     *
     * \param context Context.
     * \param source Valid pointer to a byte source.
     * \param begin First address in the range.
     * \param end First address past the range.
     */
    static void disassemble(Context &context, const image::ByteSource *source, ByteAddr begin, ByteAddr end);

    /**
     * Performs decompilation by running all the necessary
     * analyses in the given context in the right order.
     *
     * \param context Context.
     */
    static void decompile(Context &context);
};

} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
