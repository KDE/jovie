/***************************************************** vim:set ts=4 sw=4 sts=4:
  Sentence Boundary Detection Filter class.
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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 ******************************************************************************/

// Qt includes.
#include <QRegExp>
#include <qdom.h>
#include <qapplication.h>
//Added by qt3to4:
#include <QCustomEvent>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kconfig.h>

// KTTS includes.
#include "utils.h"
#include "talkercode.h"

// SdbProc includes.
#include "sbdproc.h"

/**
 * Constructor.
 */
SbdThread::SbdThread( QObject *parent) :
    QThread( parent)
{
}

/**
 * Destructor.
 */
/*virtual*/ SbdThread::~SbdThread()
{
}

/**
 * Get/Set text being processed.
 */
void SbdThread::setText( const QString& text ) { m_text = text; }
QString SbdThread::text() { return m_text; }

/**
 * Set/Get TalkerCode.
 */
void SbdThread::setTalkerCode( TalkerCode* talkerCode ) { m_talkerCode = talkerCode; }
TalkerCode* SbdThread::talkerCode() { return m_talkerCode; }

/**
 * Set Sentence Boundary Regular Expression.
 * This method will only be called if the application overrode the default.
 *
 * @param re            The sentence delimiter regular expression.
 */
void SbdThread::setSbRegExp( const QString& re ) { m_re = re; }

/**
 * The configured Sentence Boundary Regular Expression.
 *
 * @param re            The sentence delimiter regular expression.
 */
void SbdThread::setConfiguredSbRegExp( const QString& re ) { m_configuredRe = re; }

/**
 * The configured Sentence Boundary that replaces SB regular expression.
 *
 * @param sb            The sentence boundary replacement.
 *
 */
void SbdThread::setConfiguredSentenceBoundary( const QString& sb ) { m_configuredSentenceBoundary = sb; }

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
void SbdThread::setWasModified(bool wasModified) { m_wasModified = wasModified; }
bool SbdThread::wasModified() { return m_wasModified; }

// Given a tag name, returns SsmlElemType.
SbdThread::SsmlElemType SbdThread::tagToSsmlElemType( const QString tagName )
{
    if ( tagName == "speak" )       return etSpeak;
    if ( tagName == "voice" )       return etVoice;
    if ( tagName == "prosody" )     return etProsody;
    if ( tagName == "emphasis" )    return etEmphasis;
    if ( tagName == "break" )       return etBreak;
    if ( tagName == "s" )           return etPS;
    if ( tagName == "p" )           return etPS;
    return  etNotSsml;
}

// Parses an SSML element, pushing current settings onto the context stack.
void SbdThread::pushSsmlElem( SsmlElemType et, const QDomElement& elem )
{
    // TODO: Need to convert relative values into absolute values and also convert
    // only to values recognized by SSML2SABLE stylesheet.  Either that or enhance all
    // the synth stylesheets.
    QDomNamedNodeMap attrList = elem.attributes();
    int attrCount = attrList.count();
    switch ( et )
    {
        case etSpeak: {
            SpeakElem e = m_speakStack.top();
            for ( int ndx=0; ndx < attrCount; ++ndx )
            {
                QDomAttr a = attrList.item( ndx ).toAttr();
                if ( a.name() == "lang" )      e.lang = a.value();
            }
            m_speakStack.push( e );
            break; }
        case etVoice: {
            VoiceElem e = m_voiceStack.top();
            // TODO: Since Festival chokes on <voice> tags, don't output them at all.
            // This means we can't support voice changes, and probably more irritatingly,
            // gender changes either.
            m_voiceStack.push( e );
            break; }
        case etProsody: {
            ProsodyElem e = m_prosodyStack.top();
            for ( int ndx=0; ndx < attrCount; ++ndx )
            {
                QDomAttr a = attrList.item( ndx ).toAttr();
                if ( a.name() == "pitch" )    e.pitch = a.value();
                if ( a.name() == "contour" )  e.contour = a.value();
                if ( a.name() == "range" )    e.range = a.value();
                if ( a.name() == "rate" )     e.rate = a.value();
                if ( a.name() == "duration" ) e.duration = a.value();
                if ( a.name() == "volume" )   e.volume = a.value();
            }
            m_prosodyStack.push( e );
            break; }
        case etEmphasis: {
            EmphasisElem e = m_emphasisStack.top();
            for ( int ndx=0; ndx < attrCount; ++ndx )
            {
                QDomAttr a = attrList.item( ndx ).toAttr();
                if ( a.name() == "level" )    e.level = a.value();
            }
            m_emphasisStack.push( e );
            break; }
        case etPS: {
            PSElem e = m_psStack.top();
            for ( int ndx=0; ndx < attrCount; ++ndx )
            {
                QDomAttr a = attrList.item( ndx ).toAttr();
                if ( a.name() == "lang" )     e.lang = a.value();
            }
            m_psStack.push( e );
            break; }
        default: break;
    }
}

