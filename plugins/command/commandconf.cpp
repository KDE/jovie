/***************************************************** vim:set ts=4 sw=4 sts=4:
  commandconf.cpp
  Configuration for the Command Plug in
  -------------------
  Copyright : (C) 2002,2004 by Gunnar Schmi Dt and Gary Cramblitt
  -------------------
  Original author: Gunnar Schmi Dt <kmouth@schmi-dt.de>
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
#include <qcheckbox.h>
#include <qfile.h>
#include <qapplication.h>
#include <qtextcodec.h>

// KDE includes.
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

// KTTS includes.
#include <pluginconf.h>

// Command Plugin includes.
#include "commandproc.h"
#include "commandconf.h"
#include "commandconf.moc"

/** Constructor */
CommandConf::CommandConf( QWidget* parent, const char* name, const QStringList& /*args*/) : 
  CommandConfWidget (parent, name)
{
    kdDebug() << "CommandConf::CommandConf: Running" << endl;
    m_playObj = 0;
    m_artsServer = 0;
    m_commandProc = 0;
    defaults();
    connect(commandTestButton, SIGNAL(clicked()), this, SLOT(slotCommandTest_clicked()));
}

/** Destructor */
CommandConf::~CommandConf()
{
    kdDebug() << "CommandConf::~CommandConf: Running" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_commandProc;
}

void CommandConf::load(KConfig *config, const QString &langGroup) {
    kdDebug() << "CommandConf::load: Running" << endl;
    config->setGroup(langGroup);
    urlReq->setURL (config->readEntry("Command", "cat -"));
    stdInButton->setChecked(config->readBoolEntry("StdIn", true));
    m_language = langGroup;
    QString codecString = config->readEntry("Codec", "Local");
    int codec;
    if (codecString == "Local")
        codec = CommandProc::Local;
    else if (codecString == "Latin1")
        codec = CommandProc::Latin1;
    else if (codecString == "Unicode")
        codec = CommandProc::Unicode;
    else {
        codec = CommandProc::Local;
        for (int i = CommandProc::UseCodec; i < characterCodingBox->count(); i++ )
            if (codecString == characterCodingBox->text(i))
                codec = i;
    }
    characterCodingBox->setCurrentItem(codec);
}

void CommandConf::save(KConfig *config, const QString &langGroup) {
    kdDebug() << "CommandConf::save: Running" << endl;
    config->setGroup(langGroup);
    config->writeEntry("Command", urlReq->url());
    config->writeEntry("StdIn", stdInButton->isChecked());
    m_language = langGroup;
    int codec = characterCodingBox->currentItem();
    if (codec == CommandProc::Local)
        config->writeEntry("Codec", "Local");
    else if (codec == CommandProc::Latin1)
        config->writeEntry("Codec", "Latin1");
    else if (codec == CommandProc::Unicode)
        config->writeEntry("Codec", "Unicode");
    else config->writeEntry("Codec",
        characterCodingBox->text(codec));
}

void CommandConf::defaults(){
    kdDebug() << "CommandConf::defaults: Running" << endl;
    urlReq->setURL("cat -");
    stdInButton->setChecked(true);
    buildCodecList();
    urlReq->setShowLocalProtocol (false);
    buildCodecList();
    characterCodingBox->setCurrentItem(0);
}

void CommandConf::buildCodecList () {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name();
   local += ")";
   characterCodingBox->insertItem (local, CommandProc::Local);
   characterCodingBox->insertItem (i18n("Latin1"), CommandProc::Latin1);
   characterCodingBox->insertItem (i18n("Unicode"), CommandProc::Unicode);
   for (int i = 0; (QTextCodec::codecForIndex(i)); i++ )
      characterCodingBox->insertItem(QTextCodec::codecForIndex(i)->name(),
        CommandProc::UseCodec + i);
}
        
void CommandConf::slotCommandTest_clicked()
{
    kdDebug() << "CommandConf::slotCommandTest_clicked(): " << endl;
    // If currently synthesizing, stop it.
    if (m_commandProc)
        m_commandProc->stopText();
    else
    {
        m_commandProc = new CommandProc();
        connect (m_commandProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "commandplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();
    // Play an English test.
    // TODO: Need a way to generate language-specific text.
    m_commandProc->synth(
        "KDE is a modern graphical desktop for Unix computers.",
        tmpWaveFile,
        urlReq->url(),
        stdInButton->isChecked(),
        characterCodingBox->currentItem(),
        QTextCodec::codecForName(characterCodingBox->text(characterCodingBox->currentItem())),
        m_language);
}

void CommandConf::slotSynthFinished()
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
    m_waveFile = m_commandProc->getFilename();
    m_commandProc->ackFinished();
    // Start playback of the wave file.
    KArtsDispatcher dispatcher;
    m_artsServer = new KArtsServer;
    KDE::PlayObjectFactory factory (m_artsServer->server());
    m_playObj = factory.createPlayObject (m_waveFile, true);
    m_playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let CommandConf destructor take care of
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
