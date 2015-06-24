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

#include <stdexcept>

#include <boost/exception/all.hpp>

#include <QString>
#include <QByteArray>

namespace nc {

/** Error info structure for the error message.  */
typedef boost::error_info<struct MessageTag, QString> ExceptionMessage;

/**
 * Base exception class for all nocode exceptions.
 * 
 * Provides unicode error messages via unicodeWhat() function.
 */
class Exception: virtual public std::exception, virtual public boost::exception {
public:
    /**
     * Constructor.
     */
    Exception() {}

    /**
     * Constructor.
     *
     * \param message Exception message.
     */
    explicit
    Exception(const QString &message);

    /**
     * Virtual destructor.
     */
    virtual ~Exception() noexcept;

    /**
     * \return A Unicode string containing a generic description of the exception.
     */
    virtual QString unicodeWhat() const noexcept;

    /**
     * \return A null terminated string containing a generic description of the exception.
     */
    virtual const char *what() const noexcept override;

private:
    mutable QByteArray mCachedWhat;
};

} // namespace nc

/* vim:set et sts=4 sw=4: */