// Given an attribute name and value, constructs an XML representation of the attribute,
// i.e., name="value".
QString SbdThread::makeAttr( const QString& name, const QString& value )
{
    if ( value.isEmpty() ) return QString();
    return " " + name + "=\"" + value + "\"";
}

// Returns an XML representation of an SSML tag from the top of the context stack.
QString SbdThread::makeSsmlElem( SsmlElemType et )
{
    QString s;
    QString a;
    switch ( et )
    {
        // Must always output speak tag, otherwise kttsd won't think each sentence is SSML.
        // For all other tags, only output the tag if it contains at least one attribute.
        case etSpeak: {
            SpeakElem e = m_speakStack.top();
            s = "<speak";
            if ( !e.lang.isEmpty() ) s += makeAttr( "lang", e.lang );
            s += ">";
            break; }
        case etVoice: {
            VoiceElem e = m_voiceStack.top();
            a += makeAttr( "lang",     e.lang );
            a += makeAttr( "gender",   e.gender );
            a += makeAttr( "age",      QString::number(e.age) );
            a += makeAttr( "name",     e.name );
            a += makeAttr( "variant",  e.variant );
            if ( !a.isEmpty() ) s = "<voice" + a + ">";
            break; }
        case etProsody: {
            ProsodyElem e = m_prosodyStack.top();
            a += makeAttr( "pitch",    e.pitch );
            a += makeAttr( "contour",  e.contour );
            a += makeAttr( "range",    e.range );
            a += makeAttr( "rate",     e.rate );
            a += makeAttr( "duration", e.duration );
            a += makeAttr( "volume",   e.volume );
            if ( !a.isEmpty() ) s = "<prosody" + a + ">";
            break; }
        case etEmphasis: {
            EmphasisElem e = m_emphasisStack.top();
            a += makeAttr( "level",    e.level );
            if ( !a.isEmpty() ) s = "<emphasis" + a + ">";
            break; }
        case etPS: {
            break; }
        default: break;
    }
    return s;
}

// Pops element from the indicated context stack.
void SbdThread::popSsmlElem( SsmlElemType et )
{
    switch ( et )
    {
        case etSpeak: m_speakStack.pop(); break;
        case etVoice: m_voiceStack.pop(); break;
        case etProsody: m_prosodyStack.pop(); break;
        case etEmphasis: m_emphasisStack.pop(); break;
        case etPS: m_psStack.pop(); break;
        default: break;
    }
}

// Returns an XML representation of a break element.
QString SbdThread::makeBreakElem( const QDomElement& e )
{
    QString s = "<break";
    QDomNamedNodeMap attrList = e.attributes();
    int attrCount = attrList.count();
    for ( int ndx=0; ndx < attrCount; ++ndx )
    {
        QDomAttr a = attrList.item( ndx ).toAttr();
        s += makeAttr( a.name(), a.value() );
    }
    s += ">";
    return s;
}

// Converts a text fragment into a CDATA section.
QString SbdThread::makeCDATA( const QString& text )
{
    QString s =  "<![CDATA[";
    s += text;
    s += "]]>";
    return s;
}

