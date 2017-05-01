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

    // on s'inscrit aux changements d'états pour gérer le multi-utilisateur
    subscribeChangingStateMPV();

    // test
    loadAndPlayMPV("/home/zatteo/Music/Augenbling Respect.mp3");
}

Serveur::~Serveur() {
    socket->disconnectFromServer();
}

/* charge un fichier et lance la lecture
 * nomDuFichier
 */
void Serveur::loadAndPlayMPV(QString nomDuFichier)
{
    QJsonObject commandeMPV = buildACommand({"loadfile", nomDuFichier});

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
    QString positionFormatee = "+" + QString::number(position);

    QJsonObject commandeMPV = buildACommand({"set_property", "start", positionFormatee});

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
    qDebug() << "Envoi d'un message au serveur central";

    QByteArray bytes = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    if(socket != NULL) {
      socket->write(bytes.data(), bytes.length());
    }
}

/* récupèration de l'état actuel de MPV
 */
void Serveur::getCurrentStateMPV()
{
    // TODO : dépend de la machine à état
}

/* inscription aux changements d'états de MPV
*/
void Serveur::subscribeChangingStateMPV()
{
    send(buildACommand({"observe_property", 1, "pause"})); // play
    send(buildACommand({"observe_property", 1, "volume"})); // volume
    send(buildACommand({"observe_property", 1, "mute"})); // mute
    send(buildACommand({"observe_property", 1, "start"})); // position
}

/* récupére la MainWindow
 */
void Serveur::setMainWindow(MainWindow *window)
{
    w = window;
}
