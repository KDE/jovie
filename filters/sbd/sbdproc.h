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
#include <tqobject.h>
#include <tqstringlist.h>
#include <tqthread.h>
#include <tqvaluestack.h>
#include <tqevent.h>

// KTTS includes.
#include "filterproc.h"

class TalkerCode;
class KConfig;
class QDomElement;
class QDomNode;

class SbdThread: public TQObject, public QThread
{
    Q_OBJECT

    public:
        /**
         * Constructor.
         */
        SbdThread( TQObject *parent = 0, const char *name = 0);

        /**
         * Destructor.
         */
        virtual ~SbdThread();

        /**
         * Get/Set text being processed.
         */
        void setText( const TQString& text );
        TQString text();

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
        void setSbRegExp( const TQString& re );

        /**
         * The configured Sentence Boundary Regular Expression.
         *
         * @param re            The sentence delimiter regular expression.
         */
        void setConfiguredSbRegExp( const TQString& re );

        /**
         * The configured Sentence Boundary that replaces SB regular expression.
         *
         * @param sb            The sentence boundary replacement.
         *
         */
        void setConfiguredSentenceBoundary( const TQString& sb );

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
        virtual bool event ( TQEvent * e );

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
            TQString lang;               // xml:lang="en".
        };

        // Voice Element.
        struct VoiceElem {
            TQString lang;               // xml:lang="en".
            TQString gender;             // "male", "female", or "neutral".
            uint age;                   // Age in years.
            TQString name;               // Synth-specific voice name.
            TQString variant;            // Ignored.
        };

        // Prosody Element.
        struct ProsodyElem {
            TQString pitch;              // "x-low", "low", "medium", "high", "x-high", "default".
            TQString contour;            // Pitch contour (ignored).
            TQString range;              // "x-low", "low", "medium", "high", "x-high", "default".
            TQString rate;               // "x-slow", "slow", "medium", "fast", "x-fast", "default".
            TQString duration;           // Ignored.
            TQString volume;             // "silent", "x-soft", "soft", "medium", "load", "x-load", "default".
        };

        // Emphasis Element.
        struct EmphasisElem {
            TQString level;              // "strong", "moderate", "none" and "reduced"
        };

        // Break Element.
        struct BreakElem {
            TQString strength;           // "x-weak", "weak", "medium" (default value), "strong",
                                        // or "x-strong", "none"
            TQString time;               // Ignored.
        };

        // Paragraph and Sentence Elements.
        struct PSElem {
            TQString lang;               // xml:lang="en".
        };

        // Given a tag name, returns SsmlElemType.
        SsmlElemType tagToSsmlElemType(const TQString tagName);
        // Parses an SSML element, pushing current settings onto the context stack.
        void pushSsmlElem( SsmlElemType et, const TQDomElement& elem );
        // Given an attribute name and value, constructs an XML representation of the attribute,
        // i.e., name="value".
        TQString makeAttr( const TQString& name, const TQString& value );
        // Returns an XML representation of an SSML tag from the top of the context stack.
        TQString makeSsmlElem( SsmlElemType et );
        // Pops element from the indicated context stack.
        void popSsmlElem( SsmlElemType et );
        TQString makeBreakElem( const TQDomElement& e );
        // Converts a text fragment into a CDATA section.
        TQString makeCDATA( const TQString& text );
        // Returns an XML representation of an utterance node consisting of voice,
        // prosody, and emphasis elements.
        TQString makeSentence( const TQString& text );
        // Starts a sentence by returning a speak tag.
        TQString startSentence();
        // Ends a sentence and appends a Tab.
        TQString endSentence();
        // Parses a node of the SSML tree and recursively parses its children.
        // Returns the filtered text with each sentence a complete ssml tree.
        TQString parseSsmlNode( TQDomNode& n, const TQString& re );

        // Parses Ssml.
        TQString parseSsml( const TQString& inputText, const TQString& re );
        // Parses code.  Each newline is converted into a tab character (\t).
        TQString parseCode( const TQString& inputText );
        // Parses plain text.
        TQString parsePlainText( const TQString& inputText, const TQString& re );

        // Context stacks.
        TQValueStack<SpeakElem> m_speakStack;
        TQValueStack<VoiceElem> m_voiceStack;
        TQValueStack<ProsodyElem> m_prosodyStack;
        TQValueStack<EmphasisElem> m_emphasisStack;
        TQValueStack<PSElem> m_psStack;

        // The text being processed.
        TQString m_text;
        // Talker Code.
        TalkerCode* m_talkerCode;
        // Configured default Sentence Delimiter regular expression.
        TQString m_configuredRe;
        // Configured Sentence Boundary replacement expression.
        TQString m_configuredSentenceBoundary;
        // Application-specified Sentence Delimiter regular expression (if any).
        TQString m_re;
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
        SbdProc( TQObject *parent, const char *name, const TQStringList &args = TQStringList() );

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
        virtual bool init( KConfig *config, const TQString &configGroup );

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
        virtual TQString convert( const TQString& inputText, TalkerCode* talkerCode, const TQCString& appId );

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
        virtual bool asyncConvert( const TQString& inputText, TalkerCode* talkerCode, const TQCString& appId );

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
        virtual TQString getOutput();

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
        virtual void setSbRegExp( const TQString& re );

    private slots:
        // Received when SBD Thread finishes.
        void slotSbdThreadFilteringFinished();

    private:
        // If not empty, apply filters only to apps using talkers speaking these language codes.
        TQStringList m_languageCodeList;
        // If not empty, apply filter only to apps containing this string.
        TQStringList m_appIdList;
        // SBD Thread Object.
        SbdThread* m_sbdThread;
        // State.
        int m_state;
        // Configured default Sentence Delimiter regular expression.
        TQString m_configuredRe;
};

#endif      // _SBDPROC_H_
