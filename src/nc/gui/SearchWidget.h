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

#include <memory>

#include <QTextDocument>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QAction;
class QLineEdit;
class QStringListModel;
QT_END_NAMESPACE

namespace nc { namespace gui {

class Searcher;

/**
 * Widget providing a text search functionality.
 */
class SearchWidget: public QWidget {
    Q_OBJECT

    public:

    /**
     * Constructor.
     *
     * \param searcher  Valid pointer to the searcher.
     * \param parent    Pointer to the parent widget. Can be nullptr.
     */
    explicit SearchWidget(std::unique_ptr<Searcher> searcher, QWidget *parent = nullptr);

    /**
     * Destructor.
     */
    ~SearchWidget();

    /**
     * \return Valid pointer to the searcher.
     */
    Searcher *searcher() const { return searcher_.get(); }

    public Q_SLOTS:

    /**
     * Shows the widget and sets input focus into it.
     */
    void activate();

    /**
     * Hides the widget.
     */
    void deactivate();

    /**
     * Finds the next occurrence of the entered search string.
     */
    void findNext();

    /**
     * Finds the previous occurrence of the entered search string.
     */
    void findPrevious();

    private Q_SLOTS:

    /**
     * Schedules incremental search.
     */
    void scheduleIncrementalSearch();

    /**
     * Finds the first occurrence of the search string.
     */
    void performIncrementalSearch();

    /**
     * Remembers currently entered string for completion.
     */
    void rememberCompletion();

    private:

    /** Associated searcher. */
    std::unique_ptr<Searcher> searcher_;

    /** Input for entering a search string. */
    QLineEdit *lineEdit_;

    /** Completion model for the search string. */
    QStringListModel *completionModel_;

    /** Action for toggling incremental search. */
    QAction *incrementalSearchAction_;

    /** Action for choosing case sensitivity mode. */
    QAction *caseSensitiveAction_;

    /** Action for choosing whole words search mode. */
    QAction *wholeWordsAction_;

    /** Action for choosing regexp search mode. */
    QAction *regexpAction_;

    /** Palette of the line edit in a normal state. */
    QPalette normalPalette_;

    /** Palette of the line edit to let user know that the string was not found. */
    QPalette failurePalette_;

    /** Timer for implementing delayed incremental search. */
    QTimer *incrementalSearchTimer_;

    /**
     * \return Searcher encoding of search flags selected by the user.
     */
    int searchFlags() const;

    /**
     * Indicates that the search has succeeded.
     */
    void indicateSuccess();

    /**
     * Indicates that the search has failed.
     */
    void indicateFailure();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
