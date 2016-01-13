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

#include "CppSyntaxHighlighter.h"

#include <cassert>

#include <nc/common/Foreach.h>

namespace nc { namespace gui {

namespace {

/**
 * Array of all C++ keywords.
 */
const char *cppKeywords[] = {
    "asm",
    "auto",
    "bool", 
    "break", 
    "case", 
    "catch", 
    "char", 
    "class", 
    "const", 
    "const_cast", 
    "continue", 
    "default", 
    "delete", 
    "do", 
    "double", 
    "dynamic_cast",
    "else", 
    "enum", 
    "explicit", 
    "export", 
    "extern", 
    "false", 
    "float", 
    "for", 
    "friend", 
    "goto", 
    "if", 
    "inline",
    "int",
    "long", 
    "mutable", 
    "namespace", 
    "new", 
    "operator", 
    "private", 
    "protected", 
    "public", 
    "register", 
    "reinterpret_cast", 
    "return", 
    "short", 
    "signed", 
    "sizeof", 
    "static", 
    "static_cast", 
    "struct", 
    "switch", 
    "template", 
    "this", 
    "throw", 
    "true", 
    "try", 
    "typedef", 
    "typeid", 
    "typename", 
    "union", 
    "unsigned", 
    "using", 
    "virtual", 
    "void", 
    "volatile", 
    "wchar_t", 
    "while",
    "int8_t",
    "uint8_t",
    "int16_t",
    "uint16_t",
    "int32_t",
    "uint32_t",
    "int64_t",
    "uint64_t"
};

/* Highlighter state. */
enum State {
    IN_MACRO = 0x01,
    IN_MULTILINE_COMMENT = 0x02,
    IN_SINGLELINE_COMMENT = 0x04,
    IN_STRING = 0x08,
    IN_SINGLE_STRING = 0x10
};

} // namespace `anonymous-namespace`


CppSyntaxHighlighter::CppSyntaxHighlighter(QObject *parent): QSyntaxHighlighter(parent) {
    /* Init formats. */
    mFormats[TEXT].               setForeground(Qt::black);
    mFormats[SINGLE_LINE_COMMENT].setForeground(Qt::darkGreen);
    mFormats[MULTI_LINE_COMMENT]. setForeground(Qt::darkGreen);
    mFormats[KEYWORD].            setForeground(Qt::darkBlue);
    mFormats[KEYWORD].            setFontWeight(QFont::Bold);
    mFormats[OPERATOR].           setForeground(Qt::darkGray);
    mFormats[NUMBER].             setForeground(Qt::red);
    mFormats[MACRO].              setForeground(Qt::darkCyan);
    mFormats[STRING].             setForeground(Qt::blue);
    mFormats[ESCAPE_CHAR].        setForeground(Qt::darkBlue);
        
    /* Init keywords. */
    foreach(const char *cppKeyword, cppKeywords)
        mKeywords.insert(cppKeyword);

    /* Init regexps. */
    mNumberRegexp   = QRegExp("\\b([0-9]+|0[xX][0-9a-fA-F]+|0[0-7]+)(\\.[0-9]+)?([eE][0-9]+)?\\b");
    mOperatorRegexp = QRegExp("[\\(\\)\\[\\]{}\\:;,\\.!\\?/\\*\\-+<>%^&\\|=~]");
    mTextRegexp     = QRegExp("\\b[a-zA-Z_][a-zA-Z0-9_]+\\b");

    mIncludeRegexp  = QRegExp("^\\s*#\\s*include\\s*(<.+>|\\\".+\\\")");
    mMacroRegexp    = QRegExp("^\\s*#.*$"); 
    mMultilineMacroRegexp = QRegExp("^\\s*#\\s*(define|if|elif|pragma|warning|error)"); 
    mSpecialRegexp  = QRegExp("//|\\\"|'|/\\*");
}

CppSyntaxHighlighter::~CppSyntaxHighlighter() {
    return;
}

void CppSyntaxHighlighter::highlightBlock(const QString &text) {
    int startPos;
    int endPos;
    setCurrentBlockState(0);

    /* Handle state-specific highlighting. */
    if (processState(text, &startPos, &endPos))
        return;

    endPos = startPos;
        
    /* Handle preprocessor directives. */
    if (processPreprocessor(text))
        processRegexps(text, startPos);

    /* Highlight strings, comments... */
    startPos = endPos;
    while (true) {
        startPos = text.indexOf(mSpecialRegexp, startPos);
        if (startPos == -1)
            break;

        QString cap = mSpecialRegexp.cap();
        if (cap == "//") {
            setFormat(startPos, text.length() - startPos, itemFormat(SINGLE_LINE_COMMENT));
            if (text.endsWith("\\"))
                setCurrentBlockState(IN_SINGLELINE_COMMENT);
            return;
        } else if ((cap == "\"") || (cap == "'")) {
            endPos = findStringEnd(text, startPos + 1, cap.at(0));
                
            /* Handle unicode string. */
            if ((startPos > 0) && text.at(startPos - 1) == QChar('L'))
                --startPos;

            if (endPos == -1) {
                setFormat(startPos, text.length() - startPos, itemFormat(STRING));
                processEscapeChar(text, startPos, text.length() - startPos);
                setCurrentBlockState(cap.at(0) == QChar('"')? IN_STRING: IN_SINGLE_STRING);
                return;
            } else {
                endPos += 1;
                setFormat(startPos, endPos - startPos, itemFormat(STRING));
                processEscapeChar(text, startPos, endPos - startPos);
                startPos = endPos;
            }
        } else if (cap == "/*") {
            endPos = findMultilineCommentEnd(text, startPos + 2);
            if (endPos == -1) {
                setFormat(startPos, text.length() - startPos, itemFormat(MULTI_LINE_COMMENT));
                setCurrentBlockState(IN_MULTILINE_COMMENT);
                return;
            } else {
                endPos += 2;
                setFormat(startPos, endPos - startPos, itemFormat(MULTI_LINE_COMMENT));
                startPos = endPos;
            }
        }
    }
}

bool CppSyntaxHighlighter::processState(const QString &text, int *const startPos, int *const endPos) {
    int prevState = previousBlockState();
    *startPos = 0;
    *endPos = text.size();

    if (prevState == -1)
        prevState = 0;

    if ((prevState & IN_STRING) || (prevState & IN_SINGLE_STRING)) {
        *endPos = findStringEnd(text, *startPos, (prevState & IN_SINGLE_STRING)? '\'':'"');
        if (*endPos == -1) {
            setFormat(0, text.size(), itemFormat(STRING));
            setCurrentBlockState(previousBlockState());
            return true;
        } else {
            *endPos += 1; // "
            setFormat(0, *endPos - *startPos, itemFormat(STRING));
            *startPos = *endPos;
        }
    } else if (prevState & IN_MULTILINE_COMMENT) {
        *endPos = findMultilineCommentEnd(text, *startPos);
        if (*endPos == -1) {
            setFormat(0, text.length(), itemFormat(MULTI_LINE_COMMENT));
            setCurrentBlockState(previousBlockState());
            return true;
        } else {
            *endPos += 2; // */
            setFormat(0, *endPos - *startPos, itemFormat(MULTI_LINE_COMMENT));
            *startPos = *endPos;
        }
    } else if (prevState & IN_SINGLELINE_COMMENT) {
        setFormat(0, text.length(), itemFormat(SINGLE_LINE_COMMENT));
        if (text.endsWith("\\"))
            setCurrentBlockState(IN_SINGLELINE_COMMENT);
        return true;
    } else if (prevState & IN_MACRO) {
        setFormat(0, text.length(), itemFormat(MACRO));
        if (text.endsWith("\\"))
            setCurrentBlockState(IN_MACRO);
        /* Think:
         * 
         * #define macro \ *
         *               multiline comment 
         *               * / blablabla
         */
    }
    return false;
}

void CppSyntaxHighlighter::processRegexp(QRegExp &regexp, Item item, const QString &text, int startPos) {
    int index = 0;
    int start = startPos;

    while(true) {
        index = text.indexOf(regexp, start);
        if (index == -1)
            break;
        int length = regexp.matchedLength();
        assert(length != 0);

        start = index + length;
        QString cap = regexp.cap();

        setFormat(index, length, itemFormat(item));
        if (item == TEXT && mKeywords.contains(cap))
            setFormat(index, length, itemFormat(KEYWORD));
    }
}

void CppSyntaxHighlighter::processRegexps(const QString &text, int startPos) {
    processRegexp(mTextRegexp, TEXT, text, startPos);
    processRegexp(mOperatorRegexp, OPERATOR, text, startPos);
    processRegexp(mNumberRegexp, NUMBER, text, startPos);
}

void CppSyntaxHighlighter::processEscapeChar(const QString &text, int start, int len) {
    for (int pos = start; pos < start + len; pos++) {
        if (text.at(pos) == QChar('\\')) {
            int endPos = pos;
            for (int i = 1; i <= 3; i++) {
                if ((pos + i < text.length()) && 
                    (text.at(pos + i) >= QChar('0')) &&
                    (text.at(pos + i) <= QChar('7')) )
                    endPos = pos + i;
                else
                    break;
            }
            if (endPos == pos) {
                endPos = pos + 1;
                if ((endPos < start + len) && (text.at(endPos).toLower() == QChar('x'))) {
                    while (++endPos < start + len) {
                        QChar c = text.at(endPos).toLower();
                        if (c.isNumber() || ((c >= QChar('a')) && (c <= QChar('f'))))
                            continue;
                        else
                            break;
                    }
                    --endPos;
                }
            }
            setFormat(pos, endPos - pos + 1, itemFormat(ESCAPE_CHAR));
            pos = endPos;
        }
    }
}
        
bool CppSyntaxHighlighter::processPreprocessor(const QString &text) {
    if (text.indexOf(mMultilineMacroRegexp) != -1) {
        setFormat(0, text.length(), itemFormat(MACRO));
        /* TODO: This can't handle the following code:
         *  #define some this is / *
         *    blabla... * / a macro definition... */
        if (text.endsWith("\\"))
            setCurrentBlockState(IN_MACRO);
    } else if (text.indexOf(mIncludeRegexp) != -1) {
        /* TODO: we can highlight it in a different format */
        setFormat(0, text.length(), itemFormat(MACRO));
        int pos = mIncludeRegexp.pos(1);
        if (pos > 0)
            setFormat(pos, mIncludeRegexp.cap(1).size(), itemFormat(STRING));
    } else if (text.indexOf(mMacroRegexp) != -1) {
        setFormat(0, text.length(), itemFormat(MACRO));
        return false;
    }
    return true;
}

int CppSyntaxHighlighter::findStringEnd(const QString &text, int startPos, QChar strChar) {
    for (int pos = startPos; pos < text.length(); pos++) {
        if (text.at(pos) == QChar('\\'))
            pos++;
        else if (text.at(pos) == strChar)
            return pos;
    }
    return -1;
}

int CppSyntaxHighlighter::findMultilineCommentEnd(const QString &text, int startPos) {
    return text.indexOf("*/", startPos);
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
