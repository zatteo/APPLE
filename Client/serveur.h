#ifndef SERVEUR_H
#define SERVEUR_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

class Serveur: public QObject
{
    Q_OBJECT
public:
    explicit Serveur(QObject *parent = 0);
    ~Serveur();
    void connectMPV(QString adresse);
    void loadAndPlayMPV(QString nomDuFichier); // charge un fichier et lance la lecture sur MPV
    void playMPV(bool play); // met en play/pause la lecture sur MPV
    void volumeMPV(int volume); // change le volume sur MPV
    void muteMPV(bool mute); // mute le volume sur MPV
    void positionMPV(int position); // change la position de la musique sur MPV

private slots:
    void readSocket();

private:
    QLocalSocket *socketMPV = NULL;
};

#endif // SERVEUR_H
