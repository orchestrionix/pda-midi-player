////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2002 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

#include "common\PsConfig.h"

#ifdef __cplusplus
extern "C" {
#endif


// WavPack header format (this occurs immediately after the RIFF header in a
// standard WavPack file, or as the very first thing in a "raw" WavPack file)

typedef struct {
    int ckSize;
    short version;
    short bits;			// added for version 2.00
    short flags, shift;		// added for version 3.00
    long total_samples, crc, crc2;
} WavpackHeader;

// these flags added for version 3

#define MONO_ONLY
//#define DEBUG_WVPACK

#define MONO_FLAG	1	// not stereo
#define FAST_FLAG	2	// non-adaptive predictor and stereo mode
#define RAW_FLAG	4	// raw mode (no .wav header)
#define CALC_NOISE	8	// calc noise in lossy mode (no longer stored)
#define HIGH_FLAG	0x10	// high quality mode (all modes)
#define BYTES_3		0x20	// files have 3-byte samples
#define OVER_20		0x40	// samples are over 20 bits
#define WVC_FLAG	0x80	// create/use .wvc (no longer stored)
#define LOSSY_SHAPE	0x100	// noise shape (lossy mode only)
#define VERY_FAST_FLAG	0x200	// double fast (no longer stored)
#define NEW_HIGH_FLAG	0x400	// new high quality mode (lossless only)
#define CANCEL_EXTREME	0x800	// cancel EXTREME_DECORR
#define CROSS_DECORR	0x1000	// decorrelate chans (with EXTREME_DECORR flag)
#define NEW_DECORR_FLAG	0x2000	// new high-mode decorrelator
#define JOINT_STEREO	0x4000	// joint stereo (lossy and high lossless)
#define EXTREME_DECORR	0x8000	// extra decorrelation (+ enables other flags)

#define UNKNOWN_FLAGS	0x0000	// none of those left  :(
#define STORED_FLAGS	0xfd77	// these are only flags that affect unpacking

#define NOT_STORED_FLAGS (~STORED_FLAGS & 0xffff)

#define SHAPE_OVERRIDE	0x10000	// shaping mode specified
#define JOINT_OVERRIDE	0x20000	// joint-stereo mode specified
#define COPY_TIME	0x40000	// copy file-time from source
#define CREATE_EXE	0x80000	// create executable

// misc macros

#define BYTES_PER_SAMPLE ((flags & BYTES_3) ? ((flags & MONO_FLAG) ? 3 : 6) : ((flags & MONO_FLAG) ? 2 : 4))

#if defined OS_PALM
	#define CLEAR(destin) MemSet(&destin, sizeof (destin), 0);
	#define MEMCPY(a, b, c) MemMove(a, b, c);
#elif defined OS_SYMBIAN
	#define CLEAR(destin) Mem::FillZ(&destin, sizeof(destin));
	#define MEMCPY(a, b, c) Mem::Copy(a, b, c);
#else
	#define CLEAR(destin) memset(&destin, 0, sizeof (destin));
	#define MEMCPY(a, b, c) memcpy(a, b, c);
#endif

//#define SAVE(destin, item) { CPsSys::memcpy (destin, &item, sizeof (item)); (char *) destin += sizeof (item); }
//#define RESTORE(item, source) { CPsSys::memcpy (&item, source, sizeof (item)); (char *) source += sizeof (item); }


typedef struct {
    unsigned char *buf, *end, *ptr;
    unsigned long bufsiz;
    unsigned int bc, sr;
} Bitstream;

int bs_open_read (Bitstream *bs, void* buf, int len);
int bs_open_write (Bitstream *bs, void* buf, int len);

void bs_close_read (Bitstream *bs);
void bs_close_write (Bitstream *bs);

// macros used with BitStreams to read and write bits

#define getbit(bs) ( \
    (((bs)->bc) ? \
	((bs)->bc--, (bs)->sr & 1) : \
	    (((++((bs)->ptr) != (bs)->end) ? (void) 0 : (void) 0 ), (bs)->bc = 7, ((bs)->sr = *((bs)->ptr)) & 1) \
    ) ? \
	((bs)->sr >>= 1, 1) : \
	((bs)->sr >>= 1, 0) \
)

