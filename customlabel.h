// created by Brain at 2017.6.20
// * �̳���QLabel�࣬ʵ���˷����������ƶ����Ĺ���
// * ͨ������mouseMoveEvent�¼�ʵ��


#ifndef CUSTOMLABEL_H
#define CUSTOMLABEL_H

#include <QLabel>

class CustomLabel : public QLabel
{
    Q_OBJECT
public:
    CustomLabel(QWidget *parent = 0);
    ~CustomLabel();

signals:
    void onMouseMove(QPoint p);
public slots:

protected:
    void mouseMoveEvent(QMouseEvent *ev);
    void mouseReleaseEvent(QMouseEvent *ev);

private:
    QPoint lastPos;
};

#endif // CUSTOMLABEL_H
