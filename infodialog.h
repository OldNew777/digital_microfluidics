#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

#include "position.h"

namespace Ui {
class InfoDialog;
}

class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = nullptr);
    ~InfoDialog();

private:
    Ui::InfoDialog *ui;
    Position * _input = nullptr;
    int _input_num = 0, size_x = -1, size_y = -1;

private slots:
    void send_data();
    void choose_input();
    void set_input_num(const int &);
    void set_input_position(const int &, const int &, const int &);

signals:
    void senddata(const int &row, const int &line,
                  const int &inputnum, Position * _input,
                  const int &output_x, const int &output_y,
                  const bool &check_state);
};


bool in_center_or_illegal(const int &x, const int &y, const int &size_x, const int &size_y);

#endif // INFODIALOG_H
