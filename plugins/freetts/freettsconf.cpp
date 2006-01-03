/****************************************************************************
	Configuration widget and functions for FreeTTS (interactive) plug in
	-------------------
	Copyright : (C) 2004 Paul Giannaros
	-------------------
	Original author: Paul Giannaros <ceruleanblaze@gmail.com>
	Current Maintainer: Paul Giannaros <ceruleanblaze@gmail.com>
 ******************************************************************************/

/***************************************************************************
 *																					*
 *	 This program is free software; you can redistribute it and/or modify	*
 *	 it under the terms of the GNU General Public License as published by	*
 *	 the Free Software Foundation; version 2 of the License.				 *
 *																					 *
 ***************************************************************************/

// Qt includes. 
#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QVBoxLayout>

// KDE includes.
#include <kdialog.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kprogress.h>

// KTTS includes.
#include <pluginconf.h>
#include <testplayer.h>

// FreeTTS includes.
#include "freettsconf.h"
#include "freettsconfigwidget.h"

/** Constructor */
FreeTTSConf::FreeTTSConf( QWidget* parent, const char* name, const QStringList&/*args*/) : 
	PlugInConf( parent, name ) {
	
	// kdDebug() << "FreeTTSConf::FreeTTSConf: Running" << endl;
	m_freettsProc = 0;
        m_progressDlg = 0;
	
	QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
								KDialog::spacingHint(), "FreeTTSConfigWidgetLayout");
	layout->setAlignment (Qt::AlignTop);
	m_widget = new FreeTTSConfWidget(this, "FreeTTSConfigWidget");
	layout->addWidget(m_widget);
		
	defaults();
	
	connect(m_widget->freettsPath, SIGNAL(textChanged(const QString&)),
		this, SLOT(configChanged()));
	connect(m_widget->freettsTest, SIGNAL(clicked()), this, SLOT(slotFreeTTSTest_clicked()));
}

/** Destructor */
FreeTTSConf::~FreeTTSConf() {
	// kdDebug() << "Running: FreeTTSConf::~FreeTTSConf()" << endl;
	if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
	delete m_freettsProc;
        delete m_progressDlg;
}

void FreeTTSConf::load(KConfig *config, const QString &configGroup) {
	// kdDebug() << "FreeTTSConf::load: Running" << endl;

	config->setGroup(configGroup);
        QString freeTTSJar = config->readEntry("FreeTTSJarPath", QString());
        if (freeTTSJar.isEmpty())
        {
            config->setGroup("FreeTTS");
            freeTTSJar = config->readEntry("FreeTTSJarPath", QString());
        }
	if (freeTTSJar.isEmpty())
	    freeTTSJar = getLocation("freetts.jar");
        m_widget->freettsPath->setURL(freeTTSJar);
	/// If freettsPath is still empty, then we couldn't find the file in the path.
}

void FreeTTSConf::save(KConfig *config, const QString &configGroup){
	// kdDebug() << "FreeTTSConf::save: Running" << endl;

    config->setGroup("FreeTTS");
    config->writeEntry("FreeTTSJarPath",
        realFilePath(m_widget->freettsPath->url()));

    config->setGroup(configGroup);
    if(m_widget->freettsPath->url().isEmpty())
        KMessageBox::sorry(0, i18n("Unable to locate freetts.jar in your path.\nPlease specify the path to freetts.jar in the Properties tab before using KDE Text-to-Speech"), i18n("KDE Text-to-Speech"));
    config->writeEntry("FreeTTSJarPath",
        realFilePath(m_widget->freettsPath->url()));
}

void FreeTTSConf::defaults(){
	// kdDebug() << "Running: FreeTTSConf::defaults()" << endl;
	m_widget->freettsPath->setURL("");
}

void FreeTTSConf::setDesiredLanguage(const QString &lang)
{
    m_languageCode = lang;
}

