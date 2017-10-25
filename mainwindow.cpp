// modified by Brian at 2017.6.12
// * add a map from typeName to typeID

// modified by Brian at 2017.6.13
// * ����������ӳ�䵽����ID�Ĵ����ƶ���AddressBookWindow.cpp�Ĺ��캯����

// modified by Brian at 2017.6.14
// * ������Ҫ����AddressBookWindow��ָ����Ϊ��Ա����

// modified by Brian at 2017.6.15
// * �����ݿ������ȫ�ֱ���
// * ��Ӷ��޷����ӵ����������߼��Ĵ���
// * ���������ݿ�������Ĵ����������һ���߳���ִ��

// modified by Brian at 2017.6.16
// * ��������֤��ΪSHA-256��֤
// * �������û�����LineEdit�ĳ�edittable��comboBox
// * ��AddressBookWindow��ȫ��ָ��ĳɳ�Ա����

// modified by Brian at 2017.6.18
// * ����������Ϊͨ������༭��ʵ��

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "stdafx.h"
#include "RegisterDialog.h"
#include "threadcontroller.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QMessageBox>
#include <QSqlQuery>
#include <QCryptographicHash>
#include <QComboBox>

// ����MainWindow��ȫ��ָ��
MainWindow* mainWindowPtr;

// function: open the sqlite database and load the username and the
//           hash code of password, while initialize the interface
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    addressBookWnd(0)
{
    ui->setupUi(this);
    ui->busyProgressBar->hide();

    mainWindowPtr = this;
    global::controller = new Controller();
    //connect(global::controller, &global::controller->connectFinish(), this, SLOT(logIn));
    connect(global::controller, SIGNAL(onLogInFeedback()), this, SLOT(logIn()));

    try{
        sqliteDatabase = QSqlDatabase::addDatabase("QSQLITE", "SQLiteConnection");
        sqliteDatabase.setDatabaseName("record");
        if(!sqliteDatabase.open())
            throw QString(STRING("��SQLite���ݿ�ʧ��"));

        QSqlQuery query(sqliteDatabase);
        query.exec("SELECT id, userName, password, passwordLen FROM record");
        if(query.lastError().type() != QSqlError::ErrorType::NoError)
            throw QString(STRING("��ѯʧ�ܣ� ") + query.lastError().text());
        while(query.next()){
            QString decryptUserName = global::Decrypt(query.value(1).toByteArray());
            PasswordHashCode* hashCode = new PasswordHashCode();
            hashCode->recordID = query.value(0).toInt();
            hashCode->hashCode = query.value(2).toByteArray();
            hashCode->passwordLen = query.value(3).toInt();
            passwordHashCodes.insert(decryptUserName, hashCode);

            // ��comboBox������Ҳ���ܻᴥ��onCurrentIndexChanged�ź�
            // ����Ȳ����¼���ڴ����źŵĲۺ����в�ѯ��¼�Ͳ������
            ui->userNameComboBox->addItem(decryptUserName);
        }
    }catch(QString& e){
        QMessageBox::warning(NULL, "Info", e, QMessageBox::Ok);
    }catch(std::exception& e){
        QMessageBox::warning(NULL, "Info", e.what(), QMessageBox::Ok);
    }
}

// function: release resource and close the connection to sqlite database
MainWindow::~MainWindow()
{
    for(auto ptr : passwordHashCodes)
        delete ptr;
    if(sqliteDatabase.isOpen())
        sqliteDatabase.close();
    delete ui;
}

// function: try to connect to database in another thread
void MainWindow::on_login_clicked()
{
    global::loginHasPressed = true;
    auto geometry = ui->busyProgressBar->geometry();
    auto wndGeometry = this->geometry();

    // ����progressBar��λ��
    QPoint progressBarPos = QPoint((wndGeometry.width() - geometry.width()) / 2,
                                   (wndGeometry.height() - geometry.height()) / 2);

    ui->busyProgressBar->setGeometry(progressBarPos.x(),
                                     progressBarPos.y(),
                                     geometry.width(), geometry.height());

    ui->busyProgressBar->show();
    // �����ź������߳�
    emit global::controller->onTryToConnectServer();
}

// function: open the register window
void MainWindow::on_regist_clicked()
{
    RegisterDialog* registerWindow = new RegisterDialog(NULL);
    registerWindow->exec();
    delete registerWindow;
}

// function: show the help information
void MainWindow::on_forget_clicked()
{
    QMessageBox::warning(NULL, "Info", STRING("����ϵ����Ա������\nCrazyMan\nL��F��Flower"), QMessageBox::Ok);
}

