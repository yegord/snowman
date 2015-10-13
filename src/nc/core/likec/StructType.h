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

#include <memory> /* unique_ptr */
#include <vector>

#include "MemberDeclaration.h"
#include "Type.h"

namespace nc {
namespace core {
namespace likec {

class StructTypeDeclaration;

/**
 * Structural type.
 */
class StructType: public Type {
    std::vector<std::unique_ptr<MemberDeclaration>> members_; ///< Members of struct.
    const StructTypeDeclaration *typeDeclaration_; ///< Type declaration.

public:
    /**
     * Class constructor.
     *
     * \param[in] typeDeclaration Structural type declaration.
     */
    StructType(const StructTypeDeclaration *typeDeclaration): Type(0, STRUCT_TYPE), typeDeclaration_(typeDeclaration) {}

    /**
     * \return Members of struct.
     */
    const std::vector<const MemberDeclaration *> &members() const {
        return reinterpret_cast<const std::vector<const MemberDeclaration *> &>(members_);
    }

    /**
     * \return Type declaration.
     */
    const StructTypeDeclaration *typeDeclaration() const { return typeDeclaration_; }

    /**
     * Adds member to the structure.
     *
     * \param[in] memberDeclaration Declaration of member.
     */
    void addMember(std::unique_ptr<MemberDeclaration> memberDeclaration);

    /**
     * \return Declaration of member starting at given bit offset in the struct.
     *
     * \param[in] offset Bit offset of member.
     */
    const MemberDeclaration *getMember(BitSize offset) const;

    bool isStructure() const override { return true; }
    void print(QTextStream &out) const override;
};

} // namespace likec
} // namespace core
} // namespace nc

NC_SUBCLASS(nc::core::likec::Type, nc::core::likec::StructType, nc::core::likec::Type::STRUCT_TYPE)

/* vim:set et sts=4 sw=4: */
