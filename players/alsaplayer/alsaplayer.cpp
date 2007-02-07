/***************************************************** vim:set ts=4 sw=4 sts=4:
  ALSA player.
  -------------------
  Copyright:
  (C) 2005 by Gary Cramblitt <garycramblitt@comcast.net>
  Portions based on aplay.c in alsa-utils
  Copyright (c) by Jaroslav Kysela <perex@suse.cz>
  Based on vplay program by Michael Beck
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

// #include <sys/wait.h>
// System includes.
#include <config-kttsd.h>
#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

// Qt includes.
#include <QDir>
#include <QApplication>
#include <QMutexLocker>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

// AlsaPlayer includes.
#include "alsaplayer.h"

#if !defined(__GNUC__) || __GNUC__ >= 3
#define ERR(...) do {\
    QString dbgStr;\
    QString s = dbgStr.sprintf( "%s:%d: ERROR ", __FUNCTION__, __LINE__); \
    s += dbgStr.sprintf( __VA_ARGS__); \
    kDebug() << timestamp() << "AlsaPlayer::" << s << endl; \
} while (0)
#else
#define ERR(args...) do {\
    QString dbgStr;\
    QString s = dbgStr.sprintf( "%s:%d: ERROR ", __FUNCTION__, __LINE__); \
    s += dbgStr.sprintf( ##args ); \
    kDebug() << timestamp() << "AlsaPlayer::" << s << endl; \
} while (0)
#endif
#if !defined(__GNUC__) || __GNUC__ >= 3
#define MSG(...) do {\
    if (m_debugLevel >= 1) {\
        QString dbgStr; \
        QString s = dbgStr.sprintf( "%s:%d: ", __FUNCTION__, __LINE__); \
        s += dbgStr.sprintf( __VA_ARGS__); \
        kDebug() << timestamp() << "AlsaPlayer::" << s << endl; \
    }; \
} while (0)
#else
#define MSG(args...) do {\
    if (m_debugLevel >= 1) {\
        QString dbgStr; \
        QString s = dbgStr.sprintf( "%s:%d: ", __FUNCTION__, __LINE__); \
        s += dbgStr.sprintf( ##args ); \
        kDebug() << timestamp() << "AlsaPlayer::" << s << endl; \
    }; \
} while (0)
#endif

#if !defined(__GNUC__) || __GNUC__ >= 3
#define DBG(...) do {\
    if (m_debugLevel >= 2) {\
        QString dbgStr; \
        QString s = dbgStr.sprintf( "%s:%d: ", __FUNCTION__, __LINE__); \
        s += dbgStr.sprintf( __VA_ARGS__); \
        kDebug() << timestamp() << "AlsaPlayer::" << s << endl; \
    }; \
} while (0)
#else
#define DBG(args...) do {\
    if (m_debugLevel >= 2) {\
        QString dbgStr; \
        QString s = dbgStr.sprintf( "%s:%d: ", __FUNCTION__, __LINE__); \
        s += dbgStr.sprintf( ##args ); \
        kDebug() << timestamp() << "AlsaPlayer::" << s << endl; \
    }; \
} while (0)
#endif

QString AlsaPlayerThread::timestamp() const
{
    time_t t;
    struct timeval tv;
    char *tstr;
    t = time(NULL);
    tstr = strdup(ctime(&t));
    tstr[strlen(tstr)-1] = 0;
    gettimeofday(&tv,NULL);
    QString ts;
    ts.sprintf(" %s [%d] ",tstr, (int) tv.tv_usec);
    free(tstr);
    return ts;
}

static snd_pcm_sframes_t (*readi_func)(snd_pcm_t *handle, void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writei_func)(snd_pcm_t *handle, const void *buffer, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*readn_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);
static snd_pcm_sframes_t (*writen_func)(snd_pcm_t *handle, void **bufs, snd_pcm_uframes_t size);


////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

AlsaPlayerThread::AlsaPlayerThread(QObject* parent) :
    QThread(parent),
    m_currentVolume(1.0),
    m_pcmName("default"),
    m_defPeriodSize(128),
    m_defPeriods(8),
    m_debugLevel(1),
    m_simulatedPause(false)
{
    init();
}

AlsaPlayerThread::~AlsaPlayerThread()
{
    if (isRunning()) {
        stop();
        wait();
    }
}

//void AlsaPlayerThread::play(const FileHandle &file)
void AlsaPlayerThread::startPlay(const QString &file)
{
    if (isRunning()) {
        if (paused()) {
            if (canPause)
                snd_pcm_pause(handle, false);
            else
                m_simulatedPause = false;
        }
        return;
    }
    audiofile.setFileName(file);
    audiofile.open(QIODevice::ReadOnly);
    fd = audiofile.handle();
    if (audiofile_name) free(audiofile_name);
    audiofile_name = qstrdup(file.toAscii().constData());
    // Start thread running.
    start();
}

/*virtual*/ void AlsaPlayerThread::run()
{
    QString pName = m_pcmName.section(" ", 0, 0);
    pcm_name = qstrdup(pName.toAscii().constData());
    DBG("pName = %s", pcm_name);
    int err;
    snd_pcm_info_t *info;

    m_simulatedPause = false;

    snd_pcm_info_alloca(&info);

    err = snd_output_stdio_attach(&log, stderr, 0);
    assert(err >= 0);

    rhwdata.format = DEFAULT_FORMAT;
    rhwdata.rate = DEFAULT_SPEED;
    rhwdata.channels = 1;

    err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
    if (err < 0) {
        ERR("audio open error on pcm device %s: %s", pcm_name, snd_strerror(err));
        return;
    }

    if ((err = snd_pcm_info(handle, info)) < 0) {
        ERR("info error: %s", snd_strerror(err));
        return;
    }

    chunk_size = 1024;
    hwdata = rhwdata;

    audioBuffer.resize(1024);
    // audiobuf = (char *)malloc(1024);
    audiobuf = audioBuffer.data();
    if (audiobuf == NULL) {
        ERR("not enough memory");
        return;
    }

    if (mmap_flag) {
        writei_func = snd_pcm_mmap_writei;
        readi_func = snd_pcm_mmap_readi;
        writen_func = snd_pcm_mmap_writen;
        readn_func = snd_pcm_mmap_readn;
    } else {
        writei_func = snd_pcm_writei;
        readi_func = snd_pcm_readi;
        writen_func = snd_pcm_writen;
        readn_func = snd_pcm_readn;
    }


    playback(fd);
    cleanup();
    return;
}

void AlsaPlayerThread::pause()
{
    if (isRunning()) {
        DBG("Pause requested");
        QMutexLocker locker(&m_mutex);
        if (handle) {
            // Some hardware can pause; some can't.  canPause is set in set_params.
            if (canPause) {
                m_simulatedPause = false;
                snd_pcm_pause(handle, true);
            } else {
                // Set a flag and cause wait_for_poll to sleep.  When resumed, will get
                // an underrun.
                m_simulatedPause = true;
            }
        }
    }
}

