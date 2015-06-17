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
#include <functional>

#include <nc/common/Types.h>
#include <nc/common/Unreachable.h>

namespace nc {
namespace core {
namespace ir {

class Function;

namespace calling {

class CallAddress {
    ByteAddr value_;

public:
    explicit CallAddress(ByteAddr value): value_(value) {}

    ByteAddr value() const { return value_; }
};

class EntryAddress {
    ByteAddr value_;

public:
    explicit EntryAddress(ByteAddr value): value_(value) {}

    ByteAddr value() const { return value_; }
};

/**
 * An immutable class with value semantics used to identify functions
 * being called, even when their addresses are not known.
 */
class CalleeId {
public:
    /**
     * Kind of the id.
     */
    enum Kind {
        INVALID,    ///< Invalid, identifies nothing.
        ENTRY_ADDR, ///< Identifies a function by its entry address.
        CALL_ADDR,  ///< Identifies a function by the address of a call to it.
        FUNCTION    ///< Identifies a function by a pointer to it.
    };

private:
    union {
        ByteAddr entryAddress;     ///< Function's entry address.
        ByteAddr callAddress;      ///< Address of the call calling the function.
        const Function *function;  ///< Pointer to the function.
    } data_;

    Kind kind_; ///< Kind of this id.

public:
    /**
     * Constructs an invalid id.
     */
    CalleeId(): kind_(INVALID) {}

    /**
     * Constructs a valid id from a function's entry address.
     *
     * \param address Function's entry address.
     */
    CalleeId(EntryAddress address):
        kind_(ENTRY_ADDR)
    {
        data_.entryAddress = address.value();
    }

    /**
     * Constructs a valid id from a call address.
     *
     * \param address Function's entry address.
     */
    CalleeId(CallAddress address):
        kind_(CALL_ADDR)
    {
        data_.callAddress = address.value();
    }

    /**
     * Constructs a valid id from a function's pointer.
     *
     * \param function Pointer to a function's intermediate representation.
     */
    explicit CalleeId(const Function *function):
        kind_(FUNCTION)
    {
        assert(function != nullptr);
        data_.function = function;
    }

    /**
     * \return Kind of this id.
     */
    Kind kind() const { return kind_; }

    /**
     * \return Valid pointer to the address being called, if kind is ENTRY_ADDR, or nullptr otherwise.
     */
    const ByteAddr *entryAddress() const { return kind_ == ENTRY_ADDR ? &data_.entryAddress : nullptr; }

    /**
     * \return Valid pointer to the address of the call instruction, if kind is CALL_ADDR, or nullptr otherwise.
     */
    const ByteAddr *callAddress() const { return kind_ == CALL_ADDR ? &data_.callAddress : nullptr; }

    /**
     * \return Valid pointer to the function being called, if kind is FUNCTION, or nullptr otherwise.
     */
    const Function *function() const { return kind_ == FUNCTION ? data_.function : nullptr; }

    /**
     * \return True if this is equal to that, false otherwise.
     */
    bool operator==(const CalleeId &that) const {
        if (kind_ != that.kind_) {
            return false;
        }
        switch (kind_) {
            case ENTRY_ADDR:
                return data_.entryAddress == that.data_.entryAddress;
            case CALL_ADDR:
                return data_.callAddress == that.data_.callAddress;
            case FUNCTION:
                return data_.function == that.data_.function;
            default:
                unreachable();
        }
    }

    /**
     * \return True if this is not equal to that, false otherwise.
     */
    bool operator!=(const CalleeId &that) const {
        return !(*this == that);
    }

    /**
     * \return nullptr if this id is invalid, non-null pointer otherwise.
     */
    operator const void *() const { return kind_ == INVALID ? nullptr : this; }

    friend struct std::hash<CalleeId>;
};

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

namespace std {

/**
 * Specialization of std::hash for callee ids.
 *
 * This makes it possible to use callee ids as keys in hash maps.
 */
template<>
struct hash<nc::core::ir::calling::CalleeId>: public unary_function<nc::core::ir::calling::CalleeId, size_t> {
public:
    result_type operator()(const argument_type &value) const {
        using nc::core::ir::calling::CalleeId;

        switch (value.kind_) {
            case CalleeId::INVALID:
                return 0;
            case CalleeId::ENTRY_ADDR:
                return hash_value(value.data_.entryAddress);
            case CalleeId::CALL_ADDR:
                return hash_value(value.data_.callAddress);
            case CalleeId::FUNCTION:
                return hash_value(value.data_.function);
            default:
                unreachable();
        }
    }

protected:
    template<class T>
    result_type hash_value(const T &value) const {
        return hash<T>()(value);
    }
};

} // namespace std

namespace nc { namespace core { namespace ir { namespace calling {

/**
 * Qt hash function for memory locations.
 */
inline unsigned int qHash(const CalleeId &value) {
    return static_cast<unsigned int>(std::hash<CalleeId>()(value));
}

/**
 * Boost hash function for callee ids.
 */
inline std::size_t hash_value(const CalleeId &value) {
    return std::hash<CalleeId>()(value);
}

}}}} // namespace nc::core::ir::calling

/* vim:set et sts=4 sw=4: */
