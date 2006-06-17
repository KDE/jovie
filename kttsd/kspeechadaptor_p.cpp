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

#include <kdebug.h>

#include "kspeechadaptor_p.h"
#include <QtCore/QMetaObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

/*
 * Implementation of adaptor class KSpeechAdaptor
 */

KSpeechAdaptor::KSpeechAdaptor(QObject *parent)
   : QDBusAbstractAdaptor(parent)
{
    // constructor
    setAutoRelaySignals(true);
}

KSpeechAdaptor::~KSpeechAdaptor()
{
    // destructor
}

int KSpeechAdaptor::appendText(const QString &text, uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.appendText
    QString appId = msg.sender();
    int out0;
    QMetaObject::invokeMethod(parent(), "appendText", Q_RETURN_ARG(int, out0), Q_ARG(QString, text), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->appendText(text, jobNum);
    return out0;
}

void KSpeechAdaptor::changeTextTalker(const QString &talker, uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.changeTextTalker
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "changeTextTalker", Q_ARG(QString, talker), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->changeTextTalker(talker, jobNum);
}

uint KSpeechAdaptor::getCurrentTextJob()
{
    // handle method call org.kde.KSpeech.getCurrentTextJob
    uint out0;
    QMetaObject::invokeMethod(parent(), "getCurrentTextJob", Q_RETURN_ARG(uint, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getCurrentTextJob();
    return out0;
}

QStringList KSpeechAdaptor::getTalkers()
{
    // handle method call org.kde.KSpeech.getTalkers
    QStringList out0;
    QMetaObject::invokeMethod(parent(), "getTalkers", Q_RETURN_ARG(QStringList, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTalkers();
    return out0;
}

int KSpeechAdaptor::getTextCount(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.getTextCount
    QString appId = msg.sender();
    int out0;
    QMetaObject::invokeMethod(parent(), "getTextCount", Q_RETURN_ARG(int, out0), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTextCount(jobNum);
    return out0;
}

uint KSpeechAdaptor::getTextJobCount()
{
    // handle method call org.kde.KSpeech.getTextJobCount
    uint out0;
    QMetaObject::invokeMethod(parent(), "getTextJobCount", Q_RETURN_ARG(uint, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTextJobCount();
    return out0;
}

QByteArray KSpeechAdaptor::getTextJobInfo(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.getTextJobInfo
    QString appId = msg.sender();
    QByteArray out0;
    QMetaObject::invokeMethod(parent(), "getTextJobInfo", Q_RETURN_ARG(QByteArray, out0), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTextJobInfo(jobNum);
    return out0;
}

QString KSpeechAdaptor::getTextJobNumbers()
{
    // handle method call org.kde.KSpeech.getTextJobNumbers
    QString out0;
    QMetaObject::invokeMethod(parent(), "getTextJobNumbers", Q_RETURN_ARG(QString, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTextJobNumbers();
    return out0;
}

QString KSpeechAdaptor::getTextJobSentence(uint jobNum, uint seq, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.getTextJobSentence
    QString appId = msg.sender();
    QString out0;
    QMetaObject::invokeMethod(parent(), "getTextJobSentence", Q_RETURN_ARG(QString, out0), Q_ARG(uint, jobNum), Q_ARG(uint, seq), Q_ARG(QString, appId));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTextJobSentence(jobNum, seq);
    return out0;
}

int KSpeechAdaptor::getTextJobState(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.getTextJobState
    QString appId = msg.sender();
    int out0;
    QMetaObject::invokeMethod(parent(), "getTextJobState", Q_RETURN_ARG(int, out0), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->getTextJobState(jobNum);
    return out0;
}

bool KSpeechAdaptor::isSpeakingText()
{
    // handle method call org.kde.KSpeech.isSpeakingText
    bool out0;
    QMetaObject::invokeMethod(parent(), "isSpeakingText", Q_RETURN_ARG(bool, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->isSpeakingText();
    return out0;
}

int KSpeechAdaptor::jumpToTextPart(int partNum, uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.jumpToTextPart
    QString appId = msg.sender();
    int out0;
    QMetaObject::invokeMethod(parent(), "jumpToTextPart", Q_RETURN_ARG(int, out0), Q_ARG(int, partNum), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->jumpToTextPart(partNum, jobNum);
    return out0;
}

void KSpeechAdaptor::kttsdExit()
{
    // handle method call org.kde.KSpeech.kttsdExit
    QMetaObject::invokeMethod(parent(), "kttsdExit");

    // Alternative:
    //static_cast<YourObjectType *>(parent())->kttsdExit();
}

uint KSpeechAdaptor::moveRelTextSentence(int n, uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.moveRelTextSentence
    QString appId = msg.sender();
    uint out0;
    QMetaObject::invokeMethod(parent(), "moveRelTextSentence", Q_RETURN_ARG(uint, out0), Q_ARG(int, n), Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->moveRelTextSentence(n, jobNum);
    return out0;
}

void KSpeechAdaptor::moveTextLater(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.moveTextLater
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "moveTextLater", Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->moveTextLater(jobNum);
}

void KSpeechAdaptor::pauseText(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.pauseText
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "pauseText", Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->pauseText(jobNum);
}

void KSpeechAdaptor::reinit()
{
    // handle method call org.kde.KSpeech.reinit
    QMetaObject::invokeMethod(parent(), "reinit");

    // Alternative:
    //static_cast<YourObjectType *>(parent())->reinit();
}

void KSpeechAdaptor::removeText(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.removeText
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "removeText", Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->removeText(jobNum);
}

void KSpeechAdaptor::resumeText(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.resumeText
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "resumeText", Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->resumeText(jobNum);
}

void KSpeechAdaptor::sayMessage(const QString &message, const QString &talker, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.sayMessage
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "sayMessage", Q_ARG(QString, message), Q_ARG(QString, talker), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->sayMessage(message, talker);
}

void KSpeechAdaptor::sayScreenReaderOutput(const QString &message, const QString &talker, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.sayScreenReaderOutput
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "sayScreenReaderOutput", Q_ARG(QString, message), Q_ARG(QString, talker), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->sayScreenReaderOutput(msg, talker);
}

uint KSpeechAdaptor::sayText(const QString &text, const QString &talker, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.sayText
    QString appId = msg.sender();
    uint out0;
    QMetaObject::invokeMethod(parent(), "sayText", Q_RETURN_ARG(uint, out0), Q_ARG(QString, text), Q_ARG(QString, talker), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->sayText(text, talker);
    return out0;
}

void KSpeechAdaptor::sayWarning(const QString &warning, const QString &talker, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.sayWarning
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "sayWarning", Q_ARG(QString, warning), Q_ARG(QString, talker), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->sayWarning(warning, talker);
}

uint KSpeechAdaptor::setFile(const QString &filename, const QString &talker, const QString &encoding, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.setFile
    QString appId = msg.sender();
    uint out0;
    QMetaObject::invokeMethod(parent(), "setFile", Q_RETURN_ARG(uint, out0), Q_ARG(QString, filename), Q_ARG(QString, talker), Q_ARG(QString, encoding), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setFile(filename, talker, encoding);
    return out0;
}

void KSpeechAdaptor::setSentenceDelimiter(const QString &delimiter, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.setSentenceDelimiter
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "setSentenceDelimiter", Q_ARG(QString, delimiter), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setSentenceDelimiter(delimiter);
}

uint KSpeechAdaptor::setText(const QString &text, const QString &talker, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.setText
    QString appId = msg.sender();
    uint out0;
    QMetaObject::invokeMethod(parent(), "setText", Q_RETURN_ARG(uint, out0), Q_ARG(QString, text), Q_ARG(QString, talker), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->setText(text, talker);
    return out0;
}

void KSpeechAdaptor::showDialog()
{
    // handle method call org.kde.KSpeech.showDialog
    QMetaObject::invokeMethod(parent(), "showDialog");

    // Alternative:
    //static_cast<YourObjectType *>(parent())->showDialog();
}

void KSpeechAdaptor::speakClipboard()
{
    // handle method call org.kde.KSpeech.speakClipboard
    QMetaObject::invokeMethod(parent(), "speakClipboard");

    // Alternative:
    //static_cast<YourObjectType *>(parent())->speakClipboard();
}

void KSpeechAdaptor::startText(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.startText
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "startText", Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->startText(jobNum);
}

void KSpeechAdaptor::stopText(uint jobNum, const QDBusMessage &msg)
{
    // handle method call org.kde.KSpeech.stopText
    QString appId = msg.sender();
    QMetaObject::invokeMethod(parent(), "stopText", Q_ARG(uint, jobNum), Q_ARG(QString, appId));

    // Alternative:
    //static_cast<YourObjectType *>(parent())->stopText(jobNum);
}

bool KSpeechAdaptor::supportsMarkers(const QString &talker)
{
    // handle method call org.kde.KSpeech.supportsMarkers
    bool out0;
    QMetaObject::invokeMethod(parent(), "supportsMarkers", Q_RETURN_ARG(bool, out0), Q_ARG(QString, talker));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->supportsMarkers(talker);
    return out0;
}

bool KSpeechAdaptor::supportsMarkup(const QString &talker, uint markupType)
{
    // handle method call org.kde.KSpeech.supportsMarkup
    bool out0;
    QMetaObject::invokeMethod(parent(), "supportsMarkup", Q_RETURN_ARG(bool, out0), Q_ARG(QString, talker), Q_ARG(uint, markupType));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->supportsMarkup(talker, markupType);
    return out0;
}

QString KSpeechAdaptor::talkerCodeToTalkerId(const QString &talkerCode)
{
    // handle method call org.kde.KSpeech.talkerCodeToTalkerId
    QString out0;
    QMetaObject::invokeMethod(parent(), "talkerCodeToTalkerId", Q_RETURN_ARG(QString, out0), Q_ARG(QString, talkerCode));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->talkerCodeToTalkerId(talkerCode);
    return out0;
}

QString KSpeechAdaptor::userDefaultTalker()
{
    // handle method call org.kde.KSpeech.userDefaultTalker
    QString out0;
    QMetaObject::invokeMethod(parent(), "userDefaultTalker", Q_RETURN_ARG(QString, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->userDefaultTalker();
    return out0;
}

QString KSpeechAdaptor::version()
{
    // handle method call org.kde.KSpeech.version
    QString out0;
    QMetaObject::invokeMethod(parent(), "version", Q_RETURN_ARG(QString, out0));

    // Alternative:
    //out0 = static_cast<YourObjectType *>(parent())->version();
    return out0;
}


#include "kspeechadaptor_p.moc"
