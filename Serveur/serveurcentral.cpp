#include "serveurcentral.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>

ServeurCentral::ServeurCentral(QObject *parent) : QObject(parent), socketMPV(new QLocalSocket(this))
{
    // chargements des morceaux disponibles
    loadLocalSongs();

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

    qDebug() << "Un client s'est connecté.";
}

void ServeurCentral::clientDisconnected()
{
    QLocalSocket *socket = qobject_cast<QLocalSocket *>(sender());

    if (!socket)
        return;

    socketsClients.removeOne(socket);

    socket->deleteLater();

    qDebug() << "Un client s'est déconnecté.";
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

    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();

        QJsonParseError error;

        QJsonObject retourClient = QJsonDocument::fromJson(line, &error).object();

        if(retourClient["event"] == "request")
        {
            // on renvoie la réponse au client qui demande des informations
            if(retourClient["name"] == "songs")
            {
                // on formate la requête de réponse
                QJsonObject songsParsed;
                songsParsed["event"] = "response";
                songsParsed["name"] = "songs";
                songsParsed["data"] = songs;

                send(socket, songsParsed);
            }
            else if(retourClient["name"] == "playlists")
            {
                // on formate la requête de réponse
                QJsonObject playlistsParsed;
                playlistsParsed["event"] = "response";
                playlistsParsed["name"] = "playlists";
                playlistsParsed["data"] = playlists;

                send(socket, playlistsParsed);
            }
            else if(retourClient["name"] == "radios")
            {
                // on formate la requête de réponse
                QJsonObject radiosParsed;
                radiosParsed["event"] = "response";
                radiosParsed["name"] = "radios";
                radiosParsed["data"] = radios;

                send(socket, radiosParsed);
            }
            else if(retourClient["name"] == "song")
            {
                songRequested(socket, retourClient["title"].toString());
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
    while (socketMPV->canReadLine()) {
        QByteArray line = socketMPV->readLine().trimmed();

        QJsonParseError error;
        QJsonObject retourMPV = QJsonDocument::fromJson(line, &error).object();
        qDebug() << "MPV :" << retourMPV;

        for(int i = 0; i < socketsClients.size(); i++)
        {
            send(socketsClients.at(i), retourMPV);
        }
    }
}

void ServeurCentral::songRequested(QLocalSocket *socket, QString title)
{
    // on vérifie si le morceau est dans la liste
    for(int i = 0; i < songs.size(); i++)
    {

        QJsonObject currentSong = songs.at(i).toObject();

        if(currentSong.value("title").toString() == title)
        {
            // on ne connaît pas les métadonnées du morceau
            if(currentSong.value("taglib") == QJsonValue::Undefined )
            {
                // on récupère les métadonnées

                // on envoie les métadonnées
                send(socket, currentSong);

                // on lance la musique
                send(socketMPV, buildACommand({"loadfile", songsPath + "/" + currentSong.value("title").toString()}));
            }
            else // on connaît les métadonnées du morceau
            {
                // on envoie les métadonnées
                send(socket, currentSong);

                // on lance la musique
                send(socketMPV, buildACommand({"loadfile", songsPath + "/" + currentSong.value("title").toString()}));
            }
        }
    }
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

/* charge les morceaux disponibles localement
 */
void ServeurCentral::loadLocalSongs()
{
    // on récupère les fichiers
    QStringList list;

    if(QDir(QDir::home().absolutePath() + "/Music").exists())
    {
        list = QDir(QDir::home().absolutePath() + "/Music").entryList();
        songsPath = QDir::home().absolutePath() + "/Music";
    }
    else if(QDir(QDir::home().absolutePath() + "/Musique").exists())
    {
        list = QDir(QDir::home().absolutePath() + "/Musique").entryList();
        songsPath = QDir::home().absolutePath() + "/Musique";
    }

    // on supprime les fichiers qui ne correspondent pas à des morceaux

    list.removeOne(".");
    list.removeOne("..");

    for(int i = 0; i < list.size(); i++)
    {
        if(isValidSong(list.at(i)))
        {
            songs.append(QJsonObject({{"title", list.at(i)}}));
        }
    }
}

/* vérifie si le nom du fichier correspond à une musique
 */
bool ServeurCentral::isValidSong(QString song)
{
    if(song.endsWith(".mp3"))
    {
        return true;
    }
    else if(song.endsWith(".m4a"))
    {
        return true;
    }
    else
    {
        return false;
    }
}
