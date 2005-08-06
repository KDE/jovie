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
#include <qtooltip.h>
//Added by qt3to4:
#include <Q3CString>

// KDE includes.
#include <ksystemtray.h>

// KTTS includes.
#include "kspeech_stub.h"
#include "kspeechsink.h"

// class KttsToolTip: public QToolTip
// {
//     public:
//         KttsToolTip ( QWidget* parent );
// 
//     protected:
//         virtual void maybeTip ( const QPoint & p );
// };

class KttsMgrTray: public KSystemTray, public KSpeech_stub, virtual public KSpeechSink
{
    Q_OBJECT

    public:
        KttsMgrTray(QWidget *parent=0);
        ~KttsMgrTray();

        void setExitWhenFinishedSpeaking();
        QString getStatus();

    protected:
        // ASYNC textStarted(const QCString& appId, uint jobNum);
        ASYNC textFinished(const Q3CString& appId, uint jobNum);
        virtual bool eventFilter( QObject* o, QEvent* e );

    private slots:

        void speakClipboardSelected();
        void holdSelected();
        void resumeSelected();
        void aboutSelected();
        void helpSelected();
        void quitSelected();

    private:
        /**
         * Convert a KTTSD job state integer into a display string.
         * @param state          KTTSD job state
         * @return               Display string for the state.
         */
        QString stateToStr(int state);

        bool isKttsdRunning();
        void exitWhenFinishedSpeaking();
        // KttsToolTip* m_toolTip;
};

#endif    // KTTSMGR_H
