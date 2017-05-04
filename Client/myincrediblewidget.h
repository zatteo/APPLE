#ifndef MyIncredibleWidget_H
#define MyIncredibleWidget_H

#include <QWidget>
#include <QDialog>
#include <QtGui>
#include <QApplication>

class MyIncredibleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MyIncredibleWidget(QWidget *parent = 0, QJsonArray array = {});
signals:

public slots:

private slots:
    void create(QJsonArray array);

};

#endif // MyIncredibleWidget_H
