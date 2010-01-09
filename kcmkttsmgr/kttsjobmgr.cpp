/***************************************************** vim:set ts=4 sw=4 sts=4:
  A KPart to display running jobs in KTTSD and permit user to stop, cancel, pause,
  resume, change Talker, etc.
  -------------------
  Copyright : (C) 2004,2005 by Gary Cramblitt <garycramblitt@comcast.net>
  Copyright : (C) 2009 by Jeremy Whiting <jpwhiting@kde.org>
  -------------------

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// KttsJobMgr includes.
#include "kttsjobmgr.h"
#include "kttsjobmgr.moc"
#include "ui_kttsjobmgr.h"

// QT includes.
#include <QtCore/QObject>
#include <QtGui/QLabel>
#include <QtGui/QSplitter>
#include <QtGui/QClipboard>
#include <QtGui/QPushButton>
#include <QtCore/QList>
#include <QtGui/QTreeView>
#include <QtCore/QMimeData>
#include <QtDBus/QtDBus>

// KDE includes.
#include <kcomponentdata.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kicon.h>
#include <kdebug.h>
#include <kencodingfiledialog.h>
#include <kinputdialog.h>
#include <ktextedit.h>
#include <kvbox.h>
#include <kdemacros.h>
#include <kparts/genericfactory.h>
#include <kspeech.h>

// KTTS includes.
#include "talkercode.h"
#include "selecttalkerdlg.h"

KttsJobMgr::KttsJobMgr(QWidget *parent) :
    QWidget(parent)
{
    m_ui = new Ui::kttsjobmgr;
    m_ui->setupUi(this);

//DBusAbstractInterfacePrivate
    m_kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());
    m_kspeech->setParent(this);

    // Establish ourself as a System Manager.
    m_kspeech->setApplicationName("KCMKttsMgr");
    m_kspeech->setIsSystemManager(true);

    // All the ktts components use the same catalog.
    KGlobal::locale()->insertCatalog("kttsd");

    connect (m_ui->speedSlider, SIGNAL(valueChanged(int)), this, SIGNAL(configChanged()));
    connect (m_ui->pitchSlider, SIGNAL(valueChanged(int)), this, SIGNAL(configChanged()));
    connect (m_ui->volumeSlider, SIGNAL(valueChanged(int)), this, SIGNAL(configChanged()));
    
    connect (m_ui->moduleComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_moduleChanged(const QString &)));
    connect (m_ui->languageComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(slot_languageChanged(const QString &)));
    connect (m_ui->voiceComboBox, SIGNAL(currentIndexChanged(int)), this, SIGNAL(configChanged()));

    m_ui->stopButton->setIcon(KIcon("media-playback-stop"));
    connect (m_ui->stopButton, SIGNAL(clicked()), this, SLOT(slot_stop()));
    m_ui->cancelButton->setIcon(KIcon("edit-clear"));
    connect (m_ui->cancelButton, SIGNAL(clicked()), this, SLOT(slot_cancel()));
    m_ui->pauseButton->setIcon(KIcon("media-playback-pause"));
    connect (m_ui->pauseButton, SIGNAL(clicked()), this, SLOT(slot_pause()));
    m_ui->resumeButton->setIcon(KIcon("media-playback-start"));
    connect (m_ui->resumeButton, SIGNAL(clicked()), this, SLOT(slot_resume()));

    m_ui->speak_clipboard->setIcon(KIcon("klipper"));
    connect (m_ui->speak_clipboard, SIGNAL(clicked()), this, SLOT(slot_speak_clipboard()));
    m_ui->speak_file->setIcon(KIcon("document-open"));
    connect (m_ui->speak_file, SIGNAL(clicked()), this, SLOT(slot_speak_file()));
    m_ui->job_changetalker->setIcon(KIcon("translate"));
    connect (m_ui->job_changetalker, SIGNAL(clicked()), this, SLOT(slot_job_change_talker()));

    // Set the main widget for the part.

    // Connect DBUS Signals emitted by KTTSD to our own slots.
    connect(m_kspeech, SIGNAL(kttsdStarted()),
        this, SLOT(kttsdStarted()));
    connect(m_kspeech, SIGNAL(jobStateChanged(const QString&, int, int)),
        this, SLOT(jobStateChanged(const QString&, int, int)));
    connect(m_kspeech, SIGNAL(marker(const QString&, int, int, const QString&)),
        this, SLOT(marker(const QString&, int, int, const QString&)));
}

KttsJobMgr::~KttsJobMgr()
{
    KGlobal::locale()->removeCatalog("kttsd");
    delete m_ui;
}

/**
* Slots connected to buttons.
*/
void KttsJobMgr::slot_stop()
{
    m_kspeech->stop();
}

void KttsJobMgr::slot_cancel()
{
    m_kspeech->cancel();
}

void KttsJobMgr::slot_pause()
{
    m_kspeech->pause();
}

