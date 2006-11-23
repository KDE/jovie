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

// Qt includes.
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QtDBus>
#include <QMetaMethod>
#include <QTimer>
#include <QWaitCondition>
#include <QMutex>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <ktoolinvocation.h>
#include <kshell.h>
#include <kapplication.h>

// KTTS includes.
#include <kspeech.h>

// kspeak includes.
#include "kspeak.h"

// ====================================================================

StdinReader::StdinReader(const QString& filename, QObject* parent) :
    QThread(parent), m_inputFilename(filename), m_waitForInputRequest(0), m_mutexForInputRequest(0)
{
}

StdinReader::~StdinReader()
{
    if (0 != m_waitForInputRequest) {
        m_quit = true;
        m_waitForInputRequest->wakeOne();
        // kDebug() << "Waiting for StdinReader thread to exit." << endl;
        wait();
        // kDebug() << "StdinReader thread has exited." << endl;
        delete m_mutexForInputRequest;
        delete m_waitForInputRequest;
    }
}

void StdinReader::requestInput()
{
    // Notice this is running in caller's thread.
    // Wait for StdinReader to go into waiting state,
    // then wake it.
    m_mutexForInputRequest->lock();
    m_mutexForInputRequest->unlock();
    m_waitForInputRequest->wakeOne();
}

void StdinReader::run()
{
    m_quit = false;
    m_mutexForInputRequest = new QMutex();
    m_waitForInputRequest = new QWaitCondition();
    QTextStream* in;
    if ("-" == m_inputFilename)
        in =  new QTextStream(stdin);
    else {
        QFile inFile(m_inputFilename);
        inFile.open(QFile::ReadOnly);
        in = new QTextStream(&inFile);
    }
    m_mutexForInputRequest->lock();
    while (!m_quit) {
        // TODO: Trolltech Tracker Bug #133063 in Qt 4.2.  Change when 4.3 is released.
        // if (in->atEnd())
        //     m_quit = true;
        //     emit endInput();
        // else
        QString line = in->readLine();
        if ("PAUSE" == line.trimmed().left(5).toUpper()) {
            QString args = line.trimmed().mid(6);
            bool ok;
            int msec = args.toInt(&ok);
            if (ok) msleep(msec);
        } else {
            emit lineReady(line);
            // Wait until kspeak is ready to process more input.
            if (!m_quit) m_waitForInputRequest->wait(m_mutexForInputRequest);
        }
    }
    m_mutexForInputRequest->unlock();
    delete in;
    delete m_mutexForInputRequest;
    m_mutexForInputRequest = 0;
    delete m_waitForInputRequest;
    m_waitForInputRequest = 0;
    // kDebug() << "StdinReader stopping" << endl;
}

// ====================================================================

KSpeak::KSpeak(KCmdLineArgs* args, QObject* parent) :
    QObject(parent), m_echo(false), m_showSignals(false),
    m_stopOnError(false), m_showReply(false), m_exitCode(0)
{
    // Open output streams.
    m_out = new QTextStream(stdout);
    m_stderr = new QTextStream(stderr);

    // Create KSpeech D-Bus interface.
    m_kspeech = new OrgKdeKSpeechInterface("org.kde.kttsd", "/KSpeech", QDBusConnection::sessionBus());
    m_kspeech->setParent(this);
    m_kspeech->setApplicationName("kspeak");

    // Connect D-Bus signals.
    connect(m_kspeech, SIGNAL(jobStateChanged(const QString&, int, int)),
        this, SLOT(jobStateChanged(const QString&, int, int)));
    connect(m_kspeech, SIGNAL(marker(const QString&, int, int, const QString&)),
        this, SLOT(marker(const QString&, int, int, const QString&)));

    // Set up WAIT for signal timer.
    m_waitTimer.setInterval(0);
    m_waitTimer.setSingleShot(true);
    connect(&m_waitTimer, SIGNAL(timeout()), this, SLOT(waitForSignalTimeout()));

    // Store AppID variable.
    m_vars["_APPID"] = m_kspeech->connection().baseService();
    // kDebug() << "kspeak AppID = " << m_vars["_APPID"] << endl;

    // Input filename.
    m_inputFilename = QFile::decodeName(args->arg(0));

    // Store command-line arguments and count.
    m_vars["_ARGCOUNT"] = QString().setNum(args->count() - 1);
    for(int i = 0; i < args->count(); i++)
    {
        QString s = QString().setNum(i);
        QString varName = "_ARG" + s;
        m_vars[varName] = args->arg(i);
     }

    // Start input when kapp->exec() is called.
    QTimer::singleShot(0, this, SLOT(startInput()));
}

