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

#include <QString>

namespace nc { namespace gui {

/**
 * This is a base class for classes knowing how to perform
 * text search in various kinds of widgets. Instances of
 * its subclasses are given to SearchWidget constructors.
 */
class Searcher {
    public:

    /**
     * Find flags.
     */
    enum FindFlag {
        FindBackward        = 0x1,
        FindCaseSensitive   = 0x2,
        FindWholeWords      = 0x4,
        FindRegexp          = 0x8
    };

    /**
     * Bitmask of find flags.
     */
    typedef int FindFlags;

    /**
     * Virtual destructor.
     */
    virtual ~Searcher() {}

    /**
     * Remembers current viewport of the widget being searched in.
     */
    virtual void rememberViewport() = 0;

    /**
     * Restores the remembered viewport of the widget being searched in.
     */
    virtual void restoreViewport() = 0;

    /**
     * Start automatically remembering viewport when it is changed.
     */
    virtual void startTrackingViewport() = 0;

    /**
     * Stop automatically remembering viewport when it is changed.
     */
    virtual void stopTrackingViewport() = 0;

    /**
     * \return Bitmask of supported flags.
     */
    virtual FindFlags supportedFlags() const = 0;

    /**
     * Finds and highlights the next occurrence of given string.
     * Even if the string is not found, this function is not guaranteed
     * to preserve the viewport of the widget being search in.
     *
     * \param expression    Search expression.
     * \param flags         Search flags.
     *
     * \return True if the string was found, false otherwise.
     */
    virtual bool find(const QString &expression, FindFlags flags) = 0;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
