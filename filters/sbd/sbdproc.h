/***************************************************** vim:set ts=4 sw=4 sts=4:
  Sentence Boundary Detection (SBD) Filter class.
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

/******************************************************************************

  This class performs three kinds of SBD:
    1.  If the text is SSML, generates new SSML where the prosody and voice
        tags are fully specified for each sentence.  This allows user to
        advance or rewind by sentence without losing SSML context.
        Input is considered to be SSML if the top-level element is a
        <speak> tag.
    2.  If the text is code, each line is considered to be a sentence.
        Input is considered to be code if any of the following strings are
        detected:
            slash asterisk
            if left-paren
            pound include
    3.  If the text is plain text, performs SBD using specified Regular
        Expression.

  Text is output with tab characters (\t) separating sentences.

 ******************************************************************************/

#ifndef _SBDPROC_H_
#define _SBDPROC_H_

// Qt includes.
#include <QObject>
#include <QThread>
#include <QEvent>
#include <QStack>

// KTTS includes.
#include "filterproc.h"

class TalkerCode;
class KConfig;
class QDomElement;
class QDomNode;

class SbdThread: public QThread
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        SbdThread( QObject *parent = 0);

        /**
         * Destructor.
         */
        virtual ~SbdThread();

        /**
         * Get/Set text being processed.
         */
        void setText( const QString& text );
        QString text();

        /**
         * Set/Get TalkerCode.
         */
        void setTalkerCode( TalkerCode* talkerCode );
        TalkerCode* talkerCode();

        /**
         * Set Sentence Boundary Regular Expression.
         * This method will only be called if the application overrode the default.
         *
         * @param re            The sentence delimiter regular expression.
         */
        void setSbRegExp( const QString& re );

        /**
         * The configured Sentence Boundary Regular Expression.
         *
         * @param re            The sentence delimiter regular expression.
         */
        void setConfiguredSbRegExp( const QString& re );

        /**
         * The configured Sentence Boundary that replaces SB regular expression.
         *
         * @param sb            The sentence boundary replacement.
         *
         */
        void setConfiguredSentenceBoundary( const QString& sb );

        /**
         * Did this filter do anything?  If the filter returns the input as output
         * unmolested, it should return False when this method is called.
         */
        void setWasModified(bool wasModified);
        bool wasModified();

    signals:
        void filteringFinished();

    protected:
        virtual void run();
        virtual bool event ( QEvent * e );

    private:
        enum TextType {
            ttSsml,             // SSML
            ttCode,             // Code
            ttPlain             // Plain text
        };

        enum SsmlElemType {
            etSpeak,
            etVoice,
            etProsody,
            etEmphasis,
            etPS,               // Paragraph or sentence (we don't care).
            etBreak,
            etNotSsml
        };

        // Speak Element.
        struct SpeakElem {
            QString lang;               // xml:lang="en".
        };

        // Voice Element.
        struct VoiceElem {
            QString lang;               // xml:lang="en".
            QString gender;             // "male", "female", or "neutral".
            uint age;                   // Age in years.
            QString name;               // Synth-specific voice name.
            QString variant;            // Ignored.
        };

        // Prosody Element.
        struct ProsodyElem {
            QString pitch;              // "x-low", "low", "medium", "high", "x-high", "default".
            QString contour;            // Pitch contour (ignored).
            QString range;              // "x-low", "low", "medium", "high", "x-high", "default".
            QString rate;               // "x-slow", "slow", "medium", "fast", "x-fast", "default".
            QString duration;           // Ignored.
            QString volume;             // "silent", "x-soft", "soft", "medium", "load", "x-load", "default".
        };

        // Emphasis Element.
        struct EmphasisElem {
            QString level;              // "strong", "moderate", "none" and "reduced"
        };

        // Break Element.
        struct BreakElem {
            QString strength;           // "x-weak", "weak", "medium" (default value), "strong",
                                        // or "x-strong", "none"
            QString time;               // Ignored.
        };

        // Paragraph and Sentence Elements.
        struct PSElem {
            QString lang;               // xml:lang="en".
        };

        // Given a tag name, returns SsmlElemType.
        SsmlElemType tagToSsmlElemType(const QString tagName);
        // Parses an SSML element, pushing current settings onto the context stack.
        void pushSsmlElem( SsmlElemType et, const QDomElement& elem );
        // Given an attribute name and value, constructs an XML representation of the attribute,
        // i.e., name="value".
        QString makeAttr( const QString& name, const QString& value );
        // Returns an XML representation of an SSML tag from the top of the context stack.
        QString makeSsmlElem( SsmlElemType et );
        // Pops element from the indicated context stack.
        void popSsmlElem( SsmlElemType et );
        QString makeBreakElem( const QDomElement& e );
        // Converts a text fragment into a CDATA section.
        QString makeCDATA( const QString& text );
        // Returns an XML representation of an utterance node consisting of voice,
        // prosody, and emphasis elements.
        QString makeSentence( const QString& text );
        // Starts a sentence by returning a speak tag.
        QString startSentence();
        // Ends a sentence and appends a Tab.
        QString endSentence();
        // Parses a node of the SSML tree and recursively parses its children.
        // Returns the filtered text with each sentence a complete ssml tree.
        QString parseSsmlNode( QDomNode& n, const QString& re );

        // Parses Ssml.
        QString parseSsml( const QString& inputText, const QString& re );
        // Parses code.  Each newline is converted into a tab character (\t).
        QString parseCode( const QString& inputText );
        // Parses plain text.
        QString parsePlainText( const QString& inputText, const QString& re );

        // Context stacks.
        QStack<SpeakElem> m_speakStack;
        QStack<VoiceElem> m_voiceStack;
        QStack<ProsodyElem> m_prosodyStack;
        QStack<EmphasisElem> m_emphasisStack;
        QStack<PSElem> m_psStack;

        // The text being processed.
        QString m_text;
        // Talker Code.
        TalkerCode* m_talkerCode;
        // Configured default Sentence Delimiter regular expression.
        QString m_configuredRe;
        // Configured Sentence Boundary replacement expression.
        QString m_configuredSentenceBoundary;
        // Application-specified Sentence Delimiter regular expression (if any).
        QString m_re;
        // False if input was not modified.
        bool m_wasModified;
        // True when a sentence has been started.
        bool m_sentenceStarted;
};