// Returns an XML representation of an utterance node consisting of voice,
// prosody, and emphasis elements.
QString SbdThread::makeSentence( const QString& text )
{
    QString s;
    QString v = makeSsmlElem( etVoice );
    QString p = makeSsmlElem( etProsody );
    QString e = makeSsmlElem( etEmphasis );
    // TODO: Lang settings from psStack.
    if ( !v.isEmpty() ) s += v;
    if ( !p.isEmpty() ) s += p;
    if ( !e.isEmpty() ) s += e;
    // Escape ampersands and less thans.
    QString newText = text;
    newText.replace(QRegExp("&(?!amp;)"), "&amp;");
    newText.replace(QRegExp("<(?!lt;)"), "&lt;");
    s += newText;
    if ( !e.isEmpty() ) s += "</emphasis>";
    if ( !p.isEmpty() ) s += "</prosody>";
    if ( !v.isEmpty() ) s += "</voice>";
    return s;
}

// Starts a sentence by returning a speak tag.
QString SbdThread::startSentence()
{
    if ( m_sentenceStarted ) return QString();
    QString s;
    s += makeSsmlElem( etSpeak );
    m_sentenceStarted = true;
    return s;
}

// Ends a sentence and appends a Tab.
QString SbdThread::endSentence()
{
    if ( !m_sentenceStarted ) return QString();
    QString s = "</speak>";
    s += "\t";
    m_sentenceStarted = false;
    return s;
}

// Parses a node of the SSML tree and recursively parses its children.
// Returns the filtered text with each sentence a complete ssml tree.
QString SbdThread::parseSsmlNode( QDomNode& n, const QString& re )
{
    QString result;
    switch ( n.nodeType() )
    {
        case QDomNode::ElementNode: {               // = 1
            QDomElement e = n.toElement();
            QString tagName = e.tagName();
            SsmlElemType et = tagToSsmlElemType( tagName );
            switch ( et )
            {
                case etSpeak:
                case etVoice:
                case etProsody:
                case etEmphasis:
                case etPS:
                {
                    pushSsmlElem( et, e );
                    QDomNode t = n.firstChild();
                    while ( !t.isNull() )
                    {
                        result += parseSsmlNode( t, re );
                        t = t.nextSibling();
                    }
                    popSsmlElem( et );
                    if ( et == etPS )
                        result += endSentence();
                    break;
                }
                case etBreak:
                {
                    // Break elements are empty.
                    result += makeBreakElem( e );
                }
                // Ignore any elements we don't recognize.
                default: break;
            }
            break; }
        case QDomNode::AttributeNode: {             // = 2
            break; }
        case QDomNode::TextNode: {                  // = 3
            QString s = parsePlainText( n.toText().data(), re );
            // QString d = s;
            // d.replace("\t", "\\t");
            // kDebug() << "SbdThread::parseSsmlNode: parsedPlainText = [" << d << "]" << endl;
            QStringList sentenceList = s.split( '\t', QString::SkipEmptyParts);
            int lastNdx = sentenceList.count() - 1;
            for ( int ndx=0; ndx < lastNdx; ++ndx )
            {
                result += startSentence();
                result += makeSentence( sentenceList[ndx] );
                result += endSentence();
            }
            // Only output sentence boundary if last text fragment ended a sentence.
            if ( lastNdx >= 0 )
            {
                result += startSentence();
                result += makeSentence( sentenceList[lastNdx] );
                if ( s.endsWith( "\t" ) ) result += endSentence();
            }
            break; }
        case QDomNode::CDATASectionNode: {          // = 4
            QString s = parsePlainText( n.toCDATASection().data(), re );
            QStringList sentenceList = s.split( '\t', QString::SkipEmptyParts);
            int lastNdx = sentenceList.count() - 1;
            for ( int ndx=0; ndx < lastNdx; ++ndx )
            {
                result += startSentence();
                result += makeSentence( makeCDATA( sentenceList[ndx] ) );
                result += endSentence();
            }
            // Only output sentence boundary if last text fragment ended a sentence.
            if ( lastNdx >= 0 )
            {
                result += startSentence();
                result += makeSentence( makeCDATA( sentenceList[lastNdx] ) );
                if ( s.endsWith( "\t" ) ) result += endSentence();
            }
            break; }
        case QDomNode::EntityReferenceNode: {       // = 5
            break; }
        case QDomNode::EntityNode: {                // = 6
            break; }
        case QDomNode::ProcessingInstructionNode: { // = 7
            break; }
        case QDomNode::CommentNode: {               // = 8
            break; }
        case QDomNode::DocumentNode: {              // = 9
            break; }
        case QDomNode::DocumentTypeNode: {          // = 10
            break; }
        case QDomNode::DocumentFragmentNode: {      // = 11
            break; }
        case QDomNode::NotationNode: {              // = 12
            break; }
        case QDomNode::BaseNode: {                  // = 21
            break; }
        case QDomNode::CharacterDataNode: {         // = 22
            break; }
    }
    return result;
}

