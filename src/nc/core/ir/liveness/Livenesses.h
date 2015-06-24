/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_map.hpp>

#include "Liveness.h"

namespace nc {
namespace core {
namespace ir {

class Function;

namespace liveness {

/**
 * Mapping from a function to its liveness.
 */
class Livenesses: public boost::unordered_map<const Function *, std::unique_ptr<const Liveness>> {};

} // namespace liveness
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
