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

#include "NavigationHelper.h"

#include <QAction>
#include <QMenu>
#include <QPlainTextEdit>
#include <QTreeView>

#include <nc/common/Foreach.h>

#include <nc/core/arch/Instruction.h>

#include <nc/gui/InstructionsView.h>
#include <nc/gui/CxxView.h>
#include <nc/gui/MainWindow.h>

#include "IdaFrontend.h"

namespace nc { namespace ida {

NavigationHelper::NavigationHelper(gui::MainWindow *mainWindow):
    QObject(mainWindow), mainWindow_(mainWindow)
{
    jumpFromInstructionsViewAction_ = new QAction(tr("Show in IDA"), this);
    jumpFromInstructionsViewAction_->setShortcut(Qt::CTRL + Qt::Key_Backspace);
    jumpFromInstructionsViewAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(jumpFromInstructionsViewAction_, SIGNAL(triggered()), this, SLOT(jumpFromInstructionsView()));
    mainWindow_->instructionsView()->treeView()->addAction(jumpFromInstructionsViewAction_);

    jumpFromCxxViewAction_ = new QAction(tr("Show in IDA"), this);
    jumpFromCxxViewAction_->setShortcut(Qt::CTRL + Qt::Key_Backspace);
    jumpFromCxxViewAction_->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(jumpFromCxxViewAction_, SIGNAL(triggered()), this, SLOT(jumpFromCxxView()));
    mainWindow_->cxxView()->textEdit()->addAction(jumpFromCxxViewAction_);

    connect(mainWindow->instructionsView(), SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateInstructionsContextMenu(QMenu *)));
    connect(mainWindow->cxxView(), SIGNAL(contextMenuCreated(QMenu *)), this, SLOT(populateCxxContextMenu(QMenu *)));
}

void NavigationHelper::populateInstructionsContextMenu(QMenu *menu) {
    if (!mainWindow_->instructionsView()->selectedInstructions().empty()) {
        menu->addSeparator();
        menu->addAction(jumpFromInstructionsViewAction_);
    }
}

void NavigationHelper::populateCxxContextMenu(QMenu *menu) {
    if (!mainWindow_->cxxView()->selectedInstructions().empty()) {
        menu->addSeparator();
        menu->addAction(jumpFromCxxViewAction_);
    }
}

void NavigationHelper::jumpFromInstructionsView() {
    foreach (auto instruction, mainWindow_->instructionsView()->selectedInstructions()) {
        if (IdaFrontend::jumpToAddress(instruction->addr())) {
            break;
        }
    }
}

void NavigationHelper::jumpFromCxxView() {
    foreach (auto instruction, mainWindow_->cxxView()->selectedInstructions()) {
        if (IdaFrontend::jumpToAddress(instruction->addr())) {
            break;
        }
    }
}

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
