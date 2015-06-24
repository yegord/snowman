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

#include <boost/array.hpp>

#include <QSyntaxHighlighter>
#include <QSet>

QT_BEGIN_NAMESPACE
class QTextDocument;
QT_END_NAMESPACE

namespace nc { namespace gui {
/**
 * Syntax highlighter for C++.
 */
class CppSyntaxHighlighter: public QSyntaxHighlighter {
public:
    enum Item {
        /** Normal text. */
        TEXT, 

        /* Following items can be matched with simple regexps. */
        SINGLE_LINE_COMMENT,
        KEYWORD, 
        OPERATOR,
        NUMBER,
        ESCAPE_CHAR,

        /* Following items need states on lines. */
        MACRO, 
        MULTI_LINE_COMMENT, 
        STRING,

        ITEM_COUNT
    };

    /**
     * Constructor.
     * 
     * \param[in] parent Pointer to the parent object. Can be nullptr.
     */
    CppSyntaxHighlighter(QObject *parent = nullptr);

    /**
     * Virtual destructor.
     */
    virtual ~CppSyntaxHighlighter();

    /**
     * \param[in] item                 Item to get format for.
     * \returns                        Format for the given item.
     */
    QTextCharFormat itemFormat(Item item) {
        return mFormats[item];
    }

    /**
     * \param[in] item                 Item to set format for.
     * \param[in] format               New format for the given item.
     */
    void setItemFormat(Item item, const QTextCharFormat &format) {
        mFormats[item] = format;
    }

protected:
    virtual void highlightBlock(const QString &text) override;

private:
    bool processState(const QString &text, int *startPos, int *endPos);

    void processRegexp(QRegExp &regexp,  Item item, const QString &text, int startPos = 0);

    void processRegexps(const QString &text, int startPos = 0);

    bool processPreprocessor(const QString &text);

    void processEscapeChar(const QString &text, int start = 0, int len = 0);

    int findStringEnd(const QString &text, int startPos = 0, QChar strChar = '"');

    int findMultilineCommentEnd(const QString &text, int startPos = 0);

    /** Keywords. */ 
    QSet<QString> mKeywords;

    /* Regular expressions. */
    QRegExp mIncludeRegexp;
    QRegExp mMacroRegexp;
    QRegExp mMultilineMacroRegexp;
    QRegExp mSpecialRegexp;
    QRegExp mNumberRegexp;
    QRegExp mOperatorRegexp;
    QRegExp mTextRegexp;

    /** Text formats. */
    boost::array<QTextCharFormat, ITEM_COUNT> mFormats;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
