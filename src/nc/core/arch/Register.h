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
#include <cassert>
#include <boost/noncopyable.hpp>
#include <QString>
#include <nc/core/ir/MemoryLocation.h>

namespace nc { namespace core { namespace arch {

/**
 * Register.
 * 
 * Registers are immutable.
 */
class Register: public boost::noncopyable {
public:
    /**
     * Constructor.
     *
     * \param[in] number               Register number.
     * \param[in] name                 Register name.
     * \param[in] memoryLocation       Corresponding abstract memory location of the register.
     */
    Register(int number, const QString &name, const ir::MemoryLocation &memoryLocation):
        number_(number),
        lowercaseName_(name.toLower()),
        uppercaseName_(name.toUpper()),
        memoryLocation_(memoryLocation)
    {
        assert(number >= 0);
    }
    
    /**
     * \returns                        Register number.
     */
    int number() const { return number_; }

    /**
     * \return                         Register lowercase name.
     */
    const QString &lowercaseName() const { return lowercaseName_; }

    /**
     * \return                         Register uppercase name.
     */
    const QString &uppercaseName() const { return uppercaseName_; }

    /**
     * \return                         Corresponding abstract memory location of the register.
     */
    const ir::MemoryLocation &memoryLocation() const { return memoryLocation_; }

    /**
     * \return Register size in bits.
     */
    SmallBitSize size() const { return memoryLocation().size<SmallBitSize>(); }

private:
    int number_; ///< Register number.
    QString lowercaseName_; ///< Lowercase register name.
    QString uppercaseName_; ///< Uppercase register name.
    ir::MemoryLocation memoryLocation_; ///< Corresponding abstract memory location of the register.
};

}}} // namespace nc::core::arch
