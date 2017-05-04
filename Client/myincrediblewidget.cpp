#include "myincrediblewidget.h"
#include <QWidget>
#include <QVBoxLayout>
#include <QTextEdit>

MyIncredibleWidget::MyIncredibleWidget(QWidget *parent, QJsonArray array) :
    QWidget(parent)
{
    create(array);
}

void MyIncredibleWidget::create(QJsonArray array)
{
    QWidget *fenetre = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;

    QTextEdit *text = new QTextEdit;

    // ajout des tags de taglib
    for(int i = 0; i < array.size(); i++)
    {
        text->append(array.at(i).toString());
    }

    layout->addWidget(text);

    fenetre->setLayout(layout);

    fenetre->show();
    fenetre->setWindowTitle("TagLib");
}
