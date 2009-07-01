/***************************************************** vim:set ts=4 sw=4 sts=4:
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

// SpeechData includes.
#include "speechdata.h"
#include "speechdata.moc"

// C++ includes.
#include <stdlib.h>

// Qt includes.
#include <QtCore/QRegExp>
#include <QtCore/QPair>
#include <QtXml/QDomDocument>
#include <QtCore/QFile>
#include <QtDBus/QtDBus>

// KDE includes.
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>

// KTTS includes.
#include "talkermgr.h"
#include "configdata.h"

/* -------------------------------------------------------------------------- */

class SpeechDataPrivate
{
public:
    SpeechDataPrivate() :
    {
    }

    ~SpeechDataPrivate()
    {
    }

    friend class SpeechData;

protected:
    /**
    * Configuration data.
    */
    ConfigData* configData;

};

/* -------------------------------------------------------------------------- */

SpeechData * SpeechData::m_instance = NULL;

SpeechData * SpeechData::Instance()
{
    if (m_instance == NULL)
    {
        m_instance = new SpeechData();
    }
    return m_instance;
}

/**
* Constructor
*/
SpeechData::SpeechData()
{
    d = new SpeechDataPrivate();

}

/**
* Destructor
*/
SpeechData::~SpeechData()
{
    delete d;
}

void SpeechData::setConfigData(ConfigData* configData)
{
    d->configData = configData;

}

