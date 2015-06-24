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

#include "ReachingDefinitions.h"

#include <algorithm>
#include <iterator>

#include <QTextStream>

#include <nc/common/Foreach.h>

#include <nc/core/ir/Term.h>

namespace nc {
namespace core {
namespace ir {
namespace dflow {

void ReachingDefinitions::addDefinition(const MemoryLocation &mloc, const Term *term) {
    assert(mloc);

    killDefinitions(mloc);
    
    auto i = std::lower_bound(chunks_.begin(), chunks_.end(), mloc,
        [](const Chunk &a, const MemoryLocation &b) -> bool {
            return a.location() < b;
        });

    chunks_.insert(i, Chunk(mloc, std::vector<const Term *>(1, term)));

    selfTest();
}

void ReachingDefinitions::killDefinitions(const MemoryLocation &mloc) {
    assert(mloc);

    if (chunks_.empty()) {
        return;
    }

    std::vector<Chunk> result;
    result.reserve(chunks_.size() + 1);

    foreach (auto &chunk, chunks_) {
        if (!mloc.overlaps(chunk.location())) {
            result.push_back(std::move(chunk));
        } else {
            if (chunk.location().addr() < mloc.addr()) {
                if (mloc.endAddr() < chunk.location().endAddr()) {
                    result.push_back(Chunk(
                        MemoryLocation(mloc.domain(), chunk.location().addr(), mloc.addr() - chunk.location().addr()),
                        chunk.definitions()));
                } else {
                    result.push_back(Chunk(
                        MemoryLocation(mloc.domain(), chunk.location().addr(), mloc.addr() - chunk.location().addr()),
                        std::move(chunk.definitions())));
                }
            }
            if (mloc.endAddr() < chunk.location().endAddr()) {
                result.push_back(Chunk(
                    MemoryLocation(mloc.domain(), mloc.endAddr(), chunk.location().endAddr() - mloc.endAddr()),
                    std::move(chunk.definitions())));
            }
        }
    }

    chunks_ = std::move(result);

    selfTest();
}

void ReachingDefinitions::project(const MemoryLocation &mloc, ReachingDefinitions &result) const {
    assert(mloc);

    result.clear();

    foreach (const auto &chunk, chunks_) {
        if (chunk.location().domain() == mloc.domain()) {
            auto addr = std::max(chunk.location().addr(), mloc.addr());
            auto endAddr = std::min(chunk.location().endAddr(), mloc.endAddr());

            if (addr < endAddr) {
                result.chunks_.push_back(Chunk(
                    MemoryLocation(mloc.domain(), addr, endAddr - addr),
                    chunk.definitions()));
            }
        }
    }

    result.selfTest();
}

std::vector<MemoryLocation> ReachingDefinitions::getDefinedMemoryLocationsWithin(Domain domain) const {
    std::vector<MemoryLocation> result;
    result.reserve(chunks_.size());

    foreach (const auto &chunk, chunks_) {
        if (chunk.location().domain() == domain) {
            result.push_back(chunk.location());
        }
    }

    return result;
}

void ReachingDefinitions::merge(const ReachingDefinitions &those) {
    selfTest();

    std::vector<Chunk> result;
    result.reserve(chunks_.size() + those.chunks_.size());

    auto i = chunks_.begin();
    auto iend = chunks_.end();

    auto j = those.chunks_.begin();
    auto jend = those.chunks_.end();

    while (i != iend || j != jend) {
        auto a = i != iend ? i->location() : MemoryLocation();
        auto b = j != jend ? j->location() : MemoryLocation();

        if (!result.empty()) {
            const auto &c = result.back().location();
            if (c.domain() == a.domain() && c.endAddr() > a.addr()) {
                a = MemoryLocation(a.domain(), c.endAddr(), a.endAddr() - c.endAddr());
            }
            if (c.domain() == b.domain() && c.endAddr() > b.addr()) {
                b = MemoryLocation(b.domain(), c.endAddr(), b.endAddr() - c.endAddr());
            }
        }

        if (!b) {
            result.push_back(Chunk(a, i->definitions()));
            ++i;
        } else if (!a) {
            result.push_back(Chunk(b, j->definitions()));
            ++j;
        } else if (a.domain() < b.domain()) {
            result.push_back(Chunk(a, i->definitions()));
            ++i;
        } else if (b.domain() < a.domain()) {
            result.push_back(Chunk(b, j->definitions()));
            ++j;
        } else if (a.endAddr() <= b.addr()) {
            result.push_back(Chunk(a, i->definitions()));
            ++i;
        } else if (b.endAddr() <= a.addr()) {
            result.push_back(Chunk(b, j->definitions()));
            ++j;
        } else if (a.addr() < b.addr()) {
            result.push_back(Chunk(MemoryLocation(a.domain(), a.addr(), b.addr() - a.addr()), i->definitions()));
        } else if (b.addr() < a.addr()) {
            result.push_back(Chunk(MemoryLocation(b.domain(), b.addr(), a.addr() - b.addr()), j->definitions()));
        } else {
            std::vector<const Term *> merged;
            merged.reserve(i->definitions().size() + j->definitions().size());
            std::set_union(i->definitions().begin(), i->definitions().end(), j->definitions().begin(), j->definitions().end(), std::back_inserter(merged));

            if (a.size() < b.size()) {
                result.push_back(Chunk(a, std::move(merged)));
                ++i;
            } else if (b.size() < a.size()) {
                result.push_back(Chunk(b, std::move(merged)));
                ++j;
            } else {
                result.push_back(Chunk(a, std::move(merged)));
                ++i;
                ++j;
            }
        }
    }

    chunks_ = std::move(result);

    selfTest();
}

void ReachingDefinitions::print(QTextStream &out) const {
    out << '{';
    foreach (const auto &chunk, chunks_) {
        out << chunk.location() << ':';
        foreach (const Term *term, chunk.definitions()) {
            out << ' ' << *term;
        }
        out << ';';
    }
    out << '}';
}

} // namespace dflow
} // namespace ir
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
