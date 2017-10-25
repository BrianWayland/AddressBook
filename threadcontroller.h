// created by Brian at 2017.6.15
// * ����ͷ�����������

#ifndef THREADCONTROLLER_H
#define THREADCONTROLLER_H

#include <QObject>
#include <QThread>

/// ����ͷ��������ӵ���
class ServerConnectionManager : public QObject
{
      Q_OBJECT

  public slots:
    /// �������ӷ����������ͷ������Ͽ�����ʱ���߷�������ʱ����
      void TryToConnectToServer();

  signals:
      /// ��TryToConnectToServerִ����ɺ󷢳�����ź�
      void connectFinish();
};

class Controller : public QObject
{
    Q_OBJECT
    QThread workerThread;
public:
    Controller();
    ~Controller();

signals:
   /// ��ServerConnectionManager�Ĳ�TryToConnectToServer����һ��
   /// Ҫ�������ݿ�ͷ�������ź�
    void onTryToConnectServer();

private slots:
    void onConnectFinish();

    /// �������̵߳��źţ��൱��һ���н�
signals:
    void onLogInFeedback();
    void onRegisterFeedback();
};

#endif // THREADCONTROLLER_H
