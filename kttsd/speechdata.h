/*************************************************** vim:set ts=4 sw=4 sts=4:
  This contains the SpeechData class which is in charge of maintaining
  all the speech data.
  We could say that this is the common repository between the KTTSD class
  (dbus service) and the Speaker class (speaker, loads plug ins, call plug in
  functions)
  -------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

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

#ifndef SPEECHDATA_H
#define SPEECHDATA_H

// Qt includes.
#include <QtCore/QByteArray>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QMultiHash>
#include <QtCore/QMap>

// KDE includes.
#include <kconfig.h>
#include <kspeech.h>

// KTTSD includes.
#include "speechjob.h"
#include "talkercode.h"
#include "filtermgr.h"
#include "appdata.h"
#include "configdata.h"


class SpeechDataPrivate;
class SpeechData : public QObject {
    Q_OBJECT

public:
    /**
    * singleton accessor
    */
    static SpeechData * Instance();

    /**
    * Destructor
    */
    ~SpeechData();

Q_SIGNALS:
    /**
    * Emitted when the state of a job changes.
    */
    void jobStateChanged(const QString& appId, int jobNum, KSpeech::JobState state);
    
    /**
    * Emitted when job filtering completes.
    */
    void filteringFinished();

private slots:
    void slotFilterMgrFinished();
    void slotFilterMgrStopped();
    
    void slotServiceUnregistered(const QString& serviceName);

private:
    /**
    * Constructor
    */
    SpeechData();

    /**
    * Determines whether the given text is SSML markup.
    */
    bool isSsml(const QString &text);

    /**
    * Parses a block of text into sentences using the application-specified regular expression
    * or (if not specified), the default regular expression.
    * @param text           The message to be spoken.
    * @param appId          The DBUS senderId of the application.
    * @return               List of parsed sentences.
    */

    QStringList parseText(const QString &text, const QString &appId);

    /**
    * Deletes job, removing it from all queues.
    */
    void deleteJob(int removeJobNum);

    /**
    * Assigns a FilterMgr to a job and starts filtering on it.
    */
    void startJobFiltering(SpeechJob* job, const QString& text, bool noSBD);

    /**
    * Waits for filtering to be completed on a job.
    * This is typically called because an app has requested job info that requires
    * filtering to be completed, such as getJobInfo.
    */
    void waitJobFiltering(const SpeechJob* job);

    /**
    * Processes filters by looping across the pool of FilterMgrs.
    * As each FilterMgr finishes, emits appropriate signals and flags it as no longer busy.
    */
    void doFiltering();

    /**
    * Checks to see if an application has active jobs, and if not and
    * the application has exited, deletes the app and all its jobs.
    * @param appId          DBUS sender id of the application.
    */
    void deleteExpiredApp(const QString appId);

private:
    SpeechDataPrivate* d;
    static SpeechData * m_instance;
};

#endif // SPEECHDATA_H
