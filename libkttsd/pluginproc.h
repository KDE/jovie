/***************************************************** vim:set ts=4 sw=4 sts=4:
  pluginproc.h
  This file is the template for the processing plug ins.
  -------------------
  Copyright : (C) 2002-2003 by José Pablo Ezequiel "Pupeno" Fernández
  -------------------
  Original author: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
  Current Maintainer: José Pablo Ezequiel "Pupeno" Fernández <pupeno@kde.org>
 ******************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 ***************************************************************************/

// $Id$
 
#ifndef _PLUGINPROC_H_
#define _PLUGINPROC_H_

#include <qobject.h>
#include <qstring.h>

#include <kconfig.h>

/**
 * Types of text to be speech.
 * Warning: Short and urgent text. It interrupts paragraphs (between sentences).
 * Message: Short text text. It interrupts texts (between paragraphs).
 * Text: Long text. It is parced and navegable.
 */
enum SpeechType{
    Warning,
    Message,
    Text
};

/**
 * This is the template class for all the (processing) plug ins
 * like Festival, FreeTTS or Command
 */
class PlugInProc : public QObject{
    Q_OBJECT

    public:
        /**
         * Constructor
         */
        PlugInProc( QObject *parent = 0, const char *name = 0);

        /**
         * Destructor
         */
        virtual ~PlugInProc();

        /**
         * Initializate the speech
         */
        virtual bool init(const QString &lang, KConfig *config);

        /** 
         * Say a text
         * text: The text to be speech
         */
        virtual void sayText(const QString &text);

        /**
         * Stop text
         * This function only makes sense in asynchronus modes
         */
        virtual void stopText();
};

#endif // _PLUGINPROC_H_
