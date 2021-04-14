#ifndef ASKINPUTNUMBER_H
#define ASKINPUTNUMBER_H

#include <QDialog>

namespace Ui {
class AskInputNumber;
}

class AskInputNumber : public QDialog
{
    Q_OBJECT

public:
    explicit AskInputNumber(QWidget *parent = nullptr);
    ~AskInputNumber();

private:
    Ui::AskInputNumber *ui;

private slots:
    void send();

signals:
    void send_input_num(const int &_input_num);
};

#endif // ASKINPUTNUMBER_H
