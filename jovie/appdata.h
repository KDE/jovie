/*************************************************** vim:set ts=4 sw=4 sts=4:
  This class holds the data for a single application.
  It contains the application's default settings.
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

#ifndef APPDATA_H
#define APPDATA_H

// Qt includes.
#include <QtCore/QList>

// KDE includes.
#include <kspeech.h>

typedef QList<int> TJobList;
typedef TJobList* TJobListPtr;

class AppDataPrivate;
class AppData
{
public:
    /**
    * Constructs a new AppData object for the given DBUS AppId.
    * @param appId          DBUS sender id.
    */
    AppData(const QString& appId);
    
    /**
    * Destructor.
    */
    ~AppData();
    
    /**
    * Returns the appId for the application.
    */
    QString appId() const;
    
    /**
    * Sets the appId for the application.
    */
    void setAppId(const QString& appId);
    
    /**
    * Returns the friendly display name for the application.
    * These generally are not translated.
    */
    QString applicationName() const;
    
    /**
    * Sets the friendly display name for the application.
    * @param applicationName    Friendly application name.
    *
    * If not set, the AppId is used as the applicationName.
    */
    void setApplicationName(const QString& applicationName);
    
    /**
    * Returns the default talker code for the application.
    * Defaults to "", i.e., use the default talker configured in the system.
    */
    QString defaultTalker() const;
    
    /**
    * Sets the default talker code for the application.
    * @param defaultTalker  Talker code.
    */
    void setDefaultTalker(const QString& defaultTalker);
    
    /**
    * Returns the default priority (job type) for the application.
    */
    KSpeech::JobPriority defaultPriority() const;
    
    /**
    * Set the default priority (job type) for the application.
    * @param defaultPriority    Job Priority.
    */
    void setDefaultPriority(KSpeech::JobPriority defaultPriority);
    
    /**
    * Returns the GREP pattern that will be used as the sentence delimiter.
    *
    * The default delimiter is
      @verbatim
         ([\\.\\?\\!\\:\\;])\\s
      @endverbatim
    *
    * @see setSentenceDelimiter
    */
    QString sentenceDelimiter() const;

    /**
    * Sets the GREP pattern that will be used as the sentence delimiter.
    * @param sentenceDelimiter      A valid GREP pattern.
    *
    * Note that backward slashes must be escaped.
    *
    * @see sentenceDelimiter, sentenceparsing
    */
    void setSentenceDelimiter(const QString& sentenceDelimiter);

    /**
    * Returns the applications's current filtering enabled flag.
    */
    bool filteringOn() const;

    /**
    * Sets the applications's current filtering enabled flag.
    * @param filteringOn    True or False.
    */
    void setFilteringOn(bool filteringOn);
    
    /**
    * Returns whether the jobs of the application are currently paused.
    */
    bool isApplicationPaused() const;
    
    /**
    * Sets whether the jobs of the application are currently paused.
    * @param isApplicationPaused    True of False.
    */
    void setIsApplicationPaused(bool isApplicationPaused);
    
    /**
    * Returns the full path name of the XSLT file that performs
    * HTML filtering on jobs for the application.
    */
    QString htmlFilterXsltFile() const;
    
    /**
    * Sets the full path name of the XSLT file that performs
    * HTML filtering on jobs for the application.
    * @param filename       Name of the XSLT file.  Full path name.
    */
    void setHtmlFilterXsltFile(const QString& filename);
    
    /**
    * Returns the full path name of the XSLT file that performs
    * SSML filtering on jobs for the application.
    */
    QString ssmlFilterXsltFile() const;
    
    /**
    * Sets the full path name of the XSLT file that performs
    * SSML filtering on jobs for the application.
    * @param filename       Name of the XSLT file.  Full path name.
    */
    void setSsmlFilterXsltFile(const QString& filename);
    
    /**
    * Returns if KTTSD should attempt to automatically configure
    * talkers to meet requested talker attributes.
    */
    bool autoConfigureTalkersOn() const;
    
    /**
    * Sets whether KTTSD should attempt to automatically configure
    * talkers to meet requested talker attributes.
    * @param autoConfigureTalkersOn True if autoconfigure should be on.
    */
    void setAutoConfigureTalkersOn(bool autoConfigureTalkersOn);
    
    /**
    * Returns whether this application is a KTTS System Manager.
    * When an application is a System Manager, commands affect all jobs.
    * For example, a pause() command will pause all jobs of all applications.
    * Defaults to False.
    */
    bool isSystemManager() const;
    
    /**
    * Sets whether this application is a KTTS System Manager.
    * @param isSystemManager    True or False.
    */
    void setIsSystemManager(bool isSystemManager);
    
    /**
    * Return the JobNum of the last job queued by the application.
    * 0 if none.
    */
    int lastJobNum() const;
    
    /**
    * List of jobs for this app.  Caller may add or delete jobs,
    * but must not delete the list.
    */
    TJobListPtr jobList() const;
    
    /**
    * True when the app has exited.
    */
    bool unregistered() const;
    void setUnregistered(bool unregistered);

    // void debugDump();

private:
    AppDataPrivate* d;    
};

#endif // APPDATA_H
