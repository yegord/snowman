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

#include "Escaping.h"

#include <nc/common/Foreach.h>

#include <QString>

namespace nc {

QString escapeDotString(const QString &string) {
    QString result;
    result.reserve(string.size());

    foreach (QChar c, string) {
        switch (c.toLatin1()) {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += "\\n";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

QString escapeCString(const QString &string) {
    QString result;
    result.reserve(string.size());

    foreach (QChar c, string) {
        switch (c.toLatin1()) {
            case '\\':
                result += "\\\\";
                break;
            case '\a':
                result += "\\a";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            case '\v':
                result += "\\v";
                break;
            case '"':
                result += "\\\"";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

} // namespace nc

/* vim:set et sts=4 sw=4: */
