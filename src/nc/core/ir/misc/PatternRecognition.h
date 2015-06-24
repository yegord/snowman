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

namespace nc {
namespace core {
namespace ir {

class BasicBlock;
class Jump;
class Term;

namespace dflow {
    class Dataflow;
}

namespace misc {

class ArrayAccess;
class BoundsCheck;

/**
 * Parses an expression of the form [base + stride * index], with constant base and stride.
 *
 * \param[in] term      Valid pointer to a term.
 * \param[in] dataflow  Dataflow information.
 *
 * \returns Valid indexing expression if the term has a form like [a+b*x].
 *          Otherwise, an invalid descriptors is returned.
 */
ArrayAccess recognizeArrayAccess(const Term *term, const dflow::Dataflow &dataflow);

/**
 * Parses a jump of the form "if (index <= maxValue) then goto ifPassed else goto ifFailed",
 * with constant maxValue.
 *
 * \param[in] jump      Valid pointer to a jump.
 * \param[in] ifPassed  Valid pointer to the basic block getting control if the bounds check passes.
 * \param[in] dataflow  Dataflow information.
 *
 * \return Valid bounds check description if the jump implements a check like "x <= c".
 *         Otherwise, an invalid descriptor is returned.
 */
BoundsCheck recognizeBoundsCheck(const Jump *jump, const BasicBlock *ifPassed, const dflow::Dataflow &dataflow);

}}}} // namespace nc::core::ir::misc

/* vim:set et sts=4 sw=4: */