KSpeak::~KSpeak()
{
    delete m_out;
    delete m_stderr;
}

bool KSpeak::echo() { return m_echo; }
void KSpeak::setEcho(bool on) { m_echo = on; }
bool KSpeak::stopOnError() { return m_stopOnError; }
void KSpeak::setStopOnError(bool on) { m_stopOnError = on; }
bool KSpeak::showReply() { return m_showReply; }
void KSpeak::setShowReply(bool on) { m_showReply = on; }
bool KSpeak::showSignals() { return m_showSignals; }
void KSpeak::setShowSignals(bool on) { m_showSignals = on; }

void KSpeak::marker(const QString& appId, int jobNum, int markerType, const QString& markerData)
{
    if (m_showSignals) {
        *m_out << "KTTSD Signal: marker from appId: " << appId << " job number: " << jobNum
            << " type: " << markerType << " data: " << markerData << endl;
        m_out->flush();
    }
    checkWaitForSignal("marker", appId, jobNum, markerType, markerData);
}

void KSpeak::jobStateChanged(const QString &appId, int jobNum, int state)
{
    if (m_showSignals) {
        *m_out << "KTTSD Signal: jobStateChanged from appId: " << appId << " job number: " << jobNum
            << " state " << state << " (" << stateToStr(state) << ")" << endl;
        m_out->flush();
    }
    checkWaitForSignal("jobStateChanged", appId, jobNum, state);
}

void KSpeak::checkWaitForSignal(const QString& signalName, const QString& appId, int jobNum, int data1, const QString& data2Str)
{
    if (0 == m_waitingSignal.size()) return;
    if ("*" != m_waitingSignal[0] && signalName != m_waitingSignal[0]) return;
    if ("*" != m_waitingSignal[1] && appId != m_waitingSignal[1]) return;
    QString jNumStr = QString().setNum(jobNum);
    if ("*" != m_waitingSignal[2] && jNumStr != m_waitingSignal[2]) return;
    QString data1Str = QString().setNum(data1);
    if ("*" != m_waitingSignal[3] && data1Str != m_waitingSignal[3]) return;
    if ("*" != m_waitingSignal[4] && data2Str != m_waitingSignal[4]) return;
    // kDebug() << "WAIT for signal matched" << endl;
    if (m_waitTimer.isActive()) m_waitTimer.stop();
    m_waitingSignal = QStringList();
    m_stdinReader->requestInput();
}

void KSpeak::waitForSignalTimeout()
{
    if (m_waitingSignal.size() == 0) return;
    *m_out << "WAIT TIMEOUT" << endl;
    m_waitingSignal = QStringList();
    m_stdinReader->requestInput();
}

/**
 * Print an error message and also store the message in _ERROR variable.
 * @param msg  The message.
 */
void KSpeak::printError(const QString& msg)
{
    m_vars["_ERROR"] = msg;
    *m_stderr << msg << endl;
    m_stderr->flush();
}

/**
 * Make a DBus call.
 * @param member    Method to call.
 * @param args      QStringList of arguments to pass to method.
 * @return          Returned D-Bus message.
 *
 * The arguments are converted to the types required by the parameters of the member.
 */
