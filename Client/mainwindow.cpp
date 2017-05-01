#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serveur.h"

int mute=1; //mute = 0

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    etat =new QStateMachine(this);
    start= new QState(etat);
    play= new QState(etat);
    pause= new QState(etat);
    avance_rapide= new QState(etat);
    retour_rapide= new QState(etat);

    start->addTransition(this, SIGNAL(SPlay()), play);

    play->addTransition(ui->next_2, SIGNAL(pressed()), avance_rapide);
    play->addTransition(ui->previous_2, SIGNAL(pressed()), retour_rapide);
    play->addTransition(ui->play_2, SIGNAL(clicked()), pause);

    avance_rapide->addTransition(ui->next_2, SIGNAL(released()), play);
    retour_rapide->addTransition(ui->previous_2, SIGNAL(released()), play);
    pause->addTransition(ui->play_2, SIGNAL(clicked()), play);

    QObject::connect(start, SIGNAL(entered()), this, SLOT(getInfo()));
    QObject::connect(play, SIGNAL(entered()), this, SLOT(FPause()));
    QObject::connect(pause, SIGNAL(entered()), this, SLOT(FPlay()));
    FPause();
    etat->setInitialState(play);
    etat->start();

    s = new Serveur();
    s->connectMPV("/tmp/mpv-socket");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_lecture_valueChanged(int value)
{
    int min= value/60;
    int sec= value-60*min;
    if(sec < 10)
        ui->current->setText(QString::number(min) + ":0" + QString::number(sec));
    else
        ui->current->setText(QString::number(min) + ":" + QString::number(sec));
}

void MainWindow::FPlay()
{
    QPixmap pixmap("../ressources/play.png");
    QIcon ButtonIcon(pixmap);
    ui->play_2->setIcon(ButtonIcon);
}

void MainWindow::FPause()
{
    QPixmap pixmap("../ressources/pause.png");
    QIcon ButtonIcon(pixmap);
    ui->play_2->setIcon(ButtonIcon);
}

void MainWindow::on_sound_2_released()
{
    if(mute == 0){
        mute= 1;
        QPixmap pixmap("../ressources/sound.png");
        QIcon ButtonIcon(pixmap);
        ui->sound_2->setIcon(ButtonIcon);
    }
    else {
        mute= 0;
        QPixmap pixmap("../ressources/nosound.png");
        QIcon ButtonIcon(pixmap);
        ui->sound_2->setIcon(ButtonIcon);
    }
}

/* récupére la MainWindow
 */
void MainWindow::setMainWindow(MainWindow *window)
{
    s->setMainWindow(window);
}
