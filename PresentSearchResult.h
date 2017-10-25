// modified by Brian at 2017.6.14
// * ɾ����PresentSearchResult�๹�캯����QString�����Լ�userName��Ա

#ifndef PRESENTSEARCHRESULT_H
#define PRESENTSEARCHRESULT_H

#include <QMainWindow>
#include <QString>

namespace Ui {
class PresentSearchResult;
}

class PresentSearchResult : public QMainWindow
{
    Q_OBJECT

public:
    explicit PresentSearchResult(QWidget *parent = 0);
    ~PresentSearchResult();

    // ÿ����ʾ����ʱ��������
    void UpdateUI();

public slots:
    void onAddNewType(QString& newType);
    void onDeleteType(QString& deletedType);

private slots:
    void on_search_clicked();

private:
    Ui::PresentSearchResult *ui;
};

#endif // PRESENTSEARCHRESULT_H
