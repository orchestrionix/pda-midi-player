////////////////////////////////////////////////////////////////////////////
//			     **** WAVPACK ****				  //
//		    Hybrid Lossless Wavefile Compressor			  //
//		Copyright (c) 1998 - 2002 Conifer Software.		  //
//			    All Rights Reserved.			  //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// unpack.c

// This module actually handles the decompression of the audio data, except
// for the entropy decoding which is handled by the words? modules. For
// maximum efficiency, the conversion is isolated to tight loops that handle
// an entire buffer. The actual bitstreams are "inbits" for the main WavPack
// file and "in2bits" for the .wvc file (if present) and these must be
// initialized with bs_open_read() before unpack_samples() is called.
//
// Functions are provided to save (and restore) the complete unpacking context
// to (and from) another buffer. This may be used (as is done in the winamp
// plugin) to store index points during decoding for rapid seeking during
// playback. The exact format of these context images are not as small as
// possible (although they are much smaller than simply storing everything)
// and are not guaranteed to remain the same from different versions (or even
// compiles). Therefore, they should only be used temporarily and not stored
// to HD.
//
// The "COMPACT" define specifies an alternate version for the default lossless
// mode that uses less inline code but is somewhat slower. Use at your own
// discretion.

#include "wavpack.h"

//#include <stdlib.h>


//////////////////////////////// local macros /////////////////////////////////

#define PUT_24(ptr,val) (*((unsigned char*)ptr)=val,*(short*)((unsigned char*)(ptr)+1)=(val)>>8)

#define apply_weight(bits, weight, sample) ((weight * sample + (1 << (bits - 1))) >> bits)

#define update_weight(bits, weight, source, result) \
    if (source && result) { \
	if ((source ^ result) >= 0) { if (weight++ == (1 << bits)) weight--; } \
	else if (weight-- == min_weight) weight++; \
    }

#define apply_weight24(weight, sample) ((sample >= 0x800000 || sample < -0x800000) ? ((long)floor(((double) weight * sample + 128.0) / 256.0)) : ((weight * sample + 128) >> 8))

#define update_weight2(weight, source, result) \
    if (source && result) { \
	if ((source ^ result) >= 0) { if (weight++ == 256) weight--; } \
	else if (weight-- == min_weight) weight++; \
    }

//////////////////////////////// local tables ///////////////////////////////

// These three tables specify the characteristics of the decorrelation filters.
// Each term represents one layer of the sequential filter, where positive
// values indicate the relative sample involved from the same channel (1=prev)
// while -1 and -2 indicate cross channel decorrelation (in stereo only). The
// "simple_terms" table is no longer used for writing, but is kept for older
// file decoding.

static const signed char extreme_terms [] = { 1,1,1,2,4,-1,1,2,3,6,-2,8,5,7,4,1,2,3 };
static const signed char default_terms [] = { 1,1,1,-1,2,1,-2 };
static const char simple_terms []  = { 1,1,1,1 };

//////////////////////////////////////////////////////////////////////////////
// This function initializes everything required to unpack WavPack          //
// bitstreams and must be called before any unpacking is performed. Note    //
// that the (WavpackHeader *) in the WavpackContext struct must be valid.   //
//////////////////////////////////////////////////////////////////////////////

void unpack_init (WavpackContext *wpc)
{
    int flags = wpc->wphdr->flags;
    struct decorr_pass *dpp;
    int ti;

    CLEAR (wpc->decorr_passes);
    CLEAR (wpc->dc);

    if (flags & EXTREME_DECORR) {
	for (dpp = wpc->decorr_passes, ti = 0; ti < sizeof (extreme_terms); ti++)
	    if (extreme_terms [sizeof (extreme_terms) - ti - 1] > 0 || (flags & CROSS_DECORR))
		dpp++->term = extreme_terms [sizeof (extreme_terms) - ti - 1];
    }
    else if (flags & NEW_DECORR_FLAG) {
	for (dpp = wpc->decorr_passes, ti = 0; ti < sizeof (default_terms); ti++)
	    if (default_terms [sizeof (default_terms) - ti - 1] > 0 || (flags & CROSS_DECORR))
		dpp++->term = default_terms [sizeof (default_terms) - ti - 1];
    }
    else
	for (dpp = wpc->decorr_passes, ti = 0; ti < sizeof (simple_terms); dpp++, ti++)
	    dpp->term = simple_terms [sizeof (simple_terms) - ti - 1];

    init_word4 (wpc);
}

///////////////////////////////////////////////////////////////////////////////
// This monster actually unpacks the WavPack bitstream(s) into the specified //
// buffer as either 16-bit or 24-bit values. The function unpack_init() must //
// have been called and the bitstreams must have already been opened with    //
// bs_open_read(). For maximum clarity, the function is broken up into       //
// segments that handle various modes. This makes for a few extra infrequent //
// flag checks, but makes the code easier to follow because the nesting does //
// not become so deep. For maximum efficiency, the conversion is isolated to //
// tight loops that handle an entire buffer.                                 //
///////////////////////////////////////////////////////////////////////////////

