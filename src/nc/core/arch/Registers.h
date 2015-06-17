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
#include <nc/common/Foreach.h>
#include <nc/core/ir/MemoryLocation.h>

#include <vector>

#include <QString>
#include <QHash>

#include "Register.h"

namespace nc { namespace core { namespace arch {

class Register;

/**
 * Register container.
 */
class Registers {
public:
    enum {
        /** Invalid register. */
        INVALID = -1
    };

    /**
     * Virtual destructor.
     */
    virtual ~Registers() {
        foreach(const core::arch::Register *regizter, mRegisters)
            delete regizter;
    }

    /**
     * \returns                        All registers.
     */
    const std::vector<const Register *> &registers() const {
        return mRegisters;
    }

    /**
     * \param number                   Register number.
     * \returns                        Register for the given number, or nullptr if
     *                                 no such register exists.
     */
    const Register *getRegister(int number) const {
        if(number < 0 || static_cast<std::size_t>(number) >= mRegisterByNumber.size())
            return nullptr;

        return mRegisterByNumber[number];
    }

    /**
     * \param location                 Register's memory location.
     * \returns                        Register for the given memory location,
     *                                 or nullptr if there is no such register.
     */
    const Register *getRegister(const ir::MemoryLocation &location) const {
        return mRegisterByLocation.value(location, nullptr);
    }

protected:
    /**
     * Registers the given register. This register container takes ownership of
     * the given register.
     * 
     * \param[in] reg   Valid pointer to a register.
     */
    void registerRegister(Register *reg) {
        assert(reg != nullptr);
        assert(getRegister(reg->number()) == nullptr); /* Re-registration not allowed. */

        mRegisters.push_back(reg);

        if (static_cast<std::size_t>(reg->number()) >= mRegisterByNumber.size()) {
            mRegisterByNumber.resize((reg->number() + 1) * 2);
        }
        mRegisterByNumber[reg->number()] = reg;

        mRegisterByLocation[reg->memoryLocation()] = reg;
    }

private:
    /** All registers. */
    std::vector<const Register *> mRegisters;

    /** Register number to register map. */
    std::vector<Register *> mRegisterByNumber;

    /** Map from memory location to register. */
    QHash<ir::MemoryLocation, Register *> mRegisterByLocation;
};


/**
 * Base class for static register containers.
 */
template<class Derived>
class StaticRegisters: public Registers {
public:
    static const std::vector<const Register *> &registers() {
        return instance()->Registers::registers();
    }

    static const Register *getRegister(int number) {
        return instance()->Registers::getRegister(number);
    }

    static const Register *getRegister(const ir::MemoryLocation &location) {
        return instance()->Registers::getRegister(location);
    }

    static Derived *instance() {
        return &sInstance;
    }

private:
    static Derived sInstance;
};

template<class Derived>
Derived StaticRegisters<Derived>::sInstance;

}}} // namespace nc::core::arch
