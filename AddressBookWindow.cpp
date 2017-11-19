// created by Brian
// modified by Brian at 2017.6.12
// * constructor of class AddressBookWindow
// * SQL Script
// * not longer query the number of a specific type in constructor of AddressBookWindow
// * modify on_addItem_triggered function, no need to pay attention to the type is not exist

// * modified by Brian at 2017.6.13
// * �����������͡�ɾ�����͡��޸����͵��¼�������
// * ������ҪΪĬ�Ϸ����ر���type = NULL��

// * modified by Brian at 2017.6.14
// * ����˹��ڶԻ���
// * ��mainwindow.cpp ��������ӳ�䵽����ID�Ĵ����ƶ���AddressBookWindow�Ĺ��캯����
// * ���ٱ���PresentSearchResult��ָ����Ϊ��Ա
// * ����˽�QVector<QListWidgetItem*>��ΪQMap<int, QListWidgetItem*>��ɾ��
// *     ��ϵ����Ŀ������ȷ��ʾ������(Ŀǰ������������¹���)

// * modified by Brian at 2017.6.15
// * ������Ҽ��˵�

// * modified by Brian at 2017.6.16
// * �������ϵ�˺����͵���ϸ��Ϣ���
// * ��������ϵ�˵Ĵ���
// * ɾ����AddressBookWindow��ȫ��ָ�룬�ĳ�MainWindow��Աָ��

#include "AddressBookWindow.h"
#include "ui_AddressBookWindow.h"
#include "AddAddressDialog.h"
#include "AddTypeDialog.h"
#include "ModifyTypeDialog.h"
#include "DeleteTypeDialog.h"
#include "ModifyItemDialog.h"
#include "DeleteItemDialog.h"
#include "AboutDialog.h"
#include "MyMenuItem.h"
#include <QPixmap>
#include <QSqlQuery>
#include <QSqlError>
#include <QMouseEvent>
#include <QMessageBox>
#include <QListWidget>
#include <QLineEdit>
#include <QDebug>

AddressBookItem newItemInfo;

// MainWindow��ȫ��ָ��
#include "mainwindow.h"
extern MainWindow* mainWindowPtr;
extern QByteArray smallAvatarByteArr;

// parameters
//    parent: parent pointer of this window
//    userName: current user name
// function : intitialize interface and bind the signal with slot.
AddressBookWindow::AddressBookWindow(QWidget *parent, const QString& userName) :
    QMainWindow(parent),
    ui(new Ui::AddressBookWindow),
    userName(userName),
    presentResultWnd(new PresentSearchResult(this)),
    blankMenu(new QMenu()),
    typeMenu(new QMenu()),
    typeNameEdit(new QLineEdit()),
    detailPanel(new ItemDetail()),
    typeDetailPanel(new TypeDetail()),
    timer(new QTimer()),
    isInitializing(true)
{
    ui->setupUi(this);
    //ui->busyProgressBar->hide();

    ui->addressList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //����QSS
    ui->addressList->setStyleSheet(global::LoadQSSFile(":/QSS/QSS/AddressList.qss"));

    // ��ʼ���Ҽ��˵�
    QAction* addTypeMenuItem = new QAction(STRING("���������"), this);
    QAction* modifiedTypeMenuItem = new QAction(STRING("�޸�������"), this);
    QAction* deleteTypeMenuItem = new QAction(STRING("ɾ��������"), this);
    QAction* separator = new QAction(this);
    separator->setSeparator(true);
    QAction* addItemMenuItem = new QAction(STRING("�����ϵ��"), this);
    QAction* searchItemMenuItem = new QAction(STRING("��ѯ��ϵ��"), this);
    QAction* modifiedItemMenuItem = new QAction(STRING("�޸���ϵ��"), this);
    QAction* deleteItemMenuItem = new QAction(STRING("ɾ����ϵ��"), this);

    typeNameEdit->setParent(ui->addressList); // ������ui->setupUi(this)�����ʹ��ui�е�ָ��
    typeNameEdit->hide();

    /*blankMenu->addActions({addTypeMenuItem, modifiedTypeMenuItem, deleteTypeMenuItem,
                          separator, addItemMenuItem, searchItemMenuItem, modifiedItemMenuItem,
                          deleteItemMenuItem});

    typeMenu->addActions({modifiedTypeMenuItem, deleteTypeMenuItem});*/

    QAction* list1[] = {addTypeMenuItem, modifiedTypeMenuItem, deleteTypeMenuItem,
                        separator, addItemMenuItem, searchItemMenuItem, modifiedItemMenuItem,
                        deleteItemMenuItem};
    QList<QAction*> list_1;
    for(auto a :list1)
        list_1.append(a);
    blankMenu->addActions(list_1);

    QAction* list2[] = {modifiedItemMenuItem, deleteItemMenuItem};
    QList<QAction*> list_2;
    for(auto a :list2)
        list_2.append(a);
    typeMenu->addActions(list_2);

    //itemMenu->addActions({modifiedItemMenuItem, deleteItemMenuItem});

    // ���Զ����
    connect(addTypeMenuItem, SIGNAL(triggered()), this, SLOT(on_addType_triggered()));
    connect(modifiedTypeMenuItem, SIGNAL(triggered()),
            this, SLOT(onCustomMenuRequestModifyTypeName()));
    connect(deleteTypeMenuItem, SIGNAL(triggered()),
            this, SLOT(onCustomMenuRequestDeleteTypeName()));

    connect(addItemMenuItem, SIGNAL(triggered()),
            this, SLOT(on_addItem_triggered()));
    connect(searchItemMenuItem, SIGNAL(triggered()),
            this, SLOT(on_searchItem_triggered()));
    connect(modifiedItemMenuItem, SIGNAL(triggered()),
            this, SLOT(onCustomMenuRequestModifyItem()));
    connect(deleteItemMenuItem, SIGNAL(triggered()),
            this, SLOT(onCustomMenuRequestDeleteItem()));

    // ����enterʱeditingFinished��enterPressed��ͬʱ��������˲ۺ�������������
    connect(typeNameEdit, SIGNAL(editingFinished()), this, SLOT(onEditTypeNameFinished()));
    connect(timer, SIGNAL(timeout()), this, SLOT(CheckHasCursorPointToAnyItem()));
    connect(this, SIGNAL(onUserLogout()), mainWindowPtr, SLOT(onUserLogout()));

    connect(this, SIGNAL(addNewType(QString&)), presentResultWnd, SLOT(onAddNewType(QString&)));
    connect(this, SIGNAL(deleteType(QString&)), presentResultWnd, SLOT(onDeleteType(QString&)));

    typeNameEdit->setMaxLength(30);
}

