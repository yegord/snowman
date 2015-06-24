/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

#pragma once

#include <nc/config.h>

#include <boost/unordered_map.hpp>

#include "Graph.h"

namespace nc {
namespace core {
namespace ir {

class Function;

namespace cflow {

/**
 * Mapping from a function to its structural graph.
 */
class Graphs: public boost::unordered_map<const Function *, std::unique_ptr<const Graph>> {};

} // namespace cflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
