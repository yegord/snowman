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

namespace nc {
namespace core {
namespace likec {

/**
 * Class capable of storing.
 *
 * Use it as the second base class for the tree nodes that can be commented.
 */
class Commentable {
    QString comment_; ///< Comment.

public:
    /**
     * Constructor.
     *
     * \param[in] comment Comment.
     */
    explicit Commentable(const QString &comment = QString()): comment_(comment) {}

    /**
     * Virtual destructor.
     */
    virtual ~Commentable() {}

    /**
     * \return Comments.
     */
    const QString &comment() const { return comment_; }

    /**
     * Sets the comment.
     *
     * \param[in] comment New comment value.
     */
    void setComment(const QString &comment) { comment_ = comment; }
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
