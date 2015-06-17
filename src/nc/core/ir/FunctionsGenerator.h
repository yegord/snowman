/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

/* * SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
 * Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
 * Alexander Fokin, Sergey Levin, Leonid Tsvetkov
 *
 * This file is part of SmartDec decompiler.
 *
 * SmartDec decompiler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SmartDec decompiler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <nc/config.h>

#include <memory>
#include <vector>

#include <boost/unordered_map.hpp>

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class Function;
class Functions;
class Program;

/**
 * Generator of functions from control flow graph.
 */
class FunctionsGenerator {
public:
    /**
     * Virtual destructor.
     */
    virtual ~FunctionsGenerator() {}

    /**
     * Discovers functions in the control flow graph and creates corresponding
     * Function objects.
     *
     * \param[in] program Intermediate representation of a program.
     * \param[out] functions Where to add newly created functions.
     */
    virtual void makeFunctions(const Program &program, Functions &functions) const;

    /**
     * Creates a function out of a set of nodes and, optionally, entry basic block.
     *
     * \param[in] basicBlocks Basic blocks belonging to the function.
     * \param[in] entry Entry basic block of a function, if known.
     *
     * \return Valid pointer to the created function.
     */
    virtual std::unique_ptr<Function> makeFunction(const std::vector<const BasicBlock *> &basicBlocks, const BasicBlock *entry = nullptr) const;

    /**
     * Mapping from basic blocks to basic blocks.
     */
    typedef boost::unordered_map<const BasicBlock *, BasicBlock *> BasicBlockMap;

    /**
     * Clones basic blocks and arcs between them.
     * Pointers to basic blocks in Jump statements are patched accordingly too.
     *
     * \param basicBlocks   Vector of valid pointers to basic blocks being cloned.
     * \param function      Function to add basic blocks to.
     *
     * \return Mapping of basic blocks to their clones.
     */
    static BasicBlockMap cloneIntoFunction(const std::vector<const BasicBlock *> &basicBlocks, Function *function);
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
