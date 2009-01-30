/***************************************************** vim:set ts=4 sw=4 sts=4:
  Formats.
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

#ifndef FORMATS_H
#define FORMATS_H		1

#include <endian.h>
#include <byteswap.h>

/* Definitions for .VOC files */

#define VOC_MAGIC_STRING	"Creative Voice File\x1A"
#define VOC_ACTUAL_VERSION	0x010A
#define VOC_SAMPLESIZE		8

#define VOC_MODE_MONO		0
#define VOC_MODE_STEREO		1

#define VOC_DATALEN(bp)		((u_long)(bp->datalen) | \
                         	((u_long)(bp->datalen_m) << 8) | \
                         	((u_long)(bp->datalen_h) << 16) )

typedef struct voc_header {
	uchar magic[20];	/* must be MAGIC_STRING */
	ushort headerlen;	/* Headerlength, should be 0x1A */
	ushort version;	/* VOC-file version */
	ushort coded_ver;	/* 0x1233-version */
} VocHeader;

typedef struct voc_blocktype {
	uchar type;
	uchar datalen;		/* low-byte    */
	uchar datalen_m;	/* medium-byte */
	uchar datalen_h;	/* high-byte   */
} VocBlockType;

typedef struct voc_voice_data {
	uchar tc;
	uchar pack;
} VocVoiceData;

typedef struct voc_ext_block {
	ushort tc;
	uchar pack;
	uchar mode;
} VocExtBlock;

/* Definitions for Microsoft WAVE format */

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((a) | ((b)<<8) | ((c)<<16) | ((d)<<24))
#define LE_SHORT(v)		(v)
#define LE_INT(v)		(v)
#define BE_SHORT(v)		bswap_16(v)
#define BE_INT(v)		bswap_32(v)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define COMPOSE_ID(a,b,c,d)	((d) | ((c)<<8) | ((b)<<16) | ((a)<<24))
#define LE_SHORT(v)		bswap_16(v)
#define LE_INT(v)		bswap_32(v)
#define BE_SHORT(v)		(v)
#define BE_INT(v)		(v)
#else
#error "Wrong endian"
#endif

#define WAV_RIFF		COMPOSE_ID('R','I','F','F')
#define WAV_WAVE		COMPOSE_ID('W','A','V','E')
#define WAV_FMT			COMPOSE_ID('f','m','t',' ')
#define WAV_DATA		COMPOSE_ID('d','a','t','a')
#define WAV_PCM_CODE		1

/* it's in chunks like .voc and AMIGA iff, but my source say there
   are in only in this combination, so I combined them in one header;
   it works on all WAVE-file I have
 */
typedef struct {
	uint magic;		/* 'RIFF' */
	uint length;		/* filelen */
	uint type;		/* 'WAVE' */
} WaveHeader;

typedef struct {
	ushort format;		/* should be 1 for PCM-code */
	ushort modus;		/* 1 Mono, 2 Stereo */
	uint sample_fq;	/* frequence of sample */
	uint byte_p_sec;
	ushort byte_p_spl;	/* samplesize; 1 or 2 bytes */
	ushort bit_p_spl;	/* 8, 12 or 16 bit */
} WaveFmtBody;

typedef struct {
	uint type;		/* 'data' */
	uint length;		/* samplecount */
} WaveChunkHeader;

/* Definitions for Sparc .au header */

#define AU_MAGIC		COMPOSE_ID('.','s','n','d')

#define AU_FMT_ULAW		1
#define AU_FMT_LIN8		2
#define AU_FMT_LIN16		3

typedef struct au_header {
	uint magic;		/* '.snd' */
	uint hdr_size;		/* size of header (min 24) */
	uint data_size;	/* size of data */
	uint encoding;		/* see to AU_FMT_XXXX */
	uint sample_rate;	/* sample rate */
	uint channels;		/* number of channels (voices) */
} AuHeader;

#endif				/* FORMATS */
