#include "serveur.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

Serveur::Serveur(QObject *parent) : QObject(parent), socket(new QLocalSocket(this)){}

/* se connecte au serveur central de manière synchrone (toutes les autres commandes sont asynchrones)
 * QString adresse
 */
void Serveur::connect(QString adresse)
{
    socket->connectToServer(adresse); // connexion au serveur

    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readSocket()));

    if(socket->waitForConnected())
        qDebug() << "Connecté au serveur central :" << adresse;
    else {
        socket->error();
    }

    // on récupère l'état actuel du lecteur
    getCurrentStateMPV();
}

Serveur::~Serveur() {
    socket->disconnectFromServer();
}

/* charge un fichier et lance la lecture
 * nomDuFichier
 */
void Serveur::loadAndPlayMPV(QString nomDuFichier)
{
    // TODO : rien à faire ici

    // requête des métadonnées
    QJsonObject songParsed;
    songParsed["event"] = "request";
    songParsed["name"] = "song";
    songParsed["data"] = nomDuFichier;

    send(songParsed);

    // requête de la pochette
    QJsonObject songCoverParsed;
    songCoverParsed["event"] = "request";
    songCoverParsed["name"] = "cover";
    songCoverParsed["data"] = nomDuFichier;

    send(songCoverParsed);

    // requête de lancement de la musique
    QJsonObject commandeMPV = buildACommand({"loadfile", nomDuFichier});

    send(commandeMPV);
}

/* charge une playlist et lance la lecture sur le serveur central
 * nomDuFichier
 */
void Serveur::loadAndPlayAPlaylistMPV(QString nomDuFichier)
{
    // requête de lancement de la playlist
    QJsonObject commandeMPV = buildACommand({"loadlist", "Playlists/" + nomDuFichier});

    send(commandeMPV);
}

/* charge une radio et lance la lecture sur le serveur central
 * nomDuFichier
 */
void Serveur::loadAndPlayARadioMPV(QString nomDuFichier)
{
    // requête de lancement de la playlist
    QJsonObject commandeMPV = buildACommand({"loadlist", "Radios/" + nomDuFichier});

    send(commandeMPV);
}

/* met en play/pause la lecture
 * bool play
 */
void Serveur::playMPV(bool play)
{
    QJsonObject commandeMPV = buildACommand({"set_property", "pause", !play});

    send(commandeMPV);
}

/* change le volume
 * int volume : 0 < volume < 100
 */
void Serveur::setVolumeMPV(int volume)
{
    if(volume > 100)
        volume = 100;
    else if(volume < 0)
        volume = 0;

    QJsonObject commandeMPV = buildACommand({"set_property", "volume", volume});

    send(commandeMPV);
}

/* mute le volume
 * bool mute
 */
void Serveur::muteMPV(bool mute)
{
    QJsonObject commandeMPV = buildACommand({"set_property", "mute", mute});

    send(commandeMPV);
}

/* change la position de la musique
 * int position : en seconde
 */
void Serveur::setPositionMPV(int position)
{
    QString positionFormatee = QString::number(position);
    qDebug() << positionFormatee;
    QJsonObject commandeMPV = buildACommand({"set_property", "time-pos", positionFormatee});

    send(commandeMPV);
}

void Serveur::next(QString currentSong)
{
    for(int i = 0; i < w->getSongs().size(); i++)
    {
        if(w->getSongs().at(i).toObject().value("title") == currentSong)
        {
            if(i == w->getSongs().size() - 1)
            {
                loadAndPlayMPV(w->getSongs().at(0).toObject().value("title").toString());
            }
            else
            {
                loadAndPlayMPV(w->getSongs().at(i + 1).toObject().value("title").toString());
            }
        }
    }
}

void Serveur::previous(QString currentSong)
{
    for(int i = 0; i < w->getSongs().size(); i++)
    {
        if(w->getSongs().at(i).toObject().value("title") == currentSong)
        {
            if(i == 0)
            {
                loadAndPlayMPV(w->getSongs().at(w->getSongs().size() - 1).toObject().value("title").toString());
            }
            else
            {
                loadAndPlayMPV(w->getSongs().at(i - 1).toObject().value("title").toString());
            }
        }
    }
}

/* passe en mode avance rapide
 */
void Serveur::setVitesseAvantRapide()
{
    QJsonObject commandeMPV = buildACommand({"set_property", "speed", 50});

    send(commandeMPV);
}

/* passe en mode retour rapide
 */
void Serveur::setVitesseArriereRapide()
{
    QJsonObject commandeMPV = buildACommand({"set_property", "speed", 0.5});

    send(commandeMPV);
}

/* passe en mode vitesse normale
 */
void Serveur::setVitesseNormale()
{
    QJsonObject commandeMPV = buildACommand({"set_property", "speed", 1});

    send(commandeMPV);
}

/* lecture du socket et action nécessaire
 */
void Serveur::readSocket()
{
    while (socket->canReadLine()) {
        QByteArray line = socket->readLine().trimmed();

        QJsonParseError error;
        QJsonObject retourMPV = QJsonDocument::fromJson(line, &error).object();

        qDebug() << retourMPV;

        // ici on a reçu une réponse de MPV
        // -> appeler une fonction update qui met à jour l'interface et l'état du programme
        w->UpdateInt(retourMPV);
    }
}

/* construit une commande JSON
 * QList arguments : la liste des arguments de la commande
 */
QJsonObject Serveur::buildACommand(QJsonArray arguments)
{
    QJsonObject commandeMPV; // objet qui contient la commande

    commandeMPV["command"] = arguments;

    return commandeMPV;
}

/* envoie un objet JSON au serveur central
 * QJsonObject json : l'objet à envoyer qui est formaté pour MPV
 */
void Serveur::send(QJsonObject json)
{
    QByteArray bytes = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    if(socket != NULL) {
      socket->write(bytes.data(), bytes.length());
    }
}

/* fonction pour simplifier la demande des morceaux
 */
void Serveur::requestAllSongs()
{
    QJsonObject commandeMPV;

    commandeMPV["event"] = "request";
    commandeMPV["name"] = "songs";

    send(commandeMPV);
}

/* fonction pour simplifier la demande des playlists
 */
void Serveur::requestAllPlaylists()
{
    QJsonObject commandeMPV;

    commandeMPV["event"] = "request";
    commandeMPV["name"] = "playlists";

    send(commandeMPV);
}

/* fonction pour simplifier la demande des morceaux
 */
void Serveur::requestAllRadios()
{
    QJsonObject commandeMPV;

    commandeMPV["event"] = "request";
    commandeMPV["name"] = "radios";

    send(commandeMPV);
}

/* récupèration de l'état actuel de MPV
 */
void Serveur::getCurrentStateMPV()
{
    QJsonObject jsonPause = buildACommand({"get_property", "pause"});
    jsonPause["request_id"] = 1;
    send(jsonPause);

    QJsonObject jsonFilename = buildACommand({"get_property", "filename"});
    jsonFilename["request_id"] = 2;
    send(jsonFilename);

    QJsonObject jsonMute = buildACommand({"get_property", "mute"});
    jsonMute["request_id"] = 3;
    send(jsonMute);

    QJsonObject jsonVolume = buildACommand({"get_property", "volume"});
    jsonVolume["request_id"] = 4;
    send(jsonVolume);
}

/* récupére la MainWindow
 */
void Serveur::setMainWindow(MainWindow *window)
{
    w = window;
}