void KttsJobMgr::slot_resume()
{
    m_kspeech->resume();
}

void KttsJobMgr::save()
{
    m_kspeech->setSpeed(m_ui->speedSlider->value());
    m_kspeech->setPitch(m_ui->pitchSlider->value());
    m_kspeech->setVolume(m_ui->volumeSlider->value());
    m_kspeech->setVoiceType(m_ui->voiceComboBox->currentIndex() + 1);
}

void KttsJobMgr::load()
{
}

void KttsJobMgr::slot_moduleChanged(const QString & module)
{
    kDebug() << "changing the output module to " << module;
    m_kspeech->setOutputModule(module);
    emit configChanged();
}

void KttsJobMgr::slot_languageChanged(const QString & language)
{
    kDebug() << "changing the language to " << language;
    m_kspeech->setLanguage(language);
    emit configChanged();
}

void KttsJobMgr::slot_job_change_talker()
{
    //QModelIndex index = m_ui->m_jobTableView->currentIndex();
    //if (index.isValid())
    //{
    //    JobInfo job = m_jobListModel->getRow(index.row());
    //    QString talkerID = job.talkerID;
    //    QStringList talkerIDs = m_talkerCodesToTalkerIDs.values();
    //    int ndx = talkerIDs.indexOf(talkerID);
    //    QString talkerCode;
    //    if (ndx >= 0)
    //        talkerCode = m_talkerCodesToTalkerIDs.keys()[ndx];
    //    QPointer<SelectTalkerDlg> dlg = new SelectTalkerDlg(widget(), "selecttalkerdialog", i18n("Select Talker"), talkerCode, true);
    //    int dlgResult = dlg->exec();
    //    if (dlgResult != KDialog::Accepted)
    //        return;
    //    talkerCode = dlg->getSelectedTalkerCode();
    //    int jobNum = job.jobNum;
    //    m_kspeech->changeJobTalker(jobNum, talkerCode);
    //    refreshJob(jobNum);
    //}
}

void KttsJobMgr::slot_speak_clipboard()
{
    // kDebug() << "KttsJobMgr::slot_speak_clipboard: running";

    // Get the clipboard object.
    QClipboard *cb = QApplication::clipboard();

    // Copy text from the clipboard.
    QString text;
    KSpeech::SayOptions sayOptions = KSpeech::soNone;
    const QMimeData* data = cb->mimeData();
    if (data)
    {
        if (data->hasFormat("text/html"))
        {
            // if (m_kspeech->supportsMarkup(NULL, KSpeech::mtHtml))
                text = data->html();
                sayOptions = KSpeech::soHtml;
        }
        if (data->hasFormat("text/ssml"))
        {
            // if (m_kspeech->supportsMarkup(NULL, KSpeech::mtSsml))
            {
                QByteArray d = data->data("text/ssml");
                text = QString(d);
                sayOptions = KSpeech::soSsml;
            }
        }
    }
    if (text.isEmpty()) {
        text = cb->text();
        sayOptions = KSpeech::soPlainText;
    }

    // Speak it.
    if ( !text.isEmpty() )
    {
        m_kspeech->say(text, sayOptions);
        // int jobNum = m_kspeech->say(text, sayOptions);
        // kDebug() << "KttsJobMgr::slot_speak_clipboard: started jobNum " << jobNum;
    }
}

void KttsJobMgr::slot_speak_file()
{
    KEncodingFileDialog dlg;
    KEncodingFileDialog::Result result = dlg.getOpenFileNameAndEncoding();
    if (result.fileNames.count() == 1)
    {
        // kDebug() << "KttsJobMgr::slot_speak_file: calling setFile with filename " <<
        //     result.fileNames[0] << " and encoding " << result.encoding << endl;
        m_kspeech->sayFile(result.fileNames[0], result.encoding);
    }
}

/**
* Return the Talker ID corresponding to a Talker Code, retrieving from cached list if present.
* @param talkerCode    Talker Code.
* @return              Talker ID.
*/
QString KttsJobMgr::cachedTalkerCodeToTalkerID(const QString& talkerCode)
{
    // If in the cache, return that.
    if (m_talkerCodesToTalkerIDs.contains(talkerCode))
        return m_talkerCodesToTalkerIDs[talkerCode];
    else
    {
        // Otherwise, retrieve Talker ID from KTTSD and cache it.
        QString talkerID = m_kspeech->talkerToTalkerId(talkerCode);
        m_talkerCodesToTalkerIDs[talkerCode] = talkerID;
        // kDebug() << "KttsJobMgr::cachedTalkerCodeToTalkerID: talkerCode = " << talkerCode << " talkerID = " << talkerID;
        return talkerID;
    }
}

/** Slots connected to DBUS Signals emitted by KTTSD. */

/**
* This signal is emitted when KTTSD starts or restarts after a call to reinit.
*/
Q_SCRIPTABLE void KttsJobMgr::kttsdStarted()
{
}
