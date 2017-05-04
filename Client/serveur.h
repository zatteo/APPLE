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
#include <QJsonArray>

class MainWindow;

class Serveur: public QObject
{
    Q_OBJECT

public:
    explicit Serveur(QObject *parent = 0);
    ~Serveur();
    void connect(QString adresse);
    void loadAndPlayMPV(QString nomDuFichier); // charge un fichier et lance la lecture
    void loadAndPlayAPlaylistMPV(QString nomDuFichier); // charge une playlist et lance la lecture
    void loadAndPlayARadioMPV(QString nomDuFichier); // charge une radio et lance la lecture
    void playMPV(bool play); // met en play/pause la lecture
    void setVolumeMPV(int volume); // change le volume
    void muteMPV(bool mute); // mute le volume
    void setPositionMPV(int position); // change la position de la musique
    void next(QString currentSong);
    void previous(QString currentSong);
    void setMainWindow(MainWindow *window); // récupére la MainWindow
    void requestAllSongs();
    void requestAllPlaylists();
    void requestAllRadios();

public slots:
    void setVitesseAvantRapide(); // passe en mode avance rapide
    void setVitesseArriereRapide(); // passe en mode retour rapide
    void setVitesseNormale(); // passe en mode vitesse normale
    void getCurrentStateMPV(); // récupèration de l'état actuel du serveur central


private slots:
    void readSocket();

private:
    QLocalSocket *socket = NULL;
    QJsonObject buildACommand(QJsonArray arguments); // construit une commande JSON
    void send(QJsonObject json); // envoie un objet JSON au serveur central
    MainWindow *w;
};

#endif // SERVEUR_H
