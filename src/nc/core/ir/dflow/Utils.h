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

class Jump;
class JumpTarget;
class Term;

namespace dflow {

class Dataflow;

/**
 * \param[in] term      Valid pointer to a term.
 * \param[in] dataflow  Dataflow information.
 *
 * \return Valid pointer to the term being the first copy of the given one.
 */
const ir::Term *getFirstCopy(const Term *term, const Dataflow &dataflow);

/**
 * \param[in] jump      Valid pointer to a jump.
 * \param[in] dataflow  Dataflow information.
 *
 * \return True iff the jump has a jump target being a return address.
 */
bool isReturn(const Jump *jump, const Dataflow &dataflow);

/**
 * \param[in] target    Jump target.
 * \param[in] dataflow  Dataflow information.
 *
 * \return True iff the jump target is a return address.
 */
bool isReturnAddress(const JumpTarget &target, const Dataflow &dataflow);

/**
 * \param[in] term      Valid pointer to a term.
 * \param[in] dataflow  Dataflow information.
 *
 * \return True iff the term contains a return address.
 */
bool isReturnAddress(const Term *term, const Dataflow &dataflow);

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
