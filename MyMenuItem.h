// Created by Brian at 2017.6.19
// �̳�QAction�࣬ʵ���˵��˵�����ʱ�����Լ�������

#ifndef MYMENUITEM_H
#define MYMENUITEM_H
#include <QAction>
#include <QString>


class MyMenuItem : public QAction
{
    Q_OBJECT // Ҫ���Զ���Ĳۺ��ź���Ч��������������

public:
    MyMenuItem(QObject *parent = nullptr);
    MyMenuItem(const QString &text, QObject *parent = nullptr);
    MyMenuItem(const QIcon &icon, const QString &text, QObject *parent = nullptr);
    ~MyMenuItem();

signals:
    void onTriggered(QString menuItemName);

private slots:
    void onTriggeredSlot(); // ��QActon��triggered�źź�onTriggeredSlot������
    // onTriggeredSlot�ٷ���onTriggered�ź�

private:
    QString menuName;
};

#endif // MYMENUITEM_H
