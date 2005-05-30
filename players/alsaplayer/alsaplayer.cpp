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
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 ******************************************************************************/

// #include <sys/wait.h>
// System includes.
#include <sys/time.h>

// Qt includes.
#include <qdir.h>
#include <qapplication.h>
#include <qcstring.h>

// KDE includes.
#include <kdebug.h>
#include <kconfig.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <klocale.h>

// AlsaPlayer includes.
#include "alsaplayer.h"

#if __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ >= 95)
#define error(...) do {\
    QString s = dbgStr.sprintf( "%s:%d: ", __FUNCTION__, __LINE__); \
	s += dbgStr.sprintf( __VA_ARGS__); \
	kdDebug() << s << endl; \
} while (0)
#else
#define error(args...) do {\
	QString s = dbgStr.sprintf( "%s:%d: ", __FUNCTION__, __LINE__); \
	s += dbgStr.sprintf( ##args); \
	kdDebug() << s << endl; \
} while (0)
#endif	

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

AlsaPlayer::AlsaPlayer(QObject* parent, const char* name, const QStringList& args) : 
    Player(parent, name, args),
    m_currentVolume(1.0),
    m_pcmName("default")
{
    init();
}

AlsaPlayer::~AlsaPlayer()
{
    if (running()) {
        stop();
        wait();
    }
}

//void AlsaPlayer::play(const FileHandle &file)
void AlsaPlayer::startPlay(const QString &file)
{
    if (running()) {
        if (paused()) snd_pcm_pause(handle, false);
        return;
    }
    audiofile.setName(file);
    audiofile.open(IO_ReadOnly);
    fd = audiofile.handle();
    // Start thread running.
    start();
}