// Parses Ssml.
QString SbdThread::parseSsml( const QString& inputText, const QString& re )
{
    QRegExp sentenceDelimiter = QRegExp( re );

    // Read the text into xml dom tree.
    QDomDocument doc( "" );
    // If an error occurs parsing the SSML, return "invalid S S M L".
    if ( !doc.setContent( inputText ) ) return i18n("Invalid S S M L.");

    // Set up context stacks and set defaults for all element attributes.
    m_speakStack.clear();
    m_voiceStack.clear();
    m_prosodyStack.clear();
    m_emphasisStack.clear();
    m_psStack.clear();
    SpeakElem se = { "" };
    m_speakStack.push ( se );
    VoiceElem ve = {"", "neutral", 40, "", ""};
    m_voiceStack.push( ve );
    ProsodyElem pe = { "medium", "", "medium", "medium", "", "medium" };
    m_prosodyStack.push( pe );
    EmphasisElem em = { "" };
    m_emphasisStack.push( em );
    PSElem pse = { "" };
    m_psStack.push ( pse );

    // This flag is used to close out a previous sentence.
    m_sentenceStarted = false;

    // Get the root element (speak) and recursively process its children.
    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    QString ssml = parseSsmlNode( docElem, re );

    // Close out last sentence.
    if ( m_sentenceStarted ) ssml += "</speak>";

    return ssml;
}

// Parses code.  Each newline is converted into a tab character (\t).
QString SbdThread::parseCode( const QString& inputText )
{
    QString temp = inputText;
    // Replace newlines with tabs.
    temp.replace("\n","\t");
    // Remove leading spaces.
    temp.replace(QRegExp("\\t +"), "\t");
    // Remove trailing spaces.
    temp.replace(QRegExp(" +\\t"), "\t");
    // Remove blank lines.
    temp.replace(QRegExp("\t\t+"),"\t");
    return temp;
}

// Parses plain text.
QString SbdThread::parsePlainText( const QString& inputText, const QString& re )
{
    // kDebug() << "SbdThread::parsePlainText: parsing " << inputText << " with re " << re << endl;
    QRegExp sentenceDelimiter = QRegExp( re );
    QString temp = inputText;
    // Replace sentence delimiters with tab.
    temp.replace(sentenceDelimiter, m_configuredSentenceBoundary);
    // Replace remaining newlines with spaces.
    temp.replace("\n"," ");
    temp.replace("\r"," ");
    // Remove leading spaces.
    temp.replace(QRegExp("\\t +"), "\t");
    // Remove trailing spaces.
    temp.replace(QRegExp(" +\\t"), "\t");
    // Remove blank lines.
    temp.replace(QRegExp("\t\t+"),"\t");
    return temp;
}

