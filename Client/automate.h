#ifndef AUTOMATE_H
#define AUTOMATE_H

#include <QObject>
#include <QStateMachine>
#include <QState>
#include <QHistoryState>
#include <QFinalState>
#include <QSignalTransition>

class automate: public QObject
{
  Q_OBJECT

public:
    explicit automate(QObject *parent = 0);

public:
    QStateMachine * etat;

    QState * play;
    QState * pause;
    QState * avance_rapide;
    QState * retour_rapide;
    QState * start;
};

#endif // AUTOMATE_H
