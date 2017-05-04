#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "serveur.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QFileInfo>
#include <QDir>

int mute=1; //mute = 0
int duree=0;
int click= 0, modif=0;
int radio=0;


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //timer pour savoir si on a un appui long ou court sur le bouton suivant/avance rapide
    timer= new QTimer(this);
    timer->setSingleShot(true);
    timer->setTimerType(Qt::PreciseTimer);
    timer->setInterval(500);

    ui->setupUi(this);
    ui->Titre_2->hide();

    //etat de l'automate
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


    QObject::connect(play, SIGNAL(entered()), this, SLOT(FPause()));
    QObject::connect(pause, SIGNAL(entered()), this, SLOT(FPlay()));

    QObject::connect(start, SIGNAL(entered()), this, SLOT(getInfo()));
    QObject::connect(next, SIGNAL(entered()), timer, SLOT(start()));
    QObject::connect(next, SIGNAL(exited()), this, SLOT(NextSong()));
    QObject::connect(previous, SIGNAL(entered()), timer, SLOT(start()));
    QObject::connect(previous, SIGNAL(exited()), timer, SLOT(PreviousSong()));
    QObject::connect(avance_rapide, SIGNAL(entered()), this, SLOT(AvanceRapide()));
    QObject::connect(avance_rapide, SIGNAL(exited()), this, SLOT(AvanceNormal()));
    QObject::connect(ui->liste_musique, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(Update(QListWidgetItem*)));

    etat->setInitialState(start);
    etat->start();

    ui->liste_musique->clear();
    ui->liste_groupe->clear();

    // traductions

    QString configPath = QDir::homePath() + "/.apple.json";
    QFile jsonFile(configPath);
    if(!jsonFile.open(QFile::ReadOnly))
    {
        // crée le fichier et passe en français par défaut
        updateLanguage("FR");
    }
    else
    {
        QJsonDocument d = QJsonDocument().fromJson(jsonFile.readAll());
        language = d.object().value("language").toString();
    }

    qDebug() << "Langue détecté :" << language;

    // traductions en dur pour une meilleure portabilité

    translations["FR"] = QJsonArray({"Options", "Langues", "À propos"});
    translations["EN"] = QJsonArray({"Options", "Languages", "About"});

    updateLanguage(language);

    if(language == "EN")
    {
        add_liste_groupe("Titles");
    }
    else
    {
        add_liste_groupe("Titres");
    }
    add_liste_groupe("Radios");

    s = new Serveur();
    s->connect("/tmp/socketClient");
    s->requestAllSongs();
    s->requestAllPlaylists();
    s->requestAllRadios();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// convertit les secondes en min:secondes (string)
QString MainWindow::intToTimer(int value)
{
    int min= value/60;
    int sec= value-60*min;
    if(sec < 10)
        return (QString::number(min) + ":0" + QString::number(sec));
    else
        return (QString::number(min) + ":" + QString::number(sec));
}

//fonction play
void MainWindow::FPlay()
{
    s->playMPV(false);
    QPixmap pixmap("../ressources/play.png");
    QIcon ButtonIcon(pixmap);
    ui->play_2->setIcon(ButtonIcon);
}

//fonction pause
void MainWindow::FPause()
{
    s->playMPV(true);
    QPixmap pixmap("../ressources/pause.png");
    QIcon ButtonIcon(pixmap);
    ui->play_2->setIcon(ButtonIcon);
}

