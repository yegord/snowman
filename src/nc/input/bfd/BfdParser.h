/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/input/Parser.h>

namespace nc {
namespace input {
namespace bfdparser {

/**
 * Parser for formats supported by libbfd.
 */
class BfdParser: public core::input::Parser {
  public:
    /**
     * Constructor.
     */
    BfdParser();

  protected:
    virtual bool doCanParse(QIODevice *source) const override;
    virtual void doParse(QIODevice *source, core::image::Image *image, const LogToken &logToken) const override;
};

} // namespace bfdparser
} // namespace input
} // namespace nc

/* vim:set et sts=4 sw=4: */
