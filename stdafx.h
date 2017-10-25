#ifndef STDAFX_H
#define STDAFX_H

#define STRING(x) QStringLiteral(x).toStdString().c_str()
#include <QString>
#include <QMap>
#include <QPixmap>
#include <QSqlDatabase>
#include <QByteArray>
#include "threadcontroller.h"

struct AddressBookItem
{
    int itemID;
    QString name;
    QString type;
    QString sex;
    QString phone;
    QString email;
    QString note;
    //QString avatarPath;
    QByteArray avatarByteArr;
    bool succeed;

    bool operator < (const AddressBookItem& item)
    {
        return this->name < item.name;
    }
};

// external variable declaration
class global
{
public:
    // ����QSS�ļ��ĺ���
    static QByteArray LoadQSSFile(const QString& qssName);

    // ����ȫ�ֱ����ĺ���
    static void ClearGlobalVaraible();

    // ���ݿ����
    static bool OpenDatabase();
    static void CloseDatabase();

    static QString getHexString(const QByteArray& str);

public:
    static QString userName;
    static QString tableAddressItemName;
    static QString tableAddressTypeName;
    static QMap<QString, int>typeNameTotypeID;
    static const char* selectFilter;
    static const char* defaultAvatarDir;
    static const char* createAddressTypeSQL;
    static const char* createAddressItemSQL;
    static const char* createDefaultTypeSQL;
    static const int avatarWidth = 40;
    static const int avatarHeight = 40;
    static const int addItemDialogAvatarWidth = 80;
    static const int addItemDialogAvatarHeight = 80;
    // �ж��Ƿ����������������
    static bool hasInsertNewType;
    static QString newTypeName;
    static QString newTypeDescription;

    // �ж��Ƿ��޸�����������
    static bool hasModifyTypeName;
    static QString originalTypeName;
    static QString modifiedTypeName;
    static QString modifiedDetail;
    // �ж��Ƿ�ɾ������������
    static bool hasDeleteTypeName;
    static QString deletedTypeName;

    //��ѯ�õ���SQL���
    static const char* baseQuerySQL;
    static const char* nameConditionSQL;
    static const char* sexConditionSQL;
    static const char* typeConditionSQL;

    // �ж��Ƿ��޸�����ϵ��
    static bool hasModifyAddressItem;
    static int modifiedItemID;
    static QString modifiedName;
    static QString modifiedSex;
    static QString originalType;
    static QString modifiedType;
    static QString modifiedPhone;
    static QString modifiedEmail;
    static QString modifiedNote;
    static QString modifiedAvatarDir;

    // �ж��Ƿ�ɾ������ϵ��
    static bool hasDeleteAddressItem;
    static int deletedItemID;
    static QString deletedItemTypeName;

    // ָ���޸�ĳ���ض�����ϵ��
    static int specificItemID;

    // ʹ��QPixmap�����ȹ���GUI
    //static QPixmap specificAvatar;

    static const char* qqAvatarDir;
    static const char* downArrowDir;
    static const char* rightArrowDir;

    static QSqlDatabase db;
    static bool connectToDatabase;

    // ����ͷ�����������
    static Controller* controller;

    // ����ѡ��������̵߳ķ����ź�
    static bool loginHasPressed;
    static bool registerHasPressed;

    static unsigned char key;
    static QByteArray Encrypt(const QString& plaintext);
    static QString Decrypt(QByteArray ciphertext);

    // �ж��Ƿ�ѡ�����µ�ͷ��
    static bool hasSelectedNewAvatar;

};
#endif // STDAFX_H
