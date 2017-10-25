// modified by Brian at 2017.6.12
// * create new table AddressItem_UserName AddressType_UserName
// modified by Brian at 2017.6.13
// * ��Ĭ�Ϸ�����ӵ����ݿ���
// modified by Brian at 2017.6.14
// * ���������SHA-256����

#include "RegisterDialog.h"
#include "ui_RegisterDialog.h"
#include "stdafx.h"
#include <exception>
#include <stdexcept>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>

// function: setup the interface and bind signal with slot
RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(global::LoadQSSFile(":/QSS/QSS/Dialog.qss"));

    connect(global::controller, SIGNAL(onRegisterFeedback()),
            this, SLOT(onRegister()));
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

// function: called when user press the "ok" button
//           to insert new data entry to database
void RegisterDialog::on_buttonBox_accepted()
{
    try
    {
        // �Ľ������߳��д����ݿ������
        if(!global::connectToDatabase)
            global::connectToDatabase = global::OpenDatabase();

        if(!global::connectToDatabase)
            throw std::runtime_error(STRING("�޷����ӵ�������"));

        QString userName = ui->userNameLineEdit->text();
        QString password = ui->passwordLineEdit->text();
        QString ensure = ui->ensureLineEdit->text();
        if(userName == "")
            throw std::runtime_error(STRING("�û�������Ϊ��"));
            //QMessageBox::warning(NULL, "warning", STRING("�û�������Ϊ��"), QMessageBox::Ok, QMessageBox::Ok);
        if(password == "")
            throw std::runtime_error(STRING("���벻��Ϊ��"));
        if(ensure == "")
            throw std::runtime_error(STRING("ȷ�����벻��Ϊ��"));
        if(password != ensure)
            throw std::runtime_error(STRING("�����ȷ�����벻һ��"));

        QByteArray hashCode = QCryptographicHash::hash(QByteArray(password.toStdString().c_str()),
                                              QCryptographicHash::Algorithm::Sha256);

        QSqlQuery query(global::db);
        query.prepare("INSERT INTO UserInfo (userName, password) VALUES(?, ?)");
        query.addBindValue(userName);
        query.addBindValue(hashCode);
        bool result = query.exec();

        if(!result)
            throw std::runtime_error(query.lastError().text().toStdString());

        result = query.exec(QString(global::createAddressTypeSQL).arg(userName));
        if(!result)
            throw std::runtime_error(query.lastError().text().toStdString());

        result = query.exec(QString(global::createAddressItemSQL)
                            .arg(userName).arg(userName)
                            .arg(STRING("��")).arg(STRING("Ů")));
        if(!result)
            throw std::runtime_error(STRING("ע��ʧ�ܣ�") + query.lastError().text().toStdString());

        result = query.exec(QString(global::createDefaultTypeSQL).arg(userName)
                            .arg(STRING("Ĭ��")).arg(STRING("ϵͳ�����ķ��飬����ɾ��")));
        if(!result)
            throw std::runtime_error(STRING("ע��ʧ�ܣ�") + query.lastError().text().toStdString());
    }
    catch(std::exception e)
    {
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}

void RegisterDialog::on_buttonBox_rejected()
{

}

void RegisterDialog::onRegister()
{

}
