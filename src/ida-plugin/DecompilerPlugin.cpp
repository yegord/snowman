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

#include "DecompilerPlugin.h"

#include <QApplication>
#include <QAction>
#include <QMessageBox>
#include <QStackedLayout>

#include <nc/common/Branding.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>
#include <nc/core/Context.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/image/Image.h>
#include <nc/core/image/Relocation.h>
#include <nc/gui/MainWindow.h>
#include <nc/gui/Project.h>

#include "IdaDemangler.h"
#include "IdaFrontend.h"
#include "NavigationHelper.h"

namespace nc { namespace ida {

namespace {

const char *decompileFunctionHotkey = "F3";
const char *decompileProgramHotkey  = "Ctrl-F3";

const char *subviewsMenuPath = "View/Open subviews";
const char *hexDumpMenuItem = "Hex dump";
const char *decompileFunctionMenuItem = "Decompile a function";
const char *decompileProgramMenuItem  = "Decompile a program";

DecompilerPlugin *globalPluginInstance = NULL;

} // anonymous namespace

extern "C" {
    static bool decompileFunctionCallback() {
        globalPluginInstance->decompileFunction();
        return true;
    }

    static bool decompileAllCallback() {
        globalPluginInstance->decompileProgram();
        return true;
    }
}

DecompilerPlugin::DecompilerPlugin():
    programWindow_(NULL)
{
    branding_ = nc::branding();
    branding_.setApplicationName("Snowman");

    if (QApplication::instance() == NULL) {
        static int argc = 1;
        static char *argv[] = {const_cast<char *>("dummy.exe")};

        mApplication.reset(new QApplication(argc, argv));
    }

    globalPluginInstance = this;

    IdaFrontend::addMenuItem(
        tr("%1/%2").arg(subviewsMenuPath).arg(hexDumpMenuItem),
        tr(decompileProgramMenuItem),
        decompileProgramHotkey,
        &decompileAllCallback);
    IdaFrontend::addMenuItem(
        tr("%1/%2").arg(subviewsMenuPath).arg(hexDumpMenuItem),
        tr(decompileFunctionMenuItem),
        decompileFunctionHotkey,
        &decompileFunctionCallback);

    IdaFrontend::print(tr(
        "%1 plugin %2 loaded.\n"
        "  Press %3 to decompile the function under cursor, %4 to decompile the whole program.\n"
        "  Press %3 (%4) again to jump to the address under cursor.\n")
        .arg(branding_.applicationName())
        .arg(branding_.applicationVersion())
        .arg(decompileFunctionHotkey)
        .arg(decompileProgramHotkey));
}

DecompilerPlugin::~DecompilerPlugin() {
    IdaFrontend::deleteMenuItem(tr("%1/%2").arg(subviewsMenuPath).arg(decompileFunctionMenuItem));
    IdaFrontend::deleteMenuItem(tr("%1/%2").arg(subviewsMenuPath).arg(decompileProgramMenuItem));
}

void DecompilerPlugin::decompileFunction() {
    auto functionRanges = IdaFrontend::functionAddresses(IdaFrontend::screenAddress());
    if (functionRanges.empty()) {
        QMessageBox::warning(NULL, branding_.applicationName(), tr("Please, put the text cursor inside a function that you would like to decompile."));
        return;
    }

    ByteAddr functionAddress = functionRanges.front().start();

    if (auto window = function2window_[functionAddress]) {
        activateWindow(window);
        window->jumpToAddress(IdaFrontend::screenAddress());
        return;
    }

    auto window = createWindow();
    window->open(createIdaProject());

    window->project()->setName(IdaFrontend::functionName(functionAddress));

    foreach (const AddressRange &range, functionRanges) {
        window->project()->disassemble(window->project()->image().get(), range.start(), range.end());
    }

    if (window->decompileAutomatically()) {
        window->project()->decompile();
    }

    function2window_[functionAddress] = window;
    window2function_[window] = functionAddress;
}

void DecompilerPlugin::decompileProgram() {
    if (programWindow_) {
        activateWindow(programWindow_);
        programWindow_->jumpToAddress(IdaFrontend::screenAddress());
        return;
    }

    auto window = createWindow();
    window->open(createIdaProject());
    window->project()->disassemble();
    if (window->decompileAutomatically()) {
        window->project()->decompile();
    }
    programWindow_ = window;
}

gui::MainWindow *DecompilerPlugin::createWindow() {
    gui::MainWindow *window = new gui::MainWindow(branding_);
    window->setWindowFlags(Qt::Widget);
    window->setAttribute(Qt::WA_DeleteOnClose);

    connect(window, SIGNAL(destroyed(QObject *)), this, SLOT(windowDestroyed(QObject *)));

    QWidget *widget = IdaFrontend::createWidget(window->windowTitle());
    widget->setLayout(new QStackedLayout());
    widget->layout()->addWidget(window);

    window->openAction()->setEnabled(false);
    window->openAction()->setVisible(false);

    window->quitAction()->setEnabled(false);
    window->quitAction()->setVisible(false);

    connect(window, SIGNAL(windowTitleChanged(const QString &)), widget, SLOT(setWindowTitle(const QString &)));

    window2widget_[window] = widget;

    /* Owned by MainWindow. */
    new NavigationHelper(window);

    return window;
}

void DecompilerPlugin::activateWindow(gui::MainWindow *window) {
    assert(window != NULL);

    IdaFrontend::activateWidget(window2widget_[window]);
}

void DecompilerPlugin::windowDestroyed(QObject *object) {
    /*
     * checked_cast is not applicable here: destructor of
     * gui::MainWindow was already called.
     */
    auto window = static_cast<gui::MainWindow *>(object);

    if (programWindow_ == window) {
        programWindow_ = NULL;
    } else {
        function2window_.erase(window2function_[window]);
        window2function_.erase(window);
    }

    window2widget_.erase(window);
}

std::unique_ptr<gui::Project> DecompilerPlugin::createIdaProject() const {
    auto project = std::make_unique<gui::Project>();
    auto image = project->image().get();

    /* Set architecture. */
    image->platform().setArchitecture(IdaFrontend::architecture());

    /* Set the OS. */
    image->platform().setOperatingSystem(IdaFrontend::operatingSystem());

    /* Set demangler. */
    image->setDemangler(std::make_unique<IdaDemangler>());

    /* Create sections. */
    IdaFrontend::createSections(image);

    using core::image::Relocation;
    using core::image::Symbol;
    using core::image::SymbolType;

    /* Add function names. */
    foreach(ByteAddr address, IdaFrontend::functionStarts()) {
        QString name = IdaFrontend::functionName(address);

        if (!name.isEmpty()) {
            image->addSymbol(std::make_unique<Symbol>(SymbolType::FUNCTION, name, address));
        }
    }

    /* Add imported function names. */
    typedef std::pair<ByteAddr, QString> Import;
    foreach(const Import &import, IdaFrontend::importedFunctions()) {
        image->addRelocation(std::make_unique<Relocation>(
            import.first,
            image->addSymbol(std::make_unique<Symbol>(SymbolType::FUNCTION, import.second, boost::none))));
    }

    return project;
}

NC_IDA_REGISTER_PLUGIN(DecompilerPlugin);

}} // namespace nc::ida

/* vim:set et sts=4 sw=4: */
