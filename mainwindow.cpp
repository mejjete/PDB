#include "mainwindow.h"
#include "codeeditor.h"
#include <QAction>
#include <QToolBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow{parent},
    m_editor(nullptr),
    m_startAct(nullptr),
    m_stepAct(nullptr),
    m_stopAct(nullptr)
{
    m_editor = new CodeEditor(parent);

    QStringList lines;
    lines << "#include <iostream>"
          << "using namespace std;"
          << ""
          << "int square(int x) { return x * x; }"
          << ""
          << "int main() {"
          << "    cout << \"Demo start\" << endl;";
    for (int i = 0; i < 150; ++i) {
        lines << QString("    cout << \"line %1 => square(%1)=\" << square(%1) << endl;").arg(i);
    }
    lines << "    cout << \"Demo end\" << endl;"
          << "    return 0;"
          << "}";
    m_editor->loadSample(lines.join('\n'));

    setCentralWidget(m_editor);

    m_startAct = new QAction("Start Debugging");
    m_startAct->setIcon(QIcon(":/images/start.png"));
    m_stepAct  = new QAction("Step Over");
    m_stepAct->setIcon(QIcon(":/images/step_over.png"));
    m_stopAct  = new QAction("Stop Debugging");
    m_stopAct->setIcon(QIcon(":/images/stop.png"));

    QObject::connect(m_startAct, &QAction::triggered, m_editor, &CodeEditor::startDebug);
    QObject::connect(m_stepAct,  &QAction::triggered, m_editor, &CodeEditor::step);
    QObject::connect(m_stopAct,  &QAction::triggered, m_editor, &CodeEditor::stopDebug);

    m_running = false;
    m_bpCount = 0;

    QObject::connect(m_editor, &CodeEditor::debugStateChanged,
                     [this](bool isRunning){ m_running = isRunning; updateStartText(); });
    QObject::connect(m_editor, &CodeEditor::breakpointsChanged,
                     [this](int count){ m_bpCount = count; updateStartText(); });

    // TODO: find a better way to do that.
    // ------------------------------------------------------------
    m_stepAct->setEnabled(false);
    m_stopAct->setEnabled(false);
    QObject::connect(m_editor, &CodeEditor::debugStateChanged,
                     [this](bool isRunning) {
                         m_stepAct->setEnabled(isRunning);
                         m_stopAct->setEnabled(isRunning);
                     });
    // ------------------------------------------------------------

    auto toolbar = addToolBar("Debug Toolbar");
    toolbar->addAction(m_startAct);
    toolbar->addAction(m_stepAct);
    toolbar->addAction(m_stopAct);
    toolbar->setIconSize(QSize(16, 16));
}

void MainWindow::updateStartText()
{
    m_startAct->setText( (m_running && m_bpCount > 1) ? "Continue" : "Start Debugging" );
}

