#include "askinputposition.h"
#include "ui_askinputposition.h"

#include <QStringBuilder>

AskInputPosition::AskInputPosition(QWidget *parent, const int &_now) :
    QDialog(parent),
    ui(new Ui::AskInputPosition)
{
    ui->setupUi(this);
    now = _now;
    ui->label->setText("请输入第" % QString::number(now + 1) % "个输入端口的位置");

    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(send()));
    connect(this, SIGNAL(send_input_position(const int &, const int &, const int &)), parent, SLOT(set_input_position(const int &, const int &, const int &)));
}

AskInputPosition::~AskInputPosition()
{
    delete ui;
}

void AskInputPosition::send()
{
    emit send_input_position(now, ui->x_position->value(), ui->y_position->value());
    this->close();
}