QDBusMessage KSpeak::placeCall(const QString member, QStringList args)
{
    int argc = args.size();
    QString membr;

    const QMetaObject *mo = m_kspeech->metaObject();
    QByteArray match = member.toUpper().toLatin1();
    match += '(';

    int midx = -1;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        QByteArray signature = mm.signature();
        QByteArray ucSignature = signature.toUpper();
        if (ucSignature.startsWith(match)) {
            midx = i;
            membr = signature.left(signature.indexOf("("));
            break;
        }
    }

    if (midx == -1) {
        QString msg = i18n("Cannot find method '%1'", member);
        return QDBusMessage::createError("org.kde.kspeak.InvalidCommand", msg);
    }

    QMetaMethod mm = mo->method(midx);
    QList<QByteArray> types = mm.parameterTypes();
    for (int i = 0; i < types.count(); ++i)
        if (types.at(i).endsWith('&')) {
            // reference (and not a reference to const): output argument
            // we're done with the inputs
            while (types.count() > i)
                types.removeLast();
            break;
        }        

    QVariantList params;
    for (int i = 0; argc && i < types.count(); ++i) {
        int id = QVariant::nameToType(types.at(i));
        if ((id == QVariant::UserType || id == QVariant::Map) && types.at(i) != "QDBusVariant") {
            QString msg = i18n("Sorry, can't pass arg of type %1 yet.", types.at(i).constData());
            return QDBusMessage::createError("org.kde.kspeak.InvalidArgument", msg);
        }
        if (id == QVariant::UserType)
            id = QMetaType::type(types.at(i));

        Q_ASSERT(id);

        QVariant p = args[i];
//        if ((id == QVariant::List || id == QVariant::StringList) && QLatin1String("(") == argv[0])
//            p = readList(argc, argv);
//        else
//            p = QString::fromLocal8Bit(argv[0]);

        if (id < int(QVariant::UserType)) {
            // avoid calling it for QVariant
            p.convert( QVariant::Type(id) );
            if (p.type() == QVariant::Invalid) {
                QString msg = i18n("Could not convert '%1' to type '%2'.",
                    args[i], types.at(i).constData());
                return QDBusMessage::createError("org.kde.kspeak.UnsupportedType", msg);
            }
        } else if (types.at(i) == "QDBusVariant") {
            QDBusVariant tmp(p);
            p = qVariantFromValue(tmp);
        }
        params += p;
        --argc;
    }
    if (params.count() != types.count() || argc != 0) {
        QString msg = i18n("Invalid number of parameters.");
        return QDBusMessage::createError("org.kde.kspeak.InvalidParameterCount", msg);
    }

    QDBusMessage reply = m_kspeech->callWithArgumentList(QDBus::AutoDetect, membr, params);
    return reply;
}

/**
 * Prints a member in format user would enter it into kspeak.
 * The return types and parameter types are given inside angle brackets.
 * @param mm    QMethodMethod to print.
 */
void KSpeak::printMethodHelp(const QMetaMethod& mm)
{
    QByteArray rt = mm.typeName();
    if (!rt.isEmpty()) rt = "<" + rt + "> = ";
    QByteArray signature = mm.signature();
    QByteArray mn = signature.left(signature.indexOf("("));
    QList<QByteArray> ptl = mm.parameterTypes();
    QList<QByteArray> pnl = mm.parameterNames();
    QStringList params;
    for (int i = 0; i < ptl.size(); ++i)
        params.append("<" + ptl[i] + ">" + pnl[i]);
    *m_out << rt << mn << " " << params.join(" ") << endl;
}

/**
 * Prints help.  If no member is specified, lists all members
 * that are not signals.  If "signals" is specified, lists
 * all the signals.  If a specific member is specified,
 * lists that member only.
 * @param member    (optional) "signals" or a specific member name.
 */
