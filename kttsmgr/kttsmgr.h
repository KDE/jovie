//
// C++ Interface: kttsmgr
//
// Description: 
//
//
// Author: Gary Cramblitt <garycramblitt@comcast.net>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef KTTSMGR_H
#define KTTSMGR_H

// Qt includes.
#include <qevent.h>

// KDE includes.
#include <ksystemtray.h>

// KTTS includes.
#include "kspeech_stub.h"
#include "kspeechsink.h"

// class KttsMgrTray: public KSystemTray, public KSpeech_stub, virtual public KSpeechSink
 class KttsMgrTray: public KSystemTray, public KSpeech_stub
{
    Q_OBJECT

    public:
        KttsMgrTray(QWidget *parent=0);
        ~KttsMgrTray();

        void setExitWhenFinishedSpeaking();

    protected:
        // ASYNC textStarted(const QCString& appId, uint jobNum);
        ASYNC textFinished(const QCString& appId, uint jobNum);
        virtual bool eventFilter( QObject* o, QEvent* e );

    private slots:

        void speakClipboardSelected();
        void holdSelected();
        void resumeSelected();
        void aboutSelected();
        void helpSelected();
        void quitSelected();

    private:
        bool isKttsdRunning();
        void exitWhenFinishedSpeaking();
};

#endif    // KTTSMGR_H
