/***************************************************** vim:set ts=4 sw=4 sts=4:
  This file is the template for the processing plug ins.
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// Qt includes. 
#include <qstring.h>
#include <qtextcodec.h>

// KDE includes.
#include <kdebug.h>
#include <kstandarddirs.h>
#include <klocale.h>

// PlugInProc includes.
#include "pluginproc.h"
#include "pluginproc.moc"

/**
* Constructor
*/
PlugInProc::PlugInProc( QObject *parent, const char *name) : QObject(parent, name){
    // kdDebug() << "PlugInProc::PlugInProc: Running" << endl;
}

/**
* Destructor
*/
PlugInProc::~PlugInProc(){
    // kdDebug() << "PlugInProc::~PlugInProc: Running" << endl;
}

/**
* Initializate the speech plugin.
*/
bool PlugInProc::init(KConfig* /*config*/, const QString& /*configGroup*/){
    // kdDebug() << "PlugInProc::init: Running" << endl;
    return false;
}

/** 
* Say a text.  Synthesize and audibilize it.
* @param text                    The text to be spoken.
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void PlugInProc::sayText(const QString& /*text*/){
    // kdDebug() << "PlugInProc::sayText: Running" << endl;
}

/**
* Synthesize text into an audio file, but do not send to the audio device.
* @param text                    The text to be synthesized.
* @param suggestedFilename       Full pathname of file to create.  The plugin
*                                may ignore this parameter and choose its own
*                                filename.  KTTSD will query the generated
*                                filename using getFilename().
*
* If the plugin supports asynchronous operation, it should return immediately.
*/
void PlugInProc::synthText(const QString& /*text*/, const QString& /*suggestedFilename*/) { };

/**
* Get the generated audio filename from synthText.
* @return                        Name of the audio file the plugin generated.
*                                Null if no such file.
*
* The plugin must not re-use the filename.
*/
QString PlugInProc::getFilename() { return QString::null; };

/**
* Stop current operation (saying or synthesizing text).
* This function only makes sense in asynchronus modes.
* The plugin should return to the psIdle state.
*/
void PlugInProc::stopText(){
    // kdDebug() << "PlugInProc::stopText: Running" << endl;
}

/**
* Return the current state of the plugin.
* This function only makes sense in asynchronous mode.
* @return                        The pluginState of the plugin.
*
* @ref pluginState
*/
pluginState PlugInProc::getState() { return psIdle; }

/**
* Acknowledges a finished state and resets the plugin state to psIdle.
*
* If the plugin is not in state psFinished, nothing happens.
* The plugin may use this call to do any post-processing cleanup,
* for example, blanking the stored filename (but do not delete the file).
* Calling program should call getFilename prior to ackFinished.
*/
void PlugInProc::ackFinished() { }

/**
* Returns True if the plugin supports asynchronous processing,
* i.e., returns immediately from sayText or synthText.
* @return                        True if this plugin supports asynchronous processing.
*/
bool PlugInProc::supportsAsync() { return false; }

/**
* Returns True if the plugin supports synthText method,
* i.e., is able to synthesize text to a sound file without
* audibilizing the text.
* @return                        True if this plugin supports synthText method.
*/
bool PlugInProc::supportsSynth() { return false; }

/**
* Returns the name of an XSLT stylesheet that will convert a valid SSML file
* into a format that can be processed by the synth.  For example,
* The Festival plugin returns a stylesheet that will convert SSML into
* SABLE.  Any tags the synth cannot handle should be stripped (leaving
* their text contents though).  The default stylesheet strips all
* tags and converts the file to plain text.
* @return            Name of the XSLT file.
*/
QString PlugInProc::getSsmlXsltFilename()
{
    return KGlobal::dirs()->resourceDirs("data").last() + "kttsd/xslt/SSMLtoPlainText.xsl";
}

/**
* Given the name of a codec, returns the QTextCodec for the name.
* Handles the following "special" codec names:
*   Local               The user's current Locale codec.
*   Latin1              Latin1 (ISO 8859-1)
*   Unicode             UTF-16
* @param codecName      Name of desired codec.
* @return               The codec object.  Calling program must not delete this object
*                       as it is a reference to an existing QTextCodec object.
*
* Caution: Do not pass translated codec names to this routine.
*/
/*static*/ QTextCodec* PlugInProc::codecNameToCodec(const QString &codecName)
{
    QTextCodec* codec = 0;
    if (codecName == "Local")
        codec = QTextCodec::codecForLocale();
    else if (codecName == "Latin1")
        codec = QTextCodec::codecForName("ISO8859-1");
    else if (codecName == "Unicode")
        codec = QTextCodec::codecForName("utf16");
    else
        codec = QTextCodec::codecForName(codecName.latin1());
    if (!codec)
    {
        kdDebug() << "PluginProc::codecNameToCodec: Invalid codec name " << codecName << endl;
        kdDebug() << "PluginProc::codecNameToCodec: Defaulting to ISO 8859-1" << endl;
        codec = QTextCodec::codecForName("ISO8859-1");
    }
    return codec;
}

