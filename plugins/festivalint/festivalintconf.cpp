/***************************************************** vim:set ts=4 sw=4 sts=4:
  festivalintconf.cpp
  Configuration widget and functions for Festival (Interactive) plug in
  -------------------
  Copyright : (C) 2004 Gary Cramblitt
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// C++ includes.
#include <math.h>

// Qt includes.
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qdir.h>
#include <qslider.h>

// KDE includes.
#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kglobal.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
#include <knuminput.h>

// KTTS includes.
#include <pluginconf.h>

// FestivalInt includes.
#include "festivalintproc.h"
#include "festivalintconf.h"
#include "festivalintconf.moc"

/** Constructor */
FestivalIntConf::FestivalIntConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
    PlugInConf(parent, name)
{
    // kdDebug() << "FestivalIntConf::FestivalIntConf: Running" << endl;
    m_festProc = 0;
    m_artsServer = 0;
    m_playObj = 0;
    
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "FestivalIntConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new FestivalIntConfWidget(this, "FestivalIntConfigWidget");
    layout->addWidget(m_widget);
    
    m_widget->festivalVoicesPath->setMode(KFile::Directory);
    defaults();
    
    connect(m_widget->festivalVoicesPath, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->selectVoiceCombo, SIGNAL(activated(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->testButton, SIGNAL(clicked()), this, SLOT(slotTest_clicked()));
    connect(m_widget->rescan, SIGNAL(clicked()), this, SLOT(scanVoices()));
    connect(m_widget->timeBox, SIGNAL(valueChanged(int)),
        this, SLOT(timeBox_valueChanged(int)));
    connect(m_widget->timeSlider, SIGNAL(valueChanged(int)),
        this, SLOT(timeSlider_valueChanged(int)));
    connect(m_widget->timeBox, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
    connect(m_widget->timeSlider, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
}

/** Destructor */
FestivalIntConf::~FestivalIntConf(){
    // kdDebug() << "FestivalIntConf::~FestivalIntConf: Running" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_festProc;
}

void FestivalIntConf::load(KConfig *config, const QString &configGroup){
    // kdDebug() << "FestivalIntConf::load: Running" << endl;

    config->setGroup("FestivalInt");
    m_widget->festivalVoicesPath->setURL(config->readPathEntry("VoicesPath",
        getDefaultVoicesPath()));
    config->setGroup(configGroup);
    scanVoices();
    setDefaultVoice();
    QString voiceSelected(config->readEntry("Voice"));
    for(uint index = 0 ; index < voiceList.count(); ++index){
        // kdDebug() << "Testing: " << voiceSelected << " == " << voiceList[index].code << endl;
        if(voiceSelected == voiceList[index].code){
            // kdDebug() << "FestivalIntConf::load: setting voice to " << voiceSelected << endl;
            m_widget->selectVoiceCombo->setCurrentItem(index);
            break;
        }
    }
    m_widget->timeBox->setValue(config->readNumEntry("time",    100));
}

void FestivalIntConf::save(KConfig *config, const QString &configGroup){
    // kdDebug() << "FestivalIntConf::save: Running" << endl;

    config->setGroup("FestivalInt");
    config->writePathEntry("VoicesPath", m_widget->festivalVoicesPath->url());
    config->setGroup(configGroup);
    config->writeEntry("Voice", voiceList[m_widget->selectVoiceCombo->currentItem()].code);
    config->writeEntry ("time", m_widget->timeBox->value());
}

void FestivalIntConf::defaults(){
    // kdDebug() << "FestivalIntConf::defaults: Running" << endl;
    m_widget->festivalVoicesPath->setURL(getDefaultVoicesPath());
    m_widget->timeBox->setValue(100);
    timeBox_valueChanged(100);
    scanVoices();
}

void FestivalIntConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = splitLanguageCode(lang, m_countryCode);
}

QString FestivalIntConf::getTalkerCode()
{
    QString normalTalkerCode;
    if (voiceList.count() > 0)
    {
        voiceStruct voiceTemp = voiceList[m_widget->selectVoiceCombo->currentItem()];
        // Determine rate attribute.  slow < 75% <= medium <= 125% < fast.
        QString rate = "medium";
        if (m_widget->timeBox->value() < 75) rate = "slow";
        if (m_widget->timeBox->value() > 125) rate = "fast";
        normalTalkerCode = QString(
                "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                "<prosody volume=\"%4\" rate=\"%5\" />"
                "<kttsd synthesizer=\"%6\" />")
                .arg(voiceTemp.languageCode)
                .arg(voiceTemp.name)
                .arg(voiceTemp.gender)
                .arg("medium")
                .arg(rate)
                .arg("Festival Interactive");
    } else normalTalkerCode = QString::null;
    return normalTalkerCode;
}

QString FestivalIntConf::getDefaultVoicesPath()
{
    // Get default path to voice files.
    // TODO: Ask Festival where they are:
    //    $ festival
    //    Festival Speech Synthesis System 1.4.3:release Jan 2003
    //    Copyright (C) University of Edinburgh, 1996-2003. All rights reserved.
    //    For details type `(festival_warranty)'
    //    festival> datadir
    //    "/usr/share/festival"
    //    festival> (quit)
    QDir voicesPath;
    voicesPath = voicesPath.homeDirPath() + "/festival/voices/";
    if (!voicesPath.exists()) voicesPath = "/usr/local/share/festival/voices/";
    if (!voicesPath.exists()) voicesPath = "/usr/share/festival/voices/";
    return voicesPath.path() + voicesPath.separator();
}

void FestivalIntConf::setDefaultVoice()
{
    // If language code is known, auto pick first voice that matches the language code.
    if (!m_languageCode.isEmpty())
    {
        bool found = false;
        // First search for a match on both language code and country code.
        QString languageCode = m_languageCode;
        if (!m_countryCode.isNull()) languageCode += "_" + m_countryCode;
        // kdDebug() << "FestivalIntConf::setDefaultVoice:: looking for default voice to match language code " << languageCode << endl;
        uint index;
        for(index = 0 ; index < voiceList.count(); ++index)
        {
            QString vlCode = voiceList[index].languageCode.left(languageCode.length());
            // kdDebug() << "FestivalIntConf::setDefaultVoice: testing " << vlCode << endl;
            if(languageCode == vlCode)
            {
                found = true;
                break;
            }
        }
        // If not found, search for a match on just the language code.
        if (!found)
        {
            languageCode = m_languageCode;
            for(index = 0 ; index < voiceList.count(); ++index)
            {
                QString vlCode = voiceList[index].languageCode.left(languageCode.length());
                // kdDebug() << "FestivalIntConf::setDefaultVoice: testing " << vlCode << endl;
                if(languageCode == vlCode)
                {
                    found = true;
                    break;
                }
            }
        }
        if (found)
        {
            // kdDebug() << "FestivalIntConf::setDefaultVoice: auto picking voice code " << voiceList[index].code << endl;
            m_widget->selectVoiceCombo->setCurrentItem(index);
        }
    }
}

void FestivalIntConf::scanVoices(){
    // kdDebug() << "FestivalIntConf::scanVoices: Running" << endl;
    voiceList.clear();
    m_widget->selectVoiceCombo->clear();
    KConfig voices(KGlobal::dirs()->resourceDirs("data").last() + "/kttsd/festivalint/voices", true, false);
    QStringList groupList = voices.groupList();
    QDir mainPath(m_widget->festivalVoicesPath->url());
    voice voiceTemp;
    for(QStringList::Iterator it = groupList.begin(); it != groupList.end(); ++it ){
        voices.setGroup(*it);
        voiceTemp.path = voices.readEntry("Path");
        mainPath.setPath(m_widget->festivalVoicesPath->url() + voiceTemp.path);
        if(!mainPath.exists()){
            // kdDebug() << "For " << *it << " the path " << m_widget->festivalVoicesPath->url() + voiceTemp.path << " doesn't exist" << endl;
            continue;
        } else {
            // kdDebug() << "For " << *it << " the path " << m_widget->festivalVoicesPath->url() + voiceTemp.path << " exists" << endl;
        }
        voiceTemp.code = *it;
        voiceTemp.name = voices.readEntry("Name");
        voiceTemp.comment = voices.readEntry("Comment");
        voiceTemp.languageCode = voices.readEntry("Language");
        voiceTemp.gender = voices.readEntry("Gender", "neutral");
        voiceList.append(voiceTemp);
        m_widget->selectVoiceCombo->insertItem(voiceTemp.name + " (" + voiceTemp.comment + ")");
    }
    setDefaultVoice();
}

void FestivalIntConf::slotTest_clicked()
{
    // kdDebug() << "FestivalIntConf::slotTest_clicked: Running " << endl;
    // If currently synthesizing, stop it.
    if (m_festProc)
        m_festProc->stopText();
    else
    {
        m_festProc = new FestivalIntProc();
        connect (m_festProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "festivalintplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();
    
    // Get the code for the selected voice.
    KConfig voices(KGlobal::dirs()->resourceDirs("data").last() +
        "/kttsd/festivalint/voices", true, false);
    voices.setGroup(voiceList[m_widget->selectVoiceCombo->currentItem()].code);
    QString voiceCode = "("+voices.readEntry("Code")+")";
    // Use the translated name of the voice as the test message.
    QString testMsg = voices.readEntry("Comment[" + m_languageCode + "]");
    // Fall back to English if no such translation.
    if (testMsg.isNull()) testMsg = voices.readEntry("Comment");
    // Fall back if none.
    if (testMsg.isNull()) testMsg = "KDE is a modern graphical desktop for UNIX computers.";
    // kdDebug() << "FestivalIntConf::slotTest_clicked: calling synth with voiceCode: " << voiceCode << " time percent: " << m_widget->timeBox->value() << endl;
    m_festProc->synth(
        testMsg,
        tmpWaveFile,
        voiceCode,
        m_widget->timeBox->value());
}

void FestivalIntConf::slotSynthFinished()
{
    // If currently playing (or finished playing), stop and delete play object.
    if (m_playObj)
    {
       m_playObj->halt();
       // Clean up.
       QFile::remove(m_waveFile);
    }
    delete m_playObj;
    delete m_artsServer;
    // Get new wavefile name.
    m_waveFile = m_festProc->getFilename();
    m_festProc->ackFinished();
    // Start playback of the wave file.
    KArtsDispatcher dispatcher;
    m_artsServer = new KArtsServer;
    KDE::PlayObjectFactory factory (m_artsServer->server());
    m_playObj = factory.createPlayObject (m_waveFile, true);
    m_playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let FestivalIntConf destructor take care of
    // cleaning up the play object.  However, because we've been called from DCOP,
    // this seems to be necessary.  The call to processEvents is problematic because
    // it can cause re-entrancy.
    while (m_playObj->state() == Arts::posPlaying) qApp->processEvents();
    m_playObj->halt();
    delete m_playObj;
    m_playObj = 0;
    delete m_artsServer;
    m_artsServer = 0;
    QFile::remove(m_waveFile);
    m_waveFile = QString::null;
}

// Basically the slider values are logarithmic (0,...,1000) whereas percent
// values are linear (50%,...,200%).
//
// slider = alpha * (log(percent)-log(50))
// with alpha = 1000/(log(200)-log(50))

int FestivalIntConf::percentToSlider(int percentValue) {
   double alpha = 1000 / (log(200) - log(50));
   return (int)floor (0.5 + alpha * (log(percentValue)-log(50)));
}

int FestivalIntConf::sliderToPercent(int sliderValue) {
   double alpha = 1000 / (log(200) - log(50));
   return (int)floor(0.5 + exp (sliderValue/alpha + log(50)));
}

void FestivalIntConf::timeBox_valueChanged(int percentValue) {
   m_widget->timeSlider->setValue (percentToSlider (percentValue));
}

void FestivalIntConf::timeSlider_valueChanged(int sliderValue) {
   m_widget->timeBox->setValue (sliderToPercent (sliderValue));
}