/*virtual*/ void AlsaPlayer::run()
{
    QString pName = m_pcmName.section(" ", 0, 0);
    // kdDebug() << "AlsaPlayer::run: pName = " << pName << endl;
	pcm_name = qstrdup(pName.ascii());
	int err;
	snd_pcm_info_t *info;

	snd_pcm_info_alloca(&info);

	err = snd_output_stdio_attach(&log, stderr, 0);
	assert(err >= 0);

	rhwparams.format = DEFAULT_FORMAT;
	rhwparams.rate = DEFAULT_SPEED;
	rhwparams.channels = 1;

	err = snd_pcm_open(&handle, pcm_name, stream, open_mode);
	if (err < 0) {
		error("audio open error: %s", snd_strerror(err));
		return;
	}

	if ((err = snd_pcm_info(handle, info)) < 0) {
		error("info error: %s", snd_strerror(err));
		return;
	}

//	if ((err = snd_pcm_info(handle, info)) < 0) {
//		error("info error: %s", snd_strerror(err));
//		return;
//	}

	if (nonblock) {
		err = snd_pcm_nonblock(handle, 1);
		if (err < 0) {
			error("nonblock setting error: %s", snd_strerror(err));
			return;
		}
	}

	chunk_size = 1024;
	hwparams = rhwparams;

    audioBuffer.resize(1024);
	// audiobuf = (char *)malloc(1024);
    audiobuf = audioBuffer.data();
	if (audiobuf == NULL) {
		error("not enough memory");
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


	// signal(SIGINT, signal_handler);
	// signal(SIGTERM, signal_handler);
	// signal(SIGABRT, signal_handler);
    playback(fd);
    cleanup();
	return;
}

void AlsaPlayer::pause()
{
    if (running()) {
        // Some hardware can pause; some can't.  canPause is set in set_params.
        if (canPause)
            snd_pcm_pause(handle, true);
        else
            // TODO: Need to support pausing for hardware that does not support it.
            // Perhaps by setting a flag and causing pcm_write routine to sleep?
            stop();
    }
}

void AlsaPlayer::stop()
{
    if (running()) {
        /* Stop PCM device and drop pending frames */
        snd_pcm_drop(handle);
        /* Wait for thread to exit */
        wait();
    }
    cleanup();
}

void AlsaPlayer::setVolume(float volume)
{
    m_currentVolume = volume;
}

float AlsaPlayer::volume() const
{
    return m_currentVolume;
}

/////////////////////////////////////////////////////////////////////////////////
// player status functions
/////////////////////////////////////////////////////////////////////////////////

bool AlsaPlayer::playing() const
{
    bool result = false;
    if (running() && handle) {
        snd_pcm_status_t *status;
        snd_pcm_status_alloca(&status);
        int res;
        if ((res = snd_pcm_status(handle, status)) < 0)
            kdDebug() << "AlsaPlayer::playing: status error: " << snd_strerror(res) << endl;
        else {
            result = (snd_pcm_status_get_state(status) == SND_PCM_STATE_RUNNING)
                  || (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING);
            kdDebug() << "AlsaPlayer:playing: state = " << snd_pcm_state_name(snd_pcm_status_get_state(status)) << endl;
        }
        // TODO: This crashes.  Why?
        // snd_pcm_status_free(status);
    }
    return result;
}

bool AlsaPlayer::paused() const
{
    bool result = false;
    if (running() && handle) {
        snd_pcm_status_t *status;
        snd_pcm_status_alloca(&status);
        int res;
        if ((res = snd_pcm_status(handle, status)) < 0)
            kdDebug() << "AlsaPlayer::paused: status error: " << snd_strerror(res) << endl;
        else {
            result = (snd_pcm_status_get_state(status) == SND_PCM_STATE_PAUSED);
            kdDebug() << "AlsaPlayer:paused: state = " << snd_pcm_state_name(snd_pcm_status_get_state(status)) << endl;
        }
        // snd_pcm_status_free(status);
    }
    return result;
}

int AlsaPlayer::totalTime() const
{
    return 0;
}

int AlsaPlayer::currentTime() const
{
    return 0;
}

int AlsaPlayer::position() const
{
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////
// player seek functions
/////////////////////////////////////////////////////////////////////////////////

void AlsaPlayer::seek(int /*seekTime*/)
{
}

void AlsaPlayer::seekPosition(int /*position*/)
{
}

/*
 * Returns a list of PCM devices.
 */
QStringList AlsaPlayer::getPluginList( const QCString& /*classname*/ )
{
    QStringList assumed("default");
    snd_config_t *conf;
    int err = snd_config_update();
    if (err < 0) {
        error("snd_config_update: %s", snd_strerror(err));
        return assumed;
    }
    err = snd_config_search(snd_config, "pcm", &conf);
    if (err < 0) return QStringList();
    snd_config_iterator_t it = snd_config_iterator_first(conf);
    snd_config_iterator_t itEnd = snd_config_iterator_end(conf);
    const char* id;
    snd_config_t *entry;
    QStringList result;
    snd_ctl_card_info_t *info;
    snd_ctl_card_info_alloca(&info);
    snd_pcm_info_t *pcminfo;
    snd_pcm_info_alloca(&pcminfo);
    while (it != itEnd) {
        entry = snd_config_iterator_entry(it);
        err = snd_config_get_id(entry, &id);
        if (err >= 0) {
            if (QString(id) != "default")
            {
                int card = -1;
                while (snd_card_next(&card) >= 0 && card >= 0) {
                    char name[32];
                    sprintf(name, "%s:%d", id, card);
                    snd_ctl_t *handle;
                    if ((err = snd_ctl_open(&handle, name, 0)) >= 0) {
                        if ((err = snd_ctl_card_info(handle, info)) >= 0) {
                            int dev = -1;
                            snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
                            while (snd_ctl_pcm_next_device(handle, &dev) >= 0 && dev >= 0) {
                                snd_pcm_info_set_device(pcminfo, dev);
                                snd_pcm_info_set_subdevice(pcminfo, 0);
                                snd_pcm_info_set_stream(pcminfo, stream);
                                if ((err = snd_ctl_pcm_info(handle, pcminfo)) >= 0) {
                                    QString pluginName = name;
                                    pluginName += ",";
                                    pluginName += QString::number(dev);
                                    pluginName += " ";
                                    pluginName += snd_ctl_card_info_get_name(info);
                                    pluginName += ",";
                                    pluginName += snd_pcm_info_get_name(pcminfo);
                                    result.append(pluginName);
                                    kdDebug() << pluginName << endl;
                                }
                            }
                        }
                        snd_ctl_close(handle);
                    }
                }
                if (card == -1) result.append(id);
            } else result.append(id);
        }
        it = snd_config_iterator_next(it);
    }
    snd_config_update_free_global();
    // snd_pcm_info_free(pcminfo);
    // snd_ctl_card_info_free(info);
    return result;
}

void AlsaPlayer::setSinkName(const QString& sinkName) { m_pcmName = sinkName; }
bool AlsaPlayer::requireVersion(uint /*major*/, uint /*minor*/, uint /*micro*/) { return true; }

/////////////////////////////////////////////////////////////////////////////////
// private
/////////////////////////////////////////////////////////////////////////////////

void AlsaPlayer::init()
{
    pcm_name = 0;
    handle = 0;
    canPause = false;
    timelimit = 0;
    quiet_mode = 0;
    file_type = FORMAT_DEFAULT;
    sleep_min = 0;
    open_mode = 0;
    stream = SND_PCM_STREAM_PLAYBACK;
    mmap_flag = 0;
    interleaved = 1;
    nonblock = 0;
    audiobuf = NULL;
    chunk_size = 0;
    period_time = 0;
    buffer_time = 0;
    period_frames = 0;
    buffer_frames = 0;
    avail_min = -1;
    start_delay = 0;
    stop_delay = 0;
    verbose = 0;
    buffer_pos = 0;
    log = 0;
    fd = -1;
    pbrec_count = LLONG_MAX;
}

void AlsaPlayer::cleanup()
{
    if (pcm_name) delete pcm_name;
    if (fd >= 0) audiofile.close();
    if (handle) snd_pcm_close(handle);
    if (audiobuf) audioBuffer.resize(0);
    if (log) snd_output_close(log);
    snd_config_update_free_global();
    init();
}

/*
 * Stop playback, cleanup and exit thread.
 */
void AlsaPlayer::stopAndExit()
{
    if (handle) snd_pcm_drop(handle);
    cleanup();
    exit();
}

/*
 * Safe read (for pipes)
 */
 
ssize_t AlsaPlayer::safe_read(int fd, void *buf, size_t count)
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
int AlsaPlayer::test_vocfile(void *buffer)
{
	VocHeader *vp = (VocHeader*)buffer;

	if (!memcmp(vp->magic, VOC_MAGIC_STRING, 20)) {
		vocminor = LE_SHORT(vp->version) & 0xFF;
		vocmajor = LE_SHORT(vp->version) / 256;
		if (LE_SHORT(vp->version) != (0x1233 - LE_SHORT(vp->coded_ver)))
			return -2;	/* coded version mismatch */
		return LE_SHORT(vp->headerlen) - sizeof(VocHeader);	/* 0 mostly */
	}
	return -1;		/* magic string fail */
}

/*
 * helper for test_wavefile
 */

size_t AlsaPlayer::test_wavefile_read(int fd, char *buffer, size_t *size, size_t reqsize, int line)
{
	if (*size >= reqsize)
		return *size;
	if ((size_t)safe_read(fd, buffer + *size, reqsize - *size) != reqsize - *size) {
		error("read error (called from line %i)", line);
		stopAndExit();
	}
	return *size = reqsize;
}

#define check_wavefile_space(buffer, len, blimit) \
	if (len > blimit) { \
		blimit = len; \
		if (((void*)buffer = realloc(buffer, blimit)) == NULL) { \
			error("not enough memory"); \
			stopAndExit(); \
		} \
	}

/*
 * test, if it's a .WAV file, > 0 if ok (and set the speed, stereo etc.)
 *                            == 0 if not
 * Value returned is bytes to be discarded.
 */
ssize_t AlsaPlayer::test_wavefile(int fd, char *_buffer, size_t size)
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
		error("unknown length of 'fmt ' chunk (read %u, should be %u at least)", len, (u_int)sizeof(WaveFmtBody));
		stopAndExit();
	}
	check_wavefile_space(buffer, len, blimit);
	test_wavefile_read(fd, buffer, &size, len, __LINE__);
	f = (WaveFmtBody*) buffer;
	if (LE_SHORT(f->format) != WAV_PCM_CODE) {
		error("can't play not PCM-coded WAVE-files");
		stopAndExit();
	}
	if (LE_SHORT(f->modus) < 1) {
		error("can't play WAVE-files with %d tracks", LE_SHORT(f->modus));
		stopAndExit();
	}
	hwparams.channels = LE_SHORT(f->modus);
	switch (LE_SHORT(f->bit_p_spl)) {
	case 8:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_U8)
			kdDebug() << dbgStr.sprintf( "Warning: format is changed to U8") << endl;
		hwparams.format = SND_PCM_FORMAT_U8;
		break;
	case 16:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_S16_LE)
			kdDebug() << dbgStr.sprintf( "Warning: format is changed to S16_LE") << endl;
		hwparams.format = SND_PCM_FORMAT_S16_LE;
		break;
	case 24:
		switch (LE_SHORT(f->byte_p_spl) / hwparams.channels) {
		case 3:
			if (hwparams.format != DEFAULT_FORMAT &&
			    hwparams.format != SND_PCM_FORMAT_S24_3LE)
				kdDebug() << dbgStr.sprintf( "Warning: format is changed to S24_3LE") << endl;
			hwparams.format = SND_PCM_FORMAT_S24_3LE;
			break;
		case 4:
			if (hwparams.format != DEFAULT_FORMAT &&
			    hwparams.format != SND_PCM_FORMAT_S24_LE)
				kdDebug() << dbgStr.sprintf( "Warning: format is changed to S24_LE") << endl;
			hwparams.format = SND_PCM_FORMAT_S24_LE;
			break;
		default:
			error(" can't play WAVE-files with sample %d bits in %d bytes wide (%d channels)", LE_SHORT(f->bit_p_spl), LE_SHORT(f->byte_p_spl), hwparams.channels);
			stopAndExit();
		}
		break;
	case 32:
		hwparams.format = SND_PCM_FORMAT_S32_LE;
		break;
	default:
		error(" can't play WAVE-files with sample %d bits wide", LE_SHORT(f->bit_p_spl));
		stopAndExit();
	}
	hwparams.rate = LE_INT(f->sample_fq);
	
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

 */

