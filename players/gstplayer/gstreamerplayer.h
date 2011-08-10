/***************************************************************************
    copyright            : (C) 2004 Scott Wheeler
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

#ifndef GSTREAMERPLAYER_H
#define GSTREAMERPLAYER_H

#include "config.h"

#include <glib.h>
extern "C" {
#include <gst/gstversion.h>
}
#include <gst/gst.h>
#include <tqstring.h>
#include "player.h"

class GStreamerPlayer : public Player
{
    Q_OBJECT
  TQ_OBJECT

public:
    GStreamerPlayer(TQObject* parent = 0, const char* name = 0, const TQStringList& args=TQStringList());
    virtual ~GStreamerPlayer();

    // virtual void play(const FileHandle &file = FileHandle::null());
    virtual void startPlay(const TQString& file);

    virtual void setVolume(float volume = 1.0);
    virtual float volume() const;

    virtual bool playing() const;
    virtual bool paused() const;

    virtual int totalTime() const;
    virtual int currentTime() const;
    virtual int position() const; // in this case not really the percent

    virtual void seek(int seekTime);
    virtual void seekPosition(int position);

    virtual TQStringList getPluginList( const TQCString& classname );
    virtual void setSinkName(const TQString &sinkName);

    virtual bool requireVersion(uint major, uint minor, uint micro);

    void pause();
    void stop();

private:
    void readConfig();
    void setupPipeline();
    long long time(GstQueryType type) const;

    TQString m_sinkName;
    // True once gst_init() has been called.
    bool m_initialized;

    GstElement *m_pipeline;
    GstElement *m_source;
    GstElement *m_decoder;
    GstElement *m_volume;
    GstElement *m_sink;
};

#endif
