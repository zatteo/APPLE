/* AUTHOR : SUTTER Nicolas et POIZAT Théo
 * L3 CMI ISR
 */

#include "mainwindow.h"
#include "serveur.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    MainWindow w;
    w.setMainWindow(&w);
    w.show();

    return a.exec();
}
