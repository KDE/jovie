/***************************************************************************
    copyright            : (C) 2004 Scott Wheeler
    email                : wheeler@kde.org

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

#ifndef GSTREAMERPLAYER_H
#define GSTREAMERPLAYER_H

#include "config.h"

#include <glib.h>
#include <QByteArray>

extern "C" {
#include <gst/gstversion.h>
}
#include <gst/gst.h>
#include <qstring.h>
#include "player.h"

class GStreamerPlayer : public Player
{
    Q_OBJECT

public:
    GStreamerPlayer(QObject* parent = 0, const char* name = 0, const QStringList& args=QStringList());
    virtual ~GStreamerPlayer();

    // virtual void play(const FileHandle &file = FileHandle::null());
    virtual void startPlay(const QString& file);

    virtual void setVolume(float volume = 1.0);
    virtual float volume() const;

    virtual bool playing() const;
    virtual bool paused() const;

    virtual int totalTime() const;
    virtual int currentTime() const;
    virtual int position() const; // in this case not really the percent

    virtual void seek(int seekTime);
    virtual void seekPosition(int position);

    virtual QStringList getPluginList( const QByteArray& classname );
    virtual void setSinkName(const QString &sinkName);

    virtual bool requireVersion(uint major, uint minor, uint micro);

    void pause();
    void stop();

private:
    void readConfig();
    void setupPipeline();
    long long time(GstQueryType type) const;

    QString m_sinkName;
    // True once gst_init() has been called.
    bool m_initialized;

    GstElement *m_pipeline;
    GstElement *m_source;
    GstElement *m_decoder;
    GstElement *m_volume;
    GstElement *m_sink;
};

#endif
