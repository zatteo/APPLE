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
    QJsonObject songs; // liste des morceaux
    QJsonObject playlists; // liste des playlists
    QJsonObject radios; // liste des radios
    QLocalServer *serveurClient = NULL; // le serveur
    QList<QLocalSocket *> socketsClients; // la liste des clients
    QLocalSocket *socketMPV = NULL;
    QJsonObject buildACommand(QJsonArray arguments); // construit une commande JSON
    void send(QLocalSocket *socket, QJsonObject json); // envoie un objet JSON
    void songRequested(QString title);
}

#endif // SERVEURCENTRAL_H
