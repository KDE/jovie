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

// Qt includes.
#include <qlayout.h>
#include <qcheckbox.h>
#include <qfile.h>
#include <qapplication.h>
#include <qtextcodec.h>

// KDE includes.
#include <kdialog.h>
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
#include <kprogress.h>

// KTTS includes.
#include <pluginconf.h>

// Command Plugin includes.
#include "commandproc.h"
#include "commandconf.h"

/** Constructor */
CommandConf::CommandConf( QWidget* parent, const char* name, const QStringList& /*args*/) :
    PlugInConf(parent, name)
{
    // kdDebug() << "CommandConf::CommandConf: Running" << endl;
    m_playObj = 0;
    m_artsServer = 0;
    m_commandProc = 0;
    m_progressDlg = 0;

    QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "CommandConfigWidgetLayout");
    layout->setAlignment (Qt::AlignTop);
    m_widget = new CommandConfWidget(this, "CommandConfigWidget");
    layout->addWidget(m_widget);

    defaults();
    connect(m_widget->characterCodingBox, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->characterCodingBox, SIGNAL(activated(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->stdInButton, SIGNAL(toggled(bool)),
        this, SLOT(configChanged()));
    connect(m_widget->urlReq, SIGNAL(textChanged(const QString&)),
        this, SLOT(configChanged()));
    connect(m_widget->commandTestButton, SIGNAL(clicked()),
        this, SLOT(slotCommandTest_clicked()));
}

/** Destructor */
CommandConf::~CommandConf()
{
    // kdDebug() << "CommandConf::~CommandConf: Running" << endl;
    if (m_playObj) m_playObj->halt();
    delete m_playObj;
    delete m_artsServer;
    if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
    delete m_commandProc;
    delete m_progressDlg;
}

void CommandConf::load(KConfig *config, const QString &configGroup) {
    // kdDebug() << "CommandConf::load: Running" << endl;
    config->setGroup(configGroup);
    m_widget->urlReq->setURL (config->readEntry("Command", "cat -"));
    m_widget->stdInButton->setChecked(config->readBoolEntry("StdIn", true));
    QString codecString = config->readEntry("Codec", "Local");
    m_languageCode = config->readEntry("LanguageCode", m_languageCode);
    int codec;
    if (codecString == "Local")
        codec = CommandProc::Local;
    else if (codecString == "Latin1")
        codec = CommandProc::Latin1;
    else if (codecString == "Unicode")
        codec = CommandProc::Unicode;
    else {
        codec = CommandProc::Local;
        for (int i = CommandProc::UseCodec; i < m_widget->characterCodingBox->count(); i++ )
            if (codecString == m_widget->characterCodingBox->text(i))
                codec = i;
    }
    m_widget->characterCodingBox->setCurrentItem(codec);
}

void CommandConf::save(KConfig *config, const QString &configGroup) {
    // kdDebug() << "CommandConf::save: Running" << endl;
    config->setGroup(configGroup);
    config->writeEntry("Command", m_widget->urlReq->url());
    config->writeEntry("StdIn", m_widget->stdInButton->isChecked());
    int codec = m_widget->characterCodingBox->currentItem();
    if (codec == CommandProc::Local)
        config->writeEntry("Codec", "Local");
    else if (codec == CommandProc::Latin1)
        config->writeEntry("Codec", "Latin1");
    else if (codec == CommandProc::Unicode)
        config->writeEntry("Codec", "Unicode");
    else config->writeEntry("Codec",
        m_widget->characterCodingBox->text(codec));
}

void CommandConf::defaults(){
    // kdDebug() << "CommandConf::defaults: Running" << endl;
    m_widget->urlReq->setURL("cat -");
    m_widget->stdInButton->setChecked(true);
    buildCodecList();
    m_widget->urlReq->setShowLocalProtocol (false);
    buildCodecList();
    m_widget->characterCodingBox->setCurrentItem(0);
}

void CommandConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = lang;
}

QString CommandConf::getTalkerCode()
{
    QString url = m_widget->urlReq->url();
    if (!url.isEmpty())
    {
        // Must contain either text or file parameter, otherwise, does nothing!
        if ((url.contains("%t") > 0) || (url.contains("%f") > 0))
        {
            return QString(
                "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                "<prosody volume=\"%4\" rate=\"%5\" />"
                "<kttsd synthesizer=\"%6\" />")
                .arg(m_languageCode)
                .arg("fixed")
                .arg("neutral")
                .arg("medium")
                .arg("medium")
                .arg("Command");
        }
    }
    return QString::null;
}

void CommandConf::buildCodecList () {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name();
   local += ")";
   m_widget->characterCodingBox->clear();
   m_widget->characterCodingBox->insertItem (local, CommandProc::Local);
   m_widget->characterCodingBox->insertItem (i18n("Latin1"), CommandProc::Latin1);
   m_widget->characterCodingBox->insertItem (i18n("Unicode"), CommandProc::Unicode);
   for (int i = 0; (QTextCodec::codecForIndex(i)); i++ )
      m_widget->characterCodingBox->insertItem(QTextCodec::codecForIndex(i)->name(),
        CommandProc::UseCodec + i);
}

void CommandConf::slotCommandTest_clicked()
{
    // kdDebug() << "CommandConf::slotCommandTest_clicked(): " << endl;
    // If currently synthesizing, stop it.
    if (m_commandProc)
        m_commandProc->stopText();
    else
    {
        m_commandProc = new CommandProc();
        connect (m_commandProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
    }

    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "commandplugin-"), ".wav");
    QString tmpWaveFile = tempFile.file()->name();
    tempFile.close();

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_command_testdlg",
        i18n("Testing"),
        i18n("Testing."),
        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // Play an English test.
    // TODO: Need a way to generate language-specific text.
    connect (m_commandProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    m_commandProc->synth(
        "K D E is a modern graphical desktop for Unix computers.",
        tmpWaveFile,
        m_widget->urlReq->url(),
        m_widget->stdInButton->isChecked(),
        m_widget->characterCodingBox->currentItem(),
        QTextCodec::codecForName(m_widget->characterCodingBox->text(m_widget->characterCodingBox->currentItem())),
        m_languageCode);

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_commandProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
    if (m_progressDlg->wasCancelled()) m_commandProc->stopText();
    delete m_progressDlg;
    m_progressDlg = 0;
}

void CommandConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_commandProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
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
    if (m_progressDlg) m_progressDlg->close();
}

void CommandConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_commandProc->getFilename();
    if (!filename.isNull()) QFile::remove(filename);
}

#include "commandconf.moc"