int AlsaPlayer::test_au(int fd, char *buffer)
{
	AuHeader *ap = (AuHeader*)buffer;

	if (ap->magic != AU_MAGIC)
		return -1;
	if (BE_INT(ap->hdr_size) > 128 || BE_INT(ap->hdr_size) < 24)
		return -1;
	pbrec_count = BE_INT(ap->data_size);
	switch (BE_INT(ap->encoding)) {
	case AU_FMT_ULAW:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_MU_LAW)
			kdDebug() << dbgStr.sprintf( "Warning: format is changed to MU_LAW") << endl;
		hwparams.format = SND_PCM_FORMAT_MU_LAW;
		break;
	case AU_FMT_LIN8:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_U8)
			kdDebug() << dbgStr.sprintf( "Warning: format is changed to U8") << endl;
		hwparams.format = SND_PCM_FORMAT_U8;
		break;
	case AU_FMT_LIN16:
		if (hwparams.format != DEFAULT_FORMAT &&
		    hwparams.format != SND_PCM_FORMAT_S16_BE)
			kdDebug() << dbgStr.sprintf( "Warning: format is changed to S16_BE") << endl;
		hwparams.format = SND_PCM_FORMAT_S16_BE;
		break;
	default:
		return -1;
	}
	hwparams.rate = BE_INT(ap->sample_rate);
	if (hwparams.rate < 2000 || hwparams.rate > 256000)
		return -1;
	hwparams.channels = BE_INT(ap->channels);
	if (hwparams.channels < 1 || hwparams.channels > 128)
		return -1;
	if ((size_t)safe_read(fd, buffer + sizeof(AuHeader), BE_INT(ap->hdr_size) - sizeof(AuHeader)) != BE_INT(ap->hdr_size) - sizeof(AuHeader)) {
		error("read error");
		stopAndExit();
	}
	return 0;
}

