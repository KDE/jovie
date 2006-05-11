/***************************************************** vim:set ts=4 sw=4 sts=4:
  Manages all the Talker (synth) plugins.
  -------------------
  Copyright:
  (C) 2004-2005 by Gary Cramblitt <garycramblitt@comcast.net>
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

#ifndef _TALKERMGR_H_
#define _TALKERMGR_H_

// Qt includes.
#include <QString>
#include <QStringList>
#include <qmap.h>
#include <QList>

// KTTS includes.
#include "talkercode.h"
#include "pluginproc.h"

typedef QList<PlugInProc*> PlugInList;

class TalkerMgr: public QObject
{
public:

    /**
     * Constructor.
     */
    TalkerMgr(QObject *parent = 0, const char *name = 0);

    /**
     * Destructor.
     */
    ~TalkerMgr();

    /**
     * Load all the configured plug ins populating loadedPlugIns
     */
    int loadPlugIns(KConfig* config);

    /**
     * Get a list of the talkers configured in KTTS.
     * @return               A QStringList of fully-specified talker codes, one
     *                       for each talker user has configured.
     */
    QStringList getTalkers();

    /**
     * Returns a list of all the loaded plugins.
     */
    PlugInList getLoadedPlugIns();

    /**
     * Given a talker code, returns pointer to the closest matching plugin.
     * @param talker          The talker (language) code.
     * @return                Index to m_loadedPlugins array of Talkers.
     *
     * If a plugin has not been loaded to match the talker, returns the default
     * plugin.
     */
    int talkerToPluginIndex(const QString& talker) const;

    /**
     * Given a talker code, returns pointer to the closest matching plugin.
     * @param talker          The talker (language) code.
     * @return                Pointer to closest matching plugin.
     *
     * If a plugin has not been loaded to match the talker, returns the default
     * plugin.
     */
    PlugInProc* talkerToPlugin(const QString& talker) const;

    /**
     * Given a talker code, returns the parsed TalkerCode of the closest matching Talker.
     * @param talker          The talker (language) code.
     * @return                Parsed TalkerCode structure.
     *
     * If a plugin has not been loaded to match the talker, returns the default
     * plugin.
     *
     * The returned TalkerCode is a copy and should be destroyed by caller.
     *
     * TODO: When picking a talker, %KTTSD will automatically determine if text contains
     * markup and pick a talker that supports that markup, if available.  This
     * overrides all other attributes, i.e, it is treated as an automatic "top priority"
     * attribute.
     */
    TalkerCode* talkerToTalkerCode(const QString& talker);

    /**
     * Given a Talker Code, returns the Talker ID of the talker that would speak
     * a text job with that Talker Code.
     * @param talkerCode     Talker Code.
     * @return               Talker ID of the talker that would speak the text job.
     */
    QString talkerCodeToTalkerId(const QString& talkerCode);

    /**
     * Get the user's default talker.
     * @return               A fully-specified talker code.
     *
     * @see talkers
     * @see getTalkers
     */
    QString userDefaultTalker() const;

    /**
     * Determine whether the currently-configured speech plugin supports a speech markup language.
     * @param talker         Code for the talker to do the speaking.  Example "en".
     *                       If NULL, defaults to the user's default talker.
     * @param markupType     The kttsd code for the desired speech markup language.
     * @return               True if the plugin currently configured for the indicated
     *                       talker supports the indicated speech markup language.
     * @see kttsdMarkupType
     */
    bool supportsMarkup(const QString& talker, const uint markupType) const;

    /**
     * Try to automatically configure a Talker in the specified language.
     * @param langCode      Two-letter language code.
     * @param config        KConfig to be updated if successful.
     * @return              True if successful.
     *
     * If successful, the KConfig rc file is updated but the talker has not been loaded.
     */
    bool autoconfigureTalker(const QString& langCode, KConfig* config);

private:

    /**
     * Array of the loaded plug ins for different Talkers.
     * Array of parsed Talker Codes for the plugins.
     */
    PlugInList m_loadedPlugIns;
    QStringList m_loadedTalkerIds;
    TalkerCode::TalkerCodeList m_loadedTalkerCodes;

    /**
     * Cache of talker codes and index of closest matching Talker.
     */
    mutable QMap<QString,int> m_talkerToPlugInCache;
};

#endif      // _TALKERMGR_H_
