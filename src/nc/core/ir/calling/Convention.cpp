/* The file is part of Snowman decompiler. */
/* See doc/licenses.asciidoc for the licensing information. */

//
// SmartDec decompiler - SmartDec is a native code to C/C++ decompiler
// Copyright (C) 2015 Alexander Chernov, Katerina Troshina, Yegor Derevenets,
// Alexander Fokin, Sergey Levin, Leonid Tsvetkov
//
// This file is part of SmartDec decompiler.
//
// SmartDec decompiler is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SmartDec decompiler is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SmartDec decompiler.  If not, see <http://www.gnu.org/licenses/>.
//

#include "Convention.h"

#include <nc/common/Foreach.h>
#include <nc/common/Range.h>
#include <nc/common/Unused.h>

#include <nc/core/ir/Statement.h>
#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace calling {

Convention::Convention(QString name):
    name_(std::move(name)),
    firstArgumentOffset_(0),
    argumentAlignment_(0),
    calleeCleanup_(false)
{}

Convention::~Convention() {}

namespace {

/**
 * Rounds given number down to the closest multiple of the multiple.
 */
template<class T>
T roundDown(T number, T multiple) {
    assert(multiple > 0);

    auto remainder = number % multiple;
    if (remainder == 0) {
        return number;
    }

    if (number >= 0) {
        assert(remainder > 0);
        return number - remainder;
    } else {
        assert(remainder < 0);
        return number - remainder - multiple;
    }
}

/**
 * Rounds given number up to the closest multiple of the multiple.
 */
template<class T>
T roundUp(T number, T multiple) {
    assert(multiple > 0);

    auto remainder = number % multiple;
    if (remainder == 0) {
        return number;
    }

    if (number >= 0) {
        assert(remainder > 0);
        return number - remainder + multiple;
    } else {
        assert(remainder < 0);
        return number - remainder;
    }
}

#ifndef NDEBUG
bool testRounding() {
    assert(roundDown(3, 4) == 0);
    assert(roundDown(5, 4) == 4);
    assert(roundDown(-3, 4) == -4);
    assert(roundDown(-5, 4) == -8);

    assert(roundUp(3, 4) == 4);
    assert(roundUp(5, 4) == 8);
    assert(roundUp(-3, 4) == 0);
    assert(roundUp(-5, 4) == -4);

    return true;
}

const bool roundingWorks = (NC_UNUSED(roundingWorks), testRounding());
#endif

} // anonymous namespace

MemoryLocation Convention::getArgumentLocationCovering(const MemoryLocation &memoryLocation) const {
    if (!memoryLocation) {
        return MemoryLocation();
    }

    /* Note: this assumes the stack growing down. */
    if (memoryLocation.domain() == MemoryDomain::STACK &&
        memoryLocation.addr() >= firstArgumentOffset()
    ) {
        /* Align the location properly. */
        if (argumentAlignment()) {
            auto addr = roundDown(memoryLocation.addr(), argumentAlignment());
            auto endAddr = roundUp(memoryLocation.endAddr(), argumentAlignment());
            return MemoryLocation(MemoryDomain::STACK, addr, endAddr - addr);
        } else {
            return memoryLocation;
        }
    }

    foreach (const auto &argumentLocations, argumentGroups()) {
        foreach (const auto &argumentLocation, argumentLocations) {
            if (argumentLocation.covers(memoryLocation)) {
                return argumentLocation;
            }
        }
    }

    return MemoryLocation();
}

std::vector<MemoryLocation> Convention::sortArguments(std::vector<MemoryLocation> arguments) const {
    assert(!nc::contains(arguments, MemoryLocation()));

    std::vector<MemoryLocation> result;
    result.reserve(arguments.size());

    /* Copy non-stack arguments in the order. */
    bool someGroupIsFilled = argumentGroups().empty();

    foreach (const auto &argumentLocations, argumentGroups()) {
        bool groupIsFilled = true;

        foreach (const auto &argumentLocation, argumentLocations) {
            bool argumentFound = false;

            foreach (MemoryLocation &memoryLocation, arguments) {
                if (argumentLocation.covers(memoryLocation)) {
                    result.push_back(memoryLocation);
                    memoryLocation = MemoryLocation();
                    argumentFound = true;
                }
            }

            if (!argumentFound) {
                groupIsFilled = false;
                break;
            }
        }

        someGroupIsFilled = someGroupIsFilled || groupIsFilled;
    }

    /* If some group was completely filled, copy the stack arguments. */
    if (someGroupIsFilled) {
        arguments.erase(
            std::remove_if(arguments.begin(), arguments.end(),
                [](const MemoryLocation &memoryLocation) {
                    return memoryLocation.domain() != MemoryDomain::STACK;
                }),
            arguments.end());

        if (!arguments.empty()) {
            std::sort(arguments.begin(), arguments.end());

            BitAddr nextArgumentOffset = firstArgumentOffset();

            foreach (const auto &memoryLocation, arguments) {
                if (nextArgumentOffset <= memoryLocation.addr() &&
                    memoryLocation.addr() < nextArgumentOffset + argumentAlignment())
                {
                    result.push_back(memoryLocation);
                    nextArgumentOffset = getArgumentLocationCovering(memoryLocation).endAddr();
                } else {
                    break;
                }
            }
        }
    }

    return result;
}

void Convention::addArgumentGroup(std::vector<MemoryLocation> memoryLocations) {
    argumentGroups_.push_back(std::move(memoryLocations));
}

MemoryLocation Convention::getReturnValueLocationCovering(const MemoryLocation &memoryLocation) const {
    foreach (const auto &returnValueLocation, returnValueLocations_) {
        if (returnValueLocation.covers(memoryLocation)) {
            return returnValueLocation;
        }
    }
    return MemoryLocation();
}

void Convention::addReturnValueLocation(const MemoryLocation &memoryLocation) {
    assert(memoryLocation);
    returnValueLocations_.push_back(memoryLocation);
}

void Convention::addEnterStatement(std::unique_ptr<Statement> statement) {
    assert(statement != nullptr);
    entryStatements_.push_back(std::move(statement));
}

} // namespace calling
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
