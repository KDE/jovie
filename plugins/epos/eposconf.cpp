/***************************************************** vim:set ts=4 sw=4 sts=4:
  eposconf.cpp
  Configuration widget and functions for Epos plug in
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
#include <qfile.h>
#include <qapplication.h>
#include <qtextcodec.h>

// KDE includes.
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kcombobox.h>
#include <klocale.h>

// Epos Plugin includes.
#include "eposproc.h"
#include "eposconf.h"

/** Constructor */
EposConf::EposConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
   EposConfWidget( parent, name )
{
    kdDebug() << "EposConf::EposConf: Running" << endl;
    m_eposProc = 0;
    m_artsServer = 0;
    m_playObj = 0;
    connect(this->eposTest, SIGNAL(clicked()), this, SLOT(slotEposTest_clicked()));
}

/** Destructor */
EposConf::~EposConf(){
    kdDebug() << "Running: EposConf::~EposConf()" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_eposProc;
}

void EposConf::load(KConfig *config, const QString &langGroup){
    kdDebug() << "EposConf::load: Loading configuration for language " << langGroup << " with plug in " << "Epos" << endl;

    config->setGroup(langGroup);
    this->eposServerPath->setURL(config->readPathEntry("EposServerExePath", "epos"));
    this->eposClientPath->setURL(config->readPathEntry("EposClientExePath", "say"));
    this->eposServerOptions->setText(config->readEntry("EposServerOptions", ""));
    this->eposClientOptions->setText(config->readEntry("EposClientOptions", ""));
    QString codecString = config->readEntry("Codec", "Local");
    int codec;
    if (codecString == "Local")
        codec = EposProc::Local;
    else if (codecString == "Latin1")
        codec = EposProc::Latin1;
    else if (codecString == "Unicode")
        codec = EposProc::Unicode;
    else {
        codec = EposProc::Local;
        for (int i = EposProc::UseCodec; i < characterCodingBox->count(); i++ )
            if (codecString == characterCodingBox->text(i))
                codec = i;
    }
    characterCodingBox->setCurrentItem(codec);
}

void EposConf::save(KConfig *config, const QString &langGroup){
    kdDebug() << "EposConf::save: Saving configuration for language " << langGroup << " with plug in " << "Epos" << endl;

    config->setGroup(langGroup);
    config->writePathEntry("EposServerExePath", this->eposServerPath->url());
    config->writePathEntry("EposClientExePath", this->eposClientPath->url());
    config->writeEntry("EposServerOptions", this->eposServerOptions->text());
    config->writeEntry("EposClientOptions", this->eposClientOptions->text());
    int codec = characterCodingBox->currentItem();
    if (codec == EposProc::Local)
        config->writeEntry("Codec", "Local");
    else if (codec == EposProc::Latin1)
        config->writeEntry("Codec", "Latin1");
    else if (codec == EposProc::Unicode)
        config->writeEntry("Codec", "Unicode");
    else config->writeEntry("Codec",
        characterCodingBox->text(codec));
}

void EposConf::defaults(){
    kdDebug() << "EposConf::defaults: Running" << endl;
    this->eposServerPath->setURL("epos");
    this->eposClientPath->setURL("say");
    this->eposServerOptions->setText("");
    this->eposClientOptions->setText("");
    buildCodecList();
    characterCodingBox->setCurrentItem(0);
}

void EposConf::buildCodecList () {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name();
   local += ")";
   characterCodingBox->insertItem (local, EposProc::Local);
   characterCodingBox->insertItem (i18n("Latin1"), EposProc::Latin1);
   characterCodingBox->insertItem (i18n("Unicode"), EposProc::Unicode);
   for (int i = 0; (QTextCodec::codecForIndex(i)); i++ )
      characterCodingBox->insertItem(QTextCodec::codecForIndex(i)->name(),
        EposProc::UseCodec + i);
}
        
void EposConf::slotEposTest_clicked()
{
    kdDebug() << "EposConf::slotEposTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_eposProc)
        m_eposProc->stopText();
    else
    {
        m_eposProc = new EposProc();
        connect (m_eposProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "eposplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();
    // Play an English test.
    // TODO: Need czeck or slavak test message.
    // TODO: Whenever server options change, the server must be restarted.
    m_eposProc->synth(
        "KDE is a modern graphical desktop for Unix computers.",
        tmpWaveFile,
        this->eposServerPath->url(),
        this->eposClientPath->url(),
        this->eposServerOptions->text(),
        this->eposClientOptions->text(),
        characterCodingBox->currentItem(),
        QTextCodec::codecForName(characterCodingBox->text(characterCodingBox->currentItem())));
}

void EposConf::slotSynthFinished()
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
    m_waveFile = m_eposProc->getFilename();
    m_eposProc->ackFinished();
    // Start playback of the wave file.
    KArtsDispatcher dispatcher;
    m_artsServer = new KArtsServer;
    KDE::PlayObjectFactory factory (m_artsServer->server());
    m_playObj = factory.createPlayObject (m_waveFile, true);
    m_playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let EposConf destructor take care of
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

#include "eposconf.moc"
