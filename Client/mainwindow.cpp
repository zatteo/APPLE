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
    next= new QState(etat);
    previous= new QState(etat);

    start->addTransition(this, SIGNAL(SPlay()), play);
    start->addTransition(ui->liste_musique, SIGNAL(doubleClicked(QModelIndex)), play);

    play->addTransition(ui->next_2, SIGNAL(pressed()), avance_rapide);
    play->addTransition(ui->previous_2, SIGNAL(pressed()), retour_rapide);
    play->addTransition(ui->play_2, SIGNAL(clicked()), pause);
    play->addTransition(ui->next_2, SIGNAL(clicked()), next);
    play->addTransition(ui->previous_2, SIGNAL(clicked()), previous);


    avance_rapide->addTransition(ui->next_2, SIGNAL(released()), play);
    retour_rapide->addTransition(ui->previous_2, SIGNAL(released()), play);
    pause->addTransition(ui->play_2, SIGNAL(clicked()), play);

    // QObject::connect(start, SIGNAL(entered()), this, SLOT(getInfo()));
    QObject::connect(play, SIGNAL(entered()), this, SLOT(FPause()));
    QObject::connect(pause, SIGNAL(entered()), this, SLOT(FPlay()));
    QObject::connect(start, SIGNAL(exited()), this, SLOT(Beginning()));
    QObject::connect(ui->liste_musique, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(Update(QListWidgetItem*)));

    FPause();
    etat->setInitialState(start);
    etat->start();

    ui->play_2->setDisabled(true);
    ui->artiste_2->hide();
    ui->Titre_2->hide();
    ui->next_2->setDisabled(true);
    ui->previous_2->setDisabled(true);
    ui->lecture->setDisabled(true);
    ui->current->setDisabled(true);
    ui->end->setDisabled(true);

    s = new Serveur();
    s->connect("/tmp/socketClient");
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

void MainWindow::Beginning()
{
    ui->play_2->setDisabled(false);
    ui->artiste_2->show();
    ui->Titre_2->show();
    ui->next_2->setDisabled(false);
    ui->previous_2->setDisabled(false);
    ui->lecture->setDisabled(false);
    ui->current->setDisabled(false);
    ui->end->setDisabled(false);
}

void MainWindow::Update(QListWidgetItem * item)
{
    ui->Titre_2->setText(item->text());
    ui->lecture->setValue(0);
    ui->artiste_2->hide();
}

void MainWindow::UpdateInt(QJsonObject json)
{
    if(json["event"] == "property-changed"){
        if(json["name"] == "volume"){
            int valeur= json.value("data").toInt();
            ui->volume->setValue(valeur);
        }
        else if(json["name"] == "mute"){
            if(json["data"] == true && mute == 0){
                on_sound_2_released();
            }
            else if(json["data"] == false && mute == 1){
                on_sound_2_released();
            }
        }
    }
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