void AlsaPlayer::set_params(void)
{
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_pcm_uframes_t buffer_size;
	int err;
	size_t n;
	snd_pcm_uframes_t xfer_align;
	unsigned int rate;
	snd_pcm_uframes_t start_threshold;
    snd_pcm_uframes_t stop_threshold;
	snd_pcm_hw_params_alloca(&params);
	snd_pcm_sw_params_alloca(&swparams);
	err = snd_pcm_hw_params_any(handle, params);
	if (err < 0) {
		error("Broken configuration for this PCM: no configurations available");
		stopAndExit();
	}
	if (mmap_flag) {
		snd_pcm_access_mask_t *mask = (snd_pcm_access_mask_t *)alloca(snd_pcm_access_mask_sizeof());
		snd_pcm_access_mask_none(mask);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
		snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
		err = snd_pcm_hw_params_set_access_mask(handle, params, mask);
	} else if (interleaved)
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_INTERLEAVED);
	else
		err = snd_pcm_hw_params_set_access(handle, params,
						   SND_PCM_ACCESS_RW_NONINTERLEAVED);
	if (err < 0) {
		error("Access type not available");
		stopAndExit();
	}
	err = snd_pcm_hw_params_set_format(handle, params, hwparams.format);
	if (err < 0) {
		error("Sample format non available");
		stopAndExit();
	}
	err = snd_pcm_hw_params_set_channels(handle, params, hwparams.channels);
	if (err < 0) {
		error("Channels count non available");
		stopAndExit();
	}

#if 0
	err = snd_pcm_hw_params_set_periods_min(handle, params, 2);
	assert(err >= 0);
