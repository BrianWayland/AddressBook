// modified by Brian at 2017.6.14
// * ȡ�����ڹ��캯���в�ѯ�������������ӿ�����
// * ��ɾ������������ϵ����ԭ�е�����ID����Ϊ���޸�Ϊ����Ĭ�Ϸ����ID
// * comboxBox�����ݲ�����ʾĬ�Ϸ��飬ͬʱҲɾ���˶�Ĭ�Ϸ�����Ĵ���

#include "DeleteTypeDialog.h"
#include "ui_DeleteTypeDialog.h"
#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

// function: setup the interface
DeleteTypeDialog::DeleteTypeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DeleteTypeDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(global::LoadQSSFile(":/QSS/QSS/Dialog.qss"));

    for(auto key : global::typeNameTotypeID.keys()){
        if(key == STRING("Ĭ��"))
            continue;
        ui->comboBox->addItem(key);
    }
}

DeleteTypeDialog::~DeleteTypeDialog()
{
    delete ui;
}

// called when user press the "ok" button
// to remove a data entry from database
void DeleteTypeDialog::on_buttonBox_accepted()
{
    try{
        QString selectedTypeName = ui->comboBox->currentText();

        //�������˸�typeName��ID����ϵ�������ֶ�����ΪĬ�Ϸ����ID
        int referencedTypeID = global::typeNameTotypeID.value(selectedTypeName);
        int defaultTypeID = global::typeNameTotypeID.value(STRING("Ĭ��"));
        QSqlQuery query(global::db);

        query.prepare(QString("UPDATE %1 SET type = ? WHERE type = ?").arg(global::tableAddressItemName));
        query.addBindValue(defaultTypeID);
        query.addBindValue(referencedTypeID);

        if(!query.exec())
            throw QString(STRING("ɾ����ϵ�����ʧ�ܣ�") + query.lastError().text());

        query.prepare(QString("DELETE FROM %1 WHERE typeName = ?").arg(global::tableAddressTypeName));
        query.addBindValue(selectedTypeName);
        if(!query.exec())
            throw QString(STRING("ɾ����ϵ�����ʧ�ܣ�") + query.lastError().text());

        // �Ľ���ʹ���źźͲ�
        global::deletedTypeName = selectedTypeName;
        global::hasDeleteTypeName = true;

    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}
