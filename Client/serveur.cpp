#include "serveur.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDataStream>

Serveur::Serveur(QObject *parent) : QObject(parent), socketMPV(new QLocalSocket(this))
{
    socketMPV->connectToServer("/tmp/mpv-socket"); // nom du socket en statique pour le moment

    connect(socketMPV, SIGNAL(readyRead()), this, SLOT(readSocket()));

    if(socketMPV->waitForConnected())
        qDebug() << "connected to mpv";
    else {
        socketMPV->error();
    }

    loadfileMPV("");
}

Serveur::~Serveur() {
    socketMPV->disconnectFromServer();
}

void Serveur::loadfileMPV(QString nomDuFichier){
    QJsonObject commandeMPV; // objet qui contient la commande

    QJsonArray a; // commande loadfile
    a.append("loadfile");
    a.append("/home/zatteo/Music/Augenbling Respect.mp3"); // nom de la musique en statique pour le moment

    commandeMPV["command"] = a;

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
    }
}