// function : release resource
AddressBookWindow::~AddressBookWindow()
{
    if(timer->isActive())timer->stop();
    delete ui;
}

// function : get data from database and initialize interface.
void AddressBookWindow::PrepareData()
{
    QSqlQuery query(global::db);
    query.prepare(QString("SELECT typeID, typeName, detail FROM %1").arg(global::tableAddressTypeName));
    query.exec();

    while(query.next())
    {
        int typeID = query.value(0).toInt();
        QString typeName = query.value(1).toString();

        // ���������������ŵ�ӳ��
        global::typeNameTotypeID.insert(typeName, typeID);

        QSqlQuery selectTypeCount(global::db);
        auto groupItem = AddGroupItem(typeName, query.value(2).toString());

        // get the content, i.e. the item inside group
        // there is a little confusing, but it is ok
        selectTypeCount.prepare(QString("SELECT itemID, itemName, sex, phone, email, note, avatar FROM %1 "
                                "WHERE type = ?").arg(global::tableAddressItemName));
        selectTypeCount.bindValue(0, typeID);
        selectTypeCount.exec();
        while(selectTypeCount.next())
        {
            try{
                QByteArray imgByteArray= selectTypeCount.value(6).toByteArray();
                QPixmap avatar;
                if(!imgByteArray.isEmpty()){
                    QDataStream ds(imgByteArray);
                    ds >> avatar;
                }
                else
                    avatar = QPixmap(global::defaultAvatarDir);

                AddAddressItem(groupItem,
                               selectTypeCount.value(0).toInt(), // ID
                               selectTypeCount.value(1).toString(), // name
                               query.value(1).toString(), // typeName
                               selectTypeCount.value(2).toString(), // sex
                               selectTypeCount.value(3).toString(), // phone
                               selectTypeCount.value(4).toString(), // email
                               selectTypeCount.value(5).toString(), // note
                               avatar);
            }catch(std::exception e){
                QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
            }
        }
    }
    timer->start(500);
    isInitializing = false;
}

// function: add an new group type
AddressBookWindow::AddressGroupItem* AddressBookWindow::AddGroupItem(const QString& str, const QString &detail)
{
    AddressGroupItem* newItem = new AddressGroupItem(QIcon(QPixmap(global::downArrowDir)),str, detail);
    newItem->setSizeHint(QSize(this->width(),25));
    // Ĭ�Ϸ����ر��ܷ�����ǰ��
    if(str == STRING("Ĭ��"))
        ui->addressList->insertItem(0, newItem);
    else
        ui->addressList->addItem(newItem);
    groupList.insert(str, newItem); // ��AddressGroupItem��ָ��ת��ΪQListWidgetItem��ָ��
    //groupHiddenList.insert(newItem, false);
    return newItem;
}