void KSpeak::printHelp(const QString& member)
{
    const QMetaObject *mo = m_kspeech->metaObject();

    if (member.isEmpty()) {
        *m_out << i18n("Enter HELP <option> where <option> may be:") << endl;
        *m_out << i18n("  COMMANDS to list local commands understood by kspeak.") << endl;
        *m_out << i18n("  SIGNALS to list KTTSD signals sent via D-Bus.") << endl;
        *m_out << i18n("  MEMBERS to list all commands that may be sent to KTTSD via D-Bus.") << endl;
        *m_out << i18n("  <member> to show a single command that may be sent to KTTSD via D-Bus.") << endl;
        *m_out << i18n("Options may be entered in lower- or uppercase.  Examples:") << endl;
        *m_out << i18n("  help commands") << endl;
        *m_out << i18n("  help say") << endl;
        *m_out << i18n("Member argument types are displayed in brackets.") << endl;
    } else if ("COMMANDS" == member.toUpper()) {
        *m_out << "QUIT                 " << i18n("Exit kspeak.") << endl;
        *m_out << "SET ECHO ON          " << i18n("Echo inputs to stdin.") << endl;
        *m_out << "SET ECHO OFF         " << i18n("Do not echo inputs.") << endl;
        *m_out << "SET STOPONERROR ON   " << i18n("Stop if any errors occur.") << endl;
        *m_out << "SET STOPONERROR OFF  " << i18n("Do not stop if an error occurs.") << endl;
        *m_out << "SET REPLIES ON       " << i18n("Display values returned by KTTSD.") << endl;
        *m_out << "SET REPLIES OFF      " << i18n("Do not display KTTSD return values.") << endl;
        *m_out << "SET SIGNALS ON       " << i18n("Display signals emitted by KTTSD.") << endl;
        *m_out << "SET SIGNALS OFF      " << i18n("Do not display KTTSD signals.") << endl;
        *m_out << "SET WTIMEOUT <msec>  " << i18n("Set the WAIT timeout to <msec> milliseconds. 0 waits forever.") << endl;
        *m_out << "BUFFERBEGIN          " << i18n("Start filling a buffer.") << endl;
        *m_out << "BUFFEREND            " << i18n("Stop filling buffer.") << endl;
        *m_out << i18n("  Example buffer usage:") << endl;
        *m_out << "    mybuf = BUFFERBEGIN" << endl;
        *m_out << "    KDE is a powerful Free Software graphical desktop environment" << endl;
        *m_out << "    for Linux and Unix workstations." << endl;
        *m_out << "    BUFFEREND" << endl;
        *m_out << "    say \"$(mybuf)\" 0" << endl;
        *m_out << "PAUSE <msec>         " << i18n("Pause <msec> milliseconds.  Example") << endl;
        *m_out << "  pause 500" << endl;
        *m_out << "WAIT <signal> <args> " << i18n("Wait for <signal> with (optional) <args> arguments.  Example:")  << endl;
        *m_out << "  set wtimeout 5000" << endl;
        *m_out << "  wait marker" << endl;
    } else if ("MEMBERS" == member.toUpper()) {
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            QMetaMethod mm = mo->method(i);
            if (mm.methodType() != QMetaMethod::Signal)
                printMethodHelp(mm);
        }
        *m_out << endl;
        *m_out << i18n("Values returned by a member may be assigned to a variable.") << endl;
        *m_out << i18n("Variables may be substituted in format $(variable).  Examples:") << endl;
        *m_out << "  jobnum = say \"Hello World\" 0" << endl;
        *m_out << "  remove $(jobnum)" << endl;
    } else if ("SIGNALS" == member.toUpper()) {
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            QMetaMethod mm = mo->method(i);
            if (mm.methodType() == QMetaMethod::Signal)
                printMethodHelp(mm);
        }
    } else {
        QByteArray match = member.toUpper().toLatin1();
        match += '(';
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            QMetaMethod mm = mo->method(i);
            QByteArray signature = mm.signature();
            QByteArray ucSignature = signature.toUpper();
            if (ucSignature.startsWith(match)) {
                printMethodHelp(mm);
                return;
            }
        }
        printError(i18n("ERROR: No such member."));
    }
}

