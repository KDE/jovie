/***************************************************** vim:set ts=4 sw=4 sts=4:
  ALSA player.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  -------------------
  Original author: Gary Cramblitt <garycramblitt@comcast.net>

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

#ifndef ALSAPLAYER_H
#define ALSAPLAYER_H

// System includes.
#include <alsa/asoundlib.h>

// Qt includes.
#include <qstring.h>
#include <qobject.h>
#include <qthread.h>
#include <qfile.h>
#include <qmutex.h>

// KDE includes.
#include <config.h>
#include <kdemacros.h>
#include "kdeexportfix.h"
#include <kurl.h>

// AlsaPlayer includes.
#include "formats.h"
#include "player.h"

#ifndef LLONG_MAX
#define LLONG_MAX    9223372036854775807LL
#endif

#define DEFAULT_FORMAT		SND_PCM_FORMAT_U8
#define DEFAULT_SPEED 		8000

#define FORMAT_DEFAULT		-1
#define FORMAT_RAW		0
#define FORMAT_VOC		1
#define FORMAT_WAVE		2
#define FORMAT_AU		3

static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);

class KDE_EXPORT AlsaPlayer : public Player, QThread
{
    Q_OBJECT

public:
    AlsaPlayer(QObject* parent = 0, const char* name = 0, const QStringList& args=QStringList());
    ~AlsaPlayer();

    virtual void startPlay(const QString& file);
    virtual void pause();
    virtual void stop();

    virtual void setVolume(float volume = 1.0);
    virtual float volume() const;

    virtual bool playing() const;
    virtual bool paused() const;

    virtual int totalTime() const;
    virtual int currentTime() const;
    virtual int position() const; // in this case not really the percent

    virtual void seek(int seekTime);
    virtual void seekPosition(int position);

    virtual QStringList getPluginList( const QCString& classname );
    virtual void setSinkName(const QString &sinkName);
    virtual bool requireVersion(uint major, uint minor, uint micro);

protected:
    virtual void run();

private slots:

private:
    void init();
    void cleanup();
    void stopAndExit();

    QString timestamp() const;

    ssize_t safe_read(int fd, void *buf, size_t count);
    int test_vocfile(void *buffer);
    size_t test_wavefile_read(int fd, char *buffer, size_t *size, size_t reqsize, int line);
    ssize_t test_wavefile(int fd, char *_buffer, size_t size);
    int test_au(int fd, char *buffer);
    void set_params(void);
    void xrun(void);
    void suspend(void);
    void compute_max_peak(char *data, size_t count);
    ssize_t pcm_write(char *data, size_t count);
    ssize_t voc_pcm_write(u_char *data, size_t count);
    void voc_write_silence(unsigned x);
    void voc_pcm_flush(void);
    void voc_play(int fd, int ofs, const char *name);
    void init_raw_data(void);
    off64_t calc_count(void);
    void header(int rtype, const char *name);
    void playback_go(int fd, size_t loaded, off64_t count, int rtype, const char *name);
    void playback(int fd);

    KURL m_currentURL;
    float m_currentVolume;
    QString m_pcmName;
    char* pcm_name;
    mutable QMutex m_mutex;

    QFile audiofile;
    QString name;
    QString dbgStr;
    bool canPause;

    snd_pcm_t *handle;
    struct {
        snd_pcm_format_t format;
        unsigned int channels;
        unsigned int rate;
    } hwparams, rhwparams;
    int timelimit;
    int quiet_mode;
    int file_type;
    unsigned int sleep_min;
    int open_mode;
    snd_pcm_stream_t stream;
    int mmap_flag;
    int interleaved;
    int nonblock;
    QByteArray audioBuffer;
    char *audiobuf;
    snd_pcm_uframes_t chunk_size;
    unsigned period_time;
    unsigned buffer_time;
    snd_pcm_uframes_t period_frames;
    snd_pcm_uframes_t buffer_frames;
    int avail_min;
    int start_delay;
    int stop_delay;
    int verbose;
    int buffer_pos;
    size_t bits_per_sample;
    size_t bits_per_frame;
    size_t chunk_bytes;
    snd_output_t *log;
    int fd;
    off64_t pbrec_count;
    off64_t fdcount;
    int vocmajor;
    int vocminor;

};

#endif              // ALSAPLAYER_H

// vim: sw=4 ts=8 et
