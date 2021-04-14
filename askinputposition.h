#ifndef ASKINPUTPOSITION_H
#define ASKINPUTPOSITION_H

#include <QDialog>

namespace Ui {
class AskInputPosition;
}

class AskInputPosition : public QDialog
{
    Q_OBJECT

public:
    explicit AskInputPosition(QWidget *parent = nullptr, const int &_now = 0);
    ~AskInputPosition();

private:
    Ui::AskInputPosition *ui;
    int now;

private slots:
    void send();

signals:
    void send_input_position(const int &_now, const int &x, const int &y);
};

#endif // ASKINPUTPOSITION_H