/**
* Builds a list of codec names, suitable for display in a QComboBox.
* The list includes the 3 special codec names (translated) at the top:
*   Local               The user's current Locale codec.
*   Latin1              Latin1 (ISO 8859-1)
*   Unicode             UTF-16
*/
/*static*/ QStringList PlugInProc::buildCodecList()
{
    // kdDebug() << "PlugInConf::buildCodecList: Running" << endl;
    QStringList codecList;
    QString local = i18n("Local")+" (";
    local += QTextCodec::codecForLocale()->name();
    local += ")";
    codecList.append(local);
    codecList.append(i18n("Latin1"));
    codecList.append(i18n("Unicode"));
    for (int i = 0; (QTextCodec::codecForIndex(i)); i++ )
        codecList.append(QTextCodec::codecForIndex(i)->name());
    return codecList;
}

/**
* Given the name of a codec, returns index into the codec list.
* Handles the following "special" codec names:
*   Local               The user's current Locale codec.
*   Latin1              Latin1 (ISO 8859-1)
*   Unicode             UTF-16
* @param codecName      Name of the codec.
* @param codecList      List of codec names. The first 3 entries may be translated names.
* @return               QTextCodec object.  Caller must not delete this object.
*
* Caution: Do not pass translated codec names to this routine in codecName parameter.
*/
/*static*/ int PlugInProc::codecNameToListIndex(const QString &codecName, const QStringList &codecList)
{
    int codec;
    if (codecName == "Local")
        codec = PlugInProc::Local;
    else if (codecName == "Latin1")
        codec = PlugInProc::Latin1;
    else if (codecName == "Unicode")
        codec = PlugInProc::Unicode;
    else {
        codec = PlugInProc::Local;
        for (uint i = PlugInProc::UseCodec; i < codecList.count(); i++ )
            if (codecName == codecList[i])
                codec = i;
    }
    return codec;
}

/**
* Given index into codec list, returns the codec object.
* @param codecNum       Index of the codec.
* @param codecList      List of codec names. The first 3 entries may be translated names.
* @return               QTextCodec object.  Caller must not delete this object.
*/
/*static*/ QTextCodec* PlugInProc::codecIndexToCodec(const int codecNum, const QStringList &codecList)
{
    QTextCodec* codec = 0;
    switch (codecNum) {
        case PlugInProc::Local:
            codec = QTextCodec::codecForLocale();
            break;
        case PlugInProc::Latin1:
            codec = QTextCodec::codecForName("ISO8859-1");
            break;
        case PlugInProc::Unicode:
            codec = QTextCodec::codecForName("utf16");
            break;
        default:
            codec = QTextCodec::codecForName(codecList[codecNum].latin1());
            break;
    }
    if (!codec)
    {
        kdDebug() << "PlugInProc::codecIndexToCodec: Invalid codec index " << codecNum << endl;
        kdDebug() << "PlugInProc::codecIndexToCodec: Defaulting to ISO 8859-1" << endl;
        codec = QTextCodec::codecForName("ISO8859-1");
    }
    return codec;
}

/**
* Given index into codec list, returns the codec Name.
* Handles the following "special" codec names:
*   Local               The user's current Locale codec.
*   Latin1              Latin1 (ISO 8859-1)
*   Unicode             UTF-16
* @param codecNum       Index of the codec.
* @param codecList      List of codec names. The first 3 entries may be translated names.
* @return               Untranslated name of the codec.
*/
/*static*/ QString PlugInProc::codecIndexToCodecName(const int codecNum, const QStringList &codecList)
{
    QString codecName;
    switch (codecNum) {
        case PlugInProc::Local:
            codecName = "Local";
            break;
        case PlugInProc::Latin1:
            codecName = "Latin1";
            break;
        case PlugInProc::Unicode:
            codecName = "Unicode";
            break;
        default:
            if ((uint)codecNum < codecList.count())
                codecName = codecList[codecNum];
            else
            {
                kdDebug() << "PlugInProc::codecIndexToCodec: Invalid codec index " << codecNum << endl;
                kdDebug() << "PlugInProc::codecIndexToCodec: Defaulting to ISO 8859-1" << endl;
                codecName = "ISO8859-1";
            }
    }
    return codecName;
}
