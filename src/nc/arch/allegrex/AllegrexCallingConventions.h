/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/ir/calling/Convention.h>

namespace nc {
namespace arch {
namespace allegrex {

class AllegrexArchitecture;

class DefaultCallingConvention: public core::ir::calling::Convention {
public:
    DefaultCallingConvention(const AllegrexArchitecture *architecture_);
};

}}} // namespace nc::arch::allegrex

/* vim:set et sts=4 sw=4: */