#endif
	rate = hwparams.rate;
	err = snd_pcm_hw_params_set_rate_near(handle, params, &hwparams.rate, 0);
	assert(err >= 0);
	if ((float)rate * 1.05 < hwparams.rate || (float)rate * 0.95 > hwparams.rate) {
		if (!quiet_mode) {
			kdDebug() << dbgStr.sprintf( "Warning: rate is not accurate (requested = %iHz, got = %iHz)", rate, hwparams.rate) << endl;
			kdDebug() << dbgStr.sprintf( "         please, try the plug plugin (-Dplug:%s)", snd_pcm_name(handle)) << endl;
		}
	}
	rate = hwparams.rate;
	if (buffer_time == 0 && buffer_frames == 0) {
		err = snd_pcm_hw_params_get_buffer_time_max(params,
							    &buffer_time, 0);
		assert(err >= 0);
		if (buffer_time > 500000)
			buffer_time = 500000;
	}
	if (period_time == 0 && period_frames == 0) {
		if (buffer_time > 0)
			period_time = buffer_time / 4;
		else
			period_frames = buffer_frames / 4;
	}
	if (period_time > 0)
		err = snd_pcm_hw_params_set_period_time_near(handle, params,
							     &period_time, 0);
	else
		err = snd_pcm_hw_params_set_period_size_near(handle, params,
							     &period_frames, 0);
	assert(err >= 0);
	if (buffer_time > 0) {
		err = snd_pcm_hw_params_set_buffer_time_near(handle, params,
							     &buffer_time, 0);
	} else {
		err = snd_pcm_hw_params_set_buffer_size_near(handle, params,
							     &buffer_frames);
	}
	assert(err >= 0);
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		error("Unable to install hw params:");
		snd_pcm_hw_params_dump(params, log);
		stopAndExit();
	}
    canPause = snd_pcm_hw_params_can_pause(params);
	snd_pcm_hw_params_get_period_size(params, &chunk_size, 0);
	snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
	if (chunk_size == buffer_size) {
		error("Can't use period equal to buffer size (%lu == %lu)", chunk_size, buffer_size);
		stopAndExit();
	}
	snd_pcm_sw_params_current(handle, swparams);
	err = snd_pcm_sw_params_get_xfer_align(swparams, &xfer_align);
	if (err < 0) {
		error("Unable to obtain xfer align\n");
		stopAndExit();
	}
	if (sleep_min)
		xfer_align = 1;
	err = snd_pcm_sw_params_set_sleep_min(handle, swparams,
					      sleep_min);
	assert(err >= 0);
	if (avail_min < 0)
		n = chunk_size;
	else
		n = (unsigned int)((double) rate * avail_min / 1000000);
	err = snd_pcm_sw_params_set_avail_min(handle, swparams, n);

	/* round up to closest transfer boundary */
	n = (buffer_size / xfer_align) * xfer_align;
	if (start_delay <= 0) {
		start_threshold = (long unsigned int)(n + (double) rate * start_delay / 1000000);
	} else
		start_threshold = (long unsigned int)((double) rate * start_delay / 1000000);
	if (start_threshold < 1)
		start_threshold = 1;
	if (start_threshold > n)
		start_threshold = n;
	err = snd_pcm_sw_params_set_start_threshold(handle, swparams, start_threshold);
	assert(err >= 0);
	if (stop_delay <= 0) 
		stop_threshold = (long unsigned int)(buffer_size + (double) rate * stop_delay / 1000000);
	else
		stop_threshold = (long unsigned int)((double) rate * stop_delay / 1000000);
	err = snd_pcm_sw_params_set_stop_threshold(handle, swparams, stop_threshold);
	assert(err >= 0);

	err = snd_pcm_sw_params_set_xfer_align(handle, swparams, xfer_align);
	assert(err >= 0);

	if (snd_pcm_sw_params(handle, swparams) < 0) {
		error("unable to install sw params:");
		snd_pcm_sw_params_dump(swparams, log);
		stopAndExit();
	}

	if (verbose)
		snd_pcm_dump(handle, log);

	bits_per_sample = snd_pcm_format_physical_width(hwparams.format);
	bits_per_frame = bits_per_sample * hwparams.channels;
	chunk_bytes = chunk_size * bits_per_frame / 8;
    audioBuffer.resize(chunk_bytes);
    audiobuf = audioBuffer.data();
	if (audiobuf == NULL) {
		error("not enough memory");
		stopAndExit();
	}
	// fprintf(stderr, "real chunk_size = %i, frags = %i, total = %i\n", chunk_size, setup.buf.block.frags, setup.buf.block.frags * chunk_size);
}

