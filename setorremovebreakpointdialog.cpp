#include "setorremovebreakpointdialog.h"
#include "ui_setorremovebreakpointdialog.h"
#include <QIntValidator>

SetOrRemoveBreakpointDialog::SetOrRemoveBreakpointDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SetOrRemoveBreakpointDialog)
{
    ui->setupUi(this);

    initUI();
}

SetOrRemoveBreakpointDialog::~SetOrRemoveBreakpointDialog()
{
    delete ui;
}

void SetOrRemoveBreakpointDialog::initUI()
{
    // Makes Qt delete this widget when the widget
    // has accepted the close event.
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Breakpoint Configuration");
    setFixedSize(400, 100);

    QIntValidator* validator = new QIntValidator(1, 20000, ui->lineEdit);
    ui->lineEdit->setValidator(validator);
}

void SetOrRemoveBreakpointDialog::on_buttonBox_accepted()
{
    emit lineNumber(ui->lineEdit->text().toInt());
}

