#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class CodeEditor;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void updateStartText();

signals:

private:
    CodeEditor  *m_editor;
    QAction *m_startAct;
    QAction *m_stepAct;
    QAction *m_stopAct;

    bool m_running;
    int m_bpCount;
};

#endif // MAINWINDOW_H
