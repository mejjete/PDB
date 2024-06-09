#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "setorremovebreakpointdialog.h"
#include "highlighter.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
class QString;
class QColor;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionOpen_triggered();
    void on_actionSet_or_Remove_Breakpoint_triggered();

    void setOrRemoveBreakpoint(int lineNumber);

private:
    Ui::MainWindow *ui;
    SetOrRemoveBreakpointDialog* m_breakpointCfgDialog;
    bool m_isBreakpointSet;
    Highlighter* m_highlighter;

    void initUI();

    // Helper method.
    void loadTextFile(const QString& filePath);
};
#endif // MAINWINDOW_H