// This is where the real work takes place.
/*virtual*/ void SbdThread::run()
{
    // kDebug() << "SbdThread::run: processing text = " << m_text << endl;

    // TODO: Determine if we should do anything or not.
    m_wasModified = true;

    // Determine what kind of input text we are dealing with.
    int textType;
    if ( KttsUtils::hasRootElement( m_text, "speak" ) )
        textType = ttSsml;
    else
    {
        // Examine just the first 500 chars to see if it is code.
        QString p = m_text.left( 500 );
        if ( p.contains( QRegExp( "(/\\*)|(if\\b\\()|(^#include\\b)" ) ) )
            textType = ttCode;
        else
            textType = ttPlain;
    }

    // If application specified a sentence delimiter regular expression, use that,
    // otherwise use configured default.
    QString re = m_re;
    if ( re.isEmpty() ) re = m_configuredRe;

    // Replace spaces, tabs, and formfeeds with a single space.
    m_text.replace(QRegExp("[ \\t\\f]+"), " ");

    // Perform the filtering based on type of text.
    switch ( textType )
    {
        case ttSsml:
            m_text = parseSsml( m_text, re );
            break;

        case ttCode:
            m_text = parseCode( m_text );
            break;

        case ttPlain:
            m_text = parsePlainText( m_text, re);
            break;
    }

    // Clear app-specified sentence delimiter.  App must call setSbRegExp for each conversion.
    m_re.clear();

    // kDebug() << "SbdThread::run: filtered text = " << m_text << endl;

    // Result is in m_text;

    // Post an event.  We need to emit filterFinished signal, but not from the
    // separate thread.
    QCustomEvent* ev = new QCustomEvent(QEvent::User + 301);
    QApplication::postEvent(this, ev);
}

bool SbdThread::event ( QEvent * e )
{
    if ( e->type() == (QEvent::User + 301) )
    {
        // kDebug() << "SbdThread::event: emitting filteringFinished signal." << endl;
        emit filteringFinished();
        return true;
    }
    else return false;
}

// ----------------------------------------------------------------------------

/**
 * Constructor.
 */
SbdProc::SbdProc( QObject *parent, const char *name, const QStringList& /*args*/) :
    KttsFilterProc(parent, name) 
{
    // kDebug() << "SbdProc::SbdProc: Running" << endl;
    m_sbdThread = new SbdThread(this);
    connect( m_sbdThread, SIGNAL(filteringFinished()), this, SLOT(slotSbdThreadFilteringFinished()) );
}

/**
 * Destructor.
 */
