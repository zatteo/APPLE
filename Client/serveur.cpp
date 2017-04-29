#include "serveur.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>

Serveur::Serveur(QObject *parent) : QObject(parent), socketMPV(new QLocalSocket(this)){}

/* se connecte à MPV de manière synchrone (toutes les autres commandes sont asynchrones)
 * QString adresse
 */
void Serveur::connectMPV(QString adresse)
{
    socketMPV->connectToServer(adresse); // connexion au serveur

    connect(socketMPV, SIGNAL(readyRead()), this, SLOT(readSocket()));

    if(socketMPV->waitForConnected())
        qDebug() << "Connecté à MPV :" << adresse;
    else {
        socketMPV->error();
    }

    // on récupère l'état actuel du lecteur
    getCurrentStateMPV();

    // on s'inscrit aux changements d'états pour gérer le multi-utilisateur
    subscribeChangingStateMPV();

    // test
    loadAndPlayMPV("/home/zatteo/Music/Augenbling Respect.mp3");
}

Serveur::~Serveur() {
    socketMPV->disconnectFromServer();
}

/* charge un fichier et lance la lecture sur MPV
 * nomDuFichier
 */
void Serveur::loadAndPlayMPV(QString nomDuFichier)
{
    QJsonObject commandeMPV = buildACommandForMPV({"loadfile", nomDuFichier});

    sendToMPV(commandeMPV);
}

/* met en play/pause la lecture sur MPV
 * bool play
 */
void Serveur::playMPV(bool play)
{
    QJsonObject commandeMPV = buildACommandForMPV({"set_property", "pause", !play});

    sendToMPV(commandeMPV);
}

/* change le volume sur MPV
 * int volume : 0 < volume < 100
 */
void Serveur::setVolumeMPV(int volume)
{
    if(volume > 100)
        volume = 100;
    else if(volume < 0)
        volume = 0;

    QJsonObject commandeMPV = buildACommandForMPV({"set_property", "volume", volume});

    sendToMPV(commandeMPV);
}

/* mute le volume sur MPV
 * bool mute
 */
void Serveur::muteMPV(bool mute)
{
    QJsonObject commandeMPV = buildACommandForMPV({"set_property", "mute", mute});

    sendToMPV(commandeMPV);
}

/* change la position de la musique sur MPV
 * int position : en seconde
 */
void Serveur::setPositionMPV(int position)
{
    QString positionFormatee = "+" + QString::number(position);

    QJsonObject commandeMPV = buildACommandForMPV({"set_property", "start", positionFormatee});

    sendToMPV(commandeMPV);
}

/* lecture du socket et action nécessaire
 */
void Serveur::readSocket()
{
    while (socketMPV->canReadLine()) {
        QByteArray line = socketMPV->readLine().trimmed();

        QJsonParseError error;
        QJsonObject retourMPV = QJsonDocument::fromJson(line, &error).object();
        qDebug() << retourMPV;

        // ici on a reçu une réponse de MPV
        // -> appeler une fonction update qui met à jour l'interface et l'état du programme
    }
}

/* construit une commande JSON pour MPV
 * QList arguments : la liste des arguments de la commande
 */
QJsonObject Serveur::buildACommandForMPV(QJsonArray arguments)
{
    QJsonObject commandeMPV; // objet qui contient la commande

    commandeMPV["command"] = arguments;

    return commandeMPV;
}

/* envoie un objet JSON à MPV
 * QJsonObject json : l'objet à envoyer qui est formaté pour MPV
 */
void Serveur::sendToMPV(QJsonObject json)
{
    QByteArray bytes = QJsonDocument(json).toJson(QJsonDocument::Compact) + "\n";
    if(socketMPV != NULL) {
      socketMPV->write(bytes.data(), bytes.length());
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
    sendToMPV(buildACommandForMPV({"observe_property", 1, "pause"})); // play
    sendToMPV(buildACommandForMPV({"observe_property", 1, "volume"})); // volume
    sendToMPV(buildACommandForMPV({"observe_property", 1, "mute"})); // mute
    sendToMPV(buildACommandForMPV({"observe_property", 1, "start"})); // position
}