//Update le titre et lance la musique
void MainWindow::Update(QListWidgetItem * item)
{
    ui->Titre_2->setText(item->text());
    if(radio == 0){
        ui->lecture->show();
        ui->end->show();
        ui->current->show();
        ui->next_2->show();
        ui->previous_2->show();
        s->loadAndPlayMPV(item->text()); // charge un fichier et lance la lecture sur le serveur central
    }
    else if(radio == 1){
        ui->lecture->hide();
        ui->end->hide();
        ui->current->hide();
        ui->next_2->hide();
        ui->previous_2->hide();
        s->loadAndPlayARadioMPV(item->text());
   }
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
void MainWindow::PreviousSong(){
    if(timer->isActive()){
        timer->stop();
        s->previous(ui->Titre_2->text());
    }
}

//get info pour avoir les informations au démarrage
void MainWindow::getInfo()
{ s->getCurrentStateMPV(); }


//fonction qui est appelé à chaque message et traite tous les messages
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
                tmp = duree - tmp;
                ui->end->setText("-" + intToTimer(tmp));
                click= 0;
            }
        }
        else if(json["name"] == "filename"){
            ui->Titre_2->setText(json["data"].toString());
            UpdateLocal(json["data"].toString());
        }
    }

    if(json["event"] == "response"){

        qDebug() << "YOLO" << json;
        // toutes les musiques
        if(json["name"] == "songs"){
            songs = json["data"].toArray();
        }
        // une musique
        else if(json["name"] == "song"){
            // on traite les métadonnées
            QJsonObject jsonTmp = json["data"].toObject()["taglib"].toObject();

            songsAddTaglib(json["data"].toObject()["title"].toString(), jsonTmp);

            ui->lecture->setMaximum(jsonTmp.value("duration").toInt());
            duree= jsonTmp.value("duration").toInt();
            ui->Titre->setText(jsonTmp.value("TITLE").toString());
            ui->artiste_2->setText(jsonTmp.value("ARTIST").toString());
        }
        // une pochette
        else if(json["name"] == "cover")
        {
            // on enregistre la pochette
            QImage coverQImg = imageFromJson(json["data"].toObject()["cover"]);

            QString fileName = json["data"].toObject()["title"].toString();
            QFileInfo fileInfo(fileName);

            QString saveName = fileInfo.completeBaseName() + ".jpg";

            coverQImg.save(saveName, 0, 50);

            // on ajoute la pochette dans notre base locale
            songsAddCover(fileName, saveName);

            ui->fond->setStyleSheet("background-image: url(\"" + saveName + "\");");
        }
        // toutes les playlists
        else if(json["name"] == "playlists"){
            playlists = json["data"].toArray();
            int i;
            for(i=0; i<playlists.size(); i++){
                add_liste_groupe(playlists.at(i).toObject().value("title").toString());
            }
            // équivalent à songs mais avec les playlists (NON IMPLEMENTE)
        }
        // toutes les radios
        else if(json["name"] == "radios"){
            radios = json["data"].toArray();
            // équivalent à songs mais avec les radios
        }
    }

    if(json["error"] == "success")
    {
        //envoi au debut du statut play-pause
        if(json["request_id"] == 1)
        {
            if(json["data"] == true){
                emit SPause();
            }
            else if(json["data"] == false){
                emit SPlay();
            }
        }
        // titre de la musique en cour
        else if(json["request_id"] ==2){
            ui->Titre_2->setText(json["data"].toString());
            s->getData(json["data"].toString());
        }
        // son mute ou pas
        if(json["request_id"] == 3){
            if(json["data"] == true && mute == 1){
                on_sound_2_released();
            }
            else if(json["data"] == false && mute == 0){
                on_sound_2_released();
            }
        }
        // volume de la musique
        if(json["request_id"] == 4){
            ui->volume->setValue(json["data"].toInt());
        }
    }
}

/* met à jour l'interface avec les informations disponibles sur la musique
 * QString title: le titre du morceau
 */