#ifndef timersub
#define	timersub(a, b, result) \
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
void AlsaPlayer::xrun(void)
{
	snd_pcm_status_t *status;
	int res;
	
	snd_pcm_status_alloca(&status);
	if ((res = snd_pcm_status(handle, status))<0) {
		error("status error: %s", snd_strerror(res));
		stopAndExit();
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {
		struct timeval now, diff, tstamp;
		gettimeofday(&now, 0);
		snd_pcm_status_get_trigger_tstamp(status, &tstamp);
		timersub(&now, &tstamp, &diff);
		kdDebug() << dbgStr.sprintf( "%s!!! (at least %.3f ms long)",
			stream == SND_PCM_STREAM_PLAYBACK ? "underrun" : "overrun",
			diff.tv_sec * 1000 + diff.tv_usec / 1000.0) << endl;
		if (verbose) {
			kdDebug() << dbgStr.sprintf( "Status:") << endl;
			snd_pcm_status_dump(status, log);
		}
		if ((res = snd_pcm_prepare(handle))<0) {
			error("xrun: prepare error: %s", snd_strerror(res));
			stopAndExit();
		}
		return;		/* ok, data should be accepted again */
	} if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
		if (verbose) {
			kdDebug() << dbgStr.sprintf( "Status(DRAINING):") << endl;
			snd_pcm_status_dump(status, log);
		}
		if (stream == SND_PCM_STREAM_CAPTURE) {
			kdDebug() << dbgStr.sprintf( "capture stream format change? attempting recover...") << endl;
			if ((res = snd_pcm_prepare(handle))<0) {
				error("xrun(DRAINING): prepare error: %s", snd_strerror(res));
				stopAndExit();
			}
			return;
		}
	}
	if (verbose) {
		kdDebug() << dbgStr.sprintf( "Status(R/W):") << endl;
		snd_pcm_status_dump(status, log);
	}
	error("read/write error, state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)));
	stopAndExit();
}

/* I/O suspend handler */
void AlsaPlayer::suspend(void)
{
	int res;

	if (!quiet_mode)
		kdDebug() << dbgStr.sprintf( "Suspended. Trying resume. ") << endl;
	while ((res = snd_pcm_resume(handle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
		if (!quiet_mode)
			kdDebug() << dbgStr.sprintf( "Failed. Restarting stream. ") << endl;
		if ((res = snd_pcm_prepare(handle)) < 0) {
			error("suspend: prepare error: %s", snd_strerror(res));
			stopAndExit();
		}
	}
	if (!quiet_mode)
		kdDebug() << dbgStr.sprintf( "Done.") << endl;
}

/* peak handler */
void AlsaPlayer::compute_max_peak(char *data, size_t count)
{
	signed int val, max, max_peak = 0, perc;
	size_t ocount = count;
	
	switch (bits_per_sample) {
	case 8: {
		signed char *valp = (signed char *)data;
		signed char mask = snd_pcm_format_silence(hwparams.format);
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
		signed short mask = snd_pcm_format_silence_16(hwparams.format);
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
		signed int mask = snd_pcm_format_silence_32(hwparams.format);
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
	kdDebug() << dbgStr.sprintf("Max peak (%li samples): %05i (0x%04x) ", (long)ocount, max_peak, max_peak);
	if (bits_per_sample > 16)
		perc = max_peak / (max / 100);
	else
		perc = max_peak * 100 / max;
	for (val = 0; val < 20; val++)
		if (val <= perc / 5)
			kdDebug() << '#';
		else
            kdDebug() << ' ';
	kdDebug() << dbgStr.sprintf(" %i%%", perc) << endl;
}

/*
 *  write function
 */

ssize_t AlsaPlayer::pcm_write(char *data, size_t count)
{
	ssize_t r;
	ssize_t result = 0;

	if (sleep_min == 0 &&
	    count < chunk_size) {
		snd_pcm_format_set_silence(hwparams.format, data + count * bits_per_frame / 8, (chunk_size - count) * hwparams.channels);
		count = chunk_size;
	}
	while (count > 0) {
		r = writei_func(handle, data, count);
		if (r == -EAGAIN || (r >= 0 && (size_t)r < count)) {
			snd_pcm_wait(handle, 1000);
		} else if (r == -EPIPE) {
			xrun();
		} else if (r == -ESTRPIPE) {
			suspend();
		} else if (r < 0) {
			error("write error: %s", snd_strerror(r));
			stopAndExit();
		}
		if (r > 0) {
			if (verbose > 1)
				compute_max_peak(data, r * hwparams.channels);
			result += r;
			count -= r;
			data += r * bits_per_frame / 8;
		}
	}
	return result;
}

/*
 *  ok, let's play a .voc file
 */

ssize_t AlsaPlayer::voc_pcm_write(u_char *data, size_t count)
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

void AlsaPlayer::voc_write_silence(unsigned x)
{
	unsigned l;
	char *buf;

    QByteArray buffer(chunk_bytes);
	// buf = (char *) malloc(chunk_bytes);
    buf = buffer.data();
	if (buf == NULL) {
		error("can't allocate buffer for silence");
		return;		/* not fatal error */
	}
	snd_pcm_format_set_silence(hwparams.format, buf, chunk_size * hwparams.channels);
	while (x > 0) {
		l = x;
		if (l > chunk_size)
			l = chunk_size;
		if (voc_pcm_write((u_char*)buf, l) != (ssize_t)l) {
			error("write error");
			stopAndExit();
		}
		x -= l;
	}
	// free(buf);
}

void AlsaPlayer::voc_pcm_flush(void)
{
	if (buffer_pos > 0) {
		size_t b;
		if (sleep_min == 0) {
			if (snd_pcm_format_set_silence(hwparams.format, audiobuf + buffer_pos, chunk_bytes - buffer_pos * 8 / bits_per_sample) < 0)
				kdDebug() << dbgStr.sprintf( "voc_pcm_flush - silence error") << endl;
			b = chunk_size;
		} else {
			b = buffer_pos * 8 / bits_per_frame;
		}
		if (pcm_write(audiobuf, b) != (ssize_t)b)
			error("voc_pcm_flush error");
	}
	snd_pcm_drain(handle);
}

void AlsaPlayer::voc_play(int fd, int ofs, const char* name)
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

#define COUNT(x)	nextblock -= x; in_buffer -= x; data += x
#define COUNT1(x)	in_buffer -= x; data += x

    QByteArray buffer(64 * 1024);
	// data = buf = (u_char *)malloc(64 * 1024);
    data = buf = (u_char*)buffer.data();
	buffer_pos = 0;
	if (data == NULL) {
		error("malloc error");
		stopAndExit();
	}
	if (!quiet_mode) {
		kdDebug() << dbgStr.sprintf( "Playing Creative Labs Channel file '%s'...", name) << endl;
	}
	/* first we waste the rest of header, ugly but we don't need seek */
	while (ofs > (ssize_t)chunk_bytes) {
		if ((size_t)safe_read(fd, buf, chunk_bytes) != chunk_bytes) {
			error("read error");
			stopAndExit();
		}
		ofs -= chunk_bytes;
	}
	if (ofs) {
		if (safe_read(fd, buf, ofs) != ofs) {
			error("read error");
			stopAndExit();
		}
	}
	hwparams.format = DEFAULT_FORMAT;
	hwparams.channels = 1;
	hwparams.rate = DEFAULT_SPEED;
	set_params();

	in_buffer = nextblock = 0;
	while (1) {
	      Fill_the_buffer:	/* need this for repeat */
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
//					perror(name);
					stopAndExit();
				}
			}
		}
		while (!nextblock) {	/* this is a new block */
			if (in_buffer < sizeof(VocBlockType))
				goto __end;
			bp = (VocBlockType *) data;
			COUNT1(sizeof(VocBlockType));
			nextblock = VOC_DATALEN(bp);
			if (output && !quiet_mode)
				kdDebug() << endl;  /* write /n after ASCII-out */
			output = 0;
			switch (bp->type) {
			case 0:
#if 0
				kdDebug() << "Terminator" << endl;
#endif
				return;		/* VOC-file stop */
			case 1:
				vd = (VocVoiceData *) data;
				COUNT1(sizeof(VocVoiceData));
				/* we need a SYNC, before we can set new SPEED, STEREO ... */

				if (!was_extended) {
					hwparams.rate = (int) (vd->tc);
					hwparams.rate = 1000000 / (256 - hwparams.rate);
#if 0
					kdDebug() << dbgStr.sprintf("Channel data %d Hz", dsp_speed) << endl;
#endif
					if (vd->pack) {		/* /dev/dsp can't it */
						error("can't play packed .voc files");
						return;
					}
					if (hwparams.channels == 2)		/* if we are in Stereo-Mode, switch back */
						hwparams.channels = 1;
				} else {	/* there was extended block */
					hwparams.channels = 2;
					was_extended = 0;
				}
				set_params();
				break;
			case 2:	/* nothing to do, pure data */
#if 0
				kdDebug() << "Channel continuation") << endl;
#endif
				break;
			case 3:	/* a silence block, no data, only a count */
				sp = (u_short *) data;
				COUNT1(sizeof(u_short));
				hwparams.rate = (int) (*data);
				COUNT1(1);
				hwparams.rate = 1000000 / (256 - hwparams.rate);
				set_params();
				silence = (((size_t) * sp) * 1000) / hwparams.rate;
#if 0
				kdDebug() << dbgStr.sprintf("Silence for %d ms", (int) silence) << endl;
#endif
				voc_write_silence(*sp);
				break;
			case 4:	/* a marker for syncronisation, no effect */
				sp = (u_short *) data;
				COUNT1(sizeof(u_short));
#if 0
				kdDebug() << dbgStr.sprintf("Marker %d", *sp) << endl;
#endif
				break;
			case 5:	/* ASCII text, we copy to stderr */
				output = 1;
#if 0
				kdDebug() << "ASCII - text :") << endl;
#endif
				break;
			case 6:	/* repeat marker, says repeatcount */
				/* my specs don't say it: maybe this can be recursive, but
				   I don't think somebody use it */
				repeat = *(u_short *) data;
				COUNT1(sizeof(u_short));
#if 0
				kdDebug() << dbgStr.sprintf("Repeat loop %d times", repeat) << endl;
#endif
				if (filepos >= 0) {	/* if < 0, one seek fails, why test another */
					if ((filepos = lseek64(fd, 0, 1)) < 0) {
						error("can't play loops; %s isn't seekable\n", name);
						repeat = 0;
					} else {
						filepos -= in_buffer;	/* set filepos after repeat */
					}
				} else {
					repeat = 0;
				}
				break;
			case 7:	/* ok, lets repeat that be rewinding tape */
				if (repeat) {
					if (repeat != 0xFFFF) {
#if 0
						kdDebug() << dbgStr.sprintf("Repeat loop %d", repeat) << endl;
#endif
						--repeat;
					}
#if 0
					else
						kdDebug() << "Neverending loop" << endl;
#endif
					lseek64(fd, filepos, 0);
					in_buffer = 0;	/* clear the buffer */
					goto Fill_the_buffer;
				}
#if 0
				else
					kdDebug() << "End repeat loop" << endl;
#endif
				break;
			case 8:	/* the extension to play Stereo, I have SB 1.0 :-( */
				was_extended = 1;
				eb = (VocExtBlock *) data;
				COUNT1(sizeof(VocExtBlock));
				hwparams.rate = (int) (eb->tc);
				hwparams.rate = 256000000L / (65536 - hwparams.rate);
				hwparams.channels = eb->mode == VOC_MODE_STEREO ? 2 : 1;
				if (hwparams.channels == 2)
					hwparams.rate = hwparams.rate >> 1;
				if (eb->pack) {		/* /dev/dsp can't it */
					error("can't play packed .voc files");
					return;
				}
#if 0
				kdDebug() << dbgStr.sprintf("Extended block %s %d Hz",
					 (eb->mode ? "Stereo" : "Mono"), dsp_speed) << endl;
#endif
				break;
			default:
				error("unknown blocktype %d. terminate.", bp->type);
				return;
			}	/* switch (bp->type) */
		}		/* while (! nextblock)  */
		/* put nextblock data bytes to dsp */
		l = in_buffer;
		if (nextblock < (size_t)l)
			l = nextblock;
		if (l) {
			if (output && !quiet_mode) {
				if (write(2, data, l) != l) {	/* to stderr */
					error("write error");
					stopAndExit();
				}
			} else {
				if (voc_pcm_write(data, l) != l) {
					error("write error");
					stopAndExit();
				}
			}
			COUNT(l);
		}
	}			/* while(1) */
      __end:
        voc_pcm_flush();
        // free(buf);
}
/* that was a big one, perhaps somebody split it :-) */