class SbdProc : virtual public KttsFilterProc
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        SbdProc( QObject *parent, const QStringList &args = QStringList() );

        /**
         * Destructor.
         */
        virtual ~SbdProc();

        /**
         * Initialize the filter.
         * @param config          Settings object.
         * @param configGroup     Settings Group.
         * @return                False if filter is not ready to filter.
         *
         * Note: The parameters are for reading from kttsdrc file.  Plugins may wish to maintain
         * separate configuration files of their own.
         */
        virtual bool init( KConfig *config, const QString &configGroup );

        /**
         * Returns True if this filter is a Sentence Boundary Detector.
         * If so, the filter should implement @ref setSbRegExp() .
         * @return          True if this filter is a SBD.
         */
        virtual bool isSBD();

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
        virtual bool supportsAsync();

        /**
         * Convert input, returning output.  Runs synchronously.
         * @param inputText         Input text.
         * @param talkerCode        TalkerCode structure for the talker that KTTSD intends to
         *                          use for synthing the text.  Useful for extracting hints about
         *                          how to filter the text.  For example, languageCode.
         * @param appId             The DCOP appId of the application that queued the text.
         *                          Also useful for hints about how to do the filtering.
         */
        virtual QString convert( const QString& inputText, TalkerCode* talkerCode, const QString& appId );

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
        virtual bool asyncConvert( const QString& inputText, TalkerCode* talkerCode, const QString& appId );

        /**
         * Waits for a previous call to asyncConvert to finish.
         */
        virtual void waitForFinished();

        /**
         * Returns the state of the Filter.
         */
        virtual int getState();

        /**
         * Returns the filtered output.
         */
        virtual QString getOutput();

        /**
         * Acknowledges the finished filtering.
         */
        virtual void ackFinished();

        /**
         * Stops filtering.  The filteringStopped signal will emit when filtering
         * has in fact stopped and state returns to fsIdle;
         */
        virtual void stopFiltering();

        /**
         * Did this filter do anything?  If the filter returns the input as output
         * unmolested, it should return False when this method is called.
         */
        virtual bool wasModified();

        /**
         * Set Sentence Boundary Regular Expression.
         */
        virtual void setSbRegExp( const QString& re );

    private slots:
        // Received when SBD Thread finishes.
        void slotSbdThreadFilteringFinished();

    private:
        // If not empty, apply filters only to apps using talkers speaking these language codes.
        QStringList m_languageCodeList;
        // If not empty, apply filter only to apps containing this string.
        QStringList m_appIdList;
        // SBD Thread Object.
        SbdThread* m_sbdThread;
        // State.
        int m_state;
        // Configured default Sentence Delimiter regular expression.
        QString m_configuredRe;
};

#endif      // _SBDPROC_H_
