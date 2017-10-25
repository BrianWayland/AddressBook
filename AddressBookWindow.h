// modified by Brian at 2017.6.12
// * constructor of AddressGroupItem
// modified by Brian at 2017.6.13
// * ��AddressItem������˳�ԱitemID, itemType, itemNote��Ա
// * �޸���AddressItem�๹�캯���Լ�AddAddressItem�Ķ���
// * ��AddressGroupItem��Vector�ĳ�QMap<int, QListWidgetItem*>,
// *   ���޸���AddItem��setItemHidden�Ⱥ����Ķ���
// * ����AddressGroupItem�������getItem����
// * ����AddressItem�����UpdateItem����

// modified by Brian at 2017.6.14
// * ����ͨ��QMap<QListWidgetItem*, bool> groupHiddenList ��ά�����״̬
// *    ��ʹ���ڲ�����

// modified by Brian at 2017.6.15
// * ������mouseTracking

// modified by Brian at 2017.6.18
// * �����캯���д����ݿ��ȡ���ݵĴ���ŵ�PrepareData������

// modified by Brian at 2017.6.19
// * �Ҽ������ϵ�˵����Ĳ˵�������ƶ����Ӳ˵���ɾ���˳�Աָ��itemMenu
#ifndef ADDRESSBOOKWINDOW_H
#define ADDRESSBOOKWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QVector>
#include <QMap>
#include <QLabel>
#include <QEvent>
#include <QPainter>
#include <QTimer>
#include "PresentSearchResult.h"
#include "ItemDetail.h"
#include "TypeDetail.h"
#include "stdafx.h"

namespace Ui {
class AddressBookWindow;
}

class AddressBookWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AddressBookWindow(QWidget *parent, const QString& userName);
    ~AddressBookWindow();

    // Ϊ�ӿ���������ٶȣ��ڹ��캯��֮������ݿ��ȡ����
    void PrepareData();

    // �Զ����ź�
signals:
    void onUserLogout();
    void addNewType(QString& newType);
    void deleteType(QString& deletedType);

private slots:
    void on_addItem_triggered();

    void on_searchItem_triggered();

    void on_addType_triggered();

    void on_modifyType_triggered();

    void on_deleteType_triggered();

    void on_modifyItem_triggered();

    void on_deleteItem_triggered();

    void on_about_triggered();

    void on_logout_triggered();

    void on_exit_triggered();

    void on_destroy_triggered();

    void on_addressList_customContextMenuRequested(const QPoint &pos);

    void on_addressList_itemClicked(QListWidgetItem *item);

    /// Ҫ��������ۣ���������mousceTrackingѡ��
    /// �Զ����ItemҲ��������
    void on_addressList_itemEntered(QListWidgetItem *item);

    // �Զ����
    void onEditTypeNameFinished();
    void onCustomMenuRequestModifyTypeName();
    void onCustomMenuRequestDeleteTypeName();
    void onCustomMenuRequestModifyItem();
    void onCustomMenuRequestDeleteItem();
    // ��ָ�벻ָ���κ���ϵ�˻������͵�ʱ�򣬾���Ҫ�ر���ϸ��Ϣ���
    void CheckHasCursorPointToAnyItem();
    void onMoveToSubMenuTriggered(QString typeName);

