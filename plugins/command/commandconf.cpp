/***************************************************** vim:set ts=4 sw=4 sts=4:
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

// TQt includes.
#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqapplication.h>
#include <tqtextcodec.h>

// KDE includes.
#include <kdialog.h>
#include <kdebug.h>
#include <klocale.h>
#include <kdialog.h>
#include <kcombobox.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kprogress.h>

// KTTS includes.
#include <pluginconf.h>
#include <testplayer.h>

// Command Plugin includes.
#include "commandproc.h"
#include "commandconf.h"

/** Constructor */
CommandConf::CommandConf( TQWidget* tqparent, const char* name, const TQStringList& /*args*/) :
    PlugInConf(tqparent, name)
{
    // kdDebug() << "CommandConf::CommandConf: Running" << endl;
    m_commandProc = 0;
    m_progressDlg = 0;

    TQVBoxLayout *tqlayout = new TQVBoxLayout(this, KDialog::marginHint(),
        KDialog::spacingHint(), "CommandConfigWidgetLayout");
    tqlayout->tqsetAlignment (TQt::AlignTop);
    m_widget = new CommandConfWidget(this, "CommandConfigWidget");
    tqlayout->addWidget(m_widget);

    // Build codec list and fill combobox.
    m_codecList = PlugInProc::buildCodecList();
    m_widget->characterCodingBox->clear();
    m_widget->characterCodingBox->insertStringList(m_codecList);

    defaults();
    connect(m_widget->characterCodingBox, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->characterCodingBox, TQT_SIGNAL(activated(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->stdInButton, TQT_SIGNAL(toggled(bool)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->urlReq, TQT_SIGNAL(textChanged(const TQString&)),
        this, TQT_SLOT(configChanged()));
    connect(m_widget->commandTestButton, TQT_SIGNAL(clicked()),
        this, TQT_SLOT(slotCommandTest_clicked()));
}

/** Destructor */
CommandConf::~CommandConf()
{
    // kdDebug() << "CommandConf::~CommandConf: Running" << endl;
    if (!m_waveFile.isNull()) TQFile::remove(m_waveFile);
    delete m_commandProc;
    delete m_progressDlg;
}

void CommandConf::load(KConfig *config, const TQString &configGroup) {
    // kdDebug() << "CommandConf::load: Running" << endl;
    config->setGroup(configGroup);
    m_widget->urlReq->setURL (config->readEntry("Command", "cat -"));
    m_widget->stdInButton->setChecked(config->readBoolEntry("StdIn", false));
    TQString codecString = config->readEntry("Codec", "Local");
    m_languageCode = config->readEntry("LanguageCode", m_languageCode);
    int codec = PlugInProc::codecNameToListIndex(codecString, m_codecList);
    m_widget->characterCodingBox->setCurrentItem(codec);
}

void CommandConf::save(KConfig *config, const TQString &configGroup) {
    // kdDebug() << "CommandConf::save: Running" << endl;
    config->setGroup(configGroup);
    config->writeEntry("Command", m_widget->urlReq->url());
    config->writeEntry("StdIn", m_widget->stdInButton->isChecked());
    int codec = m_widget->characterCodingBox->currentItem();
    config->writeEntry("Codec", PlugInProc::codecIndexToCodecName(codec, m_codecList));
}

void CommandConf::defaults(){
    // kdDebug() << "CommandConf::defaults: Running" << endl;
    m_widget->urlReq->setURL("cat -");
    m_widget->stdInButton->setChecked(false);
    m_widget->urlReq->setShowLocalProtocol (false);
    m_widget->characterCodingBox->setCurrentItem(0);
}

void CommandConf::setDesiredLanguage(const TQString &lang)
{
    m_languageCode = lang;
}

TQString CommandConf::getTalkerCode()
{
    TQString url = m_widget->urlReq->url();
    if (!url.isEmpty())
    {
        // Must contain either text or file parameter, or StdIn checkbox must be checked,
        // otherwise, does nothing!
        if ((url.tqcontains("%t") > 0) || (url.tqcontains("%f") > 0) || m_widget->stdInButton->isChecked())
        {
            return TQString(
                "<voice lang=\"%1\" name=\"%2\" gender=\"%3\" />"
                "<prosody volume=\"%4\" rate=\"%5\" />"
                "<kttsd synthesizer=\"%6\" />")
                .tqarg(m_languageCode)
                .tqarg("fixed")
                .tqarg("neutral")
                .tqarg("medium")
                .tqarg("medium")
                .tqarg("Command");
        }
    }
    return TQString();
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
        connect (m_commandProc, TQT_SIGNAL(stopped()), this, TQT_SLOT(slotSynthStopped()));
    }

    // Create a temp file name for the wave file.
    KTempFile tempFile (locateLocal("tmp", "commandplugin-"), ".wav");
    TQString tmpWaveFile = tempFile.file()->name();
    tempFile.close();

    // Get test message in the language of the voice.
    TQString testMsg = testMessage(m_languageCode);

    // Tell user to wait.
    m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_command_testdlg",
        i18n("Testing"),
        i18n("Testing."),
        true);
    m_progressDlg->progressBar()->hide();
    m_progressDlg->setAllowCancel(true);

    // TODO: Do codec names contain non-ASCII characters?
    connect (m_commandProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
    m_commandProc->synth(
        testMsg,
        tmpWaveFile,
        m_widget->urlReq->url(),
        m_widget->stdInButton->isChecked(),
        PlugInProc::codecIndexToCodec(m_widget->characterCodingBox->currentItem(), m_codecList),
        m_languageCode);

    // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
    // or if user clicks Cancel button.
    m_progressDlg->exec();
    disconnect (m_commandProc, TQT_SIGNAL(synthFinished()), this, TQT_SLOT(slotSynthFinished()));
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
    // Get new wavefile name.
    m_waveFile = m_commandProc->getFilename();
    // Tell synth we're done.
    m_commandProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(m_waveFile);
    TQFile::remove(m_waveFile);
    m_waveFile = TQString();
    if (m_progressDlg) m_progressDlg->close();
}

void CommandConf::slotSynthStopped()
{
    // Clean up after canceling test.
    TQString filename = m_commandProc->getFilename();
    if (!filename.isNull()) TQFile::remove(filename);
}

#include "commandconf.moc"