/**
* Convert a KTTSD job state integer into a display string.
* @param state          KTTSD job state
* @return               Display string for the state.
*/
QString KSpeak::stateToStr(int state)
{
    switch( state )
    {
        case KSpeech::jsQueued: return        i18n("Queued");
        case KSpeech::jsFiltering: return     i18n("Filtering");
        case KSpeech::jsSpeakable: return     i18n("Waiting");
        case KSpeech::jsSpeaking: return      i18n("Speaking");
        case KSpeech::jsPaused: return        i18n("Paused");
        case KSpeech::jsInterrupted: return   i18n("Interrupted");
        case KSpeech::jsFinished: return      i18n("Finished");
        case KSpeech::jsDeleted: return       i18n("Deleted");
        default: return                       i18n("Unknown");
    }
}

/**
 * Split single string into list of arguments.  Arguments are separated by spaces.
 * If an argument contains space(s), enclose in quotes.
 * @param argsStr   The string to parse.
 * @return          List of parsed argument strings.
 */
QStringList KSpeak::parseArgs(const QString& argsStr)
{
    int error = 0;
    QStringList args = KShell::splitArgs(argsStr, KShell::NoOptions, &error);
    return args;
}

/**
 * Converts a QByteArray to a displayable string.  If the array contains undisplayable chars,
 * it is converted to a hexadecimal representation.  Code copied from kdebug.cpp.
 * @param data  The QByteArray.
 * @return      Displayable string.
 */
QString KSpeak::byteArrayToString(const QByteArray& data)
{
    QString str;
    bool isBinary = false;
    for ( int i = 0; i < data.size() && !isBinary ; ++i ) {
        if ( data[i] < 32 || (unsigned char)data[i] > 127 )
            isBinary = true;
    }
    if ( isBinary ) {
        str = QLatin1Char('[');
        // int sz = qMin( data.size(), 64 );
        int sz = data.size();
        for ( int i = 0; i < sz ; ++i ) {
            str += QString::number( (unsigned char) data[i], 16 ).rightJustified(2, QLatin1Char('0'));
            if ( i < sz )
                str += QLatin1Char(' ');
        }
        // if ( sz < data.size() )
        //    str += QLatin1String("...");
        str += QLatin1Char(']');
    } else {
        str += QLatin1String( data ); // using ascii as advertised
    }
    return str;
}

/**
 * Converts job info in a QByteArray (retrieve via getJobInfo from KTTSD) into displayable string.
 * @param jobInfo   QByteArray containing serialized job info.
 * @return          The displayable string.
 */
QString KSpeak::jobInfoToString(QByteArray& jobInfo)
{
    QString str;
    QString s;
    QDataStream stream(&jobInfo, QIODevice::ReadOnly);
    qint32 priority;
    qint32 state;
    QString appId;
    QString talkerCode;
    qint32 sentenceNum;
    qint32 sentenceCount;
    QString applicationName;
    stream >> priority;
    s.setNum(priority);
    str = s;
    stream >> state;
    s.setNum(state);
    str += ',' + s;
    stream >> appId;
    str += ',' + appId;
    stream >> talkerCode;
    str += ',' + talkerCode;
    stream >> sentenceNum;
    s.setNum(sentenceNum);
    str += ',' + s;
    stream >> sentenceCount;
    s.setNum(sentenceCount);
    str += ',' + s;
    stream >> applicationName;
    str += ',' + applicationName;
    return str;
}

/**
 * Convert the arguments of a reply DBusMessage into a QStringList.
 * @param reply The QDBusMessage to convert.
 * @param cmd   (optional) The D-Bus member that returned the reply.
 * @return      List of strings.
 *
 * If cmd is "getJobInfo", unserializes the job info in the reply.
 */