private:
    /*
     * inner class
     */
    class AddressGroupItem : public QListWidgetItem
    {
    public:
        AddressGroupItem(const QIcon &icon, const QString &name, const QString& detail):
            QListWidgetItem(icon, name + "(0)"),
            groupName(name),
            isHidden(false),
            description(detail)
        {

        }
        ~AddressGroupItem(){}

        void ResetGroupName(const QString& str){
            this->setText(str + QString("(%1)").arg(itemList.size())); //������ʾ������
            groupName = str;
        }

        void AddItem(int itemID, QListWidgetItem* item){
            itemList.insert(itemID, item);
            this->setText(QString("%1(%2)").arg(groupName).arg(itemList.size()));
        }

        void RemoveItem(int itemID){
            itemList.remove(itemID);
            this->setText(QString("%1(%2)").arg(groupName).arg(itemList.size()));
        }

        bool ContainItem(QListWidgetItem* item){
            return itemList.values().contains(item);
        }

        QListWidgetItem* getItem(int itemID)const{
            return itemList.value(itemID, NULL);
        }

        void onStatusChanged(){
            isHidden = !isHidden;
            for(auto itemPtr : itemList.values())
                itemPtr->setHidden(isHidden);
            if(isHidden)
                setIcon(QIcon(global::rightArrowDir));
            else
                setIcon(QIcon(global::downArrowDir));
        }

        int ItemNum() const {return itemList.size();}

        QMap<int, QListWidgetItem*> getItemList()const{return itemList;}

        void insertItemList(const QMap<int, QListWidgetItem*>& newItemList){
            for(auto key : newItemList.keys()){
               itemList.insert(key, newItemList.value(key));
            }
            this->setText(QString("%1(%2)").arg(groupName).arg(itemList.size()));
        }

        // ������
        bool getStatus()const{return isHidden;}
        QString getTypeName()const{return groupName;}
        QString getDescription()const{return description;}

        // ������
        void setTypeName(const QString& name){ResetGroupName(name);}
        void setDescription(const QString& str){description = str;}

    private:
        QMap<int, QListWidgetItem*> itemList;
        QString groupName;
        QString description;
        bool isHidden;
    };

    class AddressItem : public QWidget
    {
    public:
        AddressItem(QWidget* parent, int ID, const QString& name, const QString& type,
                    const QString& sex, const QString& phone, const QString& email,
                    const QString& note, const QPixmap& img):
            QWidget(parent), itemID(ID), name(name), type(type), sex(sex),
            phone(phone), email(email), note(note),
            avatarImg(img), avatar(new QWidget(this)),
            labelName(new QLabel(this)), labelPhone(new QLabel(this))
        {
            avatar->setFixedSize(40,40);
            labelName->setText(name);
            labelPhone->setText(phone);

            QPalette color;
            color.setColor(QPalette::Text,Qt::gray);
            labelPhone->setPalette(color);

            avatar->move(7,7);
            labelName->move(54,10);
            labelPhone->move(54,27);

            avatar->installEventFilter(this);

            this->setMouseTracking(true);
        }
        ~AddressItem(){}

        void UpdateItem(const QString& name, const QString& type,
                        const QString& sex, const QString& phone, const QString& email,
                        const QString& note, const QPixmap& img){
            this->name = name;
            this->type = type;
            this->sex = sex;
            this->phone = phone;
            this->email = email;
            this->note = note;
            avatarImg = img;

            QPainter painter(avatar);
            painter.drawPixmap(avatar->rect(), avatarImg);
            labelName->setText(name);
            labelPhone->setText(phone);
        }

        //������
        int getItemID()const{return itemID;}
        QString getName()const{return name;}
        QString getType()const{return type;}
        QString getSex()const{return sex;}
        QString getPhone()const{return phone;}
        QString getEmail()const{return email;}
        QString getNote()const{return note;}
        QPixmap getAvatarImg()const{return avatarImg;}

    private:
        int itemID;
        QString name, type, sex, phone, email, note;
        QWidget* avatar;
        QLabel *labelName;
        QLabel *labelPhone;
        QPixmap avatarImg;
        bool eventFilter(QObject *obj, QEvent *event)
        {
            if(obj == avatar)
            {
                if(event->type() == QEvent::Paint)
                {
                    QPainter painter(avatar);
                    painter.drawPixmap(avatar->rect(), avatarImg);
                }
            }
            return QWidget::eventFilter(obj, event);
        }
    };
    /*
     * inner class
     */

private:
    AddressBookWindow::AddressGroupItem* AddGroupItem(const QString& groupName, const QString& detail);

    void AddAddressItem(AddressGroupItem* group, int ID, const QString& name, const QString& type,
                        const QString& sex, const QString& phone, const QString& email,
                        const QString& note, const QPixmap& avatar);

    void GenerateItemCustomMenu(QListWidgetItem* selectedItem);

    Ui::AddressBookWindow *ui;
    QString userName;
    QMap<QString, QListWidgetItem*> groupList;
    QMap<QListWidgetItem*, bool> groupHiddenList;
    PresentSearchResult* presentResultWnd;

    QMenu *blankMenu;
    QMenu *typeMenu;

    QLineEdit* typeNameEdit;

    QListWidgetItem* currentEditingTypeItem;
    QListWidgetItem* customMenuRequestItem; //�����Ҽ��˵���item

    ItemDetail* detailPanel;
    TypeDetail* typeDetailPanel;
    QTimer* timer;

    bool isInitializing; // ������ǳ�ʼ���������ϵ��ʱ�Ͷ�λ����ȷ��λ��
    bool isDeletingType; // ɾ������ʱ�Լ��޸���ϵ��ʱ�������
    bool isModifyingItem;
};

#endif // ADDRESSBOOKWINDOW_H
