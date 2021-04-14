#include "infodialog.h"
#include "ui_infodialog.h"

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QMessageBox>
#include <QStringBuilder>

#include "askinputnumber.h"
#include "askinputposition.h"

InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    connect(ui->confirmButton, SIGNAL(clicked()), this, SLOT(send_data()));
    connect(ui->chooseButton, SIGNAL(clicked()), this, SLOT(choose_input()));
    connect(this, SIGNAL(senddata(const int &, const int &,
                                  const int &, Position *,
                                  const int &, const int &,
                                  const bool &)),
            parent, SLOT(get_data(const int &, const int &,
                                  const int &, Position *,
                                  const int &, const int &,
                                  const bool &)));
}

InfoDialog::~InfoDialog()
{
    delete ui;
    if (_input)
        delete [] _input;
}

void InfoDialog::send_data()
{
    if ((ui->row_box->value() <= 3 && ui->line_box->value() <= 3) ||
         _input_num <= 0 ||
        (in_center_or_illegal(ui->output_x_box->value(), ui->output_y_box->value(), ui->line_box->value(), ui->row_box->value())) )
    {
        QMessageBox::warning(this, tr("警告"), tr("芯片信息不合法！"));
        return;
    }
    for (int i = 0; i < _input_num; i++)
        if (_input[i]._x == ui->output_x_box->value() && _input[i]._y == ui->output_y_box->value())
        {
            QMessageBox::warning(this, tr("警告"), tr("芯片信息不合法！"));
            return;
        }
    emit senddata(ui->line_box->value(), ui->row_box->value(),
                  _input_num, _input,
                  ui->output_x_box->value(), ui->output_y_box->value(),
                  ui->checkBox->checkState());
    this->close();
}



bool in_center_or_illegal(const int &x, const int &y, const int &size_x, const int &size_y)
{
    if (x > size_x || y > size_y)
        return true;
    if (x > 1 && x < size_x && y > 1 && y < size_y)
        return true;
    return false;
}

void InfoDialog::choose_input()
{
    size_x = ui->line_box->value();
    size_y = ui->row_box->value();
    if (ui->row_box->value() <= 3 && ui->line_box->value() <= 3)
    {
        QMessageBox::warning(this, tr("警告"), tr("芯片信息不合法！"));
        return;
    }

    AskInputNumber ask(this);
    ask.setFixedSize(ask.size());
    ask.exec();

    _input = new Position[_input_num];
    for (int i = 0; i < _input_num; i++)
    {
        AskInputPosition ask2(this, i);
        ask2.setFixedSize(ask2.size());
        ask2.exec();

        bool judge_illegal = false;
        for (int j = 0; j < i; j++)
            if (_input[j] == _input[i])
            {
                QMessageBox::warning(this, tr("警告"), tr("芯片信息不合法！"));
                i--;
                judge_illegal = true;
                break;
            }
        if (judge_illegal)
            continue;

        if (in_center_or_illegal(_input[i]._x, _input[i]._y, size_x, size_y))
        {
            QMessageBox::warning(this, tr("警告"), tr("芯片信息不合法！"));
            i--;
        }
    }
    ui->lineEdit_input->setText("");
    for (int i = 0; i < _input_num; i++)
    {
        ui->lineEdit_input->setText(ui->lineEdit_input->text() % '(' + QString::number(_input[i]._x,10) % "," % QString::number(_input[i]._y,10) % ");");
    }
}

void InfoDialog::set_input_num(const int &num)
{
    _input_num = num;
}

void InfoDialog::set_input_position(const int &now, const int &x, const int &y)
{
    _input[now].SetValue(x, y);
}
