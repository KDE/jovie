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

#ifndef _SSMLCONVERT_H_
#define _SSMLCONVERT_H_

/**
 * SsmlConvert class: 
 * Receives a QStringList of talkers and, based on that information, 
 * evaluates received SSML to discover which of the given talkers best
 * suits it. It can then convert the given SSML into a format understandable
 * by the talker.
 */

// Qt includes
#include <qobject.h>
#include <qstringlist.h>

class KProcess;
class QString;

class SSMLConvert : public QObject {
    Q_OBJECT
public:
    /** Constructors */
    SSMLConvert();
    SSMLConvert(const QStringList &talkers);
    /** Destructor   */
    virtual ~SSMLConvert();

    enum TransformState {
        tsIdle = 0,             // Not doing anything.  Ready to transform.
        tsTransforming = 1,     // Transforming.
        tsFinished = 2          // Transforming finished.
    };

    /**
    * Set the talker codes to be used.
    * @param talkers           talker codes to be used.
    */
    void setTalkers(const QStringList &talkers);

    /**
    * Extract the synth name from a talker code (i.e festival, flite, freetts).
    * @param talkercode        the talker code to extract the talker from.
    * @returns                 the talker.
    */
    QString extractTalker(const QString &talkercode);

    /**
    * Returns the most appropriate talker for the text to synth's talker code.
    * @param text               the text that will be parsed.
    * @returns                  the appropriate talker for the job as a talker code QString.
    *
    * The appropriate talker is the one that has the most features that are required in some
    * SSML markup. In the future i'm hoping to make the importance of individual features 
    * configurable, but better to walk before you can run.
    * Currently, the searching method in place is like a filter: Those that meet the criteria we're
    * searchin for stay while others are sifted out. This should leave us with the right talker to use.
    * It's not a very good method, but should be appropriate in most cases and should do just fine for now.
    * 
    * See the implementation file for more detail.
    */
    QString appropriateTalker(const QString &text) const;

    /**
    * Applies the spreadsheet for a talker to the SSML and returns the talker-native output.
    * @param text               the markup to apply the spreadsheet to.
    * @param xsltFilename       the name of the stylesheet file that will be applied (i.e freetts, flite).
    * @returns                  the output that the synth can understand.
    *
    * This converts a piece of SSML into a format the given talker can understand. It applies
    * an XSLT spreadsheet to the SSML and returns the output.
    */
    bool transform(const QString &text, const QString &xsltFilename);

    /**
    * Returns current processing state.
    */
    int getState();

    /**
    * Returns the output from call to transform.
    */
    QString getOutput();

signals:
    /**
    * Emitted whenever tranforming is completed.
    */
    void transformFinished();

private slots:
    void slotProcessExited(KProcess* proc);

private:
    /// The XSLT processor.
    KProcess *m_xsltProc;
    /// Current talkers.
    QStringList m_talkers;
    // Current state.
    int m_state;
    // Name of XSLT file.
    QString m_xsltFilename;
    // Name of temporary input file.
    QString m_inFilename;
    // Name of temporary output file.
    QString m_outFilename;
};

#endif      // _SSMLCONVERT_H_
