/***************************************************************************
    begin                : Sun Feb 17 2002
    copyright            : (C) 2002 - 2004 by Scott Wheeler
    email                : wheeler@kde.org
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

#include <qobject.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <Q3CString>
#include <kdemacros.h>
#include "kdeexportfix.h"

// #include "filehandle.h"

class KDE_EXPORT Player : virtual public QObject
{
    Q_OBJECT

public:
    virtual ~Player() {}

//    virtual void play(const FileHandle &file = FileHandle::null()) = 0;
    virtual void startPlay(const QString& file) = 0;
    virtual void pause() = 0;
    virtual void stop() = 0;

    virtual void setVolume(float volume = 1.0) = 0;
    virtual float volume() const = 0;

    virtual bool playing() const = 0;
    virtual bool paused() const = 0;

    virtual int totalTime() const = 0;
    virtual int currentTime() const = 0;
    virtual int position() const = 0; // in this case not really the percent

    virtual void seek(int seekTime) = 0;
    virtual void seekPosition(int position) = 0;

    virtual QStringList getPluginList( const Q3CString& classname ) = 0;
    virtual void setSinkName(const QString &sinkName) = 0;

    virtual bool requireVersion(uint major, uint minor, uint micro) = 0;

protected:
    Player(QObject* parent = 0, const char* name = 0, const QStringList& args=QStringList() ) :
        QObject(parent, name) { if (args.isEmpty()); } // TODO: Avoid compiler WARNING.  Better way?

};

#endif
