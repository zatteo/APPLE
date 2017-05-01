#include "serveurcentral.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

ServeurCentral::ServeurCentral(QObject *parent) : QObject(parent), socketMPV(new QLocalSocket(this))
{
    serveurClient = new QLocalServer();
    serveurClient->setSocketOptions(QLocalServer::UserAccessOption);
    serveurClient->listen("/tmp/socketClient");

    QObject::connect(serveurClient, SIGNAL(newConnection()), this, SLOT(clientConnected()));

    // on se connecte à MPV
    connect(socketMPV, "/tmp/socketMPV");
    QObject::connect(socketMPV, SIGNAL(readyRead()), this, SLOT(readSocketMPV()));
}

void ServeurCentral::connect(QLocalSocket *socket, QString adresse)
{
    socket->connectToServer(adresse); // connexion au serveur

    if(socket->waitForConnected())
        qDebug() << "Connecté à :" << adresse;
    else {
        socket->error();
    }
}

void ServeurCentral::clientConnected()
{
    QLocalSocket *socket = serveurClient->nextPendingConnection();

    if (!socket)
        return;

    socketsClients.append(socket);

    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readSocketClient()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));

    qDebug() << "Un client s'est connecté";
}

void ServeurCentral::clientDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (!socket)
        return;

    socketsClients.removeOne(socket);

    socket->deleteLater();

    qDebug() << "Un client s'est déconnecté";
}

ServeurCentral::~ServeurCentral() {
    // déconnecter tous les clients restants
    socketMPV->disconnectFromServer();
}

/* lecture du socket client et action nécessaire
 */
void ServeurCentral::readSocketClient()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    qDebug() << "Message reçu du client.";

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();

        QJsonParseError error;
        QJsonObject retourClient = QJsonDocument::fromJson(line, &error).object();

        qDebug() << retourClient << "retransmis à MPV.";

        if(retourClient["event"] == "request")
        {
            // on renvoie la réponse au client qui demande des informations
            if(retourClient["name"] == "songs")
            {
                send(socket, songs);
            }
            else if(retourClient["name"] == "playlists")
            {
                send(socket, playlists);
            }
            else if(retourClient["name"] == "radios")
            {
                send(socket, radios);
            }
            else if(retourClient["name"] == "song")
            {
                songRequested(retourClient["title"].toString());
            }
        }
        else
        {
            // on retransmet directement à MPV
            send(socketMPV, retourClient);
        }
    }
}

/* lecture du socket MPV et action nécessaire
 */
void ServeurCentral::readSocketMPV()
{
    qDebug() << "Message reçu de MPV.";

    while (socketMPV->canReadLine()) {
        QByteArray line = socketMPV->readLine().trimmed();

        QJsonParseError error;
        QJsonObject retourMPV = QJsonDocument::fromJson(line, &error).object();
        qDebug() << retourMPV;


        qDebug() << retourMPV << "retransmis au client.";

        for(int i = 0; i < socketsClients.size(); i++)
        {
            send(socketsClients.at(i), retourMPV);
        }
    }
}

void songRequested(QString title)
{
    // taglib
}

/* construit une commande JSON
 * QList arguments : la liste des arguments de la commande
 */
QJsonObject ServeurCentral::buildACommand(QJsonArray arguments)
{
    QJsonObject commandeMPV; // objet qui contient la commande

    commandeMPV["command"] = arguments;

    return commandeMPV;
}

/* envoie un objet JSON
 * QJsonObject json : l'objet à envoyer qui est formaté
 */
void ServeurCentral::send(QLocalSocket *socket, QJsonObject json)
{
    QByteArray bytes = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    if(socket != NULL) {
      socket->write(bytes.data(), bytes.length());
    }
}
