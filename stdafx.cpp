// defination of global variables
// created by Brian at 2017.6.12
// modified by Brian at 2017.6.13
// * ����˴���AddressItem��type�ֶε�NOT NULLԼ��

#include "stdafx.h"
#include <QFile>
#include <QMessageBox>
#include <QSqlError>
#include <sstream>
#include <string>

// usage: some global varaible defination

QString global::userName;
QString global::tableAddressItemName;
QString global::tableAddressTypeName;
QMap<QString, int> global::typeNameTotypeID;
const char* global::createAddressTypeSQL = "CREATE TABLE AddressType_%1 ("
            "typeID TINYINT IDENTITY PRIMARY KEY,"
            "typeName VARCHAR(30) NOT NULL UNIQUE,"
            "detail TEXT)";

const char* global::createAddressItemSQL = "CREATE TABLE AddressItem_%1("
            "itemID SMALLINT IDENTITY PRIMARY KEY,"
            "itemName VARCHAR(20) NOT NULL,"
            "type TINYINT FOREIGN KEY REFERENCES AddressType_%2(typeID) NOT NULL,"
            "sex CHAR(2) CHECK(sex IN('%3','%4')),"
            "phone VARCHAR(20), email VARCHAR(30),"
            "note TEXT,"
            "avatar VARBINARY(6500))";

const char* global::createDefaultTypeSQL = "INSERT INTO AddressType_%1(typeName, detail)"
            "VALUES('%2', '%3')";

bool global::hasInsertNewType;
QString global::newTypeName;
QString global::newTypeDescription;

bool global::hasModifyTypeName;
QString global::originalTypeName;
QString global::modifiedTypeName;
QString global::modifiedDetail;

bool global::hasDeleteTypeName;
QString global::deletedTypeName;

const char* global::baseQuerySQL = "SELECT itemID AS %1, itemName AS %2, sex AS %3, "
                                   "phone AS %4, email AS %5, typeName AS %6 "
                                   "FROM %7, %8 WHERE %9.type = %10.typeID";

const char* global::nameConditionSQL = " AND itemName = ?";
const char* global::sexConditionSQL = " AND sex = ?";
const char* global::typeConditionSQL = " AND typeName = ?";

bool global::hasModifyAddressItem;
QString global::modifiedName;
QString global::originalType;
QString global::modifiedType;
QString global::modifiedSex;
QString global::modifiedPhone;
QString global::modifiedEmail;
QString global::modifiedNote;
QString global::modifiedAvatarDir;
int global::modifiedItemID;

bool global::hasDeleteAddressItem;
int global::deletedItemID;
QString global::deletedItemTypeName;

int global::specificItemID;

const char* global::qqAvatarDir = ":/Img/Img/qqAvatar.png";
const char* global::downArrowDir = ":/Img/Img/downArrow.png";
const char* global::rightArrowDir = ":/Img/Img/rightArrow.png";
const char* global::selectFilter = "All files(*.*);;jpg files(*.jpg);;bmp files(*.bmp);;png files(*.png)";
const char* global::defaultAvatarDir = global::qqAvatarDir;

QSqlDatabase global:: db;
bool global::connectToDatabase = false;

Controller* global::controller;

bool global::loginHasPressed;
bool global::registerHasPressed;

unsigned char global::key = 0x86;

bool global::hasSelectedNewAvatar = false;

/*
 * https://stackoverflow.com/questions/24963081/qiodeviceread-device-not-open�Ľ��
 *
 *If you're reading from a .qrc resource file you have to run qmake
 *("Build->Run qmake" in Qt Creator) before it will be available.
 * Ҫ��QRC�ļ���Ч������������QMAKE
 */
QByteArray global::LoadQSSFile(const QString& qssName){
    QFile qssFile(qssName);
    qssFile.open(QIODevice::ReadOnly);
    if(qssFile.isOpen()){
        return qssFile.readAll();
        qssFile.close();
    }else
        QMessageBox::warning(NULL, "warning", STRING("��") + qssName + STRING("ʧ��"),
                             QMessageBox::Ok);
    return QByteArray();
}

