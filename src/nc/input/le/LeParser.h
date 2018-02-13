#pragma once

#include <nc/config.h>
#include <nc/core/input/Parser.h>

namespace nc {
namespace input {
namespace le {

class LeParser: public core::input::Parser {
public:
    LeParser();

protected:
    virtual bool doCanParse(QIODevice *source) const override;
    virtual void doParse(QIODevice *source, core::image::Image *image, const LogToken &log) const override;
};

} // namespace le
} // namespace input
} // namespace nc
