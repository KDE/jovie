/***************************************************** vim:set ts=4 sw=4 sts=4:
  fliteconf.cpp
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
#include <qfile.h>
#include <qapplication.h>

// KDE includes.
#include <kdialog.h>
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
#include <ktempfile.h>
#include <kstandarddirs.h>

// Flite Plugin includes.
#include "fliteproc.h"
#include "fliteconf.h"

/** Constructor */
FliteConf::FliteConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
    PlugInConf(parent, name)
{
    kdDebug() << "FliteConf::FliteConf: Running" << endl;
    m_fliteProc = 0;
    m_artsServer = 0;
    m_playObj = 0;
    
    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "FliteConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new FliteConfWidget(this, "FliteConfigWidget");
    layout->addWidget(m_widget);
    
    defaults();
    
    connect(m_widget, SIGNAL(configChanged(bool)), this, SLOT(configChanged (bool)));
    connect(m_widget->fliteTest, SIGNAL(clicked()), this, SLOT(slotFliteTest_clicked()));
}

/** Destructor */
FliteConf::~FliteConf(){
    kdDebug() << "Running: FliteConf::~FliteConf()" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_fliteProc;
}

void FliteConf::load(KConfig *config, const QString &langGroup){
    kdDebug() << "FliteConf::load: Loading configuration for language " << langGroup << " with plug in " << "Festival Lite (flite)" << endl;

    config->setGroup(langGroup);
    m_widget->flitePath->setURL(config->readPathEntry("FliteExePath", "flite"));
}

void FliteConf::save(KConfig *config, const QString &langGroup){
    kdDebug() << "FliteConf::save: Saving configuration for language " << langGroup << " with plug in " << "Festival Lite (flite)" << endl;

    config->setGroup(langGroup);
    config->writePathEntry("FliteExePath", m_widget->flitePath->url());
}

void FliteConf::defaults(){
    kdDebug() << "FliteConf::defaults: Running" << endl;
    m_widget->flitePath->setURL("flite");
}

void FliteConf::slotFliteTest_clicked()
{
    kdDebug() << "FliteConf::slotFliteTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
    if (m_fliteProc)
        m_fliteProc->stopText();
    else
    {
        m_fliteProc = new FliteProc();
        connect (m_fliteProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    }
    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "fliteplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();
    // Play an English test.  Flite only supports English.
    m_fliteProc->synth(
        "KDE is a modern graphical desktop for Unix computers.",
        tmpWaveFile,
        m_widget->flitePath->url());
}

void FliteConf::slotSynthFinished()
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
    m_waveFile = m_fliteProc->getFilename();
    m_fliteProc->ackFinished();
    // Start playback of the wave file.
    KArtsDispatcher dispatcher;
    m_artsServer = new KArtsServer;
    KDE::PlayObjectFactory factory (m_artsServer->server());
    m_playObj = factory.createPlayObject (m_waveFile, true);
    m_playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let FliteConf destructor take care of
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

#include "fliteconf.moc"
