/***************************************************** vim:set ts=4 sw=4 sts=4:
  KTTSD DBUS Adaptor class
  ------------------------
  Copyright:
  (C) 2006 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
  Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>
  
  This file was created by customizing output from dbusidl2cpp, which
  was run on org.kde.KSpeech.xml.
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

#ifndef KSPEECHADAPTOR_P_H
#define KSPEECHADAPTOR_P_H

#include <QtCore/QObject>
#include <dbus/qdbus.h>
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;

/*
 * Adaptor class for interface org.kde.KSpeech
 */
class KSpeechAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.KSpeech")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"org.kde.KSpeech\" >\n"
"    <method name=\"supportsMarkup\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <arg direction=\"in\" type=\"u\" name=\"markupType\" />\n"
"      <arg direction=\"out\" type=\"b\" />\n"
"    </method>\n"
"    <method name=\"supportsMarkers\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <arg direction=\"out\" type=\"b\" />\n"
"    </method>\n"
"    <method name=\"sayScreenReaderOutput\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"msg\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"sayWarning\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"warning\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"sayMessage\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"message\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"setSentenceDelimiter\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"delimiter\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"setText\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"text\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <arg direction=\"out\" type=\"u\" />\n"
"    </method>\n"
"    <method name=\"sayText\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"text\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <arg direction=\"out\" type=\"u\" />\n"
"    </method>\n"
"    <method name=\"appendText\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"text\" />\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"setFile\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"filename\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <arg direction=\"in\" type=\"s\" name=\"encoding\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"getTextCount\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"getCurrentTextJob\" >\n"
"      <arg direction=\"out\" type=\"u\" />\n"
"    </method>\n"
"    <method name=\"getTextJobCount\" >\n"
"      <arg direction=\"out\" type=\"u\" />\n"
"    </method>\n"
"    <method name=\"getTextJobNumbers\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"getTextJobState\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"getTextJobInfo\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <arg direction=\"out\" type=\"ay\" />\n"
"    </method>\n"
"    <method name=\"talkerCodeToTalkerId\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"talkerCode\" />\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"getTextJobSentence\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <arg direction=\"in\" type=\"u\" name=\"seq\" />\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"isSpeakingText\" >\n"
"      <arg direction=\"out\" type=\"b\" />\n"
"    </method>\n"
"    <method name=\"removeText\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"startText\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"stopText\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"pauseText\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"resumeText\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"getTalkers\" >\n"
"      <arg direction=\"out\" type=\"as\" />\n"
"    </method>\n"
"    <method name=\"changeTextTalker\" >\n"
"      <arg direction=\"in\" type=\"s\" name=\"talker\" />\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"userDefaultTalker\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <method name=\"moveTextLater\" >\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"jumpToTextPart\" >\n"
"      <arg direction=\"in\" type=\"i\" name=\"partNum\" />\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <arg direction=\"out\" type=\"i\" />\n"
"    </method>\n"
"    <method name=\"moveRelTextSentence\" >\n"
"      <arg direction=\"in\" type=\"i\" name=\"n\" />\n"
"      <arg direction=\"in\" type=\"u\" name=\"jobNum\" />\n"
"      <arg direction=\"out\" type=\"u\" />\n"
"    </method>\n"
"    <method name=\"speakClipboard\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"showDialog\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"kttsdExit\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"reinit\" >\n"
"      <annotation value=\"true\" name=\"org.freedesktop.DBus.Method.NoReply\" />\n"
"    </method>\n"
"    <method name=\"version\" >\n"
"      <arg direction=\"out\" type=\"s\" />\n"
"    </method>\n"
"    <signal name=\"kttsdStarted\" />\n"
"    <signal name=\"kttsdExiting\" />\n"
"    <signal name=\"markerSeen\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"s\" name=\"markerName\" />\n"
"    </signal>\n"
"    <signal name=\"sentenceStarted\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"      <arg type=\"u\" name=\"seq\" />\n"
"    </signal>\n"
"    <signal name=\"sentenceFinished\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"      <arg type=\"u\" name=\"seq\" />\n"
"    </signal>\n"
"    <signal name=\"textSet\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"    <signal name=\"textAppended\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"      <arg type=\"i\" name=\"partNum\" />\n"
"    </signal>\n"
"    <signal name=\"textStarted\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"    <signal name=\"textFinished\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"    <signal name=\"textStopped\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"    <signal name=\"textPaused\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"    <signal name=\"textResumed\" >\n"
"      <arg type=\"s\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"    <signal name=\"textRemoved\" >\n"
"      <arg type=\"g\" name=\"appId\" />\n"
"      <arg type=\"u\" name=\"jobNum\" />\n"
"    </signal>\n"
"  </interface>\n"
        "")
public:
    KSpeechAdaptor(QObject *parent);
    virtual ~KSpeechAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    int appendText(const QString &text, uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void changeTextTalker(const QString &talker, uint jobNum, const QDBusMessage &msg);
    uint getCurrentTextJob();
    QStringList getTalkers();
    int getTextCount(uint jobNum, const QDBusMessage &msg);
    uint getTextJobCount();
    QByteArray getTextJobInfo(uint jobNum, const QDBusMessage &msg);
    QString getTextJobNumbers();
    QString getTextJobSentence(uint jobNum, uint seq, const QDBusMessage &msg);
    int getTextJobState(uint jobNum, const QDBusMessage &msg);
    bool isSpeakingText();
    int jumpToTextPart(int partNum, uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void kttsdExit();
    uint moveRelTextSentence(int n, uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void moveTextLater(uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void pauseText(uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void reinit();
    Q_ASYNC void removeText(uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void resumeText(uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void sayMessage(const QString &message, const QString &talker, const QDBusMessage &msg);
    Q_ASYNC void sayScreenReaderOutput(const QString &message, const QString &talker, const QDBusMessage &msg);
    uint sayText(const QString &text, const QString &talker, const QDBusMessage &msg);
    Q_ASYNC void sayWarning(const QString &warning, const QString &talker, const QDBusMessage &msg);
    uint setFile(const QString &filename, const QString &talker, const QString &encoding, const QDBusMessage &msg);
    Q_ASYNC void setSentenceDelimiter(const QString &delimiter, const QDBusMessage &msg);
    uint setText(const QString &text, const QString &talker, const QDBusMessage &msg);
    Q_ASYNC void showDialog();
    Q_ASYNC void speakClipboard();
    Q_ASYNC void startText(uint jobNum, const QDBusMessage &msg);
    Q_ASYNC void stopText(uint jobNum, const QDBusMessage &msg);
    bool supportsMarkers(const QString &talker);
    bool supportsMarkup(const QString &talker, uint markupType);
    QString talkerCodeToTalkerId(const QString &talkerCode);
    QString userDefaultTalker();
    QString version();
Q_SIGNALS: // SIGNALS
    void kttsdExiting();
    void kttsdStarted();
    void markerSeen(const QString &appId, const QString &markerName);
    void sentenceFinished(const QString &appId, uint jobNum, uint seq);
    void sentenceStarted(const QString &appId, uint jobNum, uint seq);
    void textAppended(const QString &appId, uint jobNum, int partNum);
    void textFinished(const QString &appId, uint jobNum);
    void textPaused(const QString &appId, uint jobNum);
    void textRemoved(const QString &appId, uint jobNum);
    void textResumed(const QString &appId, uint jobNum);
    void textSet(const QString &appId, uint jobNum);
    void textStarted(const QString &appId, uint jobNum);
    void textStopped(const QString &appId, uint jobNum);
};

#endif