void AlsaPlayerThread::stop()
{
    if (isRunning()) {
        DBG("STOP! Locking mutex");
        QMutexLocker locker(&m_mutex);
        m_simulatedPause = false;
        if (handle) {
            /* This constant is arbitrary */
            char buf = 42;
            DBG("Request for stop, device state is %s",
                snd_pcm_state_name(snd_pcm_state(handle)));
            write(alsa_stop_pipe[1], &buf, 1);
        }
        DBG("unlocking mutex");
        locker.unlock();
        /* Wait for thread to exit */
        DBG("waiting for thread to exit");
        wait();
        DBG("cleaning up");
        // TODO: This seems like a bug.  Why must I relock the locker
        // since I'm about to destroy the locker?
        locker.relock();
    }
    cleanup();
}

/*
 * Stop playback, cleanup and exit thread.
 */
void AlsaPlayerThread::stopAndExit()
{
    // if (handle) snd_pcm_drop(handle);
    cleanup();
    exit();
}

void AlsaPlayerThread::setVolume(float volume)
{
    m_currentVolume = volume;
}

float AlsaPlayerThread::volume() const
{
    return m_currentVolume;
}

/////////////////////////////////////////////////////////////////////////////////
// player status functions
/////////////////////////////////////////////////////////////////////////////////

