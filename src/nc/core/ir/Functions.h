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

#include <boost/noncopyable.hpp>

#include <nc/common/Printable.h>
#include <nc/common/ilist.h>

namespace nc {
namespace core {
namespace ir {

class Function;

// TODO: get rid of this class?
/**
 * Functions in intermediate representation.
 */
class Functions: public PrintableBase<Functions>, boost::noncopyable {
public:
    typedef nc::ilist<Function> FunctionList;

private:
    /** List of functions. */
    FunctionList functions_;

public:
    /**
     * Constructor.
     */
    Functions();

    /**
     * Destructor.
     */
    ~Functions();

    /**
     * \return List of functions.
     *
     * \warning Do not insert functions into the container directly.
     *          Use methods of Functions class instead.
     */
    FunctionList &list() { return functions_; }

    /**
     * \return List of functions.
     */
    const FunctionList &list() const { return functions_; }

    /**
     * Adds intermediate representation of a function and takes ownership of it.
     *
     * \param[in] function Valid pointer to a function.
     */
    void addFunction(std::unique_ptr<Function> function);

    /**
     * Prints the intermediate representation of all functions into a stream in DOT format.
     *
     * \param[in] out Output stream.
     */
    void print(QTextStream &out) const;
};

} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
