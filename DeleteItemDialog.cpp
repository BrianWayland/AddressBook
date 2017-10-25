// created by Brian at 2017.6.13
// modified from ModifiedItemDialog.cpp

#include "DeleteItemDialog.h"
#include "ui_DeleteItemDialog.h"
#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QFileDialog>

// function: intialize the interface
DeleteItemDialog::DeleteItemDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeleteItemDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(global::LoadQSSFile(":/QSS/QSS/Dialog.qss"));
    QSqlQuery query(global::db);

    // ��AddressGroupItem�ĳ��ⲿ��Ͳ���Ҫ�����ݿ��л�ȡ������
    // ���ǵ�����ƿ��ǲ�������
    query.prepare(QString("SELECT typeName FROM %1").arg(global::tableAddressTypeName));
    query.exec();
    // ��ѡ���κ����
    ui->type->addItem("");
    while(query.next()){
        ui->type->addItem(query.value(0).toString());
    }
    query.prepare(QString("SELECT itemID FROM %1").arg(global::tableAddressItemName));
    query.exec();
    // ��ѡ���κ���ϵ��ID
    ui->comboBoxItemID->addItem("");
    while(query.next())
        ui->comboBoxItemID->addItem(query.value(0).toString());
}

DeleteItemDialog::~DeleteItemDialog()
{
    delete ui;
}

// function: called when user changed the current item
//           to update the interface
void DeleteItemDialog::on_comboBoxItemID_currentIndexChanged(const QString &arg1)
{
    if(arg1.isEmpty()){
        ui->name->setText("");
        ui->type->setCurrentText("");
        ui->sex->setCurrentText("");
        ui->phone->setText("");
        ui->email->setText("");
        ui->note->setText("");
        ui->labelShowAvatar->setPixmap(QPixmap()); // ���ÿյ�ͷ��
    }
    else{
        try{

            // ��AddressItem��Ϊ�ⲿ��Ͳ���Ҫ��ô�鷳��
            // �������ʱ���ǲ��������������

            int itemID = arg1.toInt();
            QSqlQuery query(global::db);
            query.prepare(QString("SELECT itemName, typeName, sex, phone, email, note, avatar FROM %1, %2"
                                  " WHERE %3.type = %4.typeID AND itemID = ?")
                          .arg(global::tableAddressItemName)
                          .arg(global::tableAddressTypeName)
                          .arg(global::tableAddressItemName)
                          .arg(global::tableAddressTypeName));
            query.addBindValue(itemID);
            query.exec();query.next();

            // ����ԭ����������
           global::deletedItemTypeName = query.value(1).toString();

            ui->name->setText(query.value(0).toString());
            ui->type->setCurrentText(query.value(1).toString());
            ui->sex->setCurrentText(query.value(2).toString());
            ui->phone->setText(query.value(3).toString());
            ui->email->setText(query.value(4).toString());
            ui->note->setText(query.value(5).toString());

            QByteArray imgByteArray= query.value(6).toByteArray();
            QPixmap avatar;
            if(!imgByteArray.isEmpty()){
                QDataStream ds(imgByteArray);
                ds >> avatar;
            }
            ui->labelShowAvatar->setPixmap(avatar);

        }catch(std::exception e){
            QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
        }
    }
}

// function: called when user pressed the "ok" button
//           to remove a data entry from database
void DeleteItemDialog::on_buttonBox_accepted()
{
    try{
        QString sID = ui->comboBoxItemID->currentText();
        int itemID;
        if(sID.isEmpty())
            throw QString(STRING("û��ѡ���κ���ϵ��"));
        else
            itemID = sID.toInt();

        // ����ʹ���źźͲ۵Ļ��Ƹ���
        QSqlQuery query(global::db);
        query.prepare(QString("DELETE FROM %1 WHERE itemID = ?").arg(global::tableAddressItemName));
        query.addBindValue(itemID);
        if(!query.exec())
            throw QString(STRING("ִ��ʧ��") + query.lastError().text());
        global::hasDeleteAddressItem = true;
        global::deletedItemID = itemID;

    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}
