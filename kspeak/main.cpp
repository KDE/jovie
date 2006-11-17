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
#include <QHash>
#include <QtDBus>
#include <QMetaMethod>

// KDE includes.
#include <kdebug.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <ktoolinvocation.h>
#include <kshell.h>

static const KCmdLineOptions options[] =
{
    { "e", 0, 0 },
    { "echo", I18N_NOOP("Echo input. [off]"), 0},
    { "r", 0, 0 },
    { "showreply", I18N_NOOP("Show D-Bus replies. [off]"), 0},
    { "k", 0, 0 },
    { "startkttsd", I18N_NOOP("Start KTTSD if not already running. [off]"), 0},
    { "s", 0, 0 },
    { "nostoponerror", I18N_NOOP("Continue on error."), 0},
    KCmdLineLastOption // End of options.
};

// A dictionary of variables.
QHash<QString, QVariant> vars;

// Input/output streams.
QTextStream in(stdin);
QTextStream out(stdout);
QTextStream stdERR(stderr);

/**
 * Print an error message and also store the message in $ERROR variable.
 * @param msg  The message.
 */
void printError(const QString& msg)
{
    vars["$ERROR"] = msg;
    stdERR << msg << endl;
    stdERR.flush();
};

/**
 * Make a DBus call.
 * @param iface     The KSpeech DBusInterface object.
 * @param member    Method to call.
 * @param args      QStringList of arguments to pass to method.
 *
 * The arguments are converted to the types required by the parameters of the member.
 */
QDBusMessage placeCall(QDBusInterface& iface, const QString member, QStringList args)
{
    int argc = args.size();

    const QMetaObject *mo = iface.metaObject();
    QByteArray match = member.toUpper().toLatin1();
    match += '(';

    int midx = -1;
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
        QMetaMethod mm = mo->method(i);
        QByteArray signature = mm.signature();
        QByteArray ucSignature = signature.toUpper();
        if (ucSignature.startsWith(match)) {
            midx = i;
            break;
        }
    }

    if (midx == -1) {
        QString msg = i18n("Cannot find method '%1'").arg(member);
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
            QString msg = i18n("Sorry, can't pass arg of type %1 yet").arg(
                    types.at(i).constData());
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
                QString msg = i18n("Could not convert '%1' to type '%2'.").arg(
                        args[i]).arg(types.at(i).constData());
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
        QString msg = i18n("Invalid number of parameters");
        return QDBusMessage::createError("org.kde.kspeak.InvalidParameterCount", msg);
    }

    QDBusMessage reply = iface.callWithArgumentList(QDBus::AutoDetect, member, params);
    return reply;
};

/**
 * Prints a member in format user would enter it into kspeak.
 * The return types and parameter types are given inside angle brackets.
 * @param mm    QMethodMethod to print.
 */
void printMethodHelp(const QMetaMethod& mm)
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
    out << rt << mn << " " << params.join(" ") << endl;
};

/**
 * Prints help.  If no member is specified, lists all members
 * that are not signals.  If "signals" is specified, lists
 * all the signals.  If a specific member is specified,
 * lists that member only.
 * @param iface     The KSpeech DBusInteface object.
 * @param member    (Optional) "signals" or a specific member name.
 */
void printHelp(const QDBusInterface& iface, const QString& member = QString())
{
    const QMetaObject *mo = iface.metaObject();

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
};

/**
 * Split single string into list of arguments.  Arguments are separated by spaces.
 * If an argument contains space(s), enclose in quotes.
 */
QStringList parseArgs(const QString& argsStr)
{
    int error = 0;
    QStringList args = KShell::splitArgs(argsStr, KShell::NoOptions, &error);
    return args;
};

/**
 * Convert a reply DBusMessage into a QStringList.
 * @param reply The QDBusMessage to convert.
 */
QStringList dbusReplyToStringList(const QDBusMessage& reply)
{
    QStringList sl;
    foreach (QVariant v, reply.arguments()) {
        if (v.userType() == QVariant::StringList) {
            sl.append("(" + v.toStringList().join(",") + ")");
        } else {
            if (v.userType() == qMetaTypeId<QDBusVariant>())
                v = qvariant_cast<QDBusVariant>(v).variant();
            sl.append(v.toString());
        }
    }
    return sl;
};

/**
 * Convert a reply DBusMessage into a printable string.
 * A newline is appended after each returned argument.
 * StringLists are printed as "(one,two,three,etc)".
 * @param reply     The D-Bus reply message.
 */
QString dbusReplyToPrintable(const QDBusMessage& reply)
{
    QStringList sl = dbusReplyToStringList(reply);
    QStringList pl;
    foreach (QString s, sl)
        pl.append(qPrintable(s));
    return pl.join("\n");
};

/**
 * Main routine.
 */