QString FreeTTSConf::getTalkerCode()
{
    QString freeTTSJar = realFilePath(m_widget->freettsPath->url());
    if (!freeTTSJar.isEmpty())
    {
        if (!getLocation(freeTTSJar).isEmpty())
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
                    .arg("FreeTTS");
        }
    }
    return QString();
}

// QString FreeTTSConf::getLocation(const QString &name) {
// 	/// Iterate over the path and see if 'name' exists in it. Return the
// 	/// full path to it if it does. Else return an empty QString.
// 	kdDebug() << "FreeTTSConf::getLocation: Searching for " << name << " in the path... " << endl;
// 	kdDebug() << m_path << endl;
// 	for(QStringList::iterator it = m_path.begin(); it != m_path.end(); ++it) {
// 		QString fullName = *it;
// 		fullName += "/";
// 		fullName += name;
// 		/// The user either has the directory of the file in the path...
// 		if(QFile::exists(fullName)) {
// 			return fullName;
// 			kdDebug() << fullName << endl;
// 		}
// 		/// ....Or the file itself
// 		else if(QFileInfo(*it).baseName().append(QString(".").append(QFileInfo(*it).extension())) == name) {
// 			return fullName;
// 			kdDebug() << fullName << endl;
// 		}
// 	}
// 	return "";
// }


void FreeTTSConf::slotFreeTTSTest_clicked()
{
	// kdDebug() << "FreeTTSConf::slotFreeTTSTest_clicked(): Running" << endl;
        // If currently synthesizing, stop it.
	if (m_freettsProc)
		m_freettsProc->stopText();
	else
        {
		m_freettsProc = new FreeTTSProc();
                connect (m_freettsProc, SIGNAL(stopped()), this, SLOT(slotSynthStopped()));
        }
        // Create a temp file name for the wave file.
	KTempFile tempFile (locateLocal("tmp", "freettsplugin-"), ".wav");
	QString tmpWaveFile = tempFile.file()->name();
	tempFile.close();

    // Get test message in the language of the voice.
    QString testMsg = testMessage(m_languageCode);

        // Tell user to wait.
        m_progressDlg = new KProgressDialog(m_widget, "kttsmgr_freetts_testdlg",
            i18n("Testing"),
            i18n("Testing."),
            true);
        m_progressDlg->progressBar()->hide();
        m_progressDlg->setAllowCancel(true);

	// I think FreeTTS only officialy supports English, but if anyone knows of someone
	// whos built up a different language lexicon and has it working with FreeTTS gimme an email at ceruleanblaze@gmail.com
        connect (m_freettsProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
        m_freettsProc->synth(
            testMsg,
            tmpWaveFile,
            realFilePath(m_widget->freettsPath->url()));

        // Display progress dialog modally.  Processing continues when plugin signals synthFinished,
        // or if user clicks Cancel button.
        m_progressDlg->exec();
        disconnect (m_freettsProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
        if (m_progressDlg->wasCancelled()) m_freettsProc->stopText();
        delete m_progressDlg;
        m_progressDlg = 0;
}

void FreeTTSConf::slotSynthFinished()
{
    // If user canceled, progress dialog is gone, so exit.
    if (!m_progressDlg)
    {
        m_freettsProc->ackFinished();
        return;
    }
    // Hide the Cancel button so user can't cancel in the middle of playback.
    m_progressDlg->showCancelButton(false);
    // Get new wavefile name.
    m_waveFile = m_freettsProc->getFilename();
    // Tell synth we're done.
    m_freettsProc->ackFinished();
    // Play the wave file (possibly adjusting its Speed).
    // Player object deletes the wave file when done.
    if (m_player) m_player->play(m_waveFile);
    QFile::remove(m_waveFile);
    m_waveFile.clear();
    if (m_progressDlg) m_progressDlg->close();
}

void FreeTTSConf::slotSynthStopped()
{
    // Clean up after canceling test.
    QString filename = m_freettsProc->getFilename();
    if (!filename.isNull()) QFile::remove(filename);
}

#include "freettsconf.moc"
