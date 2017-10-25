// modified by Brian at 2017.6.14
// * ȡ�����ڹ��캯���в�ѯ�������������ӿ�����
// * �������޸ĺ�������������е���������ͬ�����
// * �����UpdateUI������ÿ�������������ʱ�����������
// * ����˸ı���������comboBox��������ʾ����ȷ������
// * comboxBox�����ݲ�����ʾĬ�Ϸ��飬ͬʱҲɾ���˶�Ĭ�Ϸ�����Ĵ���

#include "ModifyTypeDialog.h"
#include "ui_ModifyTypeDialog.h"
#include "stdafx.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>

// function: update the interface
ModifyTypeDialog::ModifyTypeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ModifyTypeDialog)
{
    ui->setupUi(this);
    this->setStyleSheet(global::LoadQSSFile(":/QSS/QSS/Dialog.qss"));
}

ModifyTypeDialog::~ModifyTypeDialog()
{
    delete ui;
}

// function: called when user press "ok" button
//           to update data from database
void ModifyTypeDialog::on_buttonBox_accepted()
{
    try{
        QString originTypeName = ui->originTypeName->currentText();
        QString newTypeName = ui->newTypeName->text();
        QString newDetail = ui->newDetail->toPlainText();

        if(originTypeName == STRING("Ĭ��"))
            throw QString(STRING("�����޸�Ĭ��������"));

        if(newTypeName != originTypeName && global::typeNameTotypeID.keys().contains(newTypeName))
            throw QString(STRING("������") + newTypeName + STRING("�Ѿ�����"));

        if(newTypeName == "")
        //    throw QString(STRING("����������Ϊ��"));
            newTypeName = originTypeName;

        //if(newTypeName == originTypeName)
        //    throw QString(STRING("ԭ������������������ͬ"));

        QSqlQuery query(global::db);
        query.prepare(QString("UPDATE %1 SET typeName = ?, detail = ? WHERE typeName = ?").arg(global::tableAddressTypeName));
        query.addBindValue(newTypeName);
        query.addBindValue(newDetail);
        query.addBindValue(originTypeName);

        if(!query.exec())
            throw QString(STRING("�޸�ʧ��") + query.lastError().text());


        // ʹ���źźͲۻ��Ƹ���
        global::hasModifyTypeName = true;
        global::originalTypeName = originTypeName;
        global::modifiedTypeName = newTypeName;
        global::modifiedDetail = newDetail;

    }
    catch(QString& e){
            QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}

void ModifyTypeDialog::UpdateUI()
{
    for(auto key : global::typeNameTotypeID.keys()){
        if(key == STRING("Ĭ��"))
            continue;
        ui->originTypeName->addItem(key);
    }
}

void ModifyTypeDialog::on_originTypeName_currentIndexChanged(const QString &arg1)
{
    try{
        QSqlQuery query(global::db);
        query.prepare(QString("SELECT detail FROM %1 WHERE typeName = ?").arg(global::tableAddressTypeName));
        query.addBindValue(arg1);
        if(!query.exec())
            throw QString(query.lastError().text());
        query.next();
        ui->newDetail->setText(query.value(0).toString());
        ui->newTypeName->setText(arg1);

    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}
