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

// $Id$

// Qt includes.
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qcheckbox.h>
#include <qdir.h> 

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
    kdDebug() << "FestivalIntConf::FestivalIntConf: Running" << endl;
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
}

/** Destructor */
FestivalIntConf::~FestivalIntConf(){
    kdDebug() << "FestivalIntConf::~FestivalIntConf: Running" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_festProc;
}

void FestivalIntConf::load(KConfig *config, const QString &langGroup){
    kdDebug() << "FestivalIntConf::load: Loading configuration for language " << langGroup << " with plug in " << "Festival" << endl;

    m_langGroup = langGroup;
    config->setGroup(langGroup);
    m_widget->festivalVoicesPath->setURL(config->readPathEntry("VoicesPath"));
    scanVoices();
    QString voiceSelected(config->readEntry("Voice"));
    for(uint index = 0 ; index < voiceList.count(); ++index){
        kdDebug() << "Testing: " << voiceSelected << " == " << voiceList[index].code << endl;
        if(voiceSelected == voiceList[index].code){
            kdDebug() << "Match!" << endl;
            m_widget->selectVoiceCombo->setCurrentItem(index);  
            break;
        }
    }
}

void FestivalIntConf::save(KConfig *config, const QString &langGroup){
    kdDebug() << "FestivalIntConf::save: Saving configuration for language " << langGroup << " with plug in " << "Festival" << endl;

    m_langGroup = langGroup;
    config->setGroup(langGroup);
    config->writePathEntry("VoicesPath", m_widget->festivalVoicesPath->url());
    config->writeEntry("Voice", voiceList[m_widget->selectVoiceCombo->currentItem()].code);
}

void FestivalIntConf::defaults(){
    kdDebug() << "FestivalIntConf::defaults: Running" << endl;
    m_widget->festivalVoicesPath->setURL("/usr/share/festival/voices/");
}

void FestivalIntConf::scanVoices(){
    kdDebug() << "FestivalIntConf::scanVoices: Running" << endl;
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
            kdDebug() << "For " << *it << " the path " << m_widget->festivalVoicesPath->url() + voiceTemp.path << " doesn't exist" << endl;
            continue;
        } else {
            kdDebug() << "For " << *it << " the path " << m_widget->festivalVoicesPath->url() + voiceTemp.path << " exists" << endl;
        }
        voiceTemp.code = *it;
        voiceTemp.name = voices.readEntry("Name");
        voiceTemp.comment = voices.readEntry("Comment");
        voiceList.append(voiceTemp);
        m_widget->selectVoiceCombo->insertItem(voiceTemp.name + " (" + voiceTemp.comment + ")");
    }
}

void FestivalIntConf::slotTest_clicked()
{
    kdDebug() << "FestivalIntConf::slotTest_clicked: Running " << endl;
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
    
    // Get the code for the selected voice
    KConfig voices(KGlobal::dirs()->resourceDirs("data").last() +
        "/kttsd/festivalint/voices", true, false);
    voices.setGroup(voiceList[m_widget->selectVoiceCombo->currentItem()].code);
    QString voiceCode = "("+voices.readEntry("Code")+")";
    // Use the translated name of the voice as the test message.
    QString testMsg = voices.readEntry("Comment[" + m_langGroup + "]");
    // Fall back to English if no such translation.
    if (testMsg.isNull()) testMsg = voices.readEntry("Comment");
    // Fall back if none.
    if (testMsg.isNull()) testMsg = "KDE is a modern graphical desktop for UNIX computers.";
    m_festProc->synth(
        testMsg,
        tmpWaveFile,
        voiceCode);
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