// function: when user input new username, the check box should be uncheck
void MainWindow::on_userNameComboBox_currentTextChanged(const QString &arg1)
{
    // Ϊ�����û��������˺ŵ�ʱ���������������Ϊ��
    // �Լ���checkBox����Ϊδѡ�е�״̬
    if(!passwordHashCodes.keys().contains(arg1)){
        ui->passwordLineEdit->setText("");
        ui->savePasswordCheckBox->setChecked(false);
        return;
    }
}

// function: set the number of placeholder in password line edit
//           when clear record, currentIndexChanged will be triggered,
//           this time should set the password line edit to blank
//           and uncheck the checkbox
void MainWindow::on_userNameComboBox_currentIndexChanged(const QString &arg1)
{
    // Ϊ����������м�¼��ʱ���������������Ϊ��
    // �Լ���checkBox����Ϊδѡ�е�״̬
    if(!passwordHashCodes.keys().contains(arg1)){
        ui->passwordLineEdit->setText("");
        ui->savePasswordCheckBox->setChecked(false);
        return;
    }

    char placeHolder[21] = "                    ";
    int passwordLen = passwordHashCodes.value(arg1)->passwordLen;
    placeHolder[passwordLen] = 0;
    if(passwordLen > 0)
        ui->savePasswordCheckBox->setChecked(true);
    else
        ui->savePasswordCheckBox->setChecked(false);
    ui->passwordLineEdit->setText(placeHolder);
}

// function: when user uncheck the checkbox, the password line edit
//           should be clear
void MainWindow::on_savePasswordCheckBox_stateChanged(int arg1)
{
    if(arg1 == Qt::Unchecked){
        ui->passwordLineEdit->setText("");
        auto ptr = passwordHashCodes.value(ui->userNameComboBox->currentText());
        if(!ptr) return;
        ptr->passwordLen = 0;
        ptr->hashCode.clear();
    }
}

// function: called when signal onLogInFeedback is received
//           the connection to database may succeed or failed
void MainWindow::logIn()
{
    QString userName,password;
    try
    {
        //busyProgressBar->close();
        if(!global::connectToDatabase)
            throw QString(STRING("�޷����ӵ�������"));

        userName = ui->userNameComboBox->currentText();
        password = ui->passwordLineEdit->text();
        if(userName == "")
            throw std::runtime_error(STRING("�û�������Ϊ��"));
        if(password == "")
            throw std::runtime_error(STRING("���벻��Ϊ��"));

        QByteArray hashCode;
        if(passwordHashCodes.keys().contains(userName) &&
                passwordHashCodes.value(userName)->passwordLen != 0){
            hashCode = passwordHashCodes.value(userName)->hashCode;
        }else{
            hashCode = QCryptographicHash::hash(QByteArray(password.toStdString().c_str()),
                                                QCryptographicHash::Algorithm::Sha256);
        }

        // ��hashCodeת����16�����ַ������бȽ�
        // ��ȻҲ���Ի�ȡ����Ĺ�ϣ�뵽���������Ƚ�
        auto hashCodeStr = global::getHexString(hashCode);
        QSqlQuery query(global::db);
        query.prepare(QString("SELECT * FROM UserInfo WHERE userName = ? AND password = %1").arg(hashCodeStr));
        query.addBindValue(userName);
        query.exec();
        if(!query.next())
        {
            throw std::runtime_error(STRING("�û������������"));
        }

        // �������ݵ�SQLite���ݿ�
        if(ui->savePasswordCheckBox->isChecked()){
            try{
                // �Ѿ����ڼ�¼�����
                if(passwordHashCodes.keys().contains(userName))
                {
                    auto ptr = passwordHashCodes.value(userName);
                    int recordID = ptr->recordID;
                    QSqlQuery query(sqliteDatabase);
                    query.prepare("UPDATE record SET password = ?, "
                                  "passwordLen = ? WHERE id = ?");
                    query.addBindValue(hashCode);
                    query.addBindValue(password.length());
                    query.addBindValue(recordID);
                    query.exec();
                    if(!query.exec())
                        throw QString(STRING("����SQLite���ݿ�ʧ��: ") + query.lastError().text());

                    // ��������
                    ptr->hashCode = hashCode;
                    ptr->passwordLen = password.length();
                }
                else
                {
                    QByteArray encryptUserName = global::Encrypt(userName);
                    int passwordLength = password.length();
                    QSqlQuery query(sqliteDatabase);
                    query.prepare("INSERT INTO record(userName, password, passwordLen) "
                                  "VALUES(?, ?, ?)");
                    query.addBindValue(encryptUserName);
                    query.addBindValue(hashCode);
                    query.addBindValue(passwordLength);
                    if(!query.exec())
                        throw QString(STRING("����SQLite���ݿ�ʧ��: ") + query.lastError().text());

                    PasswordHashCode* record = new PasswordHashCode();
                    query.exec("SELECT MAX(id) FROM record");
                    query.next();
                    record->recordID = query.value(0).toInt();
                    record->hashCode = hashCode;
                    record->passwordLen = password.length();
                    passwordHashCodes.insert(userName, record);
                    ui->userNameComboBox->addItem(userName);
                }
            }catch(QString& e){
                QMessageBox::warning(NULL, "Info", e, QMessageBox::Ok);
            }catch(std::exception& e){
                QMessageBox::warning(NULL, "Info", e.what(), QMessageBox::Ok);
            }
        }
        else{
            if(passwordHashCodes.keys().contains(userName))
            {
                try{
                    auto ptr = passwordHashCodes.value(userName);
                    int recordID = ptr->recordID;
                    QSqlQuery query(sqliteDatabase);
                    query.prepare("UPDATE record SET password = NULL, "
                                  "passwordLen = 0 WHERE id = ?");
                    query.addBindValue(recordID);
                    query.exec();
                    if(!query.exec())
                        throw QString(STRING("����SQLite���ݿ�ʧ��: ") + query.lastError().text());

                    // ��������
                    ptr->hashCode.clear();
                    ptr->passwordLen = 0;

                }catch(QString& e){
                    QMessageBox::warning(NULL, "Info", e, QMessageBox::Ok);
                }catch(std::exception& e){
                    QMessageBox::warning(NULL, "Info", e.what(), QMessageBox::Ok);
                }

            }
            else
            {
                try{
                    QByteArray encryptUserName = global::Encrypt(userName);
                    QSqlQuery query(sqliteDatabase);
                    query.prepare("INSERT INTO record(userName, passwordLen) "
                                                      "VALUES(?, ?)");
                    query.addBindValue(encryptUserName);
                    query.addBindValue(QVariant(0));
                    if(!query.exec())
                        throw QString(STRING("����SQLite���ݿ�ʧ��: ") + query.lastError().text());

                    PasswordHashCode* record = new PasswordHashCode();
                    query.exec("SELECT MAX(id) FROM record");
                    query.next();
                    record->recordID = query.value(0).toInt();
                    record->passwordLen = 0;
                    passwordHashCodes.insert(userName, record);
                    ui->userNameComboBox->addItem(userName);
                }catch(QString& e){
                    QMessageBox::warning(NULL, "Info", e, QMessageBox::Ok);
                }catch(std::exception& e){
                    QMessageBox::warning(NULL, "Info", e.what(), QMessageBox::Ok);
                }
            }
        }

        this->hide();
        ui->busyProgressBar->hide();

        //��ʼ���û������ͱ����Լ���ϵ�˱���
        global::userName = userName;
        global::tableAddressTypeName = "AddressType_" + userName;
        global::tableAddressItemName = "AddressItem_" + userName;

        addressBookWnd = new AddressBookWindow(NULL, userName);
        addressBookWnd->show();
        addressBookWnd->PrepareData();
    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }
    ui->busyProgressBar->hide();
}

