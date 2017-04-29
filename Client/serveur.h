#ifndef SERVEUR_H
#define SERVEUR_H

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

class Serveur: public QObject
{
    Q_OBJECT
public:
    explicit Serveur(QObject *parent = 0);
    ~Serveur();

private slots:
    void readSocket();

private:
    QLocalSocket *socketMPV = NULL;
    void loadfileMPV(QString nomDuFichier);
};

#endif // SERVEUR_H