void global::ClearGlobalVaraible()
{
    userName.clear();
    tableAddressItemName.clear();
    tableAddressTypeName.clear();
    typeNameTotypeID.clear();

    hasInsertNewType = false;
    hasModifyTypeName = false;
    hasDeleteTypeName = false;
    hasModifyAddressItem = false;
    hasDeleteAddressItem = false;
}

bool global::OpenDatabase()
{
    //try{
    if(!global::db.isValid()){
        // Ҫ����ͬʱ�򿪶�����ݿ�ʱ�ĳ�ͻ��Ҫ�������ӵ�����
        global::db = QSqlDatabase::addDatabase("QODBC", "SqlServerConnection");
        db.setDatabaseName(QString("DRIVER={SQL SERVER};" "SERVER=%1;" "DATABASE=%2;" "UID=%3;" "PWD=%4;")
                               .arg("192.168.1.177")
                               .arg("AddressBookDatabase")
                               .arg("sa")
                               .arg("1Q2W3EASD")
                               );
    }

    if (!db.open())
    {
        //QSqlError error = db.lastError();
        //QString errorStr = QString("Failed to open database!Error: %1").arg(error.text());

        /*Exception at 0x767fb802, code: 0xe06d7363: C++ exception, flags=0x1
 * (execution cannot be continued) (first chance)
 * at e:\project\addressbook\project\addressbook\stdafx.cpp:123
ASSERT failure in QWidget: "Widgets must be created in the GUI thread.",
file kernel\qwidget.cpp, line 1150
Debug Error!*/

        //throw std::runtime_error(errorStr.toStdString().c_str());
        return false;
    }
    return true;
    //}catch(std::exception& e){
    //    QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    //    return false;
    //}
}

void global::CloseDatabase()
{
    if(db.open()){
        db.close();
    }
}

QString global::getHexString(const QByteArray& str)
{
    std::stringstream ss; ss << "0x";
    for(int i = 0; i < str.size(); i++){
        ss.width(2);ss.fill('0');
        // 1.����ֱ��ת��unsigned int,�����������룬��ΪС��0����ת��unsignedʱ
        //      C++�������λ��f����ʵ���ǽ�����ֱ�������������ѣ�ת��unsigned char��Ϊ�˽ض�
        // 2.����ת��unsigned int ������������������ΪC++���ַ����ǰ��ַ�����ģ�����������
        ss << std::hex << (unsigned int)(unsigned char)str[i];
    }
    return QString::fromStdString(ss.str());
}

QByteArray global::Encrypt(const QString& plaintext)
{
    std::string stdStr = plaintext.toStdString();
    int plainTextLen = plaintext.length();
    unsigned char cipherText[20];

    cipherText[0] = ((unsigned char)stdStr[0]) ^ key;
    // ��Ҫ���Զ�ȡstd::string c_str()���ص�ָ������ַ�
    // C++ 98 ָ�� ��Ҫ�޸����ָ����κ��ַ�
    // C++ 11 ָ�����ָ��ָ������ڲ���һ�������ַ�������
    for(int i = 1; i < plainTextLen; i++){
        cipherText[i] = ((unsigned char)stdStr[i])  ^ cipherText[i - 1];
    }
    return QByteArray((char*)cipherText, plainTextLen);
}

QString global::Decrypt(QByteArray ciphertext)
{
    unsigned char plainText[21];
    int Len = ciphertext.length();

    plainText[0] = ciphertext[0] ^ global::key;
    for(int i = 1; i < Len; i++){
        plainText[i] = ciphertext[i] ^ ciphertext[i - 1];
    }
    plainText[Len] = 0; // ��ӿ��ַ�
    return QString((char*)plainText);
}