QStringList KSpeak::dbusReplyToStringList(const QDBusMessage& reply, const QString& cmd)
{
    QStringList sl;
    foreach (QVariant v, reply.arguments()) {
        if (QVariant::StringList == v.userType()) {
            sl.append(v.toStringList().join(","));
        } else {
            if (QVariant::ByteArray == v.userType()) {
                QByteArray ba = v.toByteArray();
                if ("GETJOBINFO" == cmd.toUpper())
                    sl.append(jobInfoToString(ba));
                else
                    sl.append(byteArrayToString(ba));
            } else {
                if (v.userType() == qMetaTypeId<QDBusVariant>())
                    v = qvariant_cast<QDBusVariant>(v).variant();
                sl.append(v.toString());
            }
        }
    }
    return sl;
}

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
QString KSpeak::dbusReplyToPrintable(const QDBusMessage& reply, const QString& cmd)
{
    QStringList sl = dbusReplyToStringList(reply, cmd);
    QStringList pl;
    foreach (QString s, sl)
        pl.append(qPrintable(s));
    return pl.join("\n");
}

bool KSpeak::isKttsdRunning(bool autoStart)
{
    // See if KTTSD is running.
    // kDebug() << "Checking for running KTTSD." << endl;
    bool kttsdRunning = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kttsd"));
    // if (kttsdRunning)
    //     kDebug() << "KTTSD is already running" << endl;
    // else
    //     kDebug() << "KTTSD is not running." << endl;

    // If not running, and autostart requested, start KTTSD.
    if (!kttsdRunning && autoStart) {
        QString error;
        if (0 != KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error))
            printError(i18n("Starting KTTSD failed with message: %1", qPrintable(error)));
        else
            kttsdRunning = true;
    }
    return kttsdRunning;
}