// parameters
//     group: the pointer of group item this item belong to
//     ID: the ID in database
//     name: the item name
//     type: the type name this item belong to
//     sex: the sex of the new people
//     phone: the phone of the new people
//     email: the email of the new people
//     note: the detail description of the new item
//     avatar: avatar show in interface
// function: add a new item below the group item, and store the information
//inside the item
void AddressBookWindow::AddAddressItem(AddressGroupItem* group, int ID, const QString& name, const QString& type,
                                       const QString& sex, const QString& phone, const QString& email,
                                       const QString& note, const QPixmap& avatar)
{
    QListWidgetItem* newItem = new QListWidgetItem();
    newItem->setSizeHint(QSize(ui->addressList->width(), 54));
    ui->addressList->insertItem(ui->addressList->row(group) + group->ItemNum() + 1, newItem);
    QWidget* newWidget = new AddressItem(NULL, ID, name, type, sex, phone, email, note, avatar);
    ui->addressList->setItemWidget(newItem, newWidget);

    // ����Ĭ�����״̬�����ػ�����ʾ��
    if(group->getStatus())
        newItem->setHidden(true);
    else
        if(!isInitializing && !isDeletingType && !isModifyingItem)
            ui->addressList->setCurrentItem(newItem);
    // bind the newItem to groupItem logically, so that control it easily
    group->AddItem(ID, newItem);
}

// function: add an new type
void AddressBookWindow::on_addType_triggered()
{
    AddTypeDialog* typeDialog = new AddTypeDialog();
    typeDialog->exec();
    delete typeDialog;

    // ����Ӧ��ʹ���źźͲ۵Ļ���
    if(global::hasInsertNewType){
        auto newGroup = AddGroupItem(global::newTypeName, global::newTypeDescription);

        // ����������newTypeItem��ӳ��
        groupList.insert(global::newTypeName, newGroup);
        //groupHiddenList.insert(newGroup, false);

        // �����ѯ���ڱ��򿪣���ô��Ҫ���������������б������
        if(!presentResultWnd->isHidden())
            emit addNewType(global::newTypeName);
        global::hasInsertNewType = false;
    }
}

