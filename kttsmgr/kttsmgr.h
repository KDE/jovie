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

#include <ksystemtray.h>
#include "kspeech_stub.h"

class KttsMgrTray: public KSystemTray, public KSpeech_stub
{
   Q_OBJECT

   public:
       KttsMgrTray(QWidget *parent=0);
       ~KttsMgrTray();
       
   private slots:
   
       void speakClipboardSelected();
       void aboutSelected();
       void helpSelected();
       void quitSelected();
};

#endif    // KTTSMGR_H