void MainWindow::UpdateLocal(QString title)
{
    QJsonObject currentSongToPrint;

    for(int i = 0; i < songs.size(); i++)
    {
        if(songs.at(i).toObject()["title"].toString() == title)
        {
             currentSongToPrint = songs.at(i).toObject();
        }
    }

    ui->Titre_2->setText(currentSongToPrint["title"].toString());
    ui->Titre_2->hide();
    ui->Titre->setText(currentSongToPrint["taglib"].toObject()["TITLE"].toString());
    ui->artiste_2->setText(currentSongToPrint["taglib"].toObject()["ARTIST"].toString());
    ui->fond->setStyleSheet("background-image: url(\"" + currentSongToPrint["saveName"].toString() + "\");");
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

/* on ajoute les métadonnées du morceau dans songs
 * QString title : le morceau
 * QString taglib : taglib
 */
void MainWindow::songsAddTaglib(QString title, QJsonObject taglib)
{
    qDebug() << "songsAddTaglib :" << title << "-" << taglib;

    for(int i = 0; i < songs.size(); i++)
    {
        if(songs.at(i).toObject()["title"].toString() == title)
        {
            QJsonObject songWithoutTaglib = songs.at(i).toObject();

            songWithoutTaglib["taglib"] = taglib;

            songs.replace(i, songWithoutTaglib);
        }
    }
}

/* on ajoute le chemin de la pochette du morceau dans songs
 * QString title : le morceau
 * QString saveName : le chemin de la pochette
 */
void MainWindow::songsAddCover(QString title, QString saveName)
{
    qDebug() << "songsAddCover :" << title << "-" << saveName;

    for(int i = 0; i < songs.size(); i++)
    {
        if(songs.at(i).toObject()["title"].toString() == title)
        {
            qDebug() << "Ajout de" << saveName << "pour" << title;

            QJsonObject songWithoutCover = songs.at(i).toObject();

            songWithoutCover["saveName"] = saveName;

            songs.replace(i, songWithoutCover);
        }
    }
}

/* vérifie si un morceau possède déjà des métadonnées
 * QString title : le morceau à vérifier
 */
bool MainWindow::isTaglibPresent(QString title)
{
    for(int i = 0; i < songs.size(); i++)
    {
        if(songs.at(i).toObject()["title"].toString() == title)
        {
            qDebug() << title << "isTaglibPresent" << songs.at(i).toObject().contains("taglib");
            return songs.at(i).toObject().contains("taglib");
        }
    }

    qDebug() << title << "isTaglibPresent fin de fonction";
    return false;
}

/* vérifie si un morceau possède déjà une pochette
 * QString title : le morceau à vérifier
 */
bool MainWindow::isCoverPresent(QString title)
{
    for(int i = 0; i < songs.size(); i++)
    {
        if(songs.at(i).toObject()["title"].toString() == title)
        {
            qDebug() << title << "isCoverPresent" << songs.at(i).toObject().contains("saveName");
            return songs.at(i).toObject().contains("saveName");
        }
    }

    qDebug() << title << "isCoverPresent fin de fonction";
    return false;
}


void MainWindow::on_liste_groupe_itemClicked(QListWidgetItem *item)
{
    QJsonArray tab;
    if(item->text() == "Titres" || item->text() == "Titles"){
        tab= songs;
        radio= 0;
    }
    else if(item->text() == "Radios"){
        tab= radios;
        radio= 1;
    }
    else{
        s->loadAndPlayAPlaylistMPV(item->text());
        ui->liste_groupe->clear();
        return;
    }

    ui->liste_musique->clear();
    int i;
    for(i=0; i< tab.size(); i++){
        add_liste_musique(tab.at(i).toObject().value("title").toString());
    }
}

void MainWindow::updateLanguage(QString l)
{
    // on modifie l'état
    language = l;

    ui->menuParam_tres->setTitle(translations.value(l).toArray().at(0).toString());
    ui->menuLangues->setTitle(translations[l].toArray().at(1).toString());
    ui->action_propos->setText(translations[l].toArray().at(2).toString());

    // on modifie le fichier de configuration
    QJsonObject newConfig;
    newConfig["language"] = l;

    QJsonDocument document(newConfig);

    QString configPath = QDir::homePath() + "/.apple.json";

    QFile newFileConfig(configPath);
    newFileConfig.open(QFile::WriteOnly);
    newFileConfig.write(document.toJson());
}

void MainWindow::on_actionFrances_triggered()
{
    updateLanguage("FR");
}

void MainWindow::on_actionEnglish_triggered()
{
    updateLanguage("EN");
}
