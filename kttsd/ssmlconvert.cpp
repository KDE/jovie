/***************************************************** vim:set ts=4 sw=4 sts=4:
  SSMLConvert class

  This class is in charge of converting SSML text into a format that can
  be handled by individual synths. 
  -------------------
  Copyright:
  (C) 2004 by Paul Giannaros <ceruleanblaze@gmail.com>
  (C) 2004 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Paul Giannaros <ceruleanblaze@gmail.com>
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
#include <qstringlist.h>
#include <qdom.h>
#include <qfile.h>
#include <qtextstream.h>

// KDE includes.
#include <kdeversion.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <ktempfile.h>
#include <kdebug.h>

// SSMLConvert includes.
#include "ssmlconvert.h"
#include "ssmlconvert.moc"

/// Constructor.
SSMLConvert::SSMLConvert() {
    m_talkers = QStringList();
    m_xsltProc = 0;
    m_state = tsIdle;
}

/// Constructor. Set the talkers to be used as reference for entered text.
SSMLConvert::SSMLConvert(const QStringList &talkers) {
    m_talkers = talkers;
    m_xsltProc = 0;
    m_state = tsIdle;
}

/// Destructor.
SSMLConvert::~SSMLConvert() {
    delete m_xsltProc;
    if (!m_inFilename.isEmpty()) QFile::remove(m_inFilename);
    if (!m_outFilename.isEmpty()) QFile::remove(m_outFilename);
}

/// Set the talkers to be used as reference for entered text.
void SSMLConvert::setTalkers(const QStringList &talkers) {
    m_talkers = talkers;
}

QString SSMLConvert::extractTalker(const QString &talkercode) {
    QString t = talkercode.section("synthesizer=", 1, 1);
    t = t.section('"', 1, 1);
    if(t.contains("flite"))
        return "flite";
    else
        return t.left(t.find(" ")).toLower();
}

/**
* Return the most appropriate talker for the text to synth talker code.
* @param text               the text that will be parsed.
* @returns                  the appropriate talker for the job as a talker code.
*
* The appropriate talker is the one that has the most features that are required in some
* SSML markup. In the future i'm hoping to make the importance of individual features 
* configurable, but better to walk before you can run.
* Currently, the searching method in place is like a filter: Those that meet the criteria we're
* searchin for stay while others are sifted out. This should leave us with the right talker to use.
* It's not a very good method, but should be appropriate in most cases and should do just fine for now.
*
* As it stands, here is the list of things that are looked for, in order of most importance:
*   - Language
*      Obviously the most important. If a language is specified, look for the talkers that support it.
*      Default to en (or some form of en - en_US, en_GB, etc). Only one language at a time is allowed
*      at the moment, and must be specified in the root speak element (<speak xml:lang="en-US">)
*   - Gender
*      If a gender is specified, look for talkers that comply. There is no default so if no gender is
*      specified, no talkers will be removed. The only gender that will be searched for is the one 
*      specified in the root speak element. This should change in the future.
*   - Prosody
*      Check if prosody modification is allowed by the talker. Currently this is hardcoded (it 
*      is stated which talkers do and do not in a variable somewhere).
* 
* Bear in mind that the XSL stylesheet that will be applied to the SSML is the same regardless
* of the how the talker is chosen, meaning that you don't lose some features of the talker if this
* search doesn't encompass them.
* 
* QDom is the item of choice for the matching. Just walk the tree..
*/
QString SSMLConvert::appropriateTalker(const QString &text) const {
    QDomDocument ssml;
    ssml.setContent(text, false);  // No namespace processing.
    /// Matches are stored here. Obviously to begin with every talker matches.
    QStringList matches = m_talkers;

    /// Check that this is (well formed) SSML and all our searching will not be in vain.
    QDomElement root = ssml.documentElement();
    if(root.tagName() != "speak") {
        // Not SSML.
        return QString();
    }

    /** 
    * For each rule that we are looking through, iterate over all currently
    * matching talkers and remove all the talkers that don't match.
    *
    * Storage for talker code components.
    */
    QString talklang, talkvoice, talkgender, talkvolume, talkrate, talkname;

    kdDebug() << "SSMLConvert::appropriateTalker: BEFORE LANGUAGE SEARCH: " << matches.join(" ") << endl;;
    /**
    * Language searching
    */
    if(root.hasAttribute("xml:lang")) {
        QString lang = root.attribute("xml:lang");
        kdDebug() << "SSMLConvert::appropriateTalker: xml:lang found (" << lang << ")" << endl;
        /// If it is set to en*, then match all english speakers. They all sound the same anyways.
        if(lang.contains("en-")) {
            kdDebug() << "SSMLConvert::appropriateTalker: English" << endl;
            lang = "en";
        }
        /// Find all hits and place them in matches. We don't search for the closing " because if
        /// the talker emits lang="en-UK" or something we'll be ignoring it, which we don't what.
        matches = matches.grep("lang=\"" + lang);
    }
    else {
        kdDebug() << "SSMLConvert::appropriateTalker: no xml:lang found. Defaulting to en.." << endl;
        matches = matches.grep("lang=\"en");
    }

    kdDebug() << "SSMLConvert::appropriateTalker: AFTER LANGUAGE SEARCH: " << matches.join(" ") << endl;;

    /**
    * Gender searching
    * If, for example, male is specified and only female is found, 
    * ignore the choice and just use female.
    */
    if(root.hasAttribute("gender")) {
        QString gender = root.attribute("gender");
        kdDebug() << "SSMLConvert::appropriateTalker: gender found (" << gender << ")" << endl;
        /// If the gender found is not 'male' or 'female' then ignore it.
        if(!(gender == "male" || gender == "female")) {
            /// Make sure that we don't strip away all the talkers because of no matches.
            if(matches.grep("gender=\"" + gender).count() >= 1)
                matches = matches.grep("gender=\"" + gender);
        }
    }
    else {
        kdDebug() << "SSMLConvert::appropriateTalker: no gender found." << endl;
    }

    /**
    * Prosody
    * Search for talkers that allow modification of the synth output - louder, higher,
    * slower, etc. There should be a direct way to query each synth to find out if this
    * is supported (some function in PlugInConf), but for now, hardcode all the way :(
    */
    /// Known to support (feel free to add to the list and if search):
    ///   Festival Int (not flite), Hadifix
    if(matches.grep("synthesizer=\"Festival Interactive").count() >= 1 ||
    matches.grep("synthesizer=\"Hadifix").count() >= 1) {

        kdDebug() << "SSMLConvert::appropriateTalker: Prosody allowed" << endl;
        QStringList tmpmatches = matches.grep("synthesizer=\"Festival Interactive");
        matches = matches.grep("synthesizer=\"Hadifix");
        matches = tmpmatches + matches;
    }
    else
        kdDebug() << "SSMLConvert::appropriateTalker: No prosody-supporting talkers found" << endl;

    /// Return the first match that complies. Maybe a discrete way to 
    /// choose between all the matches could be offered in the future. Some form of preference.
    return matches[0];
}

