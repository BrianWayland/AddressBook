// modified by Brian at 2017.6.16
// * ��ע���¼��������г������ӷ�����

#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = 0);
    ~RegisterDialog();

private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    // �Զ����
    void onRegister();
private:
    Ui::RegisterDialog *ui;
};

#endif // REGISTERDIALOG_H