bool AlsaPlayerThread::playing() const
{
    bool result = false;
    if (isRunning()) {
        QMutexLocker locker(&m_mutex);
        if (handle) {
            if (canPause) {
                snd_pcm_status_t *status;
                snd_pcm_status_alloca(&status);
                int res;
                if ((res = snd_pcm_status(handle, status)) < 0)
                    ERR("status error: %s", snd_strerror(res));
                else {
                    result = (SND_PCM_STATE_RUNNING == snd_pcm_status_get_state(status))
                        || (SND_PCM_STATE_DRAINING == snd_pcm_status_get_state(status));
                    DBG("state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)));
                }
            } else
                result = !m_simulatedPause;
        }
    }
    return result;
}

bool AlsaPlayerThread::paused() const
{
    bool result = false;
    if (isRunning()) {
        QMutexLocker locker(&m_mutex);
        if (handle) {
            if (canPause) {
                snd_pcm_status_t *status;
                snd_pcm_status_alloca(&status);
                int res;
                if ((res = snd_pcm_status(handle, status)) < 0)
                    ERR("status error: %s", snd_strerror(res));
                else {
                    result = (SND_PCM_STATE_PAUSED == snd_pcm_status_get_state(status));
                    DBG("state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)));
                }
            } else
                result = m_simulatedPause;
        }
    }
    return result;
}

int AlsaPlayerThread::totalTime() const
{
    int total = 0;
    int rate = hwdata.rate;
    int channels = hwdata.channels;
    if (rate > 0 && channels > 0) {
        total = int((double(pbrec_count) / rate) / channels);
        // DBG("pbrec_count = %i rate =%i channels = %i", pbrec_count, rate, channels);
        // DBG("totalTime = %i", total);
    }
    return total;
}

int AlsaPlayerThread::currentTime() const
{
    int current = 0;
    int rate = hwdata.rate;
    int channels = hwdata.channels;
    if (rate > 0 && channels > 0) {
        current = int((double(fdcount) / rate) / channels);
        // DBG("fdcount = %i rate = %i channels = %i", fdcount, rate, channels);
        // DBG("currentTime = %i", current);
    }
    return current;
}

int AlsaPlayerThread::position() const
{
    // TODO: Make this more accurate by adding frames that have been so-far
    // played within the Alsa ring buffer.
    return pbrec_count > 0 ? int(double(fdcount) * 1000 / pbrec_count + .5) : 0;
}

/////////////////////////////////////////////////////////////////////////////////
// player seek functions
/////////////////////////////////////////////////////////////////////////////////

void AlsaPlayerThread::seek(int /*seekTime*/)
{
    // TODO:
}

void AlsaPlayerThread::seekPosition(int /*position*/)
{
    // TODO:
}

/*
 * Returns a list of PCM devices.
 * This function fills the specified list with ALSA hardware soundcards found on the system.
 * It uses plughw:xx instead of hw:xx for specifiers, because hw:xx are not practical to
 * use (e.g. they require a resampler/channel mixer in the application).
 */
QStringList AlsaPlayerThread::getPluginList( const QByteArray& classname )
{
    Q_UNUSED(classname);

    int err = 0;
    int card = -1, device = -1;
    snd_ctl_t *handle;
    snd_ctl_card_info_t *info;
    snd_pcm_info_t *pcminfo;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_alloca(&pcminfo);
    QStringList result;

    result.append("default");
    for (;;) {
        err = snd_card_next(&card);
        if (err < 0 || card < 0) break;
        if (card >= 0) {
            char name[32];
            sprintf(name, "hw:%i", card);
            if ((err = snd_ctl_open(&handle, name, 0)) < 0) continue;
            if ((err = snd_ctl_card_info(handle, info)) < 0) {
                snd_ctl_close(handle);
                continue;
            }
            for (int devCnt=0;;++devCnt) {
                err = snd_ctl_pcm_next_device(handle, &device);
                if (err < 0 || device < 0) break;

                snd_pcm_info_set_device(pcminfo, device);
                snd_pcm_info_set_subdevice(pcminfo, 0);
                snd_pcm_info_set_stream(pcminfo, SND_PCM_STREAM_PLAYBACK);
                if ((err = snd_ctl_pcm_info(handle, pcminfo)) < 0) continue;
                QString infoName = " ";
                infoName += snd_ctl_card_info_get_name(info);
                infoName += " (";
                infoName += snd_pcm_info_get_name(pcminfo);
                infoName += ')';
                if (0 == devCnt) {
                    QString pcmName = QString("default:%1").arg(card);
                    result.append(pcmName + infoName);
                }
                QString pcmName = QString("plughw:%1,%2").arg(card).arg(device);
                result.append(pcmName + infoName);
            }
            snd_ctl_close(handle);
        }
    }
    return result;
}

void AlsaPlayerThread::setSinkName(const QString& sinkName) { m_pcmName = sinkName; }

/////////////////////////////////////////////////////////////////////////////////
// private
/////////////////////////////////////////////////////////////////////////////////

void AlsaPlayerThread::init()
{
    pcm_name = 0;
    audiofile_name = 0;
    handle = 0;
    canPause = false;
    timelimit = 0;
    file_type = FORMAT_DEFAULT;
    sleep_min = 0;
    // open_mode = 0;
    open_mode = SND_PCM_NONBLOCK;
    stream = SND_PCM_STREAM_PLAYBACK;
    mmap_flag = 0;
    interleaved = 1;
    audiobuf = NULL;
    chunk_size = 0;
    period_time = 0;
    buffer_time = 0;
    avail_min = -1;
    start_delay = 0;
    stop_delay = 0;
    buffer_pos = 0;
    log = 0;
    fd = -1;
    pbrec_count = LLONG_MAX;
    alsa_stop_pipe[0] = 0;
    alsa_stop_pipe[1] = 0;
    alsa_poll_fds = 0;
    m_simulatedPause = false;
}

void AlsaPlayerThread::cleanup()
{
    DBG("cleaning up");
    QMutexLocker locker(&m_mutex);
    if (pcm_name) free(pcm_name);
    if (audiofile_name) free(audiofile_name);
    if (fd >= 0) audiofile.close();
    if (handle) {
        snd_pcm_drop(handle);
        snd_pcm_close(handle);
    }
    if (alsa_stop_pipe[0]) close(alsa_stop_pipe[0]);
    if (alsa_stop_pipe[1]) close(alsa_stop_pipe[1]);
    if (audiobuf) audioBuffer.resize(0);
    if (alsa_poll_fds) alsa_poll_fds_barray.resize(0);
    if (log) snd_output_close(log);
    snd_config_update_free_global();
    init();
}

/*
 * Safe read (for pipes)
 */
 
ssize_t AlsaPlayerThread::safe_read(int fd, void *buf, size_t count)
{
    ssize_t result = 0;
    ssize_t res;

    while (count > 0) {
        if ((res = read(fd, buf, count)) == 0)
            break;
        if (res < 0)
            return result > 0 ? result : res;
        count -= res;
        result += res;
        buf = (char *)buf + res;
    }
    return result;
}

/*
 * Test, if it is a .VOC file and return >=0 if ok (this is the length of rest)
 *                                       < 0 if not 
 */
int AlsaPlayerThread::test_vocfile(void *buffer)
{
    VocHeader *vp = (VocHeader*)buffer;

    if (!memcmp(vp->magic, VOC_MAGIC_STRING, 20)) {
        vocminor = LE_SHORT(vp->version) & 0xFF;
        vocmajor = LE_SHORT(vp->version) / 256;
        if (LE_SHORT(vp->version) != (0x1233 - LE_SHORT(vp->coded_ver)))
            return -2;    /* coded version mismatch */
        return LE_SHORT(vp->headerlen) - sizeof(VocHeader);    /* 0 mostly */
    }
    return -1;        /* magic string fail */
}

/*
 * helper for test_wavefile
 */

ssize_t AlsaPlayerThread::test_wavefile_read(int fd, char *buffer, size_t *size, size_t reqsize, int line)
{
    if (*size >= reqsize)
        return *size;
    if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
        ERR("read error (called from line %i)", line);
        stopAndExit();
    }
    return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
    if (len > blimit) { \
        blimit = len; \
        if ((buffer = (char*)realloc(buffer, blimit)) == NULL) { \
            ERR("not enough memory"); \
            stopAndExit(); \
        } \
    }

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
ssize_t AlsaPlayerThread::test_wavefile(int fd, char *_buffer, size_t size)
{
    WaveHeader *h = (WaveHeader *)_buffer;
    char *buffer = NULL;
    size_t blimit = 0;
    WaveFmtBody *f;
    WaveChunkHeader *c;
    u_int type;
    u_int len;

    if (size < sizeof(WaveHeader))
        return -1;
    if (h->magic != WAV_RIFF || h->type != WAV_WAVE)
        return -1;
    if (size > sizeof(WaveHeader)) {
        check_wavefile_space(buffer, size - sizeof(WaveHeader), blimit);
        memcpy(buffer, _buffer + sizeof(WaveHeader), size - sizeof(WaveHeader));
    }
    size -= sizeof(WaveHeader);
    while (1) {
        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = LE_INT(c->length);
        len += len % 2;
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_FMT)
            break;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    if (len < sizeof(WaveFmtBody)) {
        ERR("unknown length of 'fmt ' chunk (read %u, should be %u at least)", len, (u_int)sizeof(WaveFmtBody));
        stopAndExit();
    }
    check_wavefile_space(buffer, len, blimit);
    test_wavefile_read(fd, buffer, &size, len, __LINE__);
    f = (WaveFmtBody*) buffer;
    if (LE_SHORT(f->format) != WAV_PCM_CODE) {
        ERR("can't play not PCM-coded WAVE-files");
        stopAndExit();
    }
    if (LE_SHORT(f->modus) < 1) {
        ERR("can't play WAVE-files with %d tracks", LE_SHORT(f->modus));
        stopAndExit();
    }
    hwdata.channels = LE_SHORT(f->modus);
    switch (LE_SHORT(f->bit_p_spl)) {
    case 8:
        if (hwdata.format != DEFAULT_FORMAT &&
            hwdata.format != SND_PCM_FORMAT_U8)
            MSG("Warning: format is changed to U8");
        hwdata.format = SND_PCM_FORMAT_U8;
        break;
    case 16:
        if (hwdata.format != DEFAULT_FORMAT &&
            hwdata.format != SND_PCM_FORMAT_S16_LE)
            MSG("Warning: format is changed to S16_LE");
        hwdata.format = SND_PCM_FORMAT_S16_LE;
        break;
    case 24:
        switch (LE_SHORT(f->byte_p_spl) / hwdata.channels) {
        case 3:
            if (hwdata.format != DEFAULT_FORMAT &&
                hwdata.format != SND_PCM_FORMAT_S24_3LE)
                MSG("Warning: format is changed to S24_3LE");
            hwdata.format = SND_PCM_FORMAT_S24_3LE;
            break;
        case 4:
            if (hwdata.format != DEFAULT_FORMAT &&
                hwdata.format != SND_PCM_FORMAT_S24_LE)
                MSG("Warning: format is changed to S24_LE");
            hwdata.format = SND_PCM_FORMAT_S24_LE;
            break;
        default:
            ERR("can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)", LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), hwdata.channels);
            stopAndExit();
        }
        break;
    case 32:
        hwdata.format = SND_PCM_FORMAT_S32_LE;
        break;
    default:
        ERR("can't play WAVE-files with sample %d bits wide", LE_SHORT(f->bit_p_spl));
        stopAndExit();
    }
    hwdata.rate = LE_INT(f->sample_fq);

    if (size > len)
        memmove(buffer, buffer + len, size - len);
    size -= len;

    while (1) {
        u_int type, len;

        check_wavefile_space(buffer, sizeof(WaveChunkHeader), blimit);
        test_wavefile_read(fd, buffer, &size, sizeof(WaveChunkHeader), __LINE__);
        c = (WaveChunkHeader*)buffer;
        type = c->type;
        len = LE_INT(c->length);
        if (size > sizeof(WaveChunkHeader))
            memmove(buffer, buffer + sizeof(WaveChunkHeader), size - sizeof(WaveChunkHeader));
        size -= sizeof(WaveChunkHeader);
        if (type == WAV_DATA) {
            if (len < pbrec_count && len < 0x7ffffffe)
                pbrec_count = len;
            if (size > 0)
                memcpy(_buffer, buffer, size);
            free(buffer);
            return size;
        }
        len += len % 2;
        check_wavefile_space(buffer, len, blimit);
        test_wavefile_read(fd, buffer, &size, len, __LINE__);
        if (size > len)
            memmove(buffer, buffer + len, size - len);
        size -= len;
    }

    /* shouldn't be reached */
    return -1;
}

/*
 * Test for AU file.
 */

int AlsaPlayerThread::test_au(int fd, char *buffer)
{
    AuHeader *ap = (AuHeader*)buffer;

    if (ap->magic != AU_MAGIC)
        return -1;
    if (BE_INT(ap->hdr_size) > 128 || BE_INT(ap->hdr_size) < 24)
        return -1;
    pbrec_count = BE_INT(ap->data_size);
    switch (BE_INT(ap->encoding)) {
    case AU_FMT_ULAW:
        if (hwdata.format != DEFAULT_FORMAT &&
            hwdata.format != SND_PCM_FORMAT_MU_LAW)
            MSG("Warning: format is changed to MU_LAW");
        hwdata.format = SND_PCM_FORMAT_MU_LAW;
        break;
    case AU_FMT_LIN8:
        if (hwdata.format != DEFAULT_FORMAT &&
            hwdata.format != SND_PCM_FORMAT_U8)
            MSG("Warning: format is changed to U8");
        hwdata.format = SND_PCM_FORMAT_U8;
        break;
    case AU_FMT_LIN16:
        if (hwdata.format != DEFAULT_FORMAT &&
            hwdata.format != SND_PCM_FORMAT_S16_BE)
            MSG("Warning: format is changed to S16_BE");
        hwdata.format = SND_PCM_FORMAT_S16_BE;
        break;
    default:
        return -1;
    }
    hwdata.rate = BE_INT(ap->sample_rate);
    if (hwdata.rate < 2000 || hwdata.rate > 256000)
        return -1;
    hwdata.channels = BE_INT(ap->channels);
    if (hwdata.channels < 1 || hwdata.channels > 128)
        return -1;
    if ((size_t)safe_read(fd, buffer + sizeof(AuHeader), BE_INT(ap->hdr_size) - sizeof(AuHeader)) != BE_INT(ap->hdr_size) - sizeof(AuHeader)) {
        ERR("read error");
        stopAndExit();
    }
    return 0;
}

void AlsaPlayerThread::set_params(void)
{
    snd_pcm_hw_params_t *hwparams;
    snd_pcm_uframes_t period_size;
    int err;
    int dir;
    unsigned int rate;
    unsigned int periods;

    snd_pcm_hw_params_alloca(&hwparams);
    err = snd_pcm_hw_params_any(handle, hwparams);
    if (err < 0) {
        ERR("Broken configuration for this PCM: no configurations available");
        stopAndExit();
    }

    /* Create the pipe for communication about stop requests. */
    if (pipe(alsa_stop_pipe)) {
        ERR("Stop pipe creation failed (%s)", strerror(errno));
        stopAndExit();
    }

    /* Find how many descriptors we will get for poll(). */
    alsa_fd_count = snd_pcm_poll_descriptors_count(handle);
    if (alsa_fd_count <= 0){
        ERR("Invalid poll descriptors count returned from ALSA.");
        stopAndExit();
    }

    /* Create and fill in struct pollfd *alsa_poll_fds with ALSA descriptors. */
    // alsa_poll_fds = (pollfd *)malloc ((alsa_fd_count + 1) * sizeof(struct pollfd));
    alsa_poll_fds_barray.resize((alsa_fd_count + 1) * sizeof(struct pollfd));
    alsa_poll_fds = (pollfd *)alsa_poll_fds_barray.data();
    assert(alsa_poll_fds);
    if ((err = snd_pcm_poll_descriptors(handle, alsa_poll_fds, alsa_fd_count)) < 0) {
        ERR("Unable to obtain poll descriptors for playback: %s", snd_strerror(err));
        stopAndExit();
    }

    /* Create a new pollfd structure for requests by alsa_stop(). */
    struct pollfd alsa_stop_pipe_pfd;
    alsa_stop_pipe_pfd.fd = alsa_stop_pipe[0];
    alsa_stop_pipe_pfd.events = POLLIN;
    alsa_stop_pipe_pfd.revents = 0;

    /* Join this our own pollfd to the ALSAs ones. */
    alsa_poll_fds[alsa_fd_count] = alsa_stop_pipe_pfd;
    ++alsa_fd_count;

    if (mmap_flag) {
        snd_pcm_access_mask_t *mask = (snd_pcm_access_mask_t *)alloca(snd_pcm_access_mask_sizeof());
        snd_pcm_access_mask_none(mask);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
        err = snd_pcm_hw_params_set_access_mask(handle, hwparams, mask);
    } else if (interleaved)
        err = snd_pcm_hw_params_set_access(handle, hwparams,
                           SND_PCM_ACCESS_RW_INTERLEAVED);
    else
        err = snd_pcm_hw_params_set_access(handle, hwparams,
                           SND_PCM_ACCESS_RW_NONINTERLEAVED);
    if (err < 0) {
        ERR("Error setting access type: %s", snd_strerror(err));
        stopAndExit();
    }
    err = snd_pcm_hw_params_set_format(handle, hwparams, hwdata.format);
    if (err < 0) {
        ERR("Error setting sample format to %i: %s", hwdata.format, snd_strerror(err));
        stopAndExit();
    }
    err = snd_pcm_hw_params_set_channels(handle, hwparams, hwdata.channels);
    if (err < 0) {
        ERR("Error setting channel count to %i: %s", hwdata.channels, snd_strerror(err));
        stopAndExit();
    }

#if 0
    err = snd_pcm_hw_params_set_periods_min(handle, hwparams, 2);
    assert(err >= 0);
#endif
    rate = hwdata.rate;
    err = snd_pcm_hw_params_set_rate_near(handle, hwparams, &hwdata.rate, 0);
    assert(err >= 0);
    if ((float)rate * 1.05 < hwdata.rate || (float)rate * 0.95 > hwdata.rate) {
        MSG("Warning: rate is not accurate (requested = %iHz, got = %iHz)", rate, hwdata.rate);
        MSG("         please, try the plug plugin (-Dplug:%s)", snd_pcm_name(handle));
    }

    period_size = m_defPeriodSize;
    dir = 1;
    err = snd_pcm_hw_params_set_period_size_near(handle, hwparams, &period_size, &dir);
    if (err < 0) {
        MSG("Setting period_size to %lu failed, but continuing: %s", period_size, snd_strerror(err));
    }

    periods = m_defPeriods;
    dir = 1;
    err = snd_pcm_hw_params_set_periods_near(handle, hwparams, &periods, &dir);
    if (err < 0)
        MSG("Unable to set number of periods to %i, but continuing: %s", periods, snd_strerror(err));

    /* Install hw parameters. */
    err = snd_pcm_hw_params(handle, hwparams);
    if (err < 0) {
        MSG("Unable to install hw params: %s", snd_strerror(err));
        snd_pcm_hw_params_dump(hwparams, log);
        stopAndExit();
    }

    /* Determine if device can pause. */
    canPause = (1 == snd_pcm_hw_params_can_pause(hwparams));

    /* Get final buffer size and calculate the chunk size we will pass to device. */
    snd_pcm_hw_params_get_buffer_size(hwparams, &buffer_size);
    chunk_size = periods * period_size;

    if (0 == chunk_size) {
        ERR("Invalid periods or period_size.  Cannot continue.");
        stopAndExit();
    }

    if (chunk_size == buffer_size)
        MSG("WARNING: Shouldn't use chunk_size equal to buffer_size (%lu).  Continuing anyway.", chunk_size);

    DBG("Final buffer_size = %lu, chunk_size = %lu, periods = %i, period_size = %lu, canPause = %i",
        buffer_size, chunk_size, periods, period_size, canPause);

    if (m_debugLevel >= 2)
        snd_pcm_dump(handle, log);

    bits_per_sample = snd_pcm_format_physical_width(hwdata.format);
    bits_per_frame = bits_per_sample * hwdata.channels;
    chunk_bytes = chunk_size * bits_per_frame / 8;
    audioBuffer.resize(chunk_bytes);
    audiobuf = audioBuffer.data();
    if (audiobuf == NULL) {
        ERR("not enough memory");
        stopAndExit();
    }
}

#ifndef timersub
#define timersub(a, b, result) \
do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
        --(result)->tv_sec; \
        (result)->tv_usec += 1000000; \
    } \
} while (0)
#endif

