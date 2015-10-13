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

#include "Declaration.h"

namespace nc {
namespace core {
namespace likec {

class Type;

/**
 * Declaration of a struct or union member.
 */
class MemberDeclaration: public Declaration {
    const Type *type_; ///< Type.

public:
    /**
     * Class constructor.
     *
     * \param[in] identifier Name of this field.
     * \param[in] type Valid pointer to the type of this variable.
     */
    MemberDeclaration(const QString &identifier, const Type *type):
        Declaration(MEMBER_DECLARATION, identifier), type_(type)
    {
        assert(type);
    }

    /**
     * \return Valid pointer to the type of this variable.
     */
    const Type *type() const { return type_; }
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Declaration, nc::core::likec::MemberDeclaration, nc::core::likec::Declaration::MEMBER_DECLARATION)

/* vim:set et sts=4 sw=4: */
