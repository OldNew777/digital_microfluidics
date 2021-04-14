#include "askinputnumber.h"
#include "ui_askinputnumber.h"

AskInputNumber::AskInputNumber(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskInputNumber)
{
    ui->setupUi(this);
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(send()));
    connect(this, SIGNAL(send_input_num(const int &)), parent, SLOT(set_input_num(const int &)));
}

AskInputNumber::~AskInputNumber()
{
    delete ui;
}

void AskInputNumber::send()
{
    emit send_input_num(ui->spinBox->value());
    this->close();
}
