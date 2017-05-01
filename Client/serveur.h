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
    void connect(QString adresse);
    void loadAndPlayMPV(QString nomDuFichier); // charge un fichier et lance la lecture sur le serveur central
    void playMPV(bool play); // met en play/pause la lecture sur le serveur central
    void setVolumeMPV(int volume); // change le volume sur le serveur central
    void muteMPV(bool mute); // mute le volume sur le serveur central
    void setPositionMPV(int position); // change la position de la musique sur le serveur central
    void setMainWindow(MainWindow *window); // récupére la MainWindow

private slots:
    void readSocket();

private:
    QLocalSocket *socket = NULL;
    QJsonObject buildACommand(QJsonArray arguments); // construit une commande JSON
    void send(QJsonObject json); // envoie un objet JSON au serveur central
    void getCurrentStateMPV(); // récupèration de l'état actuel du serveur central
    void subscribeChangingStateMPV(); // inscription aux changements d'états du serveur central
    MainWindow *w;
};

#endif // SERVEUR_H
