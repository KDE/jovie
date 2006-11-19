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
#include <QTextStream>
#include <QtDBus>
#include <QMetaMethod>
#include <QTimer>

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

StdinReader::StdinReader(QObject* parent, QTextStream* in) :
    QThread(parent), m_in(in)
{
    setTerminationEnabled(true);
}

StdinReader::~StdinReader()
{
    if (isRunning()) {
        terminate();
        wait();
    }
}

void StdinReader::run()
{
    bool quit = false;
    while (!quit) {
        // TODO: Trolltech Tracker Bug #133063 in Qt 4.2.  Change when 4.3 is released.
        // if (in.atEnd())
        //     emit endInput();
        //     exit(0);
        // else
        {
            QString line = m_in->readLine();
            if (line.isEmpty()) {
                if (m_in->atEnd()) {
                    emit endInput();
                    quit = true;
                }
            } else
                emit lineReady(line);
        }
    }
    kDebug() << "StdinReader stopping" << endl;
}

// ====================================================================

KSpeak::KSpeak(QObject* parent) :
    QObject(parent), m_echo(false), m_showSignals(false),
    m_stopOnError(false), m_showReply(false), m_exitCode(0)
{
    // Open input/output streams.
    m_in = new QTextStream(stdin);
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

    // Start input when kapp->exec() is called.
    QTimer::singleShot(0, this, SLOT(startInput()));
}

KSpeak::~KSpeak()
{
    delete m_in;
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
}

void KSpeak::jobStateChanged(const QString &appId, int jobNum, int state)
{
    if (m_showSignals) {
        *m_out << "KTTSD Signal: jobStateChanged from appId: " << appId << " job number: " << jobNum
            << " state " << state << " (" << stateToStr(state) << ")" << endl;
        m_out->flush();
    }
}

/**
 * Print an error message and also store the message in $ERROR variable.
 * @param msg  The message.
 */
void KSpeak::printError(const QString& msg)
{
    m_vars["$ERROR"] = msg;
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
        for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
            QMetaMethod mm = mo->method(i);
            if (mm.methodType() != QMetaMethod::Signal)
                printMethodHelp(mm);
        }
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
        printError(i18n("ERROR: No such command."));
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
    kDebug() << "Checking for running KTTSD." << endl;
    bool kttsdRunning = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kttsd"));
    if (kttsdRunning)
        kDebug() << "KTTSD is already running" << endl;
    else
        kDebug() << "KTTSD is not running.  Will try starting it." << endl;

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

    if (line.isEmpty()) {
        // TODO: Remove next line when Qt 4.3 is released.
        if (m_in->atEnd()) {
            m_exitCode = 1;
            stopInput();
        }
        // Output blank lines.
        *m_out << endl;
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
                args.replace("$(" + var + ")", m_vars[var].toString());
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
                // Read into buffer until "BUFFEREND" is seen.
                QString buf;
                bool again = true;
                while (!m_in->atEnd() && again) {
                    line = m_in->readLine();
                    again = ("BUFFEREND" != line.left(9).toUpper());
                    if (again) buf += ' ' + line;
                }
                // Remove leading space.
                buf.remove(0, 1);
                m_vars["$BUF"] = buf;
                if (!left.isEmpty()) m_vars[left] = buf;
            } else if ("PAUSE" == ucCmd) {
                // TODO: Pause args milliseconds.
                ;
            } else if ("SET" == ucCmd) {
                QString ucArgs = args.toUpper();
                if ("ECHO ON" == ucArgs)
                    m_echo = true;
                else if ("ECHO OFF" == ucArgs)
                    m_echo = false;
                else if ("STOPONERROR ON" == ucArgs)
                    m_stopOnError = true;
                else if ("STOPONERROR OFF" == ucArgs)
                    m_stopOnError = false;
                else if ("SHOWREPLY ON" == ucArgs)
                    m_showReply = true;
                else if ("SHOWREPLY OFF" == ucArgs)
                    m_showReply = false;
                else if ("SHOWSIGNALS ON" == ucArgs)
                    m_showSignals = true;
                else if ("SHOWSIGNALS OFF" == ucArgs)
                    m_showSignals = false;
                else {
                    printError(i18n("ERROR: Invalid SET command."));
                    if (m_stopOnError) {
                        m_exitCode = 1;
                        stopInput();
                    }
                }
            } else if ("WAIT" == ucCmd) {
                // TODO: Wait for D-Bus signal.
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
                    m_vars["$REPLY"] = v;
                    if (!left.isEmpty()) m_vars[left] = v;
                }
            }
        }
    }
    m_out->flush();
}

void KSpeak::startInput()
{
    // Create Stdin Reader object in another thread.
    // It emits lineReady for each input line.
    m_stdinReader = new StdinReader(this, m_in);
    connect(m_stdinReader, SIGNAL(lineReady(const QString&)),
        this, SLOT(processCommand(const QString&)), Qt::QueuedConnection);
    connect(m_stdinReader, SIGNAL(endInput()),
        this, SLOT(stopInput()), Qt::QueuedConnection);
    m_stdinReader->start();
}

void KSpeak::stopInput()
{
    delete m_stdinReader;
    kapp->exit(m_exitCode);
}

#include "kspeak.moc"
