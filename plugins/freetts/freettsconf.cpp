/****************************************************************************
	freettsconf.cpp
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

#include <qlayout.h>
#include <qlabel.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qfile.h>
#include <qapplication.h>

#include <kdialog.h>
#include <kartsserver.h>
#include <kartsdispatcher.h>
#include <kplayobject.h>
#include <kplayobjectfactory.h>
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

#include <pluginconf.h>

#include "freettsconf.h"
#include "freettsconfigwidget.h"

/** Constructor */
FreeTTSConf::FreeTTSConf( QWidget* parent, const char* name, const QStringList&/*args*/) : 
	PlugInConf( parent, name ){
	
	kdDebug() << "FreeTTSConf::FreeTTSConf: Running" << endl;
	m_freettsProc = 0;
	m_artsServer = 0;
	m_playObj = 0;
	
	QVBoxLayout *layout = new QVBoxLayout(this, KDialog::marginHint(),
								KDialog::spacingHint(), "FreeTTSConfigWidgetLayout");
	layout->setAlignment (Qt::AlignTop);
	m_widget = new FreeTTSConfWidget(this, "FreeTTSConfigWidget");
	layout->addWidget(m_widget);
		
	defaults();
	
	connect(m_widget->freettsPath, SIGNAL(textChanged(const QString&)),
		this, SLOT(configChanged()));
	connect(m_widget->freettsTest, SIGNAL(clicked()), this, SLOT(slotFreeTTSTest_clicked()));
	
	QString systemPath(getenv("PATH"));
// 	kdDebug() << "Path is " << systemPath << endl;
	m_path = QStringList::split(":", systemPath);
}

/** Destructor */
FreeTTSConf::~FreeTTSConf(){
	kdDebug() << "Running: FreeTTSConf::~FreeTTSConf()" << endl;
	if (m_playObj) m_playObj->halt();
	delete m_playObj;
	delete m_artsServer;
	if (!m_waveFile.isNull()) QFile::remove(m_waveFile);
	delete m_freettsProc;
}

void FreeTTSConf::load(KConfig *config, const QString &langGroup) {
	static bool shown = false;
	kdDebug() << "FreeTTSConf::load: Loading configuration for language " << langGroup << " with plug in " << "FreeTTS" << endl;

	config->setGroup(langGroup);
	m_widget->freettsPath->setURL(config->readPathEntry("FreeTTSJarPath", ""));
	
	/// Changed:
	/// Doesn't set the url default to freetts.jar, but checks for the entry in the path. If it doesn't exist, it
	/// tells the user to specify it.
	
	if(m_widget->freettsPath->url().isEmpty())
		m_widget->freettsPath->setURL(getLocation("freetts.jar"));
	/// If freettsPath is still empty, then we couldn't find the file in the path.
	if(m_widget->freettsPath->url().isEmpty()) {
		if(!shown) 
			KMessageBox::sorry(0, i18n("Unable to locate freetts.jar in your path.\nPlease specify the path to freetts.jar in the Properties tab before using KDE Text-to-Speech"), i18n("KDE Text-to-Speech"));
		shown = true;
	}	
}

void FreeTTSConf::save(KConfig *config, const QString &langGroup){
	kdDebug() << "FreeTTSConf::save: Saving configuration for language " << langGroup << " with plug in " << "FreeTTS" << endl;

	config->setGroup(langGroup);
	config->writePathEntry("FreeTTSJarPath", m_widget->freettsPath->url());
}

void FreeTTSConf::defaults(){
	kdDebug() << "Running: FreeTTSConf::defaults()" << endl;
	m_widget->freettsPath->setURL("freetts.jar");
}


QString FreeTTSConf::getLocation(const QString &name) {
	/// Iterate over the path and see if 'name' exists in it. Return the
	/// full path to it if it does. Else return an empty QString.
	kdDebug() << "FreeTTSConf::getLocation: Searching for " << name << " in the path... " << endl;
	kdDebug() << m_path << endl;
	for(QStringList::iterator it = m_path.begin(); it != m_path.end(); ++it) {
		QString fullName = *it;
		fullName += "/";
		fullName += name;
		/// The user either has the directory of the file in the path...
		if(QFile::exists(fullName)) {
			return fullName;
			kdDebug() << fullName << endl;
		}
		/// ....Or the file itself
		else if(QFileInfo(*it).baseName().append(QString(".").append(QFileInfo(*it).extension())) == name) {
			return fullName;
			kdDebug() << fullName << endl;
		}
	}
	return "";
}


void FreeTTSConf::slotFreeTTSTest_clicked()
{
	kdDebug() << "FreeTTSConf::slotFreeTTSTest_clicked(): Running" << endl;
    // If currently synthesizing, stop it.
	if (m_freettsProc)
		m_freettsProc->stopText();
	else {
		m_freettsProc = new FreeTTSProc();
		connect (m_freettsProc, SIGNAL(synthFinished()), this, SLOT(slotSynthFinished()));
	}
    // Create a temp file name for the wave file.
	KTempFile tempFile (locateLocal("tmp", "freettsplugin-"), ".wav");
	QString tmpWaveFile = tempFile.file()->name();
	tempFile.close();
	// Play an English test.  I think FreeTTS only officialy supports English, but if anyone knows of someone
	// whos built up a different language lexicon and has it working with FreeTTS gimme an email at ceruleanblaze@gmail.com
	m_freettsProc->synth(
			"K D E is a modern graphical desktop for Unix computers.",
	tmpWaveFile,
	m_widget->freettsPath->url());
}

void FreeTTSConf::slotSynthFinished()
{
    // If currently playing (or finished playing), stop and delete play object.
	if (m_playObj) {
		m_playObj->halt();
       // Clean up.
		QFile::remove(m_waveFile);
	}
	delete m_playObj;
	delete m_artsServer;
    // Get new wavefile name.
	m_waveFile = m_freettsProc->getFilename();
	m_freettsProc->ackFinished();
    // Start playback of the wave file.
	KArtsDispatcher dispatcher;
	m_artsServer = new KArtsServer;
	KDE::PlayObjectFactory factory (m_artsServer->server());
	m_playObj = factory.createPlayObject (m_waveFile, true);
	m_playObj->play();

    // TODO: The following hunk of code would ideally be unnecessary.  We would just
    // return at this point and let FreeTTSConf destructor take care of
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

#include "freettsconf.moc"
