// modified by Brian at 2017.6.12
// * modify the SQL script
// * no longer need to pay attention to the id
// modified by Brian at 2017.6.13
// * ������Ҫ�ر���Ĭ�Ϸ�������
// modified by Brian at 20107.6.14
// * �����comboBox��ʾ����Ĭ�����͵�����
// * ���ڴ����ݿ��л�ȡ����������

// modifide by Brian at 2017.6.16
// * �����˴�������Ϊ�յĴ���

#include "AddAddressDialog.h"
#include "ui_AddAddressDialog.h"
#include "SelectAvatarDialog.h"
#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <exception>
#include <QMessageBox>
#include <QFileDialog>

extern AddressBookItem newItemInfo;

// 80 * 80 show in the add address item dialog
// used to update interface
// used by: ModifyItemDialog.cpp, SelectAvatarDialog.cpp
QByteArray avatarByteArr;

// 40 * 40 shown in the avatar of each item
// used to add bind value to database
// used by: ModifyItemDialog.cpp, SelectAvatarDialog.cpp
QByteArray smallAvatarByteArr;

// function: initialize the interface
//           while initialize the avatarByteArr and smallAvatarByteArr
AddAddressDialog::AddAddressDialog(QWidget *parent, const QString& userName) :
    QDialog(parent),
    ui(new Ui::AddAddressDialog),
    userName(userName)
{
    ui->setupUi(this);
    this->setStyleSheet(global::LoadQSSFile(":/QSS/QSS/Dialog.qss"));

    for(auto key : global::typeNameTotypeID.keys())
        ui->type->addItem(key);

    ui->labelShowAvatar->setPixmap(QPixmap(global::defaultAvatarDir));
    // ��ʼ��ͷ��
    avatarDir = global::defaultAvatarDir;
    QPixmap img = QPixmap(avatarDir).scaled(global::avatarWidth, global::avatarHeight);
    QDataStream ds(&avatarByteArr, QIODevice::WriteOnly);
    ds << img;
    QDataStream ds1(&smallAvatarByteArr, QIODevice::WriteOnly);
    ds1 << img.scaled(global::avatarWidth, global::avatarHeight);
}

AddAddressDialog::~AddAddressDialog()
{
    delete ui;
}

// function: called when user press "ok" button
//           then insert new data entry to database
void AddAddressDialog::on_buttonBox_accepted()
{
    try
    {
        QString name = ui->name->text();
        if(name == "")return;
        QString typeName = ui->type->currentText();
        QString sex = ui->sex->currentText();
        QString phone = ui->phone->text();
        QString email = ui->email->text();
        QString note = ui->note->text();

        QSqlQuery query(global::db);
        query.prepare(QString("INSERT INTO %1 "
                      "(itemName, type, sex, phone, email, note, avatar)"
                      "VALUES (?, ?, ?, ?, ?, ?, ?)").arg(global::tableAddressItemName));
        query.addBindValue(name);

        query.addBindValue(global::typeNameTotypeID.value(typeName));

        // To bind a NULL value, use a null QVariant;
        query.addBindValue(sex);
        query.addBindValue(phone == "" ? QVariant(QVariant::String) : phone);
        query.addBindValue(email == "" ? QVariant(QVariant::String) : email);
        query.addBindValue(note == "" ? QVariant(QVariant::String) : note);

        // smallAvatarByteArr�ڹ��캯�����Ѿ�������Ĭ��ͷ��Ķ��������ݣ����
        // ����Ϊ�գ������Ԫ���ʽ�Ƕ����
        query.addBindValue(smallAvatarByteArr.isEmpty() ? QVariant(QVariant::ByteArray) :
                                                     smallAvatarByteArr);
        newItemInfo.succeed = query.exec();
        if(newItemInfo.succeed)
        {
            query.prepare(QString("SELECT itemID FROM %1 WHERE itemName = ?").arg(global::tableAddressItemName));
            query.addBindValue(name);
            query.exec();query.next();

            // ������ӵ���Ϣ��ȫ�ֱ����У�����ʹ���źźͲ۵Ļ��Ƹ���
            newItemInfo.itemID = query.value(0).toInt();
            newItemInfo.name = name;
            newItemInfo.type = typeName;
            newItemInfo.sex = sex;
            newItemInfo.phone = phone;
            newItemInfo.email = email;
            newItemInfo.note = note;
            //newItemInfo.avatarPath = avatarDir;
            newItemInfo.avatarByteArr = avatarByteArr;
            throw QString(STRING("��ӳɹ�"));
        }
        else
            throw QString(STRING("���ʧ��") + query.lastError().text());
    }
    catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok, QMessageBox::Ok);
    }
    catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}

// function: open a dialog to modify the picture which
//           will be used as a avatar
void AddAddressDialog::on_buttonAddAvatar_clicked()
{
    SelectAvatarDialog* dialog = new SelectAvatarDialog();
    dialog->exec();
    delete dialog;

    // ����ʹ���źźͲ۵Ļ��Ƹ���
    if(global::hasSelectedNewAvatar)
    {
        // ѡ������ͷ������

        QPixmap avatar;
        QDataStream ds(&avatarByteArr, QIODevice::ReadOnly); // 80 * 80 pixels
        ds >> avatar;

        ui->labelShowAvatar->setPixmap(avatar);
    }
}
