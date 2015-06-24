/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_map.hpp>

#include "Dataflow.h"

namespace nc {
namespace core {
namespace ir {

class Function;

namespace dflow {

/**
 * Mapping from a function to its dataflow information.
 */
class Dataflows: public boost::unordered_map<const Function *, std::unique_ptr<const Dataflow>> {};

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
