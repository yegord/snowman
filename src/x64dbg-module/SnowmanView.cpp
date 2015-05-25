#include <nc/config.h>
#include <nc/common/Branding.h>
#include <nc/common/Foreach.h>
#include <nc/common/make_unique.h>
#include <nc/common/Types.h>
#include <nc/core/image/ByteSource.h>
#include <nc/core/Context.h>
#include <nc/core/arch/Instruction.h>
#include <nc/core/image/Image.h>
#include <nc/core/mangling/Demangler.h>
#include <nc/core/image/Section.h>
#include <nc/gui/MainWindow.h>
#include <nc/gui/Project.h>
#include <nc/gui/InstructionsView.h>
#include <nc/gui/CxxView.h>

#include "SnowmanView.h"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QTreeView>
#include <QMenuBar>
#include <QPlainTextEdit>

extern "C" __declspec(dllexport) SnowmanView* CreateSnowman(QWidget* parent)
{
    return new SnowmanView(parent);
}

extern "C" __declspec(dllexport) void DecompileAt(SnowmanView* snowman, duint start, duint end)
{
    snowman->decompileAt(start, end);
}

extern "C" __declspec(dllexport) void CloseSnowman(SnowmanView* snowman)
{
    snowman->close();
}

class DbgByteSource : public nc::core::image::ByteSource
{
    virtual nc::ByteSize readBytes(nc::ByteAddr addr, void *buf, nc::ByteSize size) const
    {
        if(DbgMemRead(addr, (unsigned char*)buf, size))
            return size;
        else
            return 0;
    }
};

class DbgDemangler: public nc::core::mangling::Demangler {
    public:

    virtual QString demangle(const QString &symbol) const
    {
        return symbol;
    }
};

static std::unique_ptr<nc::gui::Project> MakeProject(duint base, duint size)
{
    auto project = std::make_unique<nc::gui::Project>();
    auto image = project->image().get();

    //set architecture
#ifdef _WIN64
    image->setArchitecture(QLatin1String("x86-64"));
#else //x86
    image->setArchitecture(QLatin1String("i386"));
#endif //_WIN64
    image->setDemangler(std::make_unique<DbgDemangler>());

    //create sections
    auto section = std::make_unique<nc::core::image::Section>(".text", base, size);
    section->setReadable(true);
    section->setWritable(true);
    section->setExecutable(true);
    section->setCode(true);
    section->setData(true);
    section->setBss(true);
    section->setAllocated(true);
    section->setExternalByteSource(std::make_unique<DbgByteSource>());
    image->addSection(std::move(section));

    //add function names
    // TODO

    //add imported function names
    // TODO

    return project;
}

void SnowmanView::decompileAt(duint start, duint end)
{
    nc::gui::MainWindow* mainWindow = (nc::gui::MainWindow*)mSnowmanMainWindow;
    duint pagesize;
    duint pagebase = DbgMemFindBaseAddr(start, &pagesize);
    mainWindow->open(MakeProject(pagebase, pagesize));
    mainWindow->project()->setName("Snowman");
    mainWindow->project()->disassemble(mainWindow->project()->image().get(), start, end+1);
    mainWindow->project()->decompile();
}

SnowmanView::SnowmanView(QWidget* parent) : QWidget(parent)
{
    nc::Branding branding = nc::branding();
    branding.setApplicationName("Snowman");
    nc::gui::MainWindow* mainWindow = new nc::gui::MainWindow(std::move(branding), parent);
    mainWindow->setAutoFillBackground(true);
    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(mainWindow);
    layout->setMargin(0);
    setLayout(layout);

    mSnowmanMainWindow = mainWindow;

    //remove open/quit options
    mainWindow->openAction()->setEnabled(false);
    mainWindow->openAction()->setVisible(false);
    mainWindow->quitAction()->setEnabled(false);
    mainWindow->quitAction()->setVisible(false);

    //add command to jump from snowman to x64dbg
    mJumpFromInstructionsViewAction = new QAction(tr("Show in x64dbg"), this);
    mJumpFromInstructionsViewAction->setShortcut(Qt::CTRL + Qt::Key_Backspace);
    mJumpFromInstructionsViewAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(mJumpFromInstructionsViewAction, SIGNAL(triggered()), this, SLOT(jumpFromInstructionsView()));
    mainWindow->instructionsView()->treeView()->addAction(mJumpFromInstructionsViewAction);

    mJumpFromCxxViewAction = new QAction(tr("Show in x64dbg"), this);
    mJumpFromCxxViewAction->setShortcut(Qt::CTRL + Qt::Key_Backspace);
    mJumpFromCxxViewAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(mJumpFromCxxViewAction, SIGNAL(triggered()), this, SLOT(jumpFromCxxView()));
    mainWindow->cxxView()->textEdit()->addAction(mJumpFromCxxViewAction);

    connect(mainWindow->instructionsView(), SIGNAL(contextMenuCreated(QMenu*)), this, SLOT(populateInstructionsContextMenu(QMenu*)));
    connect(mainWindow->cxxView(), SIGNAL(contextMenuCreated(QMenu*)), this, SLOT(populateCxxContextMenu(QMenu*)));
}

void SnowmanView::closeEvent(QCloseEvent* event)
{
    Q_UNUSED(event);
    mSnowmanMainWindow->close();
}

void SnowmanView::populateInstructionsContextMenu(QMenu* menu)
{
    for (QAction* action : menu->actions())
    {
        if (action->shortcut() == QKeySequence(QKeySequence::FindNext))
            action->setShortcut(QKeySequence("Ctrl+F3"));
    }
    nc::gui::MainWindow* mainWindow = (nc::gui::MainWindow*)mSnowmanMainWindow;
    if(!mainWindow->instructionsView()->selectedInstructions().empty())
    {
        menu->addSeparator();
        menu->addAction(mJumpFromInstructionsViewAction);
    }
}

void SnowmanView::populateCxxContextMenu(QMenu* menu)
{
    for (QAction* action : menu->actions())
    {
        if (action->shortcut() == QKeySequence(QKeySequence::FindNext))
            action->setShortcut(QKeySequence("Ctrl+F3"));
    }
    nc::gui::MainWindow* mainWindow = (nc::gui::MainWindow*)mSnowmanMainWindow;
    if(!mainWindow->cxxView()->selectedInstructions().empty())
    {
        menu->addSeparator();
        menu->addAction(mJumpFromCxxViewAction);
    }
}

void SnowmanView::jumpFromInstructionsView()
{
    nc::gui::MainWindow* mainWindow = (nc::gui::MainWindow*)mSnowmanMainWindow;
    for(auto instruction : mainWindow->instructionsView()->selectedInstructions())
    {
        duint addr = instruction->addr();
        DbgCmdExecDirect(QString().sprintf("disasm \"%p\"", addr).toUtf8().constData());
        GuiShowCpu();
        break;
    }
}

void SnowmanView::jumpFromCxxView()
{
    nc::gui::MainWindow* mainWindow = (nc::gui::MainWindow*)mSnowmanMainWindow;
    for(auto instruction : mainWindow->cxxView()->selectedInstructions())
    {
        duint addr = instruction->addr();
        DbgCmdExecDirect(QString().sprintf("disasm \"%p\"", addr).toUtf8().constData());
        GuiShowCpu();
        break;
    }
}
