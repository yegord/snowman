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

#include <QMainWindow>

#include <memory> /* std::shared_ptr */

#include <nc/common/Branding.h>
#include <nc/common/Types.h>
#include <nc/common/LogToken.h>

QT_BEGIN_NAMESPACE
class QAction;
class QLabel;
class QProgressBar;
class QProgressDialog;
class QSettings;
QT_END_NAMESPACE

namespace nc {

namespace core {
    class Context;
}

namespace gui {

class CxxView;
class DisassemblyDialog;
class InspectorView;
class InstructionsView;
class LogView;
class Project;
class SectionsView;
class SymbolsView;

/**
 * Main window of the decompiler.
 */
class MainWindow: public QMainWindow {
    Q_OBJECT

    Branding branding_;

    InstructionsView *instructionsView_; ///< Instructions view.
    CxxView *cxxView_; ///< C++ view.
    SectionsView *sectionsView_; ///< Sections view.
    SymbolsView *symbolsView_; ///< Symbols view.
    InspectorView *inspectorView_; ///< Inspector view.
    LogView *logView_; ///< Log window.
    DisassemblyDialog *disassemblyDialog_; ///< Disassembly dialog.
    QProgressDialog *progressDialog_; ///< Progress dialog.
    QLabel *statusLabel_; ///< Label in the status bar.
    QProgressBar *statusProgressBar_; ///< Progress bar in the status bar.

    QAction *openAction_; ///< Action for opening a file.
    QAction *exportCfgAction_; ///< Action for exporting CFG in DOT format.
    QAction *quitAction_; ///< Action for closing the main window.
    QAction *disassembleAction_; ///< Action for opening disassembly dialog.
    QAction *decompileAction_; ///< Action for starting decompilation.
    QAction *cancelAllAction_; ///< Action for cancelling all scheduled commands.
    QAction *decompileAutomaticallyAction_; ///< Action for toggling automatic decompilation.
    QAction *instructionsViewAction_; ///< Action for showing/hiding the instructions window.
    QAction *sectionsViewAction_; ///< Action for showing/hiding the sections window.
    QAction *symbolsViewAction_; ///< Action for showing/hiding the symbols window.
    QAction *inspectorViewAction_; ///< Action for showing/hiding the tree inspector.
    QAction *logViewAction_; ///< Action for showing/hiding the log window.
    QAction *aboutAction_; ///< Action for showing 'About Application' dialog.
    QAction *aboutQtAction_; ///< Action for showing 'About Qt' dialog.
    QAction *deleteSelectedInstructionsAction_; ///< Action for deleting selected instructions.
    QAction *decompileSelectedInstructionsAction_; ///< Action for decompiling selected instructions.

    QSettings *settings_; ///< Application settings.

    std::unique_ptr<Project> project_; ///< Current project.

    LogToken logToken_; ///< Log token.

public:
    /**
     * Constructor.
     *
     * \param[in] branding  Branding.
     * \param[in] parent    Pointer to the parent widget. Can be nullptr.
     */
    MainWindow(Branding branding, QWidget *parent = nullptr);

    /**
     * Destructor.
     */
    ~MainWindow();

    /**
     * \return Valid pointer to the C++ view.
     */
    CxxView *cxxView() const { return cxxView_; }

    /**
     * \return Valid pointer to the instructions view.
     */
    InstructionsView *instructionsView() const { return instructionsView_; }

    /**
     * \return Valid pointer to the action for opening a file.
     */
    QAction *openAction() const { return openAction_; }

    /**
     * \return Valid pointer to the action closing the window.
     */
    QAction *quitAction() const { return quitAction_; }

    /**
     * \return Valid pointer to the project. Can be nullptr.
     */
    Project *project() { return project_.get(); }

    /**
     * \return True if decompilation must me automatically schedules when
     *         the project is changed, false otherwise,
     */
    bool decompileAutomatically() const;

public Q_SLOTS:
    /**
     * Sets whether decompilation must be performed when a user changes the project.
     *
     * \param value True to decompile, false to not decompile.
     */
    void setDecompileAutomatically(bool value);

    /**
     * Opens a dialog for selecting files for decompilation, parses selected files, and starts decompiling them.
     */
    void open();

