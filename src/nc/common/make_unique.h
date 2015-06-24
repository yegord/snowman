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

/*
 * This file implements std::make_unique template function.
 *
 * For motivation, refer to http://herbsutter.com/gotw/_102/
 */

#include <nc/config.h>
#include <boost/config.hpp>
#include <memory>

namespace std {

#ifndef NC_HAVE_STD_MAKE_UNIQUE

#if defined(BOOST_HAS_VARIADIC_TMPL) && defined(BOOST_HAS_RVALUE_REFS) // have C++11 support

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args && ... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#else // no C++11 support

template<typename T>
unique_ptr<T> make_unique() {
    return std::unique_ptr<T>(new T());
}

template<typename T, typename Arg1>
unique_ptr<T> make_unique(Arg1 &&arg1) {
    return std::unique_ptr<T>(new T(std::forward<Arg1>(arg1)));
}

template<typename T, typename Arg1, typename Arg2>
unique_ptr<T> make_unique(Arg1 &&arg1, Arg2 &&arg2) {
    return std::unique_ptr<T>(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2)));
}

template<typename T, typename Arg1, typename Arg2, typename Arg3>
unique_ptr<T> make_unique(Arg1 &&arg1, Arg2 &&arg2, Arg3 &&arg3) {
    return std::unique_ptr<T>(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3)));
}

template<typename T, typename Arg1, typename Arg2, typename Arg3, typename Arg4>
unique_ptr<T> make_unique(Arg1 &&arg1, Arg2 &&arg2, Arg3 &&arg3, Arg4 &&arg4) {
    return std::unique_ptr<T>(new T(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Arg3>(arg3), std::forward<Arg4>(arg4)));
}

#endif

#endif /* !defined(NC_HAVE_STD_MAKE_UNIQUE) */

} // namespace std

/* vim:set et sts=4 sw=4: */