#define getbits(value, nbits, bs) { \
    while ((nbits) > (bs)->bc) { \
	++(bs)->ptr;\
	(bs)->sr |= (long)*((bs)->ptr) << (bs)->bc; \
	(bs)->bc += 8; \
    } \
    *(value) = (bs)->sr; \
    (bs)->sr >>= (nbits); \
    (bs)->bc -= (nbits); \
}

#define putbit(bit, bs) { if (bit) (bs)->sr |= (1 << (bs)->bc); \
    if (++((bs)->bc) == 8) { \
	*((bs)->ptr) = (bs)->sr; \
	(bs)->sr = (bs)->bc = 0; \
	++((bs)->ptr);\
    }}

#define putbit_0(bs) { \
    if (++((bs)->bc) == 8) { \
	*((bs)->ptr) = (bs)->sr; \
	(bs)->sr = (bs)->bc = 0; \
	++((bs)->ptr);\
    }}

#define putbit_1(bs) { (bs)->sr |= (1 << (bs)->bc); \
    if (++((bs)->bc) == 8) { \
	*((bs)->ptr) = (bs)->sr; \
	(bs)->sr = (bs)->bc = 0; \
	++((bs)->ptr);\
    }}

#define putbits(value, nbits, bs) { \
    (bs)->sr |= (long)(value) << (bs)->bc; \
    if (((bs)->bc += (nbits)) >= 8) \
	do { \
	    *((bs)->ptr) = (bs)->sr; \
	    (bs)->sr >>= 8; \
		++((bs)->ptr);\
	} while (((bs)->bc -= 8) >= 8); \
}

#define K_DEPTH 3
#define MAX_NTERMS 18
#define MAX_TERM 8

struct decorr_pass {
	int term, weight_A, weight_B;
	long samples_A [MAX_TERM], samples_B [MAX_TERM];
	long aweight_A, aweight_B;
};

typedef struct {

    WavpackHeader *wphdr;

    Bitstream inbits, outbits;

    struct {
	double noise_sum, noise_ave, noise_max;
	long sum_level, left_level, right_level, diff_level;
	int last_extra_bits, extra_bits_count, m;
	long error [2];
	long sample [2] [5], dsample [2], csample [2];
	int weight [2] [5], cweight [4];
    } dc;

    struct decorr_pass decorr_passes [MAX_NTERMS + 1];

    struct {
	unsigned long fast_level [2], slow_level [2];
	int bits_acc [2], bitrate;
    } w4;

} WavpackContext;

// pack.c and unpack.c stuff

void pack_init (WavpackContext *wpc);
void pack_samples (WavpackContext *wpc, void *buffer, unsigned int sample_count);
double pack_noise (WavpackContext *wpc, double *peak);

void unpack_init (WavpackContext *wpc);
long unpack_samples (WavpackContext *wpc, void *buffer, unsigned int sample_count);
int unpack_size (WavpackContext *wpc);
void *unpack_save (WavpackContext *wpc, void *destin);
void *unpack_restore (WavpackContext *wpc, void *source, int keep_resources);
long unpack_crc (WavpackContext *wpc);

void init_word4 (WavpackContext *wpc);
long send_word4 (WavpackContext *wpc, long value, int chan);
long get_word4 (WavpackContext *wpc, int chan);

void wavpack16(void* pDes, unsigned int *nDes, const void *pSrc, unsigned int sample_count, int bits, int isMono);
int get_unpacked_size(void *pSrc);
void wavunpack16(void* pDes, const void *pSrc, unsigned int len);

#define WORD_EOF (1L << 31)
#ifdef  __cplusplus
}
#endif
