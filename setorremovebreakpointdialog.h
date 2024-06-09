#ifndef SETORREMOVEBREAKPOINTDIALOG_H
#define SETORREMOVEBREAKPOINTDIALOG_H

#include <QDialog>

namespace Ui {
class SetOrRemoveBreakpointDialog;
}

class SetOrRemoveBreakpointDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SetOrRemoveBreakpointDialog(QWidget *parent = nullptr);
    ~SetOrRemoveBreakpointDialog();

signals:
    void lineNumber(int);

private slots:
    void on_buttonBox_accepted();

private:
    Ui::SetOrRemoveBreakpointDialog *ui;
    void initUI();
};

#endif // SETORREMOVEBREAKPOINTDIALOG_H