/* I/O error handler */
void AlsaPlayerThread::xrun()
{
    snd_pcm_status_t *status;
    int res;

    snd_pcm_status_alloca(&status);
    if ((res = snd_pcm_status(handle, status))<0) {
        ERR("status error: %s", snd_strerror(res));
        stopAndExit();
    }
    if (SND_PCM_STATE_XRUN == snd_pcm_status_get_state(status)) {
        struct timeval now, diff, tstamp;
        gettimeofday(&now, 0);
        snd_pcm_status_get_trigger_tstamp(status, &tstamp);
        timersub(&now, &tstamp, &diff);
        MSG("%s!!! (at least %.3f ms long)",
            stream == SND_PCM_STREAM_PLAYBACK ? "underrun" : "overrun",
            diff.tv_sec * 1000 + diff.tv_usec / 1000.0);
        if (m_debugLevel >= 2) {
            DBG("Status:");
            snd_pcm_status_dump(status, log);
        }
        if ((res = snd_pcm_prepare(handle))<0) {
            ERR("xrun: prepare error: %s", snd_strerror(res));
            stopAndExit();
        }
        return;        /* ok, data should be accepted again */
    } if (SND_PCM_STATE_DRAINING == snd_pcm_status_get_state(status)) {
        if (m_debugLevel >= 2) {
            DBG("Status(DRAINING):");
            snd_pcm_status_dump(status, log);
        }
        if (stream == SND_PCM_STREAM_CAPTURE) {
            MSG("capture stream format change? attempting recover...");
            if ((res = snd_pcm_prepare(handle))<0) {
                ERR("xrun(DRAINING): prepare error: %s", snd_strerror(res));
                stopAndExit();
            }
            return;
        }
    }
    if (m_debugLevel >= 2) {
        DBG("Status(R/W):");
        snd_pcm_status_dump(status, log);
    }
    ERR("read/write error, state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)));
    stopAndExit();
}

/* I/O suspend handler */
void AlsaPlayerThread::suspend(void)
{
    int res;

    MSG("Suspended. Trying resume. ");
    while ((res = snd_pcm_resume(handle)) == -EAGAIN)
        sleep(1);    /* wait until suspend flag is released */
    if (res < 0) {
        MSG("Failed. Restarting stream. ");
        if ((res = snd_pcm_prepare(handle)) < 0) {
            ERR("suspend: prepare error: %s", snd_strerror(res));
            stopAndExit();
        }
    }
    MSG("Suspend done.");
}

/* peak handler */
void AlsaPlayerThread::compute_max_peak(char *data, size_t count)
{
    signed int val, max, max_peak = 0, perc;
    size_t ocount = count;

    switch (bits_per_sample) {
    case 8: {
        signed char *valp = (signed char *)data;
        signed char mask = snd_pcm_format_silence(hwdata.format);
        while (count-- > 0) {
            val = *valp++ ^ mask;
            val = abs(val);
            if (max_peak < val)
                max_peak = val;
        }
        break;
    }
    case 16: {
        signed short *valp = (signed short *)data;
        signed short mask = snd_pcm_format_silence_16(hwdata.format);
        count /= 2;
        while (count-- > 0) {
            val = *valp++ ^ mask;
            val = abs(val);
            if (max_peak < val)
                max_peak = val;
        }
        break;
    }
    case 32: {
        signed int *valp = (signed int *)data;
        signed int mask = snd_pcm_format_silence_32(hwdata.format);
        count /= 4;
        while (count-- > 0) {
            val = *valp++ ^ mask;
            val = abs(val);
            if (max_peak < val)
                max_peak = val;
        }
        break;
    }
    default:
        break;
    }
    max = 1 << (bits_per_sample-1);
    if (max <= 0)
        max = 0x7fffffff;
    DBG("Max peak (%li samples): %05i (0x%04x) ", (long)ocount, max_peak, max_peak);
    if (bits_per_sample > 16)
        perc = max_peak / (max / 100);
    else
        perc = max_peak * 100 / max;
    for (val = 0; val < 20; val++)
        if (val <= perc / 5)
            kDebug() << '#';
        else
            kDebug() << ' ';
    DBG(" %i%%", perc);
}

/*
 *  write function
 */

ssize_t AlsaPlayerThread::pcm_write(char *data, size_t count)
{
    ssize_t r;
    ssize_t result = 0;

    if (sleep_min == 0 && count < chunk_size) {
        DBG("calling snd_pcm_format_set_silence");
        snd_pcm_format_set_silence(hwdata.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwdata.channels);
        count = chunk_size;
    }
    while (count > 0) {
        DBG("calling writei_func, count = %i", count);
        r = writei_func(handle, data, count);
        DBG("writei_func returned %i", r);
        if (-EAGAIN == r || (r >= 0 && (size_t)r < count)) {
            DBG("r = %i calling snd_pcm_wait", r);
            snd_pcm_wait(handle, 100);
        } else if (-EPIPE == r) {
            xrun();
        } else if (-ESTRPIPE == r) {
            suspend();
        } else if (-EBUSY == r){
            MSG("WARNING: sleeping while PCM BUSY");
            usleep(1000);
            continue;
        } else if (r < 0) {
            ERR("write error: %s", snd_strerror(r));
            stopAndExit();
        }
        if (r > 0) {
            if (m_debugLevel >= 2 > 1)
                compute_max_peak(data, r * hwdata.channels);
            result += r;
            count -= r;
            data += r * bits_per_frame / 8;
        }
        /* Report current state */
        DBG("PCM state before polling: %s",
            snd_pcm_state_name(snd_pcm_state(handle)));

        int err = wait_for_poll(0);
        if (err < 0) {
            ERR("Wait for poll() failed");
            return -1;
        }
        else if (err == 1){
            MSG("Playback stopped");
            /* Drop the playback on the sound device (probably
               still in progress up till now) */
            err = snd_pcm_drop(handle);
            if (err < 0) {
                ERR("snd_pcm_drop() failed: %s", snd_strerror(err));
                return -1;
            }
            return -1;
        }
    }
    return result;
}

/*
 *  ok, let's play a .voc file
 */

ssize_t AlsaPlayerThread::voc_pcm_write(u_char *data, size_t count)
{
    ssize_t result = count, r;
    size_t size;

    while (count > 0) {
        size = count;
        if (size > chunk_bytes - buffer_pos)
            size = chunk_bytes - buffer_pos;
        memcpy(audiobuf + buffer_pos, data, size);
        data += size;
        count -= size;
        buffer_pos += size;
        if ((size_t)buffer_pos == chunk_bytes) {
            if ((size_t)(r = pcm_write(audiobuf, chunk_size)) != chunk_size)
                return r;
            buffer_pos = 0;
        }
    }
    return result;
}

void AlsaPlayerThread::voc_write_silence(unsigned x)
{
    unsigned l;
    char *buf;

    QByteArray buffer(chunk_bytes, '\0');
    // buf = (char *) malloc(chunk_bytes);
    buf = buffer.data();
    if (buf == NULL) {
        ERR("can't allocate buffer for silence");
        return;        /* not fatal error */
    }
    snd_pcm_format_set_silence(hwdata.format, buf, chunk_size * hwdata.channels);
    while (x > 0) {
        l = x;
        if (l > chunk_size)
            l = chunk_size;
        if (voc_pcm_write((u_char*)buf, l) != (ssize_t)l) {
            ERR("write error");
            stopAndExit();
        }
        x -= l;
    }
    // free(buf);
}

void AlsaPlayerThread::voc_pcm_flush(void)
{
    if (buffer_pos > 0) {
        size_t b;
        if (sleep_min == 0) {
            if (snd_pcm_format_set_silence(hwdata.format, audiobuf + buffer_pos, chunk_bytes - buffer_pos * 8 / bits_per_sample) < 0)
                MSG("voc_pcm_flush - silence error");
            b = chunk_size;
        } else {
            b = buffer_pos * 8 / bits_per_frame;
        }
        if (pcm_write(audiobuf, b) != (ssize_t)b)
            ERR("voc_pcm_flush error");
    }
    snd_pcm_drain(handle);
}

void AlsaPlayerThread::voc_play(int fd, int ofs, const char* name)
{
    int l;
    VocBlockType *bp;
    VocVoiceData *vd;
    VocExtBlock *eb;
    size_t nextblock, in_buffer;
    u_char *data, *buf;
    char was_extended = 0, output = 0;
    u_short *sp, repeat = 0;
    size_t silence;
    off64_t filepos = 0;

#define COUNT(x)    nextblock -= x; in_buffer -= x; data += x
#define COUNT1(x)    in_buffer -= x; data += x

    QByteArray buffer(64 * 1024, '\0');
    // data = buf = (u_char *)malloc(64 * 1024);
    data = buf = (u_char*)buffer.data();
    buffer_pos = 0;
    if (data == NULL) {
        ERR("malloc error");
        stopAndExit();
    }
    MSG("Playing Creative Labs Channel file '%s'...", name);
    /* first we waste the rest of header, ugly but we don't need seek */
    while (ofs > (ssize_t)chunk_bytes) {
        if ((size_t)safe_read(fd, buf, chunk_bytes) != chunk_bytes) {
            ERR("read error");
            stopAndExit();
        }
        ofs -= chunk_bytes;
    }
    if (ofs) {
        if (safe_read(fd, buf, ofs) != ofs) {
            ERR("read error");
            stopAndExit();
        }
    }
    hwdata.format = DEFAULT_FORMAT;
    hwdata.channels = 1;
    hwdata.rate = DEFAULT_SPEED;
    set_params();

    in_buffer = nextblock = 0;
    while (1) {
          Fill_the_buffer:    /* need this for repeat */
        if (in_buffer < 32) {
            /* move the rest of buffer to pos 0 and fill the buf up */
            if (in_buffer)
                memcpy(buf, data, in_buffer);
            data = buf;
            if ((l = safe_read(fd, buf + in_buffer, chunk_bytes - in_buffer)) > 0)
                in_buffer += l;
            else if (!in_buffer) {
                /* the file is truncated, so simulate 'Terminator' 
                   and reduce the datablock for safe landing */
                nextblock = buf[0] = 0;
                if (l == -1) {
//                    perror(name);
                    stopAndExit();
                }
            }
        }
        while (!nextblock) {    /* this is a new block */
            if (in_buffer < sizeof(VocBlockType))
                goto __end;
            bp = (VocBlockType *) data;
            COUNT1(sizeof(VocBlockType));
            nextblock = VOC_DATALEN(bp);
            if (output)
                MSG(" ");  /* write /n after ASCII-out */
            output = 0;
            switch (bp->type) {
            case 0:
#if 0
                MSG("Terminator");
#endif
                return;        /* VOC-file stop */
            case 1:
                vd = (VocVoiceData *) data;
                COUNT1(sizeof(VocVoiceData));
                /* we need a SYNC, before we can set new SPEED, STEREO ... */

                if (!was_extended) {
                    hwdata.rate = (int) (vd->tc);
                    hwdata.rate = 1000000 / (256 - hwdata.rate);
#if 0
                    MSG("Channel data %d Hz", dsp_speed);
#endif
                    if (vd->pack) {        /* /dev/dsp can't it */
                        ERR("can't play packed .voc files");
                        return;
                    }
                    if (hwdata.channels == 2)        /* if we are in Stereo-Mode, switch back */
                        hwdata.channels = 1;
                } else {    /* there was extended block */
                    hwdata.channels = 2;
                    was_extended = 0;
                }
                set_params();
                break;
            case 2:    /* nothing to do, pure data */
#if 0
                MSG("Channel continuation");
#endif
                break;
            case 3:    /* a silence block, no data, only a count */
                sp = (u_short *) data;
                COUNT1(sizeof(u_short));
                hwdata.rate = (int) (*data);
                COUNT1(1);
                hwdata.rate = 1000000 / (256 - hwdata.rate);
                set_params();
                silence = (((size_t) * sp) * 1000) / hwdata.rate;
#if 0
                MSG("Silence for %d ms", (int) silence);
#endif
                voc_write_silence(*sp);
                break;
            case 4:    /* a marker for syncronisation, no effect */
                sp = (u_short *) data;
                COUNT1(sizeof(u_short));
#if 0
                MSG("Marker %d", *sp);
#endif
                break;
            case 5:    /* ASCII text, we copy to stderr */
                output = 1;
#if 0
                MSG("ASCII - text :");
#endif
                break;
            case 6:    /* repeat marker, says repeatcount */
                /* my specs don't say it: maybe this can be recursive, but
                   I don't think somebody use it */
                repeat = *(u_short *) data;
                COUNT1(sizeof(u_short));
#if 0
                MSG("Repeat loop %d times", repeat);
#endif
                if (filepos >= 0) {    /* if < 0, one seek fails, why test another */
                    if ((filepos = lseek64(fd, 0, 1)) < 0) {
                        ERR("can't play loops; %s isn't seekable", name);
                        repeat = 0;
                    } else {
                        filepos -= in_buffer;    /* set filepos after repeat */
                    }
                } else {
                    repeat = 0;
                }
                break;
            case 7:    /* ok, lets repeat that be rewinding tape */
                if (repeat) {
                    if (repeat != 0xFFFF) {
#if 0
                        MSG("Repeat loop %d", repeat);
#endif
                        --repeat;
                    }
#if 0
                    else
                        MSG("Neverending loop");
#endif
                    lseek64(fd, filepos, 0);
                    in_buffer = 0;    /* clear the buffer */
                    goto Fill_the_buffer;
                }
#if 0
                else
                    MSG("End repeat loop");
#endif
                break;
            case 8:    /* the extension to play Stereo, I have SB 1.0 :-( */
                was_extended = 1;
                eb = (VocExtBlock *) data;
                COUNT1(sizeof(VocExtBlock));
                hwdata.rate = (int) (eb->tc);
                hwdata.rate = 256000000L / (65536 - hwdata.rate);
                hwdata.channels = eb->mode == VOC_MODE_STEREO ? 2 : 1;
                if (hwdata.channels == 2)
                    hwdata.rate = hwdata.rate >> 1;
                if (eb->pack) {        /* /dev/dsp can't it */
                    ERR("can't play packed .voc files");
                    return;
                }
#if 0
                MSG("Extended block %s %d Hz",
                     (eb->mode ? "Stereo" : "Mono"), dsp_speed);
#endif
                break;
            default:
                ERR("unknown blocktype %d. terminate.", bp->type);
                return;
            }    /* switch (bp->type) */
        }        /* while (! nextblock)  */
        /* put nextblock data bytes to dsp */
        l = in_buffer;
        if (nextblock < (size_t)l)
            l = nextblock;
        if (l) {
            if (output) {
                if (write(2, data, l) != l) {    /* to stderr */
                    ERR("write error");
                    stopAndExit();
                }
            } else {
                if (voc_pcm_write(data, l) != l) {
                    ERR("write error");
                    stopAndExit();
                }
            }
            COUNT(l);
        }
    }            /* while(1) */
      __end:
        voc_pcm_flush();
        // free(buf);
}
/* that was a big one, perhaps somebody split it :-) */

/* setting the globals for playing raw data */
void AlsaPlayerThread::init_raw_data(void)
{
    hwdata = rhwdata;
}

/* calculate the data count to read from/to dsp */
off64_t AlsaPlayerThread::calc_count(void)
{
    off64_t count;

    if (timelimit == 0) {
        count = pbrec_count;
    } else {
        count = snd_pcm_format_size(hwdata.format, hwdata.rate * hwdata.channels);
        count *= (off64_t)timelimit;
    }
    return count < pbrec_count ? count : pbrec_count;
}

void AlsaPlayerThread::header(int /*rtype*/, const char* /*name*/)
{
//        fprintf(stderr, "%s %s '%s' : ",
//            (stream == SND_PCM_STREAM_PLAYBACK) ? "Playing" : "Recording",
//            fmt_rec_table[rtype].what,
//            name);
    QString channels;
    if (hwdata.channels == 1)
        channels = "Mono";
    else if (hwdata.channels == 2)
        channels = "Stereo";
    else
        channels = QString("Channels %1").arg(hwdata.channels);
    QByteArray asciiChannels = channels.toAscii();
    DBG("Format: %s, Rate %d Hz, %s",
        snd_pcm_format_description(hwdata.format),
        hwdata.rate,
        asciiChannels.constData());
}

/* playing raw data */

void AlsaPlayerThread::playback_go(int fd, size_t loaded, off64_t count, int rtype, const char *name)
{
    int l, r;
    off64_t written = 0;
    off64_t c;

    if (m_debugLevel >= 1) header(rtype, name);
    set_params();

    while (loaded > chunk_bytes && written < count) {
        if (pcm_write(audiobuf + written, chunk_size) <= 0)
            return;
        written += chunk_bytes;
        loaded -= chunk_bytes;
    }
    if (written > 0 && loaded > 0)
        memmove(audiobuf, audiobuf + written, loaded);

    l = loaded;
    while (written < count) {
        do {
            c = count - written;
            if (c > chunk_bytes)
                c = chunk_bytes;
            c -= l;

            if (c == 0)
                break;
            r = safe_read(fd, audiobuf + l, c);
            if (r < 0) {
//                perror(name);
                stopAndExit();
            }
            fdcount += r;
            if (r == 0)
                break;
            l += r;
        } while (sleep_min == 0 && (size_t)l < chunk_bytes);
        l = l * 8 / bits_per_frame;
        DBG("calling pcm_write with %i frames.", l);
        r = pcm_write(audiobuf, l);
        DBG("pcm_write returned r = %i", r);
        if (r < 0) return;
        if (r != l)
            break;
        r = r * bits_per_frame / 8;
        written += r;
        l = 0;
    }

    DBG("Draining...");

    /* We want the next "device ready" notification only when the buffer is completely empty. */
    /* Do this by setting the avail_min to the buffer size. */
    int err;
    DBG("Getting swparams");
    snd_pcm_sw_params_t *swparams;
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_sw_params_current(handle, swparams);
    if (err < 0) {
        ERR("Unable to get current swparams: %s", snd_strerror(err));
        return;
    }
    DBG("Setting avail min to %lu", buffer_size);
    err = snd_pcm_sw_params_set_avail_min(handle, swparams, buffer_size);
    if (err < 0) {
        ERR("Unable to set avail min for playback: %s", snd_strerror(err));
        return;
    }
    /* write the parameters to the playback device */
    DBG("Writing swparams");
    err = snd_pcm_sw_params(handle, swparams);
    if (err < 0) {
        ERR("Unable to set sw params for playback: %s", snd_strerror(err));
        return;
    }

    DBG("Waiting for poll");
    err = wait_for_poll(1);
    if (err < 0) {
        ERR("Wait for poll() failed");
        return;
    } else if (err == 1){
        MSG("Playback stopped while draining");

        /* Drop the playback on the sound device (probably
           still in progress up till now) */
        err = snd_pcm_drop(handle);
        if (err < 0) {
            ERR("snd_pcm_drop() failed: %s", snd_strerror(err));
            return;
        }
    }
    DBG("Draining completed");
}

/*
 *  let's play or capture it (capture_type says VOC/WAVE/raw)
 */

void AlsaPlayerThread::playback(int fd)
{
    int ofs;
    size_t dta;
    ssize_t dtawave;

    pbrec_count = LLONG_MAX;
    fdcount = 0;

    /* read the file header */
    dta = sizeof(AuHeader);
    if ((size_t)safe_read(fd, audiobuf, dta) != dta) {
        ERR("read error");
        stopAndExit();
    }
    if (test_au(fd, audiobuf) >= 0) {
        rhwdata.format = hwdata.format;
        pbrec_count = calc_count();
        playback_go(fd, 0, pbrec_count, FORMAT_AU, audiofile_name);
        goto __end;
    }
    dta = sizeof(VocHeader);
    if ((size_t)safe_read(fd, audiobuf + sizeof(AuHeader),
         dta - sizeof(AuHeader)) != dta - sizeof(AuHeader)) {
        ERR("read error");
        stopAndExit();
    }
    if ((ofs = test_vocfile(audiobuf)) >= 0) {
        pbrec_count = calc_count();
        voc_play(fd, ofs, audiofile_name);
        goto __end;
    }
    /* read bytes for WAVE-header */
    if ((dtawave = test_wavefile(fd, audiobuf, dta)) >= 0) {
        pbrec_count = calc_count();
        playback_go(fd, dtawave, pbrec_count, FORMAT_WAVE, audiofile_name);
    } else {
        /* should be raw data */
        init_raw_data();
        pbrec_count = calc_count();
        playback_go(fd, dta, pbrec_count, FORMAT_RAW, audiofile_name);
    }
__end:
    return;
}

/* Wait until ALSA is ready for more samples or stop() was called.
   @return 0 if ALSA is ready for more input, +1 if a request to stop
   the sound output was received and a negative value on error.  */
int AlsaPlayerThread::wait_for_poll(int draining)
{
    unsigned short revents;
    snd_pcm_state_t state;
    int ret;

    DBG("Waiting for poll");

    /* Wait for certain events */
    while (1) {
        /* Simulated pause by not writing to alsa device, which will lead to an XRUN
           when resumed. */
        if (m_simulatedPause)
            msleep(500);
        else {

            ret = poll(alsa_poll_fds, alsa_fd_count, -1);
            DBG("activity on %d descriptors", ret);

            /* Check for stop request from alsa_stop on the last file descriptors. */
            if ((revents = alsa_poll_fds[alsa_fd_count-1].revents)) {
                if (revents & POLLIN){
                    DBG("stop requested");
                    return 1;
                }
            }

            /* Check the first count-1 descriptors for ALSA events */
            snd_pcm_poll_descriptors_revents(handle, alsa_poll_fds, alsa_fd_count-1, &revents);

            /* Ensure we are in the right state */
            state = snd_pcm_state(handle);
            DBG("State after poll returned is %s", snd_pcm_state_name(state));

            if (SND_PCM_STATE_XRUN == state){
                if (!draining){
                    MSG("WARNING: Buffer underrun detected!");
                    xrun();
                    return 0;
                }else{
                    DBG("Playback terminated");
                    return 0;
                }
            }

            if (SND_PCM_STATE_SUSPENDED == state){
                DBG("WARNING: Suspend detected!");
                suspend();
                return 0;
            }

            /* Check for errors */
            if (revents & POLLERR) {
                DBG("poll revents says POLLERR");
                return -EIO;
            }

            /* Is ALSA ready for more input? */
            if ((revents & POLLOUT)){
                DBG("Ready for more input");
                return 0;
            }
        }
    }
}

// ====================================================================
// AlsaPlayer
//
// AlsaPlayer is nothing more than a container for AlsaPlayerThread
// in order to avoid ambiguous QObject, since both Player and QThread
// derive from QObject.  Is there a better solution?

AlsaPlayer::AlsaPlayer(QObject* parent, const QStringList& args):
    Player(parent, "alsaplayer", args)
{
    m_AlsaPlayerThread = new AlsaPlayerThread(this);
}

AlsaPlayer::~AlsaPlayer()
{
    delete m_AlsaPlayerThread;
}

/*virtual*/ void AlsaPlayer::startPlay(const QString& file) { m_AlsaPlayerThread->startPlay(file); }
/*virtual*/ void AlsaPlayer::pause() { m_AlsaPlayerThread->pause(); }
/*virtual*/ void AlsaPlayer::stop() { m_AlsaPlayerThread->stop(); }

/*virtual*/ void AlsaPlayer::setVolume(float volume) { m_AlsaPlayerThread->setVolume(volume); }
/*virtual*/ float AlsaPlayer::volume() const { return m_AlsaPlayerThread->volume(); }

/*virtual*/ bool AlsaPlayer::playing() const { return m_AlsaPlayerThread->playing(); }
/*virtual*/ bool AlsaPlayer::paused() const { return m_AlsaPlayerThread->paused(); }

/*virtual*/ int AlsaPlayer::totalTime() const { return m_AlsaPlayerThread->totalTime(); }
/*virtual*/ int AlsaPlayer::currentTime() const { return m_AlsaPlayerThread->currentTime(); }
/*virtual*/ int AlsaPlayer::position() const { return m_AlsaPlayerThread->position(); } // in this case not really the percent

/*virtual*/ void AlsaPlayer::seek(int seekTime) { m_AlsaPlayerThread->seek(seekTime); }
/*virtual*/ void AlsaPlayer::seekPosition(int position) { m_AlsaPlayerThread->seekPosition(position); }

/*virtual*/ QStringList AlsaPlayer::getPluginList( const QByteArray& classname )
    { return m_AlsaPlayerThread->getPluginList(classname); }
/*virtual*/ void AlsaPlayer::setSinkName(const QString &sinkName)
    { m_AlsaPlayerThread->setSinkName(sinkName); }

#include "alsaplayer.moc"

#undef DBG
#undef MSG
#undef ERR

// vim: sw=4 ts=8 et
