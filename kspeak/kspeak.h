/***************************************************** vim:set ts=4 sw=4 sts=4:
  kspeak
  
  Command-line utility for sending commands to KTTSD service via D-Bus.
  --------------------------------------------------------------------
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

#ifndef KSPEAK_H
#define KSPEAK_H

// Qt includes.
#include <QObject>
#include <QHash>
#include <QThread>
#include <QTimer>

// KDE includes.
#include <kcmdlineargs.h>

// ktts includes.
#include "kspeechinterface.h"

class QString;
class QTextStream;
class QWaitCondition;
class QMutex;

// ====================================================================

/**
 * Class StdinReader reads from an input file in a separate thread, emitting
 * signal lineReady for each line of input.  This prevents from blocking
 * waiting for input.
 */
class StdinReader : public QThread
{
    Q_OBJECT

public:
    StdinReader(const QString& filename, QObject* parent);
    ~StdinReader();

    /**
     * Tell StdinReader to get more input.
     */
    void requestInput();

Q_SIGNALS:
    /**
     * Emitted when a line of input from stdin is ready.
     * @param line      The line of input.
     */
    void lineReady(const QString& line);
    /**
     * Emitted when EOF is encountered on stdin.
     */
    void endInput();

protected:
    void run();

private:
    // Input filename.
    QString m_inputFilename;
    // True to stop reading input.
    mutable bool m_quit;
    // Mutex and wait condition for controlling input.
    QWaitCondition* m_waitForInputRequest;
    QMutex* m_mutexForInputRequest;
};

// ====================================================================

class KSpeak : public QObject
{
    Q_OBJECT

public:
    explicit KSpeak(KCmdLineArgs* args, QObject* parent = 0);
    ~KSpeak();

    bool echo();
    void setEcho(bool on);
    bool stopOnError();
    void setStopOnError(bool on);
    bool showReply();
    void setShowReply(bool on);
    bool showSignals();
    void setShowSignals(bool on);

    Q_PROPERTY(bool echo READ echo WRITE setEcho)
    Q_PROPERTY(bool stopOnError READ stopOnError WRITE setStopOnError)
    Q_PROPERTY(bool showReply READ showReply WRITE setShowReply)
    Q_PROPERTY(bool showSignals READ showSignals WRITE setShowSignals)

    /**
    * See if KTTSD is running, and optional, if not, start it.
    * @param autoStart True to start KTTSD if not already running.
    */
    bool isKttsdRunning(bool autoStart);
    
protected Q_SLOTS:
    Q_SCRIPTABLE void jobStateChanged(const QString &appId, int jobNum, int state);
    Q_SCRIPTABLE void marker(const QString& appId, int jobNum, int markerType, const QString& markerData);

    /**
     * Start processing input from stdin.
     */
    void startInput();

    /**
     * Stop processing input and exit application with m_exitCode.
     */
    void stopInput();

    /**
     * Process a single command from input stream.
     */
    void processCommand(const QString& inputLine);

    /**
     * WAIT for signal timeout.
     */
    void waitForSignalTimeout();

private:
    /**
     * Given a signal, checks to see if it matches a pending WAIT command.
     * It it does, more input is requested from the StdinReader and the pending
     * WAIT is cleared.
     * @param signalName    Name of the signal.  "marker" or "jobStateChanged".
     * @param appId         AppID from signal.
     * @param jobNum        Job Number from signal.
     * @param data1         Marker Type or State from signal.
     * @param data2         Additional data from signal.
     *
     */
    void checkWaitForSignal(const QString& signalName, const QString& appId, int jobNum, int data1, const QString& data2Str = QString("*"));

    /**
    * Print an error message and also store the message in $ERROR variable.
    * @param msg  The message.
    */
    void printError(const QString& msg);
    
    /**
    * Make a DBus call to KSpeech.
    * @param member    Method to call.
    * @param args      QStringList of arguments to pass to method.
    * @return          Returned D-Bus message.
    *
    * The arguments are converted to the types required by the parameters of the member.
    */
    QDBusMessage placeCall(const QString member, QStringList args);
    
    /**
    * Prints a member in format user would enter it into kspeak.
    * The return types and parameter types are given inside angle brackets.
    * @param mm    QMethodMethod to print.
    */
    void printMethodHelp(const QMetaMethod& mm);
    
    /**
    * Prints help.  If no member is specified, lists all members
    * that are not signals.  If "signals" is specified, lists
    * all the signals.  If a specific member is specified,
    * lists that member only.
    * @param member    (optional) "signals" or a specific member name.
    */
    void printHelp(const QString& member = QString());
    
    /**
    * Convert a KTTSD job state integer into a display string.
    * @param state          KTTSD job state
    * @return               Display string for the state.
    */
    QString stateToStr(int state);

    /**
    * Split single string into list of arguments.  Arguments are separated by spaces.
    * If an argument contains space(s), enclose in quotes.
    * @param argsStr   The string to parse.
    * @return          List of parsed argument strings.
    */
    QStringList parseArgs(const QString& argsStr);
    
    /**
    * Converts a QByteArray to a displayable string.  If the array contains undisplayable chars,
    * it is converted to a hexadecimal representation.  Code copied from kdebug.cpp.
    * @param data  The QByteArray.
    * @return      Displayable string.
    */
    QString byteArrayToString(const QByteArray& data);
    
    /**
    * Converts job info in a QByteArray (retrieve via getJobInfo from KTTSD) into displayable string.
    * @param jobInfo   QByteArray containing serialized job info.
    * @return          The displayable string.
    */
    QString jobInfoToString(QByteArray& jobInfo);
    
    /**
    * Convert the arguments of a reply DBusMessage into a QStringList.
    * @param reply The QDBusMessage to convert.
    * @param cmd   (optional) The D-Bus member that returned the reply.
    * @return      List of strings.
    *
    * If cmd is "getJobInfo", unserializes the job info in the reply.
    */
    QStringList dbusReplyToStringList(const QDBusMessage& reply, const QString& cmd = QString());
    
    /**
    * Convert a reply DBusMessage into a printable string.
    * A newline is appended after each returned argument.
    * StringLists are printed as "(one,two,three,etc)".
    * @param reply     The D-Bus reply message.
    * @param cmd       (optional) The D-Bus member that returned the reply.
    * @return          The printable string.
    *
    * If cmd is "getJobInfo", unserializes the job info in the reply.
    */
    QString dbusReplyToPrintable(const QDBusMessage& reply, const QString& cmd = QString());
    
    // Input filename.
    QString m_inputFilename;
    // A dictionary of variables.
    QHash<QString, QString> m_vars;
    // Stdin Reader.
    StdinReader* m_stdinReader;
    // Output streams.
    QTextStream* m_out;
    QTextStream* m_stderr;
    // True if commands should be echoed to display.
    bool m_echo;
    // True if received signals are displayed.
    bool m_showSignals;
    // True if stop on D-Bus error.
    bool m_stopOnError;
    // True if automatic print of return values.
    bool m_showReply;
    // The KSpeech D-Bus Interface.
    org::kde::KSpeech* m_kspeech;
    // Exit code.
    int m_exitCode;
    // Name of buffer variable currently being filled.
    QString m_fillingBuffer;
    // Signal and arguments WAITing for.
    QStringList m_waitingSignal;
    // Timer for WAIT.
    QTimer m_waitTimer;
};

#endif  // KSPEAK_H