/* setting the globals for playing raw data */
void AlsaPlayer::init_raw_data(void)
{
	hwparams = rhwparams;
}

/* calculate the data count to read from/to dsp */
off64_t AlsaPlayer::calc_count(void)
{
	off64_t count;

	if (timelimit == 0) {
		count = pbrec_count;
	} else {
		count = snd_pcm_format_size(hwparams.format, hwparams.rate * hwparams.channels);
		count *= (off64_t)timelimit;
	}
	return count < pbrec_count ? count : pbrec_count;
}

void AlsaPlayer::header(int /*rtype*/, const char* /*name*/)
{
	if (!quiet_mode) {
//		fprintf(stderr, "%s %s '%s' : ",
//			(stream == SND_PCM_STREAM_PLAYBACK) ? "Playing" : "Recording",
//			fmt_rec_table[rtype].what,
//			name);
		QString s = dbgStr.sprintf( "%s, ", snd_pcm_format_description(hwparams.format));
		s += dbgStr.sprintf( "Rate %d Hz, ", hwparams.rate);
		if (hwparams.channels == 1)
			s += dbgStr.sprintf( "Mono");
		else if (hwparams.channels == 2)
			s += dbgStr.sprintf( "Stereo");
		else
			s += dbgStr.sprintf( "Channels %i", hwparams.channels);
		kdDebug() << s << endl;
	}
}

