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
namespace unused_detail {

template<class T>
inline void unused(const T &) {}

} // namespace unused_detail
} // namespace nc


/**
 * Macro for suppressing warnings about unused arguments, variables.
 * 
 * Rationale behind this macro is that we don't want to comment
 * unused arguments out because we want them to be found by doxygen.
 * Also, separating declaration and definition is a long way to go.
 *
 * \param x Unused argument or variable.
 */
#define NC_UNUSED(x) ::nc::unused_detail::unused(x)
