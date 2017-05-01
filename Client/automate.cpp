#include "automate.h"

automate::automate(QObject *parent) : QObject(parent)
{
    etat =new QStateMachine(this);
    start= new QState(etat);
    play= new QState(etat);
    pause= new QState(etat);
    avance_rapide= new QState(etat);
    retour_rapide= new QState(etat);

    start->addTransition(this, SIGNAL(SPlay()), play);
/*    start->addTransition(this, SIGNAL(SPlay()), avance_rapide);
    start->addTransition(this, SIGNAL(SPlay()), retour_rapide);*/

    play->addTransition(play, SIGNAL(clicked()), avance_rapide);
    play->addTransition(this, SIGNAL(SPlay()), retour_rapide);
    play->addTransition(this, SIGNAL(SPlay()), pause);

    avance_rapide->addTransition(this, SIGNAL(SPlay()), play);
    retour_rapide->addTransition(this, SIGNAL(SPlay()), play);
    pause->addTransition(this, SIGNAL(SPlay()), play);

    QObject::connect(start, SIGNAL(entered()), this, SLOT(getInfo()));
}
