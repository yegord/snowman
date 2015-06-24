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

#include <limits>
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_polymorphic.hpp>
#include <boost/type_traits/is_pointer.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/mpl/and.hpp>

namespace nc {

    /**
     * Performs a static numeric cast with range checking in debug build.
     * 
     * \param value                    Value to cast.
     * \returns                        Cast result.
     */
    template<class To, class From>
    typename boost::enable_if<
        boost::mpl::and_<
            boost::is_integral<To>,
            boost::is_integral<From>
        >,
        To
    >::type checked_cast(const From &value) {
        assert(static_cast<From>(static_cast<To>(value)) == value);

        return static_cast<To>(value);
    }

    /**
     * Performs a static cast that is checked in debug build.
     * 
     * \param source                   Pointer to cast.
     * \returns                        Cast result.
     */
    template<class To, class From>
    typename boost::enable_if<
        boost::is_polymorphic<From>,
        To
    >::type checked_cast(From *source) {
        static_assert(boost::is_pointer<To>::value, "Target type must be a pointer");
#ifndef NDEBUG
        To result = dynamic_cast<To>(source);
        assert(source == nullptr || result != nullptr);
        return result;
#else
        return static_cast<To>(source);
#endif // NDEBUG
    }

    /**
     * Performs a static cast that is checked in debug build.
     * 
     * \param source                   Value to cast.
     * \returns                        Cast result.
     */
    template<class To, class From>
    typename boost::enable_if<
        boost::is_polymorphic<From>,
        To
    >::type checked_cast(From &source) {
        static_assert(boost::is_reference<To>::value, "Target type must be a reference");
#ifndef NDEBUG
        return dynamic_cast<To>(source);
#else
        return static_cast<To>(source);
#endif // NDEBUG
    }

} // namespace nc

/* vim:set et sts=4 sw=4: */