/**
* Applies the spreadsheet for a talker to the SSML and returns the talker-native output.
* @param text               The markup to apply the spreadsheet to.
* @param xsltFilename       The name of the stylesheet file that will be applied (i.e freetts, flite).
* @returns                  False if an error occurs.
*
* This converts a piece of SSML into a format the given talker can understand. It applies
* an XSLT spreadsheet to the SSML and returns the output.
*
* Emits transformFinished signal when completed.  Caller then calls getOutput to retrieve
* the transformed text.
*/

bool SSMLConvert::transform(const QString &text, const QString &xsltFilename) {
    m_xsltFilename = xsltFilename;
    /// Write @param text to a temporary file.
    KTempFile inFile(locateLocal("tmp", "kttsd-"), ".ssml");
    m_inFilename = inFile.file()->name();
    QTextStream* wstream = inFile.textStream();
    if (wstream == 0) {
        /// wtf...
        kdDebug() << "SSMLConvert::transform: Can't write to " << m_inFilename << endl;;
        return false;
    }
    // TODO: Is encoding an issue here?
    // TODO: It would be nice if we detected whether the XML is properly formed
    // with the required xml processing instruction and encoding attribute.  If
    // not wrap it in such.  But maybe this should be handled by SpeechData::setText()?
    *wstream << text;
    inFile.close();
#if KDE_VERSION >= KDE_MAKE_VERSION (3,3,0)
    inFile.sync();
#endif

    // Get a temporary output file name.
    KTempFile outFile(locateLocal("tmp", "kttsd-"), ".output");
    m_outFilename = outFile.file()->name();
    outFile.close();
    // outFile.unlink();    // only activate this if necessary.

    /// Spawn an xsltproc process to apply our stylesheet to our SSML file.
    m_xsltProc = new KProcess;
    *m_xsltProc << "xsltproc";
    *m_xsltProc << "-o" << m_outFilename  << "--novalid"
        << m_xsltFilename << m_inFilename;
    // Warning: This won't compile under KDE 3.2.  See FreeTTS::argsToStringList().
    // kdDebug() << "SSMLConvert::transform: executing command: " <<
    //     m_xsltProc->args() << endl;

    connect(m_xsltProc, SIGNAL(processExited(KProcess*)),
        this, SLOT(slotProcessExited(KProcess*)));
    if (!m_xsltProc->start(KProcess::NotifyOnExit, KProcess::NoCommunication))
    {
        kdDebug() << "SSMLConvert::transform: Error starting xsltproc" << endl;
        return false;
    }
    m_state = tsTransforming;
    return true;
}

void SSMLConvert::slotProcessExited(KProcess* /*proc*/)
{
    m_xsltProc->deleteLater();
    m_xsltProc = 0;
    m_state = tsFinished;
    emit transformFinished();
}

/**
* Returns current processing state.
*/
int SSMLConvert::getState() { return m_state; }

/**
* Returns the output from call to transform.
*/
QString SSMLConvert::getOutput()
{
    /// Read back the data that was written to /tmp/fileName.output.
    QFile readfile(m_outFilename);
    if(!readfile.open(QIODevice::ReadOnly)) {
        /// uhh yeah... Issues writing to the SSML file.
        kdDebug() << "SSMLConvert::slotProcessExited: Could not read file " << m_outFilename << endl;
        return QString();
    }
    QTextStream rstream(&readfile);
    QString convertedData = rstream.read();
    readfile.close();

    // kdDebug() << "SSMLConvert::slotProcessExited: Read SSML file at " + m_inFilename + " and created " + m_outFilename + " based on the stylesheet at " << m_xsltFilename << endl;

    // Clean up.
    QFile::remove(m_inFilename);
    m_inFilename.clear();
    QFile::remove(m_outFilename);
    m_outFilename.clear();

    // Ready for another transform.
    m_state = tsIdle;

    return convertedData;
}

