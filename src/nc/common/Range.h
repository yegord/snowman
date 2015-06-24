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

#include <algorithm>

#include <boost/optional.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator.hpp>
#include <boost/range/value_type.hpp>

namespace nc {
    namespace range_detail {
        template<class T>
        struct Void {
            typedef void type;
        };

        template<class Range, class T, class Enable = void>
        struct Find {
            typename boost::range_iterator<const Range>::type operator()(const Range &range, const T &value) {
                return std::find(boost::begin(range), boost::end(range), value);
            }
        };

        template<class AssociativeContainer, class T>
        struct Find<AssociativeContainer, T, typename Void<typename AssociativeContainer::key_type>::type> {
            typename boost::range_iterator<const AssociativeContainer>::type operator()(const AssociativeContainer &container, const T &value) {
                return container.find(value);
            }
        };

        template<class Range, class T>
        typename boost::range_iterator<const Range>::type find(const Range &range, const T &value) {
            return Find<Range, T>()(range, value);
        }

    } // namespace range_detail

    /**
     * This function performs a containment check on a given range. It uses
     * find method of the given range if it's available (as it is in case of
     * associative containers), and std::find algorithm if it's not.
     *
     * \param range Range.
     * \param value Value.
     *
     * \return True of the given range contains given element, false otherwise.
     */
    template<class Range, class T>
    bool contains(const Range &range, const T &value) {
        return range_detail::find(range, value) != boost::end(range);
    }

    /**
     * Looks for the given key in a given associative range.
     *
     * \param range Range.
     * \param key Key.
     * \param defaultValue Value to return when the range does not have an element with a given key.
     *
     * \return Const reference to corresponding value from the range if the key was found,
     *         const reference to the default value otherwise.
     */
    template<class AssociativeRange>
    const typename boost::range_value<AssociativeRange>::type::second_type &
    find(const AssociativeRange &range, const typename boost::range_value<AssociativeRange>::type::first_type &key, const typename boost::range_value<AssociativeRange>::type::second_type &defaultValue) {
        auto pos = range_detail::find(range, key);

        return pos == boost::end(range) ? defaultValue : pos->second;
    }

    /**
     * Looks for the given key in a given associative range.
     *
     * \param range Range.
     * \param key Key.
     *
     * \return Const reference to corresponding value from the range if the key was found,
     *         const reference to the default constructed range value otherwise.
     */
    template<class AssociativeRange>
    const typename boost::range_value<AssociativeRange>::type::second_type &
    find(const AssociativeRange &range, const typename boost::range_value<AssociativeRange>::type::first_type &key) {
        static const auto defaultValue = typename boost::range_value<AssociativeRange>::type::second_type();

        return find(range, key, defaultValue);
    }

    /**
     * Looks for the given key in a given associative range.
     *
     * \param range Range.
     * \param key Key.
     *
     * \return Corresponding value from the range if the key was found, boost::none otherwise.
     */
    template<class AssociativeRange>
    boost::optional<typename boost::range_value<AssociativeRange>::type::second_type>
    find_optional(const AssociativeRange &range, const typename boost::range_value<AssociativeRange>::type::first_type &key) {
        auto pos = range_detail::find(range, key);

        if (pos != boost::end(range)) {
            return pos->second;
        } else {
            return boost::none;
        }
    }

} // namespace nc

/* vim:set et sts=4 sw=4: */
