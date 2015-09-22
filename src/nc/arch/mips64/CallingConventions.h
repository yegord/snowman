/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <nc/core/ir/calling/Convention.h>

namespace nc {
namespace arch {
namespace mips64 {

class Mips64Architecture;

class n32n64CallingConvention: public core::ir::calling::Convention {
public:
    n32n64CallingConvention(const Mips64Architecture *architecture_);
};

}}} // namespace nc::arch::mips64

/* vim:set et sts=4 sw=4: */