SbdProc::~SbdProc()
{
    // kDebug() << "SbdProc::~SbdProc: Running" << endl;
    if ( m_sbdThread )
    {
        if ( m_sbdThread->running() )
        {
            m_sbdThread->terminate();
            m_sbdThread->wait();
        }
        delete m_sbdThread;
    }
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
bool SbdProc::init(KConfig* config, const QString& configGroup){
    // kDebug() << "PlugInProc::init: Running" << endl;
    config->setGroup( configGroup );
//    m_configuredRe = config->readEntry( "SentenceDelimiterRegExp", "([\\.\\?\\!\\:\\;])\\s|(\\n *\\n)" );
    m_configuredRe = config->readEntry( "SentenceDelimiterRegExp", "([\\.\\?\\!\\:\\;])(\\s|$|(\\n *\\n))" );
    m_sbdThread->setConfiguredSbRegExp( m_configuredRe );
    QString sb = config->readEntry( "SentenceBoundary", "\\1\t" );
    sb.replace( "\\t", "\t" );
    m_sbdThread->setConfiguredSentenceBoundary( sb );
    m_appIdList = config->readEntry( "AppID", QStringList(), ',' );
    m_languageCodeList = config->readEntry( "LanguageCodes", QStringList(), ',' );
    return true;
}

/**
 * Returns True if this filter is a Sentence Boundary Detector.
 * If so, the filter should implement @ref setSbRegExp() .
 * @return          True if this filter is a SBD.
 */
/*virtual*/ bool SbdProc::isSBD() { return true; }

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
/*virtual*/ bool SbdProc::supportsAsync() { return true; }

/**
 * Convert input, returning output.  Runs synchronously.
 * @param inputText         Input text.
 * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
 *                          use for synthing the text.  Useful for extracting hints about
 *                          how to filter the text.  For example, languageCode.
 * @param appId             The DCOP appId of the application that queued the text.
 *                          Also useful for hints about how to do the filtering.
 */
/*virtual*/ QString SbdProc::convert(const QString& inputText, TalkerCode* talkerCode,
    const QByteArray& appId)
{
    if ( asyncConvert( inputText, talkerCode, appId) )
    {
        waitForFinished();
        // kDebug() << "SbdProc::convert: returning " << getOutput() << endl;
        return getOutput();
    } else return inputText;
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
/*virtual*/ bool SbdProc::asyncConvert(const QString& inputText, TalkerCode* talkerCode,
    const QByteArray& appId)
{
    m_sbdThread->setWasModified( false );
    // If language doesn't match, return input unmolested.
    if ( !m_languageCodeList.isEmpty() )
    {
        QString languageCode = talkerCode->languageCode();
        // kDebug() << "StringReplacerProc::convert: converting " << inputText << 
        // " if language code " << languageCode << " matches " << m_languageCodeList << endl;
        if ( !m_languageCodeList.contains( languageCode ) )
        {
            if ( !talkerCode->countryCode().isEmpty() )
            {
                languageCode += '_' + talkerCode->countryCode();
                // kDebug() << "StringReplacerProc::convert: converting " << inputText << 
                // " if language code " << languageCode << " matches " << m_languageCodeList << endl;
                if ( !m_languageCodeList.contains( languageCode ) ) return false;
            } else return false;
        }
    }
    // If appId doesn't match, return input unmolested.
    if ( !m_appIdList.isEmpty() )
    {
        // kDebug() << "SbdProc::convert: converting " << inputText << " if appId "
        //     << appId << " matches " << m_appIdList << endl;
        bool found = false;
        QString appIdStr = appId;
        for ( int ndx=0; ndx < m_appIdList.count(); ++ndx )
        {
            if ( appIdStr.contains(m_appIdList[ndx]) )
            {
                found = true;
                break;
            }
        }
        if ( !found ) return false;
    }
    m_sbdThread->setText( inputText );
    m_sbdThread->setTalkerCode( talkerCode );
    m_state = fsFiltering;
    m_sbdThread->start();
    return true;
}

/**
 * Waits for a previous call to asyncConvert to finish.
 */
/*virtual*/ void SbdProc::waitForFinished()
{
    if ( m_sbdThread->running() )
    {
        // kDebug() << "SbdProc::waitForFinished: waiting" << endl;
        m_sbdThread->wait();
        // kDebug() << "SbdProc::waitForFinished: finished waiting" << endl;
        m_state = fsFinished;
    }
}

/**
 * Returns the state of the Filter.
 */
/*virtual*/ int SbdProc::getState() { return m_state; }

/**
 * Returns the filtered output.
 */
/*virtual*/ QString SbdProc::getOutput() { return m_sbdThread->text(); }

/**
 * Acknowledges the finished filtering.
 */
/*virtual*/ void SbdProc::ackFinished()
{
     m_state = fsIdle;
     m_sbdThread->setText( QString() );
}

/**
 * Stops filtering.  The filteringStopped signal will emit when filtering
 * has in fact stopped and state returns to fsIdle;
 */
/*virtual*/ void SbdProc::stopFiltering()
{
    if ( m_sbdThread->running() )
    {
        m_sbdThread->terminate();
        m_sbdThread->wait();
        delete m_sbdThread;
        m_sbdThread = new SbdThread(this);
        m_sbdThread->setConfiguredSbRegExp( m_configuredRe );
        connect( m_sbdThread, SIGNAL(filteringFinished()), this, SLOT(slotSbdThreadFilteringFinished()) );
        m_state = fsIdle;
        emit filteringStopped();
    }
}

/**
 * Did this filter do anything?  If the filter returns the input as output
 * unmolested, it should return False when this method is called.
 */
/*virtual*/ bool SbdProc::wasModified() { return m_sbdThread->wasModified(); }

/**
 * Set Sentence Boundary Regular Expression.
 * This method will only be called if the application overrode the default.
 *
 * @param re            The sentence delimiter regular expression.
 */
/*virtual*/ void SbdProc::setSbRegExp(const QString& re) { m_sbdThread->setSbRegExp( re ); }

// Received when SBD Thread finishes.
void SbdProc::slotSbdThreadFilteringFinished()
{
    m_state = fsFinished;
    // kDebug() << "SbdProc::slotSbdThreadFilteringFinished: emitting filterFinished signal." << endl;
    emit filteringFinished();
}

#include "sbdproc.moc"
