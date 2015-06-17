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

#include <QString>
#include <QTextStream>

#include <nc/common/PrintCallback.h>

namespace nc {
namespace core {
namespace likec {

class TreeNode;

/**
 * Print context of tree node.
 */
class PrintContext {
    QTextStream &out_; ///< Output stream.
    PrintCallback<const TreeNode *> *callback_; ///< Print callback.
    int indentSize_; ///< Size of single indent.
    int indent_; ///< Current total indent.

    public:

    /**
     * Class constructor.
     *
     * \param[in] out Output stream.
     * \param[in] callback Pointer to print callback. Can be nullptr.
     */
    PrintContext(QTextStream &out, PrintCallback<const TreeNode *> *callback):
        out_(out), callback_(callback), indentSize_(4), indent_(0)
    {}

    /**
     * \return Output stream.
     */
    QTextStream &out() const { return out_; }

    /**
     * \return Pointer to print callback. Can be nullptr.
     */
    PrintCallback<const TreeNode *> *callback() const { return callback_; }

    /**
     * \return Size of single indent.
     */
    int indentSize() const { return indentSize_; }

    /**
     * Sets size of single indent.
     */
    void setIndentSize(int indentSize) { indentSize_ = indentSize; }

    /**
     * \return Current total indent.
     */
    int indent() const { return indent_; }

    /**
     * Sets current total indent.
     *
     * \param[in] indent New indent.
     */
    void setIndent(std::size_t indent);

    /**
     * Indent more.
     */
    void indentMore() { indent_ += indentSize_; }

    /**
     * Indent less.
     */
    void indentLess() { assert(indent_ >= indentSize_); indent_ -= indentSize_; }

    /**
     * Prints indent() number of spaces.
     *
     * \return out().
     */
    QTextStream &outIndent() { return out_ << QString(indent(), ' '); }
};

} // namespace likec
} // namespace core
} // namespace nc

/* vim:set et sts=4 sw=4: */
