/* AUTHOR : SUTTER Nicolas et POIZAT Théo
 * L3 CMI ISR
 */

#include "serveurcentral.h"
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ServeurCentral sc;

    return a.exec();
}
