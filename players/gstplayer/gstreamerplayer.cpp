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

// Qt includes.
#include <qfile.h>

// KDE includes.
#include <kapplication.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>

// GStreamerPlayer includes.
#include "gstreamerplayer.h"
#include "gstreamerplayer.moc"

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

GStreamerPlayer::GStreamerPlayer(QObject* parent, const char* name, const QStringList& args) :
    Player(parent, name, args),
    m_initialized(false),
    m_pipeline(0),
    m_source(0),
    m_decoder(0),
    m_volume(0),
    m_sink(0)
{
    // readConfig();
    setupPipeline();
}

GStreamerPlayer::~GStreamerPlayer()
{
    stop();
    gst_object_unref(GST_OBJECT(m_pipeline));
}

//void GStreamerPlayer::play(const FileHandle &file)
void GStreamerPlayer::startPlay(const QString &file)
{
    if(!file.isNull()) {
        stop();
        // g_object_set(G_OBJECT(m_source), "location", file.absoluteFilePath().toLocal8Bit().data(), 0);
        g_object_set(G_OBJECT(m_source), "location", file.toLocal8Bit().data(), 0);
    }

    gst_element_set_state(m_pipeline, GST_STATE_PLAYING);
}

void GStreamerPlayer::pause()
{
    gst_element_set_state(m_pipeline, GST_STATE_PAUSED);
}

void GStreamerPlayer::stop()
{
    gst_element_set_state(m_pipeline, GST_STATE_NULL);
}

void GStreamerPlayer::setVolume(float volume)
{
    g_object_set(G_OBJECT(m_volume), "volume", volume, 0);
}

float GStreamerPlayer::volume() const
{
    gfloat value;
    g_object_get(G_OBJECT(m_volume), "volume", &value, 0);
    return value;
}

bool GStreamerPlayer::playing() const
{
    return gst_element_get_state(m_pipeline) == GST_STATE_PLAYING;
}

bool GStreamerPlayer::paused() const
{
    return gst_element_get_state(m_pipeline) == GST_STATE_PAUSED;
}

int GStreamerPlayer::totalTime() const
{
    return time(GST_QUERY_TOTAL) / GST_SECOND;
}

int GStreamerPlayer::currentTime() const
{
    return time(GST_QUERY_POSITION) / GST_SECOND;
}

int GStreamerPlayer::position() const
{
    long long total   = time(GST_QUERY_TOTAL);
    long long current = time(GST_QUERY_POSITION);
    return total > 0 ? int((double(current) / double(total)) * double(1000) + 0.5) : 0;
}

void GStreamerPlayer::seek(int seekTime)
{
    int type = (GST_FORMAT_TIME | GST_SEEK_METHOD_SET | GST_SEEK_FLAG_FLUSH);
    gst_element_seek(m_sink, GstSeekType(type), seekTime * GST_SECOND);
}

void GStreamerPlayer::seekPosition(int position)
{
    long long total = time(GST_QUERY_TOTAL);
    if(total > 0)
        seek(int(double(position) / double(1000) * double(totalTime()) + 0.5));
}

/**
 * Returns a list of GStreamer plugins of the specified class.
 * @param classname             Desired class.  Use "Sink/Audio" for sinks.
 * @return                      List of plugin names.
 */
