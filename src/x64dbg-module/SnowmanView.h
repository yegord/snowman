#ifndef SNOWMANVIEW_H
#define SNOWMANVIEW_H

#include <QWidget>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <bridgemain.h>

class SnowmanView : public QWidget
{
    Q_OBJECT
public:
    explicit SnowmanView(QWidget* parent = 0);
    void decompileAt(duint start, duint end);

protected:
    void closeEvent(QCloseEvent* event);

private slots:
    void populateInstructionsContextMenu(QMenu* menu);
    void populateCxxContextMenu(QMenu* menu);
    void jumpFromInstructionsView();
    void jumpFromCxxView();

private:
    QMainWindow* mSnowmanMainWindow;
    QAction* mJumpFromInstructionsViewAction;
    QAction* mJumpFromCxxViewAction;
};

extern "C" __declspec(dllexport) SnowmanView* CreateSnowman(QWidget* parent);
extern "C" __declspec(dllexport) void DecompileAt(SnowmanView* snowman, duint start, duint end);

#endif // SNOWMANVIEW_H