void KSpeak::processCommand(const QString& inputLine)
{
    QString line = inputLine.trimmed();
    // kDebug() << "Line read: " << line << endl;

    bool requestMoreInput = true;
    if (line.isEmpty()) {
        stopInput();
        // Output blank lines.
        *m_out << endl;
    } else {

        // If filling a buffer, fill it until BUFFEREND is seen.
        if (!m_fillingBuffer.isEmpty()) {
            if ("BUFFEREND" == line.trimmed().toUpper()) {
                // kDebug() << "End buffer filling for " << m_fillingBuffer << endl;
                // Remove first space.
                m_vars["_BUF"].remove(0, 1);
                if ("_BUF" != m_fillingBuffer) m_vars[m_fillingBuffer] = m_vars["_BUF"];
                m_fillingBuffer = QString();
            } else {
                // kDebug() << "Appending to buffer " << m_fillingBuffer << " data = " << line << endl;
                m_vars["_BUF"] += ' ' + line;
            }
        } else {
            // An @ in column one is a comment sent to output.
            if (line.startsWith("@")) {
                line.remove(0, 1);
                *m_out << qPrintable(line) << endl;
            } else {
    
                // Look for assignment statement. left = right
                QString left = line.section("=", 0, 0).trimmed();
                QString right = line.section("=", 1).trimmed();
                // kDebug() << "left = right: " << left << " = " << right << endl;
                if (right.isEmpty()) {
                    right = left;
                    left = QString();
                }
    
                // Obtain command, which is first word, and arguments that follow.
                // cmd arg arg...
                QString cmd = right.section(" ", 0, 0).trimmed();
                QString args = right.section(" ", 1).trimmed();
                // kDebug() << "cmd: " << cmd << " args: " << args << endl;
    
                // Variable substitution.
                foreach (QString var, m_vars.keys()) {
                    // kDebug() << var << ": " + m_vars[var].toString() << endl;
                    args.replace("$(" + var + ")", m_vars[var]);
                }
                // kDebug() << "post variable substitution: " << cmd << " " << args << endl;
    
                // If echo is on, output command.
                if (m_echo) *m_out << "> " << cmd << ' ' << args << endl;
    
                // Non-KTTSD commands.
                QString ucCmd = cmd.toUpper();
                if ("QUIT" == ucCmd)
                    stopInput();
                else if ("HELP" == ucCmd)
                    printHelp(args);
                else if ("PRINT" == ucCmd)
                    *m_out << qPrintable(args);
                else if ("PRINTLINE" == ucCmd) {
                    *m_out << qPrintable(args) << endl;
                } else if ("BUFFERBEGIN" == ucCmd) {
                    if (!left.isEmpty())
                        m_fillingBuffer = left;
                    else
                        m_fillingBuffer = "_BUF";
                    m_vars["_BUF"] = QString();
                } else if ("SET" == ucCmd) {
                    QString ucArgs = args.toUpper();
                    QString property = ucArgs.section(" ", 0, 0).trimmed();
                    QString value = ucArgs.section(" ", 1).trimmed();
                    bool onOff = ("ON" == value) ? true : false;
                    if ("ECHO" == property)
                        m_echo = onOff;
                    else if ("STOPONERROR" == property)
                        m_stopOnError = onOff;
                    else if ("REPLIES" == property)
                        m_showReply = onOff;
                    else if ("SIGNALS" == property)
                        m_showSignals = onOff;
                    else if ("WTIMEOUT" == property)
                        m_waitTimer.setInterval(value.toInt());
                    else {
                        printError(i18n("ERROR: Invalid SET command."));
                        if (m_stopOnError) {
                            m_exitCode = 1;
                            stopInput();
                        }
                    }
                } else if ("WAIT" == ucCmd) {
                    m_waitingSignal = parseArgs(args);
                    if (0 == m_waitingSignal.size()) {
                        printError(i18n("ERROR: Invalid WAIT command."));
                        if (m_stopOnError) {
                            m_exitCode = 1;
                            stopInput();
                        }
                    }
                    // Pad waiting signal string list.
                    while (m_waitingSignal.size() < 5)
                        m_waitingSignal.append("*");
                    // kDebug() << "WAIT" << m_waitingSignal << endl;
                    requestMoreInput = false;
                    if (m_waitTimer.interval() > 0)
                        m_waitTimer.start();
                }
                else {
                    // Parse arguments.
                    QStringList cmdArgs = parseArgs(args);
    
                    // Send command to KTTSD.
                    QDBusMessage reply = placeCall(cmd, cmdArgs);
                    if (QDBusMessage::ErrorMessage == reply.type()) {
                        QDBusError errMsg = reply;
                        QString eMsg = i18n("ERROR: ") + errMsg.name() + ": " + errMsg.message();
                        printError(eMsg);
                        if (m_stopOnError) {
                            m_exitCode = 1;
                            stopInput();
                        }
                    } else {
                        if (m_showReply)
                            *m_out << "< " << dbusReplyToPrintable(reply, cmd) << endl;
                        QString v = dbusReplyToStringList(reply, cmd).join(",");
                        m_vars["_REPLY"] = v;
                        if (!left.isEmpty()) m_vars[left] = v;
                    }
                }
            }
        }
    }
    m_out->flush();
    // Get more input, if we aren't exiting and not waiting for a signal.
    if (0 != m_stdinReader && requestMoreInput) m_stdinReader->requestInput();
}

void KSpeak::startInput()
{
    if ("-" != m_inputFilename && !QFile::exists(m_inputFilename)) {
        printError(QString("ERROR: invalid input file name: %1").arg(m_inputFilename));
        kapp->exit(1);
    }
    // Create Stdin Reader object which runs in another thread.
    // It emits lineReady for each input line.
    // kDebug() << "KSpeak::startInput: creating StdinReader" << endl;
    m_stdinReader = new StdinReader(m_inputFilename, this);
    connect(m_stdinReader, SIGNAL(lineReady(const QString&)),
        this, SLOT(processCommand(const QString&)), Qt::QueuedConnection);
    connect(m_stdinReader, SIGNAL(endInput()),
        this, SLOT(stopInput()), Qt::QueuedConnection);
    m_stdinReader->start();
}

void KSpeak::stopInput()
{
    delete m_stdinReader;
    m_stdinReader = 0;
    kapp->exit(m_exitCode);
}

#include "kspeak.moc"
