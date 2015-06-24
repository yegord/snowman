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
#include <nc/common/Exception.h>
#include <nc/common/Types.h>

#include <QCoreApplication>

namespace nc { namespace core { namespace input {

/** Error info structure for error line.  */
typedef boost::error_info<struct LineTag, int> ErrorLine;
/** Error info structure for error column.  */
typedef boost::error_info<struct ColumnTag, int> ErrorColumn;
/** Error info structure for error offset. */
typedef boost::error_info<struct PositionTag, ByteSize> ErrorOffset;

/**
 * Parse error.
 */
class ParseError: public Exception {
    Q_DECLARE_TR_FUNCTIONS(ParseError)
public:
    /**
     * Constructor.
     */
    ParseError() {}

    /**
     * Constructor.
     *
     * \param[in] message              Description of what has happened.
     */
    explicit
    ParseError(const QString &message): Exception(message) {}

    virtual QString unicodeWhat() const noexcept;
};

}}} // namespace nc::core::input

/* vim:set et sts=4 sw=4: */
