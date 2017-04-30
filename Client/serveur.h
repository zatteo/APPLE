#ifndef SERVEUR_H
#define SERVEUR_H

#include "mainwindow.h"
#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QStateMachine>
#include <QState>
#include <QHistoryState>
#include <QFinalState>

class MainWindow;

class Serveur: public QObject
{
    Q_OBJECT

public:
    explicit Serveur(QObject *parent = 0);
    ~Serveur();
    void connectMPV(QString adresse);
    void loadAndPlayMPV(QString nomDuFichier); // charge un fichier et lance la lecture sur MPV
    void playMPV(bool play); // met en play/pause la lecture sur MPV
    void setVolumeMPV(int volume); // change le volume sur MPV
    void muteMPV(bool mute); // mute le volume sur MPV
    void setPositionMPV(int position); // change la position de la musique sur MPV
    void setMainWindow(MainWindow *window); // récupére la MainWindow

private slots:
    void readSocket();

private:
    QLocalSocket *socketMPV = NULL;
    QJsonObject buildACommandForMPV(QJsonArray arguments); // construit une commande JSON pour MPV
    void sendToMPV(QJsonObject json); // envoie un objet JSON à MPV
    void getCurrentStateMPV(); // récupèration de l'état actuel de MPV
    void subscribeChangingStateMPV(); // inscription aux changements d'états de MPV
    MainWindow *w;
};

#endif // SERVEUR_H
