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

#include <QModelIndex>
#include <QObject>

#include "Searcher.h"

QT_BEGIN_NAMESPACE
class QTreeView;
QT_END_NAMESPACE

namespace nc { namespace gui {

/**
 * Search controller for QTreeView.
 */
class TreeViewSearcher: public QObject, public Searcher {
    Q_OBJECT

    /** Controlled widget. */
    QTreeView *treeView_;

    /** Remembered current index. */
    QModelIndex currentIndex_;

    /** Remembered selection. */
    QModelIndexList selectedIndexes_;

    /** Remembered horizontal scrollbar position. */
    int hvalue_;

    /** Remembered vertical scrollbar position. */
    int vvalue_;

    public:

    /**
     * Constructor.
     *
     * \param treeView Valid pointer to the controlled widget.
     */
    explicit TreeViewSearcher(QTreeView *treeView);

    public Q_SLOTS:

    virtual void rememberViewport() override;
    virtual void restoreViewport() override;

    public:

    virtual void startTrackingViewport() override;
    virtual void stopTrackingViewport() override;

    virtual FindFlags supportedFlags() const override;
    virtual bool find(const QString &string, int flags) override;
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */

