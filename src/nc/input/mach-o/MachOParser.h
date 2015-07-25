/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/input/Parser.h>

namespace nc {
namespace input {
namespace mach_o {

/**
 * Parser for the Mach-O file format.
 */
class MachOParser: public core::input::Parser {
public:
    MachOParser();

protected:
    virtual bool doCanParse(QIODevice *source) const override;
    virtual void doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const override;
};

} // namespace mach_o
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
