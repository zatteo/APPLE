#include "serveur.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QThread>

Serveur::Serveur(QObject *parent) : QObject(parent), socketMPV(new QLocalSocket(this))
{
    socketMPV->connectToServer("/tmp/mpv-socket"); // nom du socket en statique pour le moment

    connect(socketMPV, SIGNAL(readyRead()), this, SLOT(readSocket()));

    if(socketMPV->waitForConnected())
        qDebug() << "connected to mpv";
    else {
        socketMPV->error();
    }

    loadAndPlayMPV("/home/zatteo/Music/Augenbling Respect.mp3");
}

Serveur::~Serveur() {
    socketMPV->disconnectFromServer();
}

/* charge un fichier et lance la lecture sur MPV
 * nomDuFichier : nom du fichier à charger sur MPV
 */
void Serveur::loadAndPlayMPV(QString nomDuFichier){
    QJsonObject commandeMPV; // objet qui contient la commande

    // on construit la commande
    QJsonArray commandeTemporaireMPV;
    commandeTemporaireMPV.append("loadfile"); // nom de la commande
    commandeTemporaireMPV.append(nomDuFichier); // argument(s)

    commandeMPV["command"] = commandeTemporaireMPV;

    QByteArray bytes = QJsonDocument(commandeMPV).toJson(QJsonDocument::Compact) + "\n";
    if(socketMPV != NULL) {
      socketMPV->write(bytes.data(), bytes.length());
    }
}

/* met en play/pause la lecture sur MPV
 * bool play : play = true, pause = false
 */
void Serveur::playMPV(bool play)
{
    QJsonObject commandeMPV; // objet qui contient la commande

    // on construit la commande
    QJsonArray commandeTemporaireMPV;
    commandeTemporaireMPV.append("set_property"); // nom de la commande
    commandeTemporaireMPV.append("pause"); // argument(s)
    commandeTemporaireMPV.append(!play);

    commandeMPV["command"] = commandeTemporaireMPV;

    QByteArray bytes = QJsonDocument(commandeMPV).toJson(QJsonDocument::Compact) + "\n";
    if(socketMPV != NULL) {
      socketMPV->write(bytes.data(), bytes.length());
    }
}

/* change le volume sur MPV
 * int volume : nouveau volume 0 < volume < 100
 */
void Serveur::volumeMPV(int volume)
{
    if(volume > 100)
        volume = 100;
    else if(volume < 0)
        volume = 0;

    QJsonObject commandeMPV; // objet qui contient la commande

    // on construit la commande
    QJsonArray commandeTemporaireMPV;
    commandeTemporaireMPV.append("set_property"); // nom de la commande
    commandeTemporaireMPV.append("volume"); // argument(s)
    commandeTemporaireMPV.append(volume);

    commandeMPV["command"] = commandeTemporaireMPV;

    QByteArray bytes = QJsonDocument(commandeMPV).toJson(QJsonDocument::Compact) + "\n";
    if(socketMPV != NULL) {
      socketMPV->write(bytes.data(), bytes.length());
    }
}

/* mute le volume sur MPV
 * bool mute
 */
void Serveur::muteMPV(bool mute)
{
    QJsonObject commandeMPV; // objet qui contient la commande

    // on construit la commande
    QJsonArray commandeTemporaireMPV;
    commandeTemporaireMPV.append("set_property"); // nom de la commande
    commandeTemporaireMPV.append("mute"); // argument(s)
    commandeTemporaireMPV.append(mute);

    commandeMPV["command"] = commandeTemporaireMPV;

    QByteArray bytes = QJsonDocument(commandeMPV).toJson(QJsonDocument::Compact) + "\n";
    if(socketMPV != NULL) {
      socketMPV->write(bytes.data(), bytes.length());
    }
}

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