// function: when user logout, the function will be called
//           this function just reset the interface after user logout
void MainWindow::onUserLogout()
{
    QString userName = ui->userNameComboBox->currentText();
    if(passwordHashCodes.keys().contains(userName))
    {
        PasswordHashCode* ptr = passwordHashCodes.value(userName);
        char placeHolder[21] = "                    ";
        placeHolder[ptr->passwordLen] = 0;
        ui->passwordLineEdit->setText(placeHolder);
    }
    else
    {
        ui->userNameComboBox->setCurrentText("");
        ui->passwordLineEdit->setText("");
    }
}

// function: when user choose delete current record
//           this function just remove the record from sqlite database
void MainWindow::on_deleteRecord_triggered()
{
    try
    {
        QString userName = ui->userNameComboBox->currentText();
        int itemIndex = ui->userNameComboBox->findText(userName);
        if(itemIndex == -1)return;

        int keyID = passwordHashCodes.value(userName)->recordID;
        QSqlQuery query(sqliteDatabase);
        query.prepare("DELETE FROM record WHERE id = ?");
        query.addBindValue(keyID);
        query.exec();
        if(!query.exec())
            throw QString(STRING("ɾ����ǰ��¼ʧ��"));
        ui->userNameComboBox->removeItem(itemIndex);

    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }

}

// function: called when user choose to clear all record
void MainWindow::on_clearAllRecord_triggered()
{
    try
    {
        QSqlQuery query(sqliteDatabase);
        if(!query.exec("DELETE FROM record"))
            throw QString(STRING("�����¼ʧ��"));

        for(auto key : passwordHashCodes.keys()){
            auto keyIndex = ui->userNameComboBox->findText(key);
            ui->userNameComboBox->removeItem(keyIndex);
        }
        passwordHashCodes.clear();
    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }
}
