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

namespace nc {

/**
 * Callback template class used to report events when something starts or ends printing.
 *
 * \tparam T Type of objects passed to the callback.
 */
template<class T>
class PrintCallback {
    public:

    /**
     * Callback function called when something starts being printed.
     *
     * \param[in] what What starts printing.
     */
    virtual void onStartPrinting(T what) = 0;

    /**
     * Callback function called when something ends being printed.
     *
     * \param[in] what What ends printing.
     */
    virtual void onEndPrinting(T what) = 0;

    /**
     * Virtual destructor.
     */
    virtual ~PrintCallback() {}
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