    /**
     * Parses given files and starts decompiling them.
     *
     * \param filenames Files to parse.
     */
    void open(const QStringList &filenames);

public: 
    /**
     * Opens a project.
     *
     * \param project Project instance to be used.
     */
    void open(std::unique_ptr<Project> project);

public Q_SLOTS:
    /**
     * Sets window title. And emits windowTitleChanged signal.
     *
     * \param title New title.
     */
    void setWindowTitle(const QString &title);

Q_SIGNALS:
    /**
     * This signal is emitted when the window changes its title.
     *
     * \param title New title.
     */
    void windowTitleChanged(const QString &title);

public Q_SLOTS:
    /**
     * Finds the instruction covering the given address and highlights it
     * in the instructions view.
     *
     * \param[in] address An address.
     *
     * \return True if such instruction was found, false otherwise.
     */
    bool jumpToAddress(ByteAddr address);

    /**
     * Sets the text in the status bar.
     *
     * \param text Text to show in the status bar.
     */
    void setStatusText(const QString &text = QString());

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    /**
     * Creates window widgets.
     */
    void createWidgets();

    /**
     * Creates window actions.
     */
    void createActions();

    /**
     * Creates window menu.
     */
    void createMenus();

    /**
     * Saves application settings.
     */
    void loadSettings();

    /**
     * Loads application settings.
     */
    void saveSettings();

private Q_SLOTS:
    /**
     * Disable or enable actions depending on current state.
     * Also, shows or hides progress dialog, depending on the command being currently executed.
     */
    void updateGuiState();

    /**
     * This slot handles the event of setting a new image.
     */
    void imageChanged();

    /**
     * This slot handles the changes in the set of instructions.
     */
    void instructionsChanged();

    /**
     * This slot handles the event of successful completion of decompilation.
     */
    void treeChanged();

    /**
     * Populates context menu of instructions view with actions.
     *
     * \param menu Valid pointer to the context menu.
     */
    void populateInstructionsContextMenu(QMenu *menu);

    /**
     * Populates context menu of C++ view with actions.
     *
     * \param menu Valid pointer to the context menu.
     */
    void populateCxxContextMenu(QMenu *menu);

    /**
     * Populates context menu of sections view with actions.
     *
     * \param menu Valid pointer to the context menu.
     */
    void populateSectionsContextMenu(QMenu *menu);

    /**
     * Populates context menu of symbols view with actions.
     *
     * \param menu Valid pointer to the context menu.
     */
    void populateSymbolsContextMenu(QMenu *menu);

    /**
     * Export CFG in DOT format.
     */
    void exportCfg();

    /**
     * Opens disassembly dialog.
     */
    void disassemble();

    /**
     * Decompiles the whole project.
     */
    void decompile();

    /**
     * Opens the disassembly dialog with the currently selected section chosen.
     */
    void disassembleSelectedSection();

    /**
     * Starts disassembling of the address range specified in DisassemblyDialog.
     */
    void disassembleSelectedSectionRange();

    /**
     * Deletes instructions selected in the instructions view.
     */
    void deleteSelectedInstructions();

    /**
     * Decompiles selected instructions.
     */
    void decompileSelectedInstructions();

    /**
     * Highlights code produced by selected assembler instructions in C++ view.
     */
    void highlightInstructionsInCxx();

    /**
     * Highlights instructions producing selected C++ code in instructions view.
     */
    void highlightCxxInInstructions();

    /**
     * Highlights selected C++ tree nodes in C++ view.
     */
    void highlightTreeInCxx();

    /**
     * Highlights assembler instructions selected in C++ inspector in instructions view.
     */
    void highlightTreeInInstructions();

    /**
     * Highlights selected C++ code in tree inspector.
     */
    void highlightCxxInTree();

    /**
     * Jumps to the address under cursor in C++ view.
     */
    void jumpToSelectedAddress();

    /**
     * Jumps to the address of the symbol under cursor in the symbols view.
     */
    void jumpToSymbolAddress();

    /**
     * Shows 'About Application' dialog.
     */
    void about();
};

}} // namespace nc::gui

/* vim:set et sts=4 sw=4: */
