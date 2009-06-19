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

#include "appdata.h"

#include "kdebug.h"

/* -------------------------------------------------------------------------- */

class AppDataPrivate
{
public:
    AppDataPrivate(const QString& newAppId) :
        appId(newAppId),
        applicationName(appId),
        defaultTalker(""),
        defaultPriority(KSpeech::jpMessage),
        sentenceDelimiter("([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))"),
        filteringOn(true),
        isApplicationPaused(false),
        htmlFilterXsltFile(""),
        ssmlFilterXsltFile(""),
        autoConfigureTalkersOn(false),
        isSystemManager(false),
        jobList(),
        unregistered(false) {}
        
    friend class AppData;

protected:
    QString appId;
    QString applicationName;
    QString defaultTalker;
    KSpeech::JobPriority defaultPriority;
    QString sentenceDelimiter;
    bool filteringOn;
    bool isApplicationPaused;
    QString htmlFilterXsltFile;
    QString ssmlFilterXsltFile;
    bool autoConfigureTalkersOn;
    bool isSystemManager;
    TJobList jobList;
    bool unregistered;
};

/* -------------------------------------------------------------------------- */

AppData::AppData(const QString& appId) { d = new AppDataPrivate(appId); }
AppData::~AppData() { delete d; }

QString AppData::appId() const { return d->appId; }
void AppData::setAppId(const QString& appId) { d->appId = appId; }
QString AppData::applicationName() const { return d->applicationName; }
void AppData::setApplicationName(const QString& applicationName) { d->applicationName = applicationName; }
QString AppData::defaultTalker() const { return d->defaultTalker; }
void AppData::setDefaultTalker(const QString& defaultTalker) { d->defaultTalker = defaultTalker; }
KSpeech::JobPriority AppData::defaultPriority() const { return d->defaultPriority; }
void AppData::setDefaultPriority(KSpeech::JobPriority priority) { d->defaultPriority = priority; }
QString AppData::sentenceDelimiter() const { return d->sentenceDelimiter; }
void AppData::setSentenceDelimiter(const QString& sentenceDelimiter) { d->sentenceDelimiter = sentenceDelimiter; }
bool AppData::filteringOn() const { return d->filteringOn; }
void AppData::setFilteringOn(bool filteringOn) { d->filteringOn = filteringOn; }
bool AppData::isApplicationPaused() const { return d->isApplicationPaused; }
void AppData::setIsApplicationPaused(bool isApplicationPaused) { d->isApplicationPaused = isApplicationPaused; }
QString AppData::htmlFilterXsltFile() const { return d->htmlFilterXsltFile; }
void AppData::setHtmlFilterXsltFile(const QString& filename) { d->htmlFilterXsltFile = filename; }
QString AppData::ssmlFilterXsltFile() const { return d->ssmlFilterXsltFile; }
void AppData::setSsmlFilterXsltFile(const QString& filename) { d->ssmlFilterXsltFile = filename; }
bool AppData::autoConfigureTalkersOn() const { return d->autoConfigureTalkersOn; }
void AppData::setAutoConfigureTalkersOn(bool autoConfigureTalkersOn) { d->autoConfigureTalkersOn = autoConfigureTalkersOn; }
bool AppData::isSystemManager() const { return d->isSystemManager; }
void AppData::setIsSystemManager(bool isSystemManager) { d->isSystemManager = isSystemManager; }
int AppData::lastJobNum() const 
{
    if (d->jobList.isEmpty())
        return 0;
    else
        return d->jobList.last();
}
TJobListPtr AppData::jobList() const { return &(d->jobList); }
bool AppData::unregistered() const { return d->unregistered; }
void AppData::setUnregistered(bool unregistered) { d->unregistered = unregistered; }

/*
void AppData::debugDump()
{
    kDebug() << "AppData::debugDump: appId = " << appId() << " applicationName = " << applicationName()
        << " defaultTalker = " << defaultTalker() << " defaultPriority = " << defaultPriority()
        << " sentenceDelimiter = " << sentenceDelimiter() << " filteringOn = " << filteringOn()
        << " isApplicationPaused = " << isApplicationPaused() << " htmlFilterXsltFile = " << htmlFilterXsltFile()
        << " ssmlFilterXsltFile = " << ssmlFilterXsltFile() << " autoConfigureTalkersOn = "
        << autoConfigureTalkersOn() << " isSystemManager = " << isSystemManager()
        << " lastJobNum = " << lastJobNum() << endl;
}
*/
