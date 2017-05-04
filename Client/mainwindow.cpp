#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serveur.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

int mute=1; //mute = 0
int duree=0;
int click= 0, modif=0;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    timer= new QTimer(this);
    timer->setSingleShot(true);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(500);

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
    start->addTransition(this, SIGNAL(SPause()), pause);

    play->addTransition(ui->next_2, SIGNAL(pressed()), next);
    play->addTransition(ui->previous_2, SIGNAL(pressed()), previous);
    play->addTransition(ui->play_2, SIGNAL(clicked()), pause);
    play->addTransition(this, SIGNAL(SPause()), pause);

    next->addTransition(ui->next_2, SIGNAL(released()), play);
    previous->addTransition(ui->previous_2, SIGNAL(released()), play);

    next->addTransition(timer, SIGNAL(timeout()), avance_rapide);
    previous->addTransition(timer, SIGNAL(timeout()), retour_rapide);

    avance_rapide->addTransition(ui->next_2, SIGNAL(released()), play);
    retour_rapide->addTransition(ui->previous_2, SIGNAL(released()), play);

    pause->addTransition(ui->play_2, SIGNAL(clicked()), play);
    pause->addTransition(this, SIGNAL(SPlay()), play);
    pause->addTransition(ui->next_2, SIGNAL(pressed()), next);
    pause->addTransition(ui->previous_2, SIGNAL(pressed()), previous);


    // QObject::connect(start, SIGNAL(entered()), this, SLOT(getInfo()));
    QObject::connect(play, SIGNAL(entered()), this, SLOT(FPause()));
    QObject::connect(pause, SIGNAL(entered()), this, SLOT(FPlay()));

    QObject::connect(next, SIGNAL(entered()), timer, SLOT(start()));
    QObject::connect(next, SIGNAL(exited()), this, SLOT(NextSong));
    QObject::connect(previous, SIGNAL(entered()), timer, SLOT(start()));
    QObject::connect(avance_rapide, SIGNAL(entered()), this, SLOT(AvanceRapide()));
    QObject::connect(avance_rapide, SIGNAL(exited()), this, SLOT(AvanceNormal()));
    QObject::connect(ui->liste_musique, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(Update(QListWidgetItem*)));

    etat->setInitialState(start);
    etat->start();

    ui->liste_musique->clear();
    ui->liste_groupe->clear();
    add_liste_groupe("Toutes les musiques");

    s = new Serveur();
    s->connect("/tmp/socketClient");
    s->requestAllSongs();
    s->requestAllRadios();
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

void MainWindow::AvanceRapide()
{ s->setVitesseAvantRapide(); }
void MainWindow::AvanceNormal()
{ s->setVitesseNormale(); }
void MainWindow::RetourRapide()
{ s->setVitesseArriereRapide(); }
void MainWindow::NextSong(){
    if(timer->isActive()){
        timer->stop();
        s->next(ui->Titre_2->text());
    }
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
        else if(json["name"] == "pause"){
            if(json["data"] == true){
                emit SPause();
            }
            else if(json["data"] == false){
                emit SPlay();
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
        // toutes les musiques
        if(json["name"] == "songs"){
            int i;
            songs = json["data"].toArray();

            QJsonArray tmp= json["data"].toArray();
            for(i=0; i< tmp.size(); i++){
                add_liste_musique(tmp.at(i).toObject().value("title").toString());
            }
        }
        // une musique
        else if(json["name"] == "song"){
            // on traite les métadonnées
            QJsonObject jsonTmp = json.value("data").toObject().value("taglib").toObject();
            ui->lecture->setMaximum(jsonTmp.value("duration").toInt());
            duree= jsonTmp.value("duration").toInt();
        }
        // une pochette
        else if(json["name"] == "cover")
        {
            // on traite la pochette
            QImage coverQImg = imageFromJson(json["data"].toObject()["picture"]);

            QString saveName = json["data"].toObject()["title"].toString() + ".jpg";

            coverQImg.save(json["data"].toObject()["title"].toString() + ".jpg", 0, 50);

            ui->fond->setStyleSheet("background-image: url(\"" + saveName + "\");");
        }
        // toutes les playlists
        else if(json["name"] == "playlists"){
            playlists = json["data"].toArray();

            // équivalent à songs mais avec les playlists (NON IMPLEMENTE)
        }
        // toutes les radios
        else if(json["name"] == "radios"){
            radios = json["data"].toArray();

            // équivalent à songs mais avec les radios
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

/* encodage d'une image depuis JSON
 */
QImage MainWindow::imageFromJson(const QJsonValue & val)
{
  QByteArray encoded = val.toString().toLatin1();

  QImage p;
  p.loadFromData(QByteArray::fromBase64(encoded), "PNG"); // convention = PNG = base64

  return p;
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

QJsonArray MainWindow::getSongs()
{
    return songs;
}

QJsonArray MainWindow::getPlaylists()
{
    return playlists;
}

QJsonArray MainWindow::getRadios()
{
    return radios;
}

