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

#include <cassert>

namespace nc {

/**
 * Immutable class for representing ranges.
 *
 * A range is represented by its start and end.
 * The start belongs to the range, the end does not.
 *
 * \tparam T Type of range boundaries.
 */
template<class T>
class Range {
    T start_; ///< Start of the range.
    T end_; ///< End of the range.

public:
    /**
     * Constructor of an invalid range.
     */
    Range(): start_(0), end_(-1) {}

    /**
     * Constructor of a valid range.
     *
     * \param start     Start of the range.
     * \param end       End of the range.
     */
    Range(T start, T end):
        start_(start), end_(end)
    {
        assert(start <= end);
    }

    /**
     * \return Start of the range --- the first position in the range.
     */
    T start() const { return start_; }

    /**
     * \return End of the range --- the first position not in the range.
     */
    T end() const { return end_; }

    /**
     * \return end() - start()
     */
    T length() const { return end() - start(); }

    /**
     * \return start() + length() / 2
     */
    T center() const { return start() + length() / 2; }

    /**
     * \param position Position.
     *
     * \return True if the range contains given position, false otherwise.
     */
    bool contains(T position) const { return start() <= position && position < end(); }

    /**
     * \return True iff *this and that are disjoint ranges.
     */
    bool disjoint(const Range<T> &that) const {
        return this->end() <= that.start() || that.end() <= this->start();
    }

    /**
     * \return True iff *this and that are overlapping ranges.
     */
    bool overlaps(const Range<T> &that) const {
        return !disjoint(that);
    }

    /**
     * \return This range shifted by the given offset.
     */
    Range<T> shifted(T offset) const {
        return Range<T>(start() + offset, end() + offset);
    }

    /**
     * \return Non-zero pointer if and only if the range is valid.
     */
    operator const void *() const { return start() <= end() ? this : 0; }
};

template<class T>
inline bool operator==(const Range<T> &a, const Range<T> &b) {
    return a.start() == b.start() && a.end() == b.end();
}

template<class T>
inline bool operator!=(const Range<T> &a, const Range<T> &b) {
    return !(a == b);
}

template<class T>
inline bool operator<(const Range<T> &a, const Range<T> &b) {
    return a.start() < b.start() || (a.start() == b.start() && a.end() < b.end());
}

template<class T>
Range<T> make_range(T start, T end) {
    return Range<T>(start, end);
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
