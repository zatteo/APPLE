#ifndef SERVEURCENTRAL_H
#define SERVEURCENTRAL_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QJsonObject>
#include <QJsonArray>

class ServeurCentral: public QObject
{
    Q_OBJECT

public:
    explicit ServeurCentral(QObject *parent = 0);
    ~ServeurCentral();
    void connect(QLocalSocket *socket, QString adresse);

private slots:
    void readSocketClient();
    void readSocketMPV();
    void clientConnected();
    void clientDisconnected();

private:
    QJsonArray songs; // liste des morceaux
    QJsonArray playlists; // liste des playlists
    QJsonArray radios; // liste des radios
    QString songsPath;
    QLocalServer *serveurClient = NULL; // le serveur
    QList<QLocalSocket *> socketsClients; // la liste des clients
    QLocalSocket *socketMPV = NULL;
    QJsonObject buildACommand(QJsonArray arguments); // construit une commande JSON
    void send(QLocalSocket *socket, QJsonObject json); // envoie un objet JSON
    void songRequested(QLocalSocket *socket, QString title);
    void loadLocalSongs();
    bool isValidSong(QString song); // vérifie si le nom du fichier correspond à une musique
    QJsonObject getTags(QString fileName);
};

#endif // SERVEURCENTRAL_H
