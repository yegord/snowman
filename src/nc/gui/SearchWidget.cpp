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

#include "SearchWidget.h"

#include <cassert>

#include <QAction>
#include <QCompleter>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QPalette>
#include <QPushButton>
#include <QStringListModel>
#include <QTimer>

#include "Searcher.h"

namespace nc { namespace gui {

SearchWidget::SearchWidget(std::unique_ptr<Searcher> searcher, QWidget *parent):
    QWidget(parent), searcher_(std::move(searcher))
{
    assert(searcher_ != nullptr);

    auto supportedFlags = searcher_->supportedFlags();

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(4, 0, 4, 4);

    QLabel *findLabel = new QLabel(tr("Find:"), this);
    layout->addWidget(findLabel);

    completionModel_ = new QStringListModel(this);

    lineEdit_ = new QLineEdit(this);
    lineEdit_->setCompleter(new QCompleter(completionModel_, this));
    lineEdit_->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    lineEdit_->setMinimumWidth(lineEdit_->fontMetrics().boundingRect("X").width() * 8);
    layout->addWidget(lineEdit_);

    connect(lineEdit_, SIGNAL(textChanged(const QString &)), this, SLOT(scheduleIncrementalSearch()));
    connect(lineEdit_, SIGNAL(returnPressed()), this, SLOT(findNext()));
    connect(lineEdit_, SIGNAL(returnPressed()), this, SLOT(rememberCompletion()));

    QPushButton *nextButton = new QPushButton(tr("&Next"), this);
    layout->addWidget(nextButton);

    connect(nextButton, SIGNAL(clicked()), this, SLOT(findNext()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(rememberCompletion()));

    if (supportedFlags & Searcher::FindBackward) {
        QPushButton *previousButton = new QPushButton(tr("&Previous"), this);
        layout->addWidget(previousButton);

        connect(previousButton, SIGNAL(clicked()), this, SLOT(findPrevious()));
        connect(previousButton, SIGNAL(clicked()), this, SLOT(rememberCompletion()));
    }

    incrementalSearchAction_ = new QAction(tr("&Incremental Search"), this);
    incrementalSearchAction_->setCheckable(true);
    incrementalSearchAction_->setChecked(true);

    caseSensitiveAction_ = new QAction(tr("&Case Sensitive"), this);
    caseSensitiveAction_->setCheckable(true);

    wholeWordsAction_ = new QAction(tr("&Whole Words"), this);
    wholeWordsAction_->setCheckable(true);

    regexpAction_ = new QAction(tr("&Regular Expression"), this);
    regexpAction_->setCheckable(true);

    QMenu *optionsMenu = new QMenu(this);
    optionsMenu->addAction(incrementalSearchAction_);

    if (supportedFlags & Searcher::FindCaseSensitive) {
        optionsMenu->addAction(caseSensitiveAction_);
    }
    if (supportedFlags & Searcher::FindWholeWords) {
        optionsMenu->addAction(wholeWordsAction_);
    }
    if (supportedFlags & Searcher::FindRegexp) {
        regexpAction_->setChecked(true);
        optionsMenu->addAction(regexpAction_);
    }

    QPushButton *optionsButton = new QPushButton(tr("&Options"), this);
    optionsButton->setMenu(optionsMenu);
    layout->addWidget(optionsButton);

    normalPalette_  = lineEdit_->palette();
    failurePalette_ = lineEdit_->palette();
    failurePalette_.setColor(QPalette::Base, QColor(255, 192, 192));

    incrementalSearchTimer_ = new QTimer(this);
    incrementalSearchTimer_->setInterval(250);
    incrementalSearchTimer_->setSingleShot(true);

    connect(incrementalSearchTimer_, SIGNAL(timeout()), this, SLOT(performIncrementalSearch()));
}

SearchWidget::~SearchWidget() {}

void SearchWidget::activate() {
    show();

    lineEdit_->selectAll();
    lineEdit_->setFocus();

    searcher()->startTrackingViewport();
    searcher()->rememberViewport();
}

void SearchWidget::deactivate() {
    if (!isVisible()) {
        return;
    }

    searcher()->stopTrackingViewport();
    searcher()->restoreViewport();

    incrementalSearchTimer_->stop();

    hide();
}

void SearchWidget::findNext() {
    searcher()->stopTrackingViewport();
    searcher()->restoreViewport();

    if (searcher()->find(lineEdit_->text(), searchFlags())) {
        searcher()->rememberViewport();
        indicateSuccess();
    } else {
        searcher()->restoreViewport();
        indicateFailure();
    }

    searcher()->startTrackingViewport();
}

void SearchWidget::findPrevious() {
    searcher()->stopTrackingViewport();
    searcher()->restoreViewport();

    if (searcher()->find(lineEdit_->text(), searchFlags() | Searcher::FindBackward)) {
        searcher()->rememberViewport();
        indicateSuccess();
    } else {
        searcher()->restoreViewport();
        indicateFailure();
    }

    searcher()->startTrackingViewport();
}

void SearchWidget::scheduleIncrementalSearch() {
    if (!incrementalSearchAction_->isChecked()) {
        indicateSuccess();
        return;
    }

    incrementalSearchTimer_->start();
}

void SearchWidget::performIncrementalSearch() {
    searcher()->stopTrackingViewport();
    searcher()->restoreViewport();

    if (searcher()->find(lineEdit_->text(), searchFlags())) {
        indicateSuccess();
    } else {
        searcher()->restoreViewport();
        indicateFailure();
    }

    searcher()->startTrackingViewport();
}

int SearchWidget::searchFlags() const {
    return (caseSensitiveAction_->isChecked() ? Searcher::FindCaseSensitive : 0) |
           (wholeWordsAction_->isChecked() ? Searcher::FindWholeWords : 0) |
           (regexpAction_->isChecked() ? Searcher::FindRegexp : 0);
}

void SearchWidget::indicateSuccess() {
    lineEdit_->setPalette(normalPalette_);
}

void SearchWidget::indicateFailure() {
    lineEdit_->setPalette(failurePalette_);
}

void SearchWidget::rememberCompletion() {
    QStringList list = completionModel_->stringList();
    if (!list.contains(lineEdit_->text(), lineEdit_->completer()->caseSensitivity())) {
        list.append(lineEdit_->text());
        completionModel_->setStringList(list);
    }
}

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