QStringList GStreamerPlayer::getPluginList( const QByteArray& classname )
{
    GList * pool_registries = NULL;
    GList* registries = NULL;
    GList* plugins = NULL;
    GList* features = NULL;
    QString name;
    QStringList results;

    if(!m_initialized) {
        int argc = kapp->argc();
        char **argv = kapp->argv();
        gst_init(&argc, &argv);
        m_initialized = true;
    }

    pool_registries = gst_registry_pool_list ();
    registries = pool_registries;

    while ( registries ) {
        GstRegistry * registry = GST_REGISTRY ( registries->data );
        plugins = registry->plugins;

        while ( plugins ) {
            GstPlugin * plugin = GST_PLUGIN ( plugins->data );
            features = gst_plugin_get_feature_list ( plugin );

            while ( features ) {
                GstPluginFeature * feature = GST_PLUGIN_FEATURE ( features->data );

                if ( GST_IS_ELEMENT_FACTORY ( feature ) ) {
                    GstElementFactory * factory = GST_ELEMENT_FACTORY ( feature );

                    if ( g_strrstr ( factory->details.klass, classname ) ) {
                        name = g_strdup ( GST_OBJECT_NAME ( factory ) );
                        if ( name != "artsdsink" ) results << name;
                    }
                }
                features = g_list_next ( features );
            }
            plugins = g_list_next ( plugins );
        }
        registries = g_list_next ( registries );
    }
    g_list_free ( pool_registries );
    pool_registries = NULL;

    return results;
}

bool GStreamerPlayer::requireVersion(uint major, uint minor, uint micro)
{
    guint gmajor, gminor, gmicro;

    if(!m_initialized) {
        int argc = kapp->argc();
        char **argv = kapp->argv();
        gst_init(&argc, &argv);
        m_initialized = true;
    }

    gst_version(&gmajor, &gminor, &gmicro);
    // kdDebug() << QString("GStreamerPlayer::requireVersion: You have gstreamer %1.%2.%3 installed.").arg(gmajor).arg(gminor).arg(gmicro) << endl;
    if (gmajor > major) return true;
    if (gminor > minor) return true;
    if (gmicro >= micro) return true;
    kdDebug() << QString("GStreamerPlayer::requireVersion: You have gstreamer %1.%2.%3 installed.").arg(gmajor).arg(gminor).arg(gmicro) << endl;
    kdDebug() << QString("GStreamerPlayer::requireVersion: This application requires %1.%2.%3 or greater.").arg(major).arg(minor).arg(micro) << endl;
    return false;
}

void GStreamerPlayer::setSinkName(const QString &sinkName) { m_sinkName = sinkName; }

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void GStreamerPlayer::readConfig()
{
    KConfigGroup config(KGlobal::config(), "GStreamerPlayer");
    m_sinkName = config.readEntry("SinkName", QString::null);
}

void GStreamerPlayer::setupPipeline()
{
    if(!m_initialized) {
        int argc = kapp->argc();
        char **argv = kapp->argv();
        gst_init(&argc, &argv);
        m_initialized = true;
    }

    m_pipeline = gst_thread_new("pipeline");
    m_source   = gst_element_factory_make("filesrc", "source");
    m_decoder  = gst_element_factory_make("spider", "decoder");
    m_volume   = gst_element_factory_make("volume", "volume");

    if(!m_sinkName.isNull())
        m_sink = gst_element_factory_make(m_sinkName.toUtf8().data(), "sink");
    if (!m_sink)
    {
        // m_sink = gst_element_factory_make("alsasink", "sink");
        // if(!m_sink)
        //    m_sink = gst_element_factory_make("osssink", "sink");

        // Reversing order.  OSS seems to work.  Alsa sink produces ugly echo of last
        // couple of words in each wav file.  argh!
        // kdDebug() << "GStreamerPlayer::setupPipeline: trying oss sink." << endl;
        m_sink = gst_element_factory_make("osssink", "sink");
        if(!m_sink)
        {
            // kdDebug() << "GStreamerPlayer::setupPipeline: reverting to alsa sink." << endl;
            m_sink = gst_element_factory_make("alsasink", "sink");
        }
    }

    gst_bin_add_many(GST_BIN(m_pipeline), m_source, m_decoder, m_volume, m_sink, 0);
    gst_element_link_many(m_source, m_decoder, m_volume, m_sink, 0);
}

long long GStreamerPlayer::time(GstQueryType type) const
{
    gint64 ns = 0;
    GstFormat format = GST_FORMAT_TIME;
    gst_element_query(m_sink, type, &format, &ns);
    return ns;
}

// vim: set et sw=4:
