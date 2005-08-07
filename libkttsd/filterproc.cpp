/***************************************************** vim:set ts=4 sw=4 sts=4:
  Filter Processing class.
  This is the interface definition for text filters.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
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
  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// KDE includes.
// #include <kdebug.h>

// FilterProc includes.
#include "filterproc.h"

/**
 * Constructor.
 */
KttsFilterProc::KttsFilterProc( QObject *parent, const char *name) :
        QObject(parent, name) 
{
    // kdDebug() << "KttsFilterProc::KttsFilterProc: Running" << endl;
}

/**
 * Destructor.
 */
KttsFilterProc::~KttsFilterProc()
{
    // kdDebug() << "KttsFilterProc::~KttsFilterProc: Running" << endl;
}

/**
 * Initialize the filter.
 * @param config          Settings object.
 * @param configGroup     Settings Group.
 * @return                False if filter is not ready to filter.
 *
 * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
 * separate configuration files of their own.
 */
bool KttsFilterProc::init(KConfig* /*config*/, const QString& /*configGroup*/){
    // kdDebug() << "PlugInProc::init: Running" << endl;
    return false;
}

/**
 * Returns True if this filter is a Sentence Boundary Detector.
 * If so, the filter should implement @ref setSbRegExp() .
 * @return          True if this filter is a SBD.
 */
/*virtual*/ bool KttsFilterProc::isSBD() { return false; }

/**
 * Returns True if the plugin supports asynchronous processing,
 * i.e., supports asyncConvert method.
 * @return                        True if this plugin supports asynchronous processing.
 *
 * If the plugin returns True, it must also implement @ref getState .
 * It must also emit @ref filteringFinished when filtering is completed.
 * If the plugin returns True, it must also implement @ref stopFiltering .
 * It must also emit @ref filteringStopped when filtering has been stopped.
 */
/*virtual*/ bool KttsFilterProc::supportsAsync() { return false; }

/**
 * Convert input, returning output.  Runs synchronously.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ QString KttsFilterProc::convert(const QString& inputText, TalkerCode* /*talkerCode*/,
    const QByteArray& /*appId*/)
{
    return inputText;
}

/**
 * Convert input.  Runs asynchronously.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 * @return                  False if the filter cannot perform the conversion.
 *
 * When conversion is completed, emits signal @ref filteringFinished.  Calling
 * program may then call @ref getOutput to retrieve converted text.  Calling
 * program must call @ref ackFinished to acknowledge the conversion.
 */
/*virtual*/ bool KttsFilterProc::asyncConvert(const QString& /*inputText*/,
    TalkerCode* /*talkerCode*/, const QByteArray& /*appId*/) { return false; }

/**
 * Waits for a previous call to asyncConvert to finish.
 */
/*virtual*/ void KttsFilterProc::waitForFinished() { }

/**
 * Returns the state of the Filter.
 */
/*virtual*/ int KttsFilterProc::getState() { return fsIdle; }

/**
 * Returns the filtered output.
 */
/*virtual*/ QString KttsFilterProc::getOutput() { return QString::null; }

/**
 * Acknowledges the finished filtering.
 */
/*virtual*/ void KttsFilterProc::ackFinished() { }

/**
 * Stops filtering.  The filteringStopped signal will emit when filtering
 * has in fact stopped and state returns to fsIdle;
 */
/*virtual*/ void KttsFilterProc::stopFiltering() { }

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
/*virtual*/ bool KttsFilterProc::wasModified() { return true; }

/**
 * Set Sentence Boundary Regular Expression.
 * This method will only be called if the application overrode the default.
 *
 * @param re            The sentence delimiter regular expression.
 */
/*virtual*/ void KttsFilterProc::setSbRegExp(const QString& /*re*/) { };

#include "filterproc.moc"