int main(int argc, char *argv[])
{
    KAboutData aboutdata(
        "kspeak", I18N_NOOP("kspeak"),
        "0.1.0", I18N_NOOP("A utility for sending speech commands to KTTSD service via D-Bus."),
         KAboutData::License_GPL, "(C) 2006, Gary Cramblitt <garycramblitt@comcast.net>");
    aboutdata.addAuthor("Gary Cramblitt", I18N_NOOP("Maintainer"),"garycramblitt@comcast.net");

    KCmdLineArgs::init(argc, argv, &aboutdata);
    // Tell which options are supported
    KCmdLineArgs::addCmdLineOptions(options);

    KApplication app(false);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    
    // See if KTTSD is running.
    kDebug() << "Checking for running KTTSD." << endl;
    bool kttsdRunning = (QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.kttsd"));
    if (kttsdRunning)
        kDebug() << "KTTSD is already running" << endl;
    else
        kDebug() << "KTTSD is not running.  Will try starting it." << endl;

    // If not running, and autostart requested, start KTTSD.
    if (!kttsdRunning && args->isSet("startkttsd")) {
        QString error;
        if (KToolInvocation::startServiceByDesktopName("kttsd", QStringList(), &error)) {
            printError(i18n("Starting KTTSD failed with message %1").arg(qPrintable(error)));
            exit(1);
        } else
            kttsdRunning = true;
    }

    // Create DBUS Interface object.
    kDebug() << "Creating kspeech Interface object." << endl;
    QDBusInterface kspeech("org.kde.kttsd", "/KSpeech", "org.kde.KSpeech");
    kspeech.call("setApplicationName", "kspeak");
    // kspeech.call("say", "Hello World", 0);

    // True if echoing of commands is on.
    bool echo = args->isSet("echo");

    // True if stop on D-Bus error.
    bool stopOnError = args->isSet("stoponerror");

    // True if automatic print of return values.
    bool showReply = args->isSet("showreply");

    // Loop, reading commands from stdin.
    // TODO: Trolltech Tracker Bug #133063 in Qt 4.2.  Change when 4.3 is released.
    // while (!in.atEnd()) {
    while (true) {
        QString line = in.readLine().trimmed();
        kDebug() << "Read line: " << line << endl;

        if (line.isEmpty()) {
            // TODO: Remove next line when Qt 4.3 is released.
            if (in.atEnd()) break;
            // Output blank lines.
            out << endl;
        } else {

            // An @ in column one is a comment sent to output.
            if (line.startsWith("@")) {
                line.remove(0, 1);
                out << qPrintable(line) << endl;
            } else {

                // Look for assignment statement. left = right
                QString left = line.section("=", 0, 0).trimmed();
                QString right = line.section("=", 1).trimmed();
                kDebug() << "left = right: " << left << " = " << right << endl;
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
                foreach (QString var, vars.keys()) {
                    kDebug() << var << ": " + vars[var].toString() << endl;
                    args.replace("$(" + var + ")", vars[var].toString());
                }
                kDebug() << "post variable substitution: " << cmd << " " << args << endl;

                // If echo is on, output command.
                if (echo) out << "> " << cmd << ' ' << args << endl;

                // Non-KTTSD commands.
                QString ucCmd = cmd.toUpper();
                if (ucCmd == "HELP")
                    printHelp(kspeech, args);
                else if (ucCmd == "PRINT")
                    out << qPrintable(args);
                else if (ucCmd == "PRINTLINE")
                    out << qPrintable(args) << endl;
                else if (ucCmd == "BUFFERBEGIN") {
                    // Read into buffer until "BUFFEREND" is seen.
                    QString buf;
                    bool again = true;
                    while (!in.atEnd() && again) {
                        line = in.readLine();
                        again = (line.left(9).toUpper() != "BUFFEREND");
                        if (again) buf += ' ' + line;
                    }
                    // Remove leading space.
                    buf.remove(0, 1);
                    vars["$BUF"] = buf;
                    if (!left.isEmpty()) vars[left] = buf;
                } else if (ucCmd == "PAUSE") {
                    // TODO: Pause args milliseconds.
                    ;
                } else if (ucCmd == "SET") {
                    QString ucArgs = args.toUpper();
                    if (ucArgs == "ECHO ON")
                        echo = true;
                    else if (ucArgs == "ECHO OFF")
                        echo = false;
                    else if (ucArgs == "STOPONERROR ON")
                        stopOnError = true;
                    else if (ucArgs == "STOPONERROR OFF")
                        stopOnError = false;
                    else if (ucArgs == "SHOWREPLY ON")
                        showReply = true;
                    else if (ucArgs == "SHOWREPLY OFF")
                        showReply = false;
                    else {
                        printError(i18n("ERROR: Invalid SET command."));
                        if (stopOnError) exit(1);
                    }
                } else if (ucCmd == "WAIT") {
                    // TODO: Wait for D-Bus signal.
                }
                else {
                    // Parse arguments.
                    QStringList cmdArgs = parseArgs(args);

                    // Send command to KTTSD.
                    QDBusMessage reply = placeCall(kspeech, cmd, cmdArgs);
                    if (reply.type() == QDBusMessage::ErrorMessage) {
                        QDBusError errMsg = reply;
                        QString eMsg = i18n("ERROR: ") + errMsg.name() + ": " + errMsg.message();
                        printError(eMsg);
                        if (stopOnError) exit(1);
                    } else {
                        if (showReply)
                            out << "< " << dbusReplyToPrintable(reply) << endl;
                        QString v = dbusReplyToStringList(reply).join(",");
                        vars["$REPLY"] = v;
                        if (!left.isEmpty()) vars[left] = v;
                    }
                }
            }
        }
        out.flush();
    }
    kDebug() << "At EOL" << endl;
};
