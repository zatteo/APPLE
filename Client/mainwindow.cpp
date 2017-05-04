#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serveur.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTimer>

int mute=1; //mute = 0
int duree=0;
int click= 0, modif=0;
QTimer * rapide;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    rapide= new QTimer(this);
    rapide->setSingleShot(true);
    rapide->setTimerType(Qt::PreciseTimer);
    rapide->setInterval(1500);

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

    play->addTransition(ui->next_2, SIGNAL(pressed()), next);
    play->addTransition(ui->previous_2, SIGNAL(pressed()), previous);
    play->addTransition(ui->play_2, SIGNAL(clicked()), pause);

    next->addTransition(ui->next_2, SIGNAL(released()), play);
    previous->addTransition(ui->previous_2, SIGNAL(released()), play);

    next->addTransition(rapide, SIGNAL(timeout()), avance_rapide);
    previous->addTransition(rapide, SIGNAL(timeout()), retour_rapide);

    avance_rapide->addTransition(ui->next_2, SIGNAL(released()), play);
    retour_rapide->addTransition(ui->previous_2, SIGNAL(released()), play);
    pause->addTransition(ui->play_2, SIGNAL(clicked()), play);

    // QObject::connect(start, SIGNAL(entered()), this, SLOT(getInfo()));
    QObject::connect(play, SIGNAL(entered()), this, SLOT(FPause()));
    QObject::connect(pause, SIGNAL(entered()), this, SLOT(FPlay()));


    QObject::connect(next, SIGNAL(entered()), SLOT(rapide->start();));
    QObject::connect(previous, SIGNAL(entered()), SLOT(rapide->start();));
    QObject::connect(ui->liste_musique, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(Update(QListWidgetItem*)));

    etat->setInitialState(start);
    etat->start();

    ui->liste_musique->clear();
    ui->liste_groupe->clear();
    add_liste_groupe("Toutes les musiques");

    s = new Serveur();
    s->connect("/tmp/socketClient");
    s->requestAllSongs();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::intToTimer(int value)
{
    int min= value/60;
    int sec= value-60*min;
    if(sec < 10)
        return (QString::number(min) + ":0" + QString::number(sec));
    else
        return (QString::number(min) + ":" + QString::number(sec));
}

void MainWindow::FPlay()
{
    s->playMPV(false);
    QPixmap pixmap("../ressources/play.png");
    QIcon ButtonIcon(pixmap);
    ui->play_2->setIcon(ButtonIcon);
}

void MainWindow::FPause()
{
    s->playMPV(true);
    QPixmap pixmap("../ressources/pause.png");
    QIcon ButtonIcon(pixmap);
    ui->play_2->setIcon(ButtonIcon);
}

void MainWindow::Update(QListWidgetItem * item)
{
    ui->Titre_2->setText(item->text());
    s->loadAndPlayMPV(item->text()); // charge un fichier et lance la lecture sur le serveur central
}

void MainWindow::UpdateInt(QJsonObject json)
{
    if(json["event"] == "property-change"){
        if(json["name"] == "volume"){
            int valeur= json.value("data").toInt();
            ui->volume->setValue(valeur);
        }
        else if(json["name"] == "mute"){
            if(json["data"] == true && mute == 1){
                on_sound_2_released();
            }
            else if(json["data"] == false && mute == 0){
                on_sound_2_released();
            }
        }
        else if(json["name"] == "time-pos"){
            if(modif == 0)
            {
                click= 1;
                int tmp= int(json.value("data").toDouble());
                ui->current->setText(intToTimer(tmp));
                ui->lecture->setValue(tmp);
                ui->end->setText("-" + intToTimer(duree-tmp));
                click= 0;
            }
        }
    }
    if(json["event"] == "response"){
        if(json["name"] == "songs"){
            int i;
            QJsonArray tmp= json["data"].toArray();
            for(i=0; i< tmp.size(); i++){
                add_liste_musique(tmp.at(i).toObject().value("title").toString());
            }
        }
        else if(json["name"] == "song"){
            QJsonObject jsonTmp = json.value("data").toObject().value("taglib").toObject();
            ui->lecture->setMaximum(jsonTmp.value("duration").toInt());
            duree= jsonTmp.value("duration").toInt();
//            qDebug() << jsonTmp.value("pictureData").toString().toWCharArray();

            QImage coverQImg;
//            coverQImg.loadFromData(jsonTmp.value("pictureData").toString().toWCharArray());
            coverQImg.save("yolo.jpg", 0, 100);

        }
    }
}

void MainWindow::add_liste_musique(QString nom)
{
    ui->liste_musique->addItem(nom);
}
void MainWindow::add_liste_groupe(QString nom)
{
    ui->liste_groupe->addItem(nom);
}

void MainWindow::on_sound_2_released()
{
    if(mute == 0){
        s->muteMPV(false);
        mute= 1;
        QPixmap pixmap("../ressources/sound.png");
        QIcon ButtonIcon(pixmap);
        ui->sound_2->setIcon(ButtonIcon);
    }
    else {
        s->muteMPV(true);
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

void MainWindow::on_volume_valueChanged(int value)
{
    s->setVolumeMPV(value);
}

void MainWindow::on_lecture_valueChanged(int value)
{
    if(click == 0)
        s->setPositionMPV(value);
    ui->current->setText(intToTimer(value));
    ui->end->setText("-"+intToTimer(duree-value));
}

void MainWindow::on_lecture_sliderPressed()
{
    modif= 1;
}

void MainWindow::on_lecture_sliderReleased()
{
    modif= 0;
}
