/* AUTHOR : SUTTER Nicolas et POIZAT Théo
 * L3 CMI ISR
 */

#include "serveurcentral.h"
#include "taglib/tag.h"
#include "taglib/fileref.h"
#include "taglib/tpropertymap.h"
#include "taglib/mpegfile.h"
#include "taglib/id3v2tag.h"
#include "taglib/id3v2frame.h"
#include "taglib/attachedpictureframe.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QBuffer>

ServeurCentral::ServeurCentral(QObject *parent) : QObject(parent), socketMPV(new QLocalSocket(this))
{
    // chargements des morceaux disponibles
    loadLocalFiles();

    serveurClient = new QLocalServer();
    serveurClient->setSocketOptions(QLocalServer::UserAccessOption);
    serveurClient->listen("/tmp/socketClient");

    QObject::connect(serveurClient, SIGNAL(newConnection()), this, SLOT(clientConnected()));

    // on se connecte à MPV
    connect(socketMPV, "/tmp/socketMPV");
    subscribeChangingStateMPV();
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

/* connexion d'un client, paramétrage des signaux et ajout dans la liste des clients
 */
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

/* déconnexion d'un client et suppression de la liste des clients
 */
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
        qDebug() << "client :" << retourClient;

        if(retourClient["event"].toString() == "request")
        {            
            // on renvoie la réponse au client qui demande des informations

            if(retourClient["name"].toString() == "songs") // liste des musiques demandées
            {
                // on formate la requête de réponse
                QJsonObject songsParsed;
                songsParsed["event"] = "response";
                songsParsed["name"] = "songs";
                songsParsed["data"] = songs;

                send(socket, songsParsed);
            }
            else if(retourClient["name"].toString() == "playlists") // liste des playlists demandées
            {
                // on formate la requête de réponse
                QJsonObject playlistsParsed;
                playlistsParsed["event"] = "response";
                playlistsParsed["name"] = "playlists";
                playlistsParsed["data"] = playlists;

                send(socket, playlistsParsed);
            }
            else if(retourClient["name"].toString() == "radios") // liste des radios demandées
            {
                // on formate la requête de réponse
                QJsonObject radiosParsed;
                radiosParsed["event"] = "response";
                radiosParsed["name"] = "radios";
                radiosParsed["data"] = radios;

                send(socket, radiosParsed);
            }
            else if(retourClient["name"].toString() == "song") // demande des métadonnées d'un morceau
            {
                songRequested(socket, retourClient["data"].toString());
            }
            else if(retourClient["name"].toString() == "playlist") // demande de la pochette d'un morceau
            {
                playlistRequested(socket, retourClient["data"].toString());
            }
            else if(retourClient["name"].toString() == "cover") // demande de la pochette d'un morceau
            {
                coverRequested(socket, retourClient["data"].toString());
            }
        }
        else
        {
            if(retourClient["command"].isArray() && retourClient["command"].toArray().at(0).toString() == "loadfile")
            {
                // on formate la nouvelle commande avec le chemin des musiques
                QString newPath = songsPath + "/" + retourClient["command"].toArray().at(1).toString();
                send(socketMPV, buildACommand({"loadfile", newPath}));
            }
            else if(retourClient["command"].isArray() && retourClient["command"].toArray().at(0).toString() == "loadlist")
            {                
                // on formate la nouvelle commande avec le chemin des musiques
                QString newPath = songsPath + "/" + retourClient["command"].toArray().at(1).toString();
                send(socketMPV, buildACommand({"loadlist", newPath}));
            }
            else
            {
                // on retransmet directement à MPV
                send(socketMPV, retourClient);
            }
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

/* requête pour un morceau
 */
void ServeurCentral::songRequested(QLocalSocket *socket, QString title)
{
    // on vérifie si le morceau est dans la liste
    for(int i = 0; i < songs.size(); i++)
    {
        QJsonObject currentSong = songs.at(i).toObject();

        if(currentSong["title"].toString() == title)
        {
            // on construit le chemin local du morceau
            QString newPath = songsPath + "/" + currentSong.value("title").toString();

            // on récupère les métadonnées et on les insère dans le morceau si elles existent pas
            if(currentSong.value("taglib") == QJsonValue::Undefined)
            {
                QJsonObject newTags = getTags(newPath);
                currentSong["taglib"] = newTags;
                songs.replace(i, currentSong);
            }

            // on formate la réponse et on envoie les métadonnées
            QJsonObject songParsed;
            songParsed["event"] = "response";
            songParsed["name"] = "song";
            songParsed["data"] = currentSong;

            send(socket, songParsed);

            qDebug() << "songParsed :" << songParsed;

            // on lance la musique
            send(socketMPV, buildACommand({"loadfile", newPath}));
        }
    }
}

/* requête pour la pochette
 */
void ServeurCentral::coverRequested(QLocalSocket *socket, QString title)
{
    // on vérifie si le morceau est dans la liste
    for(int i = 0; i < songs.size(); i++)
    {
        QJsonObject currentSong = songs.at(i).toObject();

        if(currentSong["title"].toString() == title)
        {
            // on construit le chemin local du morceau
            QString newPath = songsPath + "/" + currentSong.value("title").toString();

            // on récupère la cover et on les insère dans le morceau si elle existe pas
            if(currentSong.value("cover") == QJsonValue::Undefined)
            {
                QJsonObject newCover = getCover(newPath);
                currentSong["cover"] = newCover;
                songs.replace(i, currentSong);
            }

            // on formate la réponse et on envoie la cover
            QJsonObject coverParsed;
            coverParsed["event"] = "response";
            coverParsed["name"] = "cover";
            coverParsed["data"] = currentSong["cover"];

            send(socket, coverParsed);
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
void ServeurCentral::loadLocalFiles()
{
    // on récupère les différents fichiers
    QStringList songsList, playlistsList, radiosList;

    // on s'adapte au dossier par défaut local en fonction de la langue et on récupère les fichiers
    if(QDir(QDir::home().absolutePath() + "/Music").exists())
    {
        songsList = QDir(QDir::home().absolutePath() + "/Music").entryList();
        playlistsList = QDir(QDir::home().absolutePath() + "/Music/Playlists").entryList();
        radiosList = QDir(QDir::home().absolutePath() + "/Music/Radios").entryList();
        songsPath = QDir::home().absolutePath() + "/Music";
    }
    else if(QDir(QDir::home().absolutePath() + "/Musique").exists())
    {
        songsList = QDir(QDir::home().absolutePath() + "/Musique").entryList();
        playlistsList = QDir(QDir::home().absolutePath() + "/Musique/Playlists").entryList();
        radiosList = QDir(QDir::home().absolutePath() + "/Musique/Radios").entryList();
        songsPath = QDir::home().absolutePath() + "/Musique";
    }

    // on ajoute seulement les fichiers qui correspondent à des morceaux, des playlists, des radios
    for(int i = 0; i < songsList.size(); i++)
    {
        if(isValidSong(songsList.at(i)))
        {
            songs.append(QJsonObject({{"title", songsList.at(i)}}));
        }
    }
    for(int i = 0; i < playlistsList.size(); i++)
    {
        if(isValidPlaylist(playlistsList.at(i)))
        {
            playlists.append(QJsonObject({{"title", playlistsList.at(i)}}));
        }
    }
    for(int i = 0; i < radiosList.size(); i++)
    {
        if(isValidPlaylist(radiosList.at(i)))
        {
            radios.append(QJsonObject({{"title", radiosList.at(i)}}));
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

/* vérifie si le nom du fichier correspond à radio
 */
bool ServeurCentral::isValidPlaylist(QString radio)
{
    if(radio.endsWith(".pls"))
    {
        return true;
    }
    else if(radio.endsWith(".m3u"))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/* utilise taglib pour récupérer les métadonnées
 */
QJsonObject ServeurCentral::getTags(QString fileName)
{
    QJsonObject newTags;

    // on récupère les tags
    TagLib::FileRef f(fileName.toLatin1().data());
    if(!f.isNull() && f.tag())
    {
        TagLib::PropertyMap tags = f.file()->properties();
        for(TagLib::PropertyMap::ConstIterator i=tags.begin(); i != tags.end(); ++i)
        {
            for(TagLib::StringList::ConstIterator j=i->second.begin(); j!=i->second.end(); ++j)
            {
                newTags[QString::fromStdString(i->first.to8Bit(true))] = QString::fromStdString(j->to8Bit(true));
            }
        }
    }

    newTags["duration"] = f.audioProperties()->lengthInSeconds();

    return newTags;
}

/* utilise taglib pour récupérer la cover
 */
QJsonObject ServeurCentral::getCover(QString fileName)
{
    QJsonObject newCover;

    // on récupère la cover
    if(fileName.endsWith(".mp3"))
    {
        TagLib::MPEG::File file(fileName.toLatin1().data());
        TagLib::ID3v2::Tag *m_tag = file.ID3v2Tag(true);
        TagLib::ID3v2::FrameList frameList = m_tag->frameList("APIC");

        // s'il y a aucune image, on s'arrête
        if(frameList.isEmpty())
        {
           return newCover;
        }

        TagLib::ID3v2::AttachedPictureFrame *coverImg = static_cast<TagLib::ID3v2::AttachedPictureFrame *>(frameList.front());

        // sinon on encode
        QImage coverQImg;
        coverQImg.loadFromData((const uchar *) coverImg->picture().data(), coverImg->picture().size());
        coverQImg = coverQImg.scaled(1131, 581, Qt::KeepAspectRatio);

        QFileInfo fileInfo(fileName);

        newCover["title"] = fileInfo.fileName();
        newCover["cover"] = jsonValFromImage(coverQImg);
    }

    return newCover;
}

/* encodage d'une image pour JSON
 */
QJsonValue ServeurCentral::jsonValFromImage(const QImage & p)
{
  QByteArray data;

  QBuffer buffer{&data};
  buffer.open(QIODevice::WriteOnly);

  p.save(&buffer, "PNG"); // convention = PNG

  auto encoded = buffer.data().toBase64(); // convention = base64

  return QJsonValue(QString::fromLatin1(encoded));
}

/* inscription aux changements d'états de MPV
*/
void ServeurCentral::subscribeChangingStateMPV()
{
    send(socketMPV, buildACommand({"observe_property", 1, "pause"})); // play
    send(socketMPV, buildACommand({"observe_property", 2, "volume"})); // volume
    send(socketMPV, buildACommand({"observe_property", 3, "mute"})); // mute
    send(socketMPV, buildACommand({"observe_property", 4, "start"})); // utile ?
    send(socketMPV, buildACommand({"observe_property", 5, "time-pos"}));
    send(socketMPV, buildACommand({"observe_property", 6, "filename"}));
}

// récupération des morceaux d'une playlist
void ServeurCentral::playlistRequested(QLocalSocket *socket, QString playlist)
{
    QString playlistPath = songsPath + "/" + playlist;

    QJsonObject response;
    response["event"] = "response";
    response["name"] = "playlist";
    QJsonArray responseArray;

    qDebug() << playlistPath;

    QFile inputFile(playlistPath);
    if(inputFile.open(QIODevice::ReadOnly))
    {
       QTextStream in(&inputFile);
       while (!in.atEnd())
       {
          QString line = in.readLine();
          if(!line.startsWith("#EXT"))
          {
            QString tmp = line.section('/', -1);
            responseArray.append(tmp);
          }
       }
       inputFile.close();
    }

    response["data"] = responseArray;

    qDebug() << response;

    send(socket, response);
}