/* playing raw data */

void AlsaPlayer::playback_go(int fd, size_t loaded, off64_t count, int rtype, const char *name)
{
	int l, r;
	off64_t written = 0;
	off64_t c;

	header(rtype, name);
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
//				perror(name);
				stopAndExit();
			}
			fdcount += r;
			if (r == 0)
				break;
			l += r;
		} while (sleep_min == 0 && (size_t)l < chunk_bytes);
		l = l * 8 / bits_per_frame;
		r = pcm_write(audiobuf, l);
		if (r != l)
			break;
		r = r * bits_per_frame / 8;
		written += r;
		l = 0;
	}
	snd_pcm_drain(handle);
    kdDebug() << "Exiting playback_go" << endl;
}

/*
 *  let's play or capture it (capture_type says VOC/WAVE/raw)
 */

void AlsaPlayer::playback(int fd)
{
	int ofs;
	size_t dta;
	ssize_t dtawave;

	pbrec_count = LLONG_MAX;
	fdcount = 0;

	/* read the file header */
	dta = sizeof(AuHeader);
	if ((size_t)safe_read(fd, audiobuf, dta) != dta) {
		error("read error");
		stopAndExit();
	}
	if (test_au(fd, audiobuf) >= 0) {
		rhwparams.format = hwparams.format;
		pbrec_count = calc_count();
		playback_go(fd, 0, pbrec_count, FORMAT_AU, name.ascii());
		goto __end;
	}
	dta = sizeof(VocHeader);
	if ((size_t)safe_read(fd, audiobuf + sizeof(AuHeader),
		 dta - sizeof(AuHeader)) != dta - sizeof(AuHeader)) {
		error("read error");
		stopAndExit();
	}
	if ((ofs = test_vocfile(audiobuf)) >= 0) {
		pbrec_count = calc_count();
		voc_play(fd, ofs, name.ascii());
		goto __end;
	}
	/* read bytes for WAVE-header */
	if ((dtawave = test_wavefile(fd, audiobuf, dta)) >= 0) {
		pbrec_count = calc_count();
		playback_go(fd, dtawave, pbrec_count, FORMAT_WAVE, name.ascii());
	} else {
		/* should be raw data */
		init_raw_data();
		pbrec_count = calc_count();
		playback_go(fd, dta, pbrec_count, FORMAT_RAW, name.ascii());
	}
      __end:
        return;
}

#include "alsaplayer.moc"

// vim: sw=4 ts=8 et