// function: modify an existing type
void AddressBookWindow::on_modifyType_triggered()
{
    try{
        ModifyTypeDialog* modifyTypeDialog = new ModifyTypeDialog();
        modifyTypeDialog->UpdateUI();
        modifyTypeDialog->exec();
        delete modifyTypeDialog;

        // ����Ӧ��ʹ���źźͲ۵Ļ���
        if(global::hasModifyTypeName){
            auto ptr = groupList.value(global::originalTypeName);
            // �޸�AddressGroupItem�����groupName�ֶΣ��Լ���ʾ������
            ((AddressGroupItem*)ptr)->ResetGroupName(global::modifiedTypeName);
            ((AddressGroupItem*)ptr)->setDescription(global::modifiedDetail);
            // ����ָ��key
            groupList.remove(global::originalTypeName);
            groupList.insert(global::modifiedTypeName, ptr);
            global::hasModifyTypeName = false;
            // �޸���������ID��ӳ��
            int typeID = global::typeNameTotypeID.value(global::originalTypeName);
            global::typeNameTotypeID.remove(global::originalTypeName);
            global::typeNameTotypeID.insert(global::modifiedTypeName, typeID);
        }
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}

// function: deleting an existing type
void AddressBookWindow::on_deleteType_triggered()
{
    try{
        isDeletingType = true;
        DeleteTypeDialog* dialog = new DeleteTypeDialog();
        dialog->exec();
        delete dialog;

        // ����Ӧ��ʹ���źźͲ۵Ļ���
        if(global::hasDeleteTypeName){
            auto ptr = (AddressGroupItem*)groupList.value(global::deletedTypeName);
            auto itemList = ptr->getItemList();
            auto defaultType = (AddressGroupItem*)groupList.value(STRING("Ĭ��"));
            ui->addressList->takeItem(ui->addressList->row(ptr));
            // ɾ����������QListWidgetItem*��ӳ��
            groupList.remove(global::deletedTypeName);

            // Qt ��֧���ƶ�ĳ��item�������λ�ã������Ҫ���¹���
            for(auto itemPtr : itemList.values()){
                // ���¹���
                auto widget = (AddressItem*)ui->addressList->itemWidget(itemPtr);
                AddAddressItem(defaultType, widget->getItemID(), widget->getName(), STRING("Ĭ��"),
                               widget->getSex(), widget->getPhone(), widget->getEmail(),
                               widget->getNote(), widget->getAvatarImg());

                ui->addressList->takeItem(ui->addressList->row(itemPtr));
            }
            delete ptr;

            // ɾ��������������ID��ӳ��
            global::typeNameTotypeID.remove(global::deletedTypeName);

            if(!presentResultWnd->isHidden())
                emit deleteType(global::deletedTypeName);

            global::hasDeleteTypeName = false;
            isDeletingType = false;
        }
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}

// function: add an new item
void AddressBookWindow::on_addItem_triggered()
{
    AddAddressDialog* addressDialog = new AddAddressDialog(NULL, userName);
    addressDialog->exec();
    delete addressDialog;

    QPixmap avatar;
    QDataStream ds(&newItemInfo.avatarByteArr, QIODevice::ReadOnly);
    ds >> avatar;

    // ʹ���źźͲ۵Ļ��Ƹ���
    if(newItemInfo.succeed)
    {
        auto group = groupList.value(newItemInfo.type);
        AddAddressItem((AddressGroupItem*)group, newItemInfo.itemID, newItemInfo.name,
                       newItemInfo.type, newItemInfo.sex, newItemInfo.phone,
                       newItemInfo.email, newItemInfo.note, avatar.
                       scaled(global::avatarWidth, global::avatarHeight));
    }
    // restore the original value
    newItemInfo.succeed = false;
}

// function: show the search result in an new window
void AddressBookWindow::on_searchItem_triggered()
{
    if(presentResultWnd->isHidden()){
        presentResultWnd->show();
    }
    presentResultWnd->UpdateUI();
}

// function: modify an item
void AddressBookWindow::on_modifyItem_triggered()
{
    try{
        isModifyingItem = true;
        ModifyItemDialog* dialog = new ModifyItemDialog();
        dialog->exec();
        delete dialog;
        if(global::hasModifyAddressItem){
            global::hasModifyAddressItem = false;
            // ����޸������ͣ���ô����Ҫ��item�Ӿɵ����ƶ����µ�����
            QListWidgetItem* itemPtr;
            if(QString::compare(global::originalType, global::modifiedType) != 0){
                auto originalGroupPtr = (AddressGroupItem*)groupList.value(global::originalType);
                auto newGroupPtr = (AddressGroupItem*)groupList.value(global::modifiedType);
                itemPtr = originalGroupPtr->getItem(global::modifiedItemID);

                // ɾ��AddressGroupItem��itemID��QListWidgetItem* ��ӳ��
                originalGroupPtr->RemoveItem(global::modifiedItemID);

                QPixmap avatar;

                // ����λͼ��ͼ��
                auto widget = (AddressItem*)ui->addressList->itemWidget(itemPtr);
                if(global::hasSelectedNewAvatar){
                    QDataStream ds(&smallAvatarByteArr, QIODevice::ReadOnly);
                    ds >> avatar;
                }else // �����ǵ������
                    avatar = widget->getAvatarImg();

                // ���´���
                //auto widget = (AddressItem*)ui->addressList->itemWidget(itemPtr);
                AddAddressItem(newGroupPtr, global::modifiedItemID, global::modifiedName, global::modifiedType,
                               global::modifiedSex, global::modifiedPhone, global::modifiedEmail,
                               global::modifiedNote, avatar);

                ui->addressList->takeItem(ui->addressList->row(itemPtr));

            }
            else
            {
                itemPtr = ((AddressGroupItem*)groupList.value(global::modifiedType))
                        ->getItem(global::modifiedItemID);

                QPixmap avatar;
                auto widget = (AddressItem*)ui->addressList->itemWidget(itemPtr);
                if(global::hasSelectedNewAvatar){
                    QDataStream ds(&smallAvatarByteArr, QIODevice::ReadOnly);
                    ds >> avatar;
                }else
                    avatar = widget->getAvatarImg();

                // �����������item��widget
                QWidget* newWidget = new AddressItem(NULL, global::modifiedItemID,
                                     global::modifiedName, global::modifiedType,
                                     global::modifiedSex, global::modifiedPhone,
                                     global::modifiedEmail, global::modifiedNote,
                                     avatar);

                ui->addressList->setItemWidget(itemPtr, newWidget);
            }
            isModifyingItem = false;
        }
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok, QMessageBox::Ok);
    }
}

// function : called when delete an item
void AddressBookWindow::on_deleteItem_triggered()
{
    try{
        DeleteItemDialog* dialog = new DeleteItemDialog();
        dialog->exec();
        delete dialog;
        if(global::hasDeleteAddressItem){
            global::hasDeleteAddressItem = false;
            auto groupPtr = (AddressGroupItem*)groupList.value(global::deletedItemTypeName);
            auto itemPtr = groupPtr->getItem(global::deletedItemID);
            groupPtr->RemoveItem(global::deletedItemID);
            ui->addressList->takeItem(ui->addressList->row(itemPtr));
        }
    }catch(std::exception e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }
}

// function: called when about menu item is pressed
void AddressBookWindow::on_about_triggered()
{
   AboutDialog* dialog = new AboutDialog();
    dialog->exec();
    delete dialog;
}

// function: called when user logout
//           what's more, this function wiil emit an onUserLogout signal
//           to inform mainwindow to reset the inteface.
void AddressBookWindow::on_logout_triggered()
{
    auto ret = QMessageBox::question(NULL, STRING("ȷ��"), STRING("ȷ��ע����ǰ�˻���"),
                                    QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);
    if(ret == QMessageBox::Yes){
        // ����ȫ�ֱ���
        global::ClearGlobalVaraible();
        this->destroy();
        emit onUserLogout();
        mainWindowPtr->show();
    }
}

// function: called thwn user pressed exit, and close the whole application
void AddressBookWindow::on_exit_triggered()
{
    auto ret = QMessageBox::question(NULL, STRING("ȷ��"), STRING("ȷ���˳���"),
                                    QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);
    if(ret == QMessageBox::Yes){
        global::ClearGlobalVaraible();
        qApp->exit();
    }
}

// function: called when user choose to destroy this account
void AddressBookWindow::on_destroy_triggered()
{
    auto ret = QMessageBox::critical(NULL, STRING("Σ�ղ���"), STRING("ȷʵҪɾ�����˻���"),
                                    QMessageBox::Yes, QMessageBox::No, QMessageBox::NoButton);

        try{
            if(ret == QMessageBox::Yes){
                //ִ��ɾ���˻��Ĳ���
                QSqlQuery query(global::db);
                query.prepare("DELETE FROM UserInfo WHERE userName = ?");
                query.addBindValue(global::userName);
                if(!query.exec())
                    throw QString(STRING("ɾ���û�ʧ��") + query.lastError().text());

                // AddressItem����AddressType��typeID,�����ɾ��AddressItem�����
                if(!query.exec("DROP TABLE " + global::tableAddressItemName))
                    throw QString(STRING("ɾ���û�ʧ��") + query.lastError().text());
                if(!query.exec("DROP TABLE " + global::tableAddressTypeName))
                    throw QString(STRING("ɾ���û�ʧ��") + query.lastError().text());

                global::ClearGlobalVaraible();

                this->destroy();
                emit onUserLogout();
                mainWindowPtr->show();

            }
        }catch(QString& e){
            QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
        }catch(std::exception& e){
            QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
        }

}

// function: when user press right key, the function will be called
// note: Ϊ�����Զ���˵��¼���QWidget��contextMenuPolicy������ΪQt::CustomContextMenu
void AddressBookWindow::on_addressList_customContextMenuRequested(const QPoint &pos)
{
    try{

        //if(currentEditingTypeItem)
         //   onEditTypeNameFinished();

        auto itemPointTo = ui->addressList->itemAt(pos);
        auto selectedItem = ui->addressList->currentItem();
        // ��ε���takeItem�Լ�insertItem֮��itemAt�ͻ᷵�ش���Ľ����ԭ��δ��
        // Ϊ��ʹ����currentItem���棬�����ǿհ״��Ĳ˵��ͻ���ʾ������
        // ��˵�itemAt����Ϊ�յ�ʱ����ÿ�ֵ���selectedItemԭ����ֵ

        if(itemPointTo == NULL)
            selectedItem = NULL;
        if(groupList.values().contains(selectedItem)){
            if(((AddressGroupItem*)selectedItem)->getTypeName() == STRING("Ĭ��"))
                return;
            customMenuRequestItem = selectedItem;
            typeMenu->exec(QCursor::pos());
            return;
        }
        for(auto typePtr : groupList.values()){
            if(((AddressGroupItem*)typePtr)->ContainItem(selectedItem)){
                customMenuRequestItem = selectedItem;
                //itemMenu->exec(QCursor::pos());
                GenerateItemCustomMenu(selectedItem);
                return;
            }
        }
        customMenuRequestItem = NULL;
        blankMenu->exec(QCursor::pos());
    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
    }catch(std::exception& e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }
}

// function: when item in QListWidget is pressed(left key), this function is called
//           use this function to show/hide a group's sub item,
//           or close the line edit of modifying type name.
void AddressBookWindow::on_addressList_itemClicked(QListWidgetItem *item)
{
    if(groupList.values().contains(item)){
        ((AddressGroupItem*)item)->onStatusChanged();
    }

    if(currentEditingTypeItem)
        onEditTypeNameFinished();
}

// functon: called when cursor enter an item
//          use this slot to show the floating information window
//          of specific group or item
void AddressBookWindow::on_addressList_itemEntered(QListWidgetItem *item)
{
    if(groupList.values().contains(item)){
        if(!detailPanel->isHidden())detailPanel->hide();
        auto typePtr = (AddressGroupItem*)item;
        typeDetailPanel->setPresentTypeName(typePtr->getTypeName());
        typeDetailPanel->setPresentDescription(typePtr->getDescription());
        auto geometry = ui->addressList->visualItemRect(item);
        auto point = QPoint(geometry.x(), geometry.y());
        auto globalPoint = ui->addressList->mapToGlobal(point);
        typeDetailPanel->setGeometry(globalPoint.x() - typeDetailPanel->width(),
                                     globalPoint.y(),
                                     typeDetailPanel->width(),
                                     typeDetailPanel->height());

        if(typeDetailPanel->isHidden())
            typeDetailPanel->show();
        return;
    }
    else
    {
        if(!typeDetailPanel->isHidden())typeDetailPanel->hide();

        for(auto typePtr : groupList.values()){
            if(((AddressGroupItem*)typePtr)->ContainItem(item)){

                AddressItem* widget = (AddressItem*)ui->addressList->itemWidget(item);
                detailPanel->setPresentName(widget->getName());
                detailPanel->setPresentType(widget->getType());
                detailPanel->setPresentSex(widget->getSex());
                detailPanel->setPresentPhone(widget->getPhone());
                detailPanel->setPresentEmail(widget->getEmail());
                detailPanel->setPresentNote(widget->getNote());
                detailPanel->setPresentAvatar(widget->getAvatarImg());
                auto geometry = ui->addressList->visualItemRect(item);
                auto point = QPoint(geometry.x(), geometry.y());
                auto globalPoint = ui->addressList->mapToGlobal(point);
                detailPanel->setGeometry(globalPoint.x() - detailPanel->width(),
                                                 globalPoint.y(),
                                                 detailPanel->width(),
                                                 detailPanel->height());

                if(detailPanel->isHidden())
                    detailPanel->show();
                return;
            }
        }
    }
}

// function: called when user press the menu item "modify type" of custom menu.
void AddressBookWindow::onCustomMenuRequestModifyTypeName()
{
    if(!customMenuRequestItem)
        on_modifyType_triggered();
    else{
        // ��ʾ�༭���͵�line edit
        if(((AddressGroupItem*)customMenuRequestItem)->getTypeName() == STRING("Ĭ��"))
            return;

        currentEditingTypeItem = customMenuRequestItem;
        typeNameEdit->raise();
        //���ֵ�λ��
        typeNameEdit->setGeometry(ui->addressList->visualItemRect(customMenuRequestItem));
        typeNameEdit->setText(((AddressGroupItem*)customMenuRequestItem)->getTypeName());
        typeNameEdit->show();
        typeNameEdit->selectAll();
        typeNameEdit->setFocus();
    }
}

// function: called when user press the menu item "delete type" of custom menu.
void AddressBookWindow::onCustomMenuRequestDeleteTypeName()
{
    if(!customMenuRequestItem)
        on_deleteType_triggered();
    else{ // ֱ��ɾ��
        try{
            isDeletingType = true;
            auto deletedItem = (AddressGroupItem*)customMenuRequestItem;
            auto deletedTypeName = deletedItem->getTypeName();
            if(deletedTypeName == STRING("Ĭ��"))
                return;

            // ����Ĵ���Ӧ���ܹ�����
            int defaultTypeID = global::typeNameTotypeID.value(STRING("Ĭ��"));
            int referencedTypeID = global::typeNameTotypeID.value(deletedTypeName);
            QSqlQuery query(global::db);
            query.prepare(QString("UPDATE %1 SET type = ? WHERE type = ?").arg(global::tableAddressItemName));
            query.addBindValue(defaultTypeID);
            query.addBindValue(referencedTypeID);

            if(!query.exec())
                throw QString(STRING("ɾ����ϵ�����ʧ�ܣ�") + query.lastError().text());

            query.prepare(QString("DELETE FROM %1 WHERE typeName = ?").arg(global::tableAddressTypeName));
            query.addBindValue(deletedTypeName);
            if(!query.exec())
                throw QString(STRING("ɾ����ϵ�����ʧ�ܣ�") + query.lastError().text());

            auto itemList = deletedItem->getItemList();
            auto defaultType = (AddressGroupItem*)groupList.value(STRING("Ĭ��"));
            ui->addressList->takeItem(ui->addressList->row(deletedItem));
            // ɾ����������QListWidgetItem*��ӳ��
            groupList.remove(deletedTypeName);

            for(auto itemPtr : itemList.values()){
                // ���¹���
                auto widget = (AddressItem*)ui->addressList->itemWidget(itemPtr);
                AddAddressItem(defaultType, widget->getItemID(),widget->getName(), STRING("Ĭ��"),
                               widget->getSex(), widget->getPhone(), widget->getEmail(),
                               widget->getNote(), widget->getAvatarImg());

                ui->addressList->takeItem(ui->addressList->row(itemPtr));
                delete itemPtr;
            }
            delete deletedItem;

            // ɾ��������������ID��ӳ��
            global::typeNameTotypeID.remove(deletedTypeName);

            if(!presentResultWnd->isHidden())
                emit deleteType(deletedTypeName);

            customMenuRequestItem = NULL;
            isDeletingType = false;
        }catch(QString& e){
            QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
        }catch(std::exception& e){
            QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
        }
    }
}

// function: called when user press the menu item "modify item" of custom menu.
void AddressBookWindow::onCustomMenuRequestModifyItem()
{
    if(!customMenuRequestItem){
        global::specificItemID = -1;
        on_modifyItem_triggered();
    }
    else{
        try{
            // ʹ���źźͲ۸��Ӻ�
            auto widget = (AddressItem*)ui->addressList->itemWidget(customMenuRequestItem);
            global::specificItemID = widget->getItemID();
            on_modifyItem_triggered();
        }catch(QString& e){
            QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
        }catch(std::exception& e){
            QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
        }
    }
}

// function: called when user press the menu item "delete item" of custom menu.
void AddressBookWindow::onCustomMenuRequestDeleteItem()
{
    if(!customMenuRequestItem)
        on_deleteItem_triggered();
    else{
        try{
            // ʹ���źźͲ۸��Ӻ�
            auto widget = (AddressItem*)ui->addressList->itemWidget(customMenuRequestItem);
            int itemID = widget->getItemID();

            QSqlQuery query(global::db);
            query.prepare(QString("DELETE FROM %1 WHERE itemID = ?").arg(global::tableAddressItemName));
            query.addBindValue(itemID);
            if(!query.exec())
                throw QString(STRING("ɾ����ϵ��ʧ�ܣ�") + query.lastError().text());

            auto groupPtr = (AddressGroupItem*)groupList.value(widget->getType());
            groupPtr->RemoveItem(itemID);
            ui->addressList->takeItem(ui->addressList->row(customMenuRequestItem));
            customMenuRequestItem = NULL;
        }catch(QString& e){
            QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
        }catch(std::exception& e){
            QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
        }
    }
}

// function: called when user finish edit the type name.
// note: �������༭��ɲ�
void AddressBookWindow::onEditTypeNameFinished()
{
    try{
        typeNameEdit->hide();

        if(!currentEditingTypeItem)
            throw "";

        auto newTypeName = typeNameEdit->text();
        if(newTypeName.isEmpty())return;
        auto originalTypeName = ((AddressGroupItem*)currentEditingTypeItem)->getTypeName();
        if(originalTypeName == newTypeName)
            return;
        ((AddressGroupItem*)currentEditingTypeItem)->setTypeName(typeNameEdit->text());

        QSqlQuery query(global::db);
        query.prepare(QString("UPDATE %1 SET typeName = ? WHERE typeName = ?")
                      .arg(global::tableAddressTypeName));
        query.addBindValue(newTypeName);
        query.addBindValue(originalTypeName);

        if(!query.exec())
            throw QString(STRING("�޸�������ʧ�� ") + query.lastError().text());

        // ����ӳ��
        groupList.remove(originalTypeName);
        groupList.insert(newTypeName, currentEditingTypeItem);
        int typeID = global::typeNameTotypeID.value(originalTypeName);
        global::typeNameTotypeID.remove(originalTypeName);
        global::typeNameTotypeID.insert(newTypeName, typeID);

        if(!presentResultWnd->isHidden())
            emit addNewType(newTypeName);

        currentEditingTypeItem = NULL;

    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
    }catch(std::exception& e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }catch(...){
        // ���ô�����쳣
    }
}

// function: called by tiemr to hide the detail panel
// when cursor didn't point to any item
void AddressBookWindow::CheckHasCursorPointToAnyItem()
{
    try{
        auto item = ui->addressList->itemAt(ui->addressList->mapFromGlobal(QCursor::pos()));
        if(!groupList.values().contains(item)){
            if(!typeDetailPanel->isHidden())
                typeDetailPanel->hide();
        }
        bool isAddressItem = false;
        for(auto typePtr : groupList.values()){
            if(((AddressGroupItem*)typePtr)->ContainItem(item)){
                isAddressItem = true;
                break;
            }
        }
        if(!isAddressItem)
            if(!detailPanel->isHidden())
                detailPanel->hide();
    }catch(std::exception& e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }
    catch(...){
        QMessageBox::warning(NULL, "warning", STRING("��������"), QMessageBox::Ok);
    }
}

// function: called when received the self defination signal onTriggered(QString)
//           this will figure out which menu item user has choose
void AddressBookWindow::onMoveToSubMenuTriggered(QString typeName)
{
    try{
        isModifyingItem = true;
        if(!customMenuRequestItem)
            throw QString(STRING("�ƶ���ϵ��ʧ��"));
        auto widget = (AddressItem*)ui->addressList->itemWidget(customMenuRequestItem);
        int originalTypeID = global::typeNameTotypeID.value(widget->getType());
        int newTypeID = global::typeNameTotypeID.value(typeName);

        if(originalTypeID == newTypeID){
            customMenuRequestItem = NULL;
            isModifyingItem = false;
            return;
        }

        auto originalTypePtr = (AddressGroupItem*)groupList.value(widget->getType());
        auto newTypePtr = (AddressGroupItem*)groupList.value(typeName);

        QSqlQuery query(global::db);
        query.prepare(QString("UPDATE %1 SET type = ? WHERE type = ? AND itemID = ?").arg(global::tableAddressItemName));
        query.addBindValue(newTypeID);
        query.addBindValue(originalTypeID);
        query.addBindValue(widget->getItemID());

        if(!query.exec())
            throw QString(STRING("�ƶ���ϵ��ʧ�ܣ�") + query.lastError().text());

        AddAddressItem(newTypePtr, widget->getItemID(), widget->getName(), typeName,
                       widget->getSex(), widget->getPhone(), widget->getEmail(),
                       widget->getNote(), widget->getAvatarImg());

        ui->addressList->takeItem(ui->addressList->row(customMenuRequestItem));
        originalTypePtr->RemoveItem(widget->getItemID());
        customMenuRequestItem = NULL;
        isModifyingItem = false;
    }catch(QString& e){
        QMessageBox::warning(NULL, "warning", e, QMessageBox::Ok);
    }catch(std::exception& e){
        QMessageBox::warning(NULL, "warning", e.what(), QMessageBox::Ok);
    }

}

// function: according to the type of address item to
//           generate the unique custom menu
void AddressBookWindow::GenerateItemCustomMenu(QListWidgetItem* selectedItem)
{
    auto widget = (AddressItem*)ui->addressList->itemWidget(selectedItem);
    auto groupName = widget->getType();
    QMenu* itemMenu = new QMenu();
    QAction* seperator = new QAction(this);
    seperator->setSeparator(true);
    QMenu* moveToSubMenu = new QMenu();
    for(auto typeName : global::typeNameTotypeID.keys())
    {
        if(typeName != groupName){
            MyMenuItem* action = new MyMenuItem(typeName);
            connect(action, SIGNAL(onTriggered(QString)), this, SLOT(onMoveToSubMenuTriggered(QString)));
            moveToSubMenu->addAction(action);
        }
    }
    moveToSubMenu->setTitle(STRING("�ƶ���"));

    QAction* modifiedItemMenuItem = new QAction(STRING("�޸���ϵ��"), this);
    QAction* deleteItemMenuItem = new QAction(STRING("ɾ����ϵ��"), this);
    connect(modifiedItemMenuItem, SIGNAL(triggered()),
            this, SLOT(onCustomMenuRequestModifyItem()));
    connect(deleteItemMenuItem, SIGNAL(triggered()),
            this, SLOT(onCustomMenuRequestDeleteItem()));

    itemMenu->addMenu(moveToSubMenu);
    //itemMenu->addActions({seperator, modifiedItemMenuItem, deleteItemMenuItem});
    QAction* list2[] = {seperator, modifiedItemMenuItem, deleteItemMenuItem};
    QList<QAction*> list_2;
    for(auto a :list2)
        list_2.append(a);
    itemMenu->addActions(list_2);
    itemMenu->exec(QCursor::pos());
}