long unpack_samples (WavpackContext *wpc, void *buffer, unsigned int sample_count)
{
    int shift = wpc->wphdr->shift, flags = wpc->wphdr->flags, min_weight = 0, m = wpc->dc.m;
    long min_value, max_value;
    struct decorr_pass *dpp;
    long read_word;
    short *bptr;
    unsigned int i = 0, j;

    long sample [2] [5], dsample [2], csample [2];
    int weight [2] [5], cweight [4];

    MEMCPY (sample, wpc->dc.sample, sizeof (sample));
    MEMCPY (dsample, wpc->dc.dsample, sizeof (dsample));
    MEMCPY (csample, wpc->dc.csample, sizeof (csample));
    MEMCPY (weight, wpc->dc.weight, sizeof (weight));
    MEMCPY (cweight, wpc->dc.cweight, sizeof (cweight));

    if (wpc->wphdr->bits) {
	if (flags & (NEW_DECORR_FLAG | EXTREME_DECORR))
	    min_weight = -256;
    }
    else
	if (flags & NEW_DECORR_FLAG)
	    min_weight = (flags & EXTREME_DECORR) ? -512 : -256;

	min_value = -32768 >> shift;
	max_value = 32767 >> shift;

    //////////////// handle version 3 lossy/hybrid mono data //////////////////

    if (wpc->wphdr->version == 3 && wpc->wphdr->bits && (flags & MONO_FLAG)) {
		if (flags & (HIGH_FLAG | NEW_HIGH_FLAG))
			for (bptr = (short*)buffer, i = 0; i < sample_count; ++i) {
				long temp;
				
				read_word = get_word4 (wpc, 0);

#ifdef DEBUG_WVPACK
				if (read_word == WORD_EOF)
				{
					temp = 1 / (read_word - read_word);
					break;
				}
#endif

				for (dpp = wpc->decorr_passes; dpp->term; dpp++) {
					long sam = dpp->samples_A [m];
					
					temp = apply_weight (8, dpp->weight_A, sam) + read_word;
					update_weight2 (dpp->weight_A, sam, read_word);
					dpp->samples_A [(m + dpp->term) & (MAX_TERM - 1)] = read_word = temp;
				}
				
				m = (m + 1) & (MAX_TERM - 1);
				
				if (read_word < min_value)
					*bptr = min_value;
				else if (read_word > max_value)
					*bptr = max_value;
				else
					*bptr = read_word;
				bptr++;
			}
			
			if (shift)
				for (bptr = (short*)buffer, j = 0; j < i; ++j)
					*bptr++ <<= shift;
    }

    //////////////// handle version 3 lossy/hybrid stereo data ////////////////
#ifndef MONO_ONLY
    else if (wpc->wphdr->version == 3 && wpc->wphdr->bits && !(flags & MONO_FLAG)) {
	if (flags & (HIGH_FLAG | NEW_HIGH_FLAG))
	    for (bptr = (short*)buffer, i = 0; i < sample_count; ++i) {
		long left, right, left2, right2, sum, diff;

	    left = get_word4 (wpc, 0);
	    right = get_word4 (wpc, 1);

		if (left == WORD_EOF)
		    break;

		    for (dpp = wpc->decorr_passes; dpp->term; dpp++) {
			long sam_A = dpp->samples_A [m], sam_B = dpp->samples_B [m];
			int k = (m + dpp->term) & (MAX_TERM - 1);

			left2 = apply_weight (8, dpp->weight_A, sam_A) + left;
			update_weight2 (dpp->weight_A, sam_A, left);
			dpp->samples_A [k] = left = left2;

			right2 = apply_weight (8, dpp->weight_B, sam_B) + right;
			update_weight2 (dpp->weight_B, sam_B, right);
			dpp->samples_B [k] = right = right2;
		    }

		m = (m + 1) & (MAX_TERM - 1);

		if (flags & JOINT_STEREO) {
		    right = ((sum = (right << 1) | (left & 1)) - (diff = left)) >> 1;
		    left = (sum + diff) >> 1;
		}

		    if (left < min_value)
			*bptr = min_value;
		    else if (left > max_value)
			*bptr = max_value;
		    else
			*bptr = left;
			++bptr;

		    if (right < min_value)
			*bptr = min_value;
		    else if (right > max_value)
			*bptr = max_value;
		    else
			*bptr = right;
			++bptr;
	    }

	if (shift)
	    for (j = 0; j < i; ++j) {
		*((short*) bptr)++ <<= shift;
		*((short*) bptr)++ <<= shift;
	    }
    }
#endif
	
    MEMCPY (wpc->dc.sample, sample, sizeof (sample));
    MEMCPY (wpc->dc.dsample, dsample, sizeof (dsample));
    MEMCPY (wpc->dc.csample, csample, sizeof (csample));
    MEMCPY (wpc->dc.weight, weight, sizeof (weight));
    MEMCPY (wpc->dc.cweight, cweight, sizeof (cweight));

    wpc->dc.m = m;

    return i;
}

int get_unpacked_size(void *pSrc)
{
	WavpackHeader *pHdr = (WavpackHeader*)pSrc;
	int n = pHdr->flags & MONO_FLAG ? 1:2;
	return pHdr->total_samples * 2 * n;
}

void wavunpack16(void* pDes, const void *pSrc, unsigned int len)
{
	WavpackContext wpc;
	WavpackHeader *pHdr = (WavpackHeader*)pSrc;

    if (!pHdr->total_samples)
		return;

    CLEAR (wpc);
    wpc.wphdr = pHdr;
    unpack_init (&wpc);
	bs_open_read (&wpc.inbits, (char*)pSrc + sizeof(WavpackHeader), len - sizeof(WavpackHeader));
	unpack_samples (&wpc, pDes, pHdr->total_samples);
	bs_close_read(&wpc.inbits);
}
