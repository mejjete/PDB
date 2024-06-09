#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QString>
#include <QFileDialog>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QColor>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_breakpointCfgDialog(nullptr)
    , m_isBreakpointSet(false)
    , m_highlighter(nullptr)
{
    ui->setupUi(this);

    initUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    setWindowTitle("PDB");

    ui->textEdit->setReadOnly(true);
    m_highlighter = new Highlighter(ui->textEdit->document());

    ui->tabWidget->setTabText(0, "Output");
}

void MainWindow::loadTextFile(const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QString fileContent;
        while (!in.atEnd())
            fileContent = in.readAll();
        ui->textEdit->setPlainText(fileContent);
        file.close();
    } else {
        // TODO: Handler the error.
        QString errMessage = "Failed to open the file " + filePath;
        qDebug() << errMessage;
        exit(EXIT_FAILURE);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog* dialog = new QFileDialog(this, "Open File",
        QDir::homePath(), "C/C++ Files (*.h *.hpp *.c *.cpp)");

    dialog->setModal(false);

    connect(dialog, &QFileDialog::fileSelected, this, [this](const QString &fileName) {
        qDebug() << fileName;
        loadTextFile(fileName);
    });

    dialog->show();
}

void MainWindow::on_actionSet_or_Remove_Breakpoint_triggered()
{
    if (ui->textEdit->toPlainText() != QString()) {
        m_breakpointCfgDialog = new SetOrRemoveBreakpointDialog(this);
        connect(m_breakpointCfgDialog, &SetOrRemoveBreakpointDialog::lineNumber,
                this, &MainWindow::setOrRemoveBreakpoint);
        m_breakpointCfgDialog->exec();
    }
}

void MainWindow::setOrRemoveBreakpoint(int lineNumber)
{
    if (!m_isBreakpointSet) {
        qDebug() << "Breakpoint at line " << lineNumber << "is set.";
        m_isBreakpointSet = true;
    } else {
        qDebug() << "Breakpoint at line " << lineNumber << "is removed.";
        m_isBreakpointSet = false;
    }
}

