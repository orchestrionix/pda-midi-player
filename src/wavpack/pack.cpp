////////////////////////////////////////////////////////////////////////////
//			     **** WAVPACK ****				  //
//		    Hybrid Lossless Wavefile Compressor			  //
//		Copyright (c) 1998 - 2002 Conifer Software.		  //
//			    All Rights Reserved.			  //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// pack.c

// This module actually handles the compression of the audio data, except for
// the entropy coding which is handled by the words? modules. For efficiency,
// the conversion is isolated to tight loops that handle an entire buffer. The
// actual bitstreams are "outbits" for the main WavPack file and "out2bits"
// for the .wvc file (if present) and these must be initialized with
// bs_open_write() before pack_samples() is called.
//
// The "COMPACT" define specifies an alternate version for the default lossless
// mode that uses less inline code but is somewhat slower. Use at your own
// discretion.

#include "wavpack.h"

#include <windows.h>
#include <math.h>


//////////////////////////////// local macros /////////////////////////////////

#define GET_24(ptr) (*(unsigned char*)(ptr)|((long)*(short*)((unsigned char*)(ptr)+1)<<8))

#define apply_weight(bits, weight, sample) ((weight * sample + (1 << (bits - 1))) >> bits)
#define apply_weight24(weight, sample) ((sample >= 0x800000 || sample < -0x800000) ? ((long)floor(((double) weight * sample + 128.0) / 256.0)) : ((weight * sample + 128) >> 8))

#define update_weight(bits, weight, source, result) \
    if (source && result) { \
	if ((source ^ result) >= 0) { if (weight++ == (1 << bits)) weight--; } \
	else if (weight-- ==  (-(1 << bits))) weight++; \
    }

//////////////////////////////// local tables ///////////////////////////////

// These two tables specify the characteristics of the decorrelation filters.
// Each term represents one layer of the sequential filter, where positive
// values indicate the relative sample involved from the same channel (1=prev)
// while -1 and -2 indicate cross channel decorrelation (in stereo only).

static const char extreme_terms [] = { 1,1,1,2,4,-1,1,2,3,6,-2,8,5,7,4,1,2,3 };
static const char default_terms [] = { 1,1,1,-1,2,1,-2 };

//////////////////////////////////////////////////////////////////////////////
// This function initializes everything required to pack WavPack bitstreams //
// and must be called BEFORE any other function in this module.             //
//                                                                          //
// The "flags" and "bits" fields of the WavpackHeader structure control     //
// the exact compression mode employed. However, some bit combinations that //
// are valid for reading are not valid for writing because the "pack" code  //
// has been cleaned of obsoleted modes. Currently, there are 6 basic modes  //
// that are supported for both reading and writing and the corresponding    //
// "flags" and "bits" values (with allowable variations) are shown here.    //
// ------------------------------------------------------------------------ //
// 1. Lossless High:                                                        //
//                                                                          //
//    command line: -h                                                      //
//                                                                          //
//    flags: HIGH_FLAG | NEW_HIGH_FLAG | CROSS_DECORR | NEW_DECORR_FLAG |   //
//           EXTREME_DECORR | JOINT_STEREO                                  //
//                                                                          //
//    bits: 0                                                               //
//                                                                          //
//    options: JOINT_STEREO can be cleared and this sometimes (but not      //
//             usually) results in better compression                       //
// ------------------------------------------------------------------------ //
// 2. Lossless Standard:                                                    //
//                                                                          //
//    command line: default operation                                       //
//                                                                          //
//    flags: HIGH_FLAG | NEW_HIGH_FLAG | CROSS_DECORR | NEW_DECORR_FLAG |   //
//           JOINT_STEREO                                                   //
//                                                                          //
//    bits: 0                                                               //
//                                                                          //
//    options: JOINT_STEREO can be cleared and this sometimes (but not      //
//             usually) results in better compression                       //
//                                                                          //
//    note: in stereo we must additionally set EXTREME_DECORR and           //
//          CANCEL_EXTREME before final write of flags to file, BUT NOT in  //
//          calls to pack_init() or pack_samples().                         //
// ------------------------------------------------------------------------ //
// 3. Lossless Fast:                                                        //
//                                                                          //
//    command line: -f                                                      //
//    flags: 0                                                              //
//    bits: 0                                                               //
// ------------------------------------------------------------------------ //
// 4. Lossless Very Fast:                                                   //
//                                                                          //
//    command line: -ff                                                     //
//    flags: FAST_FLAG                                                      //
//    bits: 0                                                               //
// ------------------------------------------------------------------------ //
// 5. Hybrid High:                                                          //
//                                                                          //
//    command line: -hb                                                     //
//                                                                          //
//    flags: NEW_HIGH_FLAG | NEW_DECORR_FLAG | EXTREME_DECORR               //
//                                                                          //
//    bits: target bitrate (both channels) in bits/sample * 256             //
//          (the minimum is 3 bits/sample mono, 6 bits/sample stereo; the   //
//          maximum is 32 bits/sample mono, 64 bits/sample stereo)          //
//                                                                          //
//    options: JOINT_STEREO may be set to cut noise and improve compression //
//             on stereo files with high inter-channel correlation          //
// ------------------------------------------------------------------------ //
// 6. Hybrid Standard:                                                      //
//                                                                          //
//    command line: -b                                                      //
//                                                                          //
//    flags: NEW_HIGH_FLAG | NEW_DECORR_FLAG                                //
//                                                                          //
//    bits: target bitrate (both channels) in bits/sample * 256             //
//          (the minimum is 3 bits/sample mono, 6 bits/sample stereo; the   //
//          maximum is 32 bits/sample mono, 64 bits/sample stereo)          //
//                                                                          //
//    options: JOINT_STEREO may be set to cut noise and improve compression //
//             on stereo files with high inter-channel correlation          //
// ------------------------------------------------------------------------ //
// Other flag notes:                                                        //
//                                                                          //
// 1. If file is mono, then JOINT_STEREO and CROSS_DECORR are cleared and   //
//    MONO_FLAG is set.                                                     //
//                                                                          //
// 2. If hybrid mode is used and sampling rate is >= 64,000 Hz, then set    //
//    LOSSY_SHAPE.                                                          //
//////////////////////////////////////////////////////////////////////////////

void pack_init (WavpackContext *wpc)
{
    int flags = wpc->wphdr->flags, ti;
    struct decorr_pass *dpp;

    CLEAR (wpc->decorr_passes);
    CLEAR (wpc->dc);

    if (flags & EXTREME_DECORR) {
	for (dpp = wpc->decorr_passes, ti = 0; ti < sizeof (extreme_terms); ti++)
	    if (extreme_terms [ti] >= 0 || (flags & CROSS_DECORR))
		dpp++->term = extreme_terms [ti];
    }
    else
	for (dpp = wpc->decorr_passes, ti = 0; ti < sizeof (default_terms); ti++)
	    if (default_terms [ti] >= 0 || (flags & CROSS_DECORR))
		dpp++->term = default_terms [ti];

    init_word4 (wpc);
}

///////////////////////////////////////////////////////////////////////////////
// This monster actually packs the 16-bit or 24-bit audio data and writes it //
// into the open bitstream(s). The function pack_init() must have been       //
// called and the bitstreams must have been opened with bs_open_write(). For //
// clarity, the function is broken up into segments that handle various      //
// modes. This results in a few extra flag checks, but makes the code easier //
// to follow because the nesting does not become so deep. For maximum        //
// efficiency, the conversion is isolated to tight loops that handle an      //
// entire buffer. The running CRC calculations are retrieved from and stored //
// back into the WavPack header each call. Note that two CRCs are required   //
// in hybrid mode because errors are detected with or without the correction //
// file. If noise calculations are desired, set the CALC_NOISE bit in the    //
// WavPack header and retrieve the results with the pack_noise() function at //
// the end of the pack operation. The decorrelation filters here provide     //
// efficient operation on values up to about 20-22 bits (using 32-bit ints), //
// which is not quite enough to handle 24-bit data. For this reason, special //
// provisions are made for this case. In lossless modes, the data values are //
// truncated to 20-bits before compression and the lower 4 bits are sent     //
// directly (or compressed with a very simple RLE). In lossy modes we simply //
// use floating point math in cases where there might be an overflow.        //
///////////////////////////////////////////////////////////////////////////////

void pack_samples (WavpackContext *wpc, void *buffer, unsigned int sample_count)
{
    int flags = wpc->wphdr->flags, shift = wpc->wphdr->shift, m = wpc->dc.m;
    long crc = wpc->wphdr->crc, crc2 = wpc->wphdr->crc2;
    double noise_acc = 0.0, noise;
    struct decorr_pass *dpp;
    unsigned char *bptr;
    unsigned int i;

    long sample [2] [5], dsample [2], csample [2];
    int weight [2] [5], cweight [4];

    memcpy (sample, wpc->dc.sample, sizeof (sample));
    memcpy (dsample, wpc->dc.dsample, sizeof (dsample));
    memcpy (csample, wpc->dc.csample, sizeof (csample));
    memcpy (weight, wpc->dc.weight, sizeof (weight));
    memcpy (cweight, wpc->dc.cweight, sizeof (cweight));

    /////////////////// handle the lossy/hybrid mono mode /////////////////////

    if (wpc->wphdr->bits && (flags & MONO_FLAG))
	for (bptr = (unsigned char*)buffer, i = 0; i < sample_count; ++i) {
	    long code;

	    if (flags & BYTES_3) {
		code = GET_24 (bptr) >> shift;
		bptr += 3;
	    }
	    else
		{
			code = *((short*) bptr);
			bptr += 2;
		}

	    crc2 = crc2 * 3 + code;

	    if (flags & LOSSY_SHAPE)
		wpc->dc.error [0] = -(code -= wpc->dc.error [0]);

	    if ((flags & BYTES_3) && shift < 4)
		for (dpp = wpc->decorr_passes; dpp->term; dpp++)
		    code -= (dpp->aweight_A = apply_weight24 (dpp->weight_A, dpp->samples_A [m]));
	    else
		for (dpp = wpc->decorr_passes; dpp->term; dpp++)
		    code -= (dpp->aweight_A = apply_weight (8, dpp->weight_A, dpp->samples_A [m]));

	    code = send_word4 (wpc, code, 0);

	    while (--dpp >= wpc->decorr_passes) {
		long sam = dpp->samples_A [m];

		update_weight (8, dpp->weight_A, sam, code);
		dpp->samples_A [(m + dpp->term) & (MAX_TERM - 1)] = (code += dpp->aweight_A);
	    }

	    m = (m + 1) & (MAX_TERM - 1);
	    crc = crc * 3 + code;
	    wpc->dc.error [0] += code;

	    if (flags & CALC_NOISE) {
		if (flags & BYTES_3)
		    noise = (code << shift) - GET_24 (bptr-3);
		else
		    noise = code - ((short*) bptr) [-1];

		noise_acc += noise *= noise;
		wpc->dc.noise_ave = (wpc->dc.noise_ave * 0.99) + (noise * 0.01);

		if (wpc->dc.noise_ave > wpc->dc.noise_max)
		    wpc->dc.noise_max = wpc->dc.noise_ave;
	    }
	}
      /////////////////// handle the lossy/hybrid stereo mode ///////////////////
	else if (wpc->wphdr->bits && !(flags & MONO_FLAG))
	for (bptr = (unsigned char*)buffer, i = 0; i < sample_count; ++i) {
	    long left, right, sum, diff;

	    if (flags & BYTES_3) {
		left = GET_24 (bptr) >> shift;
		bptr += 3;
		right = GET_24 (bptr) >> shift;
		bptr += 3;
	    }
	    else {
		left = *((short*) bptr);
		bptr += 2;
		right = *((short*) bptr);
		bptr += 2;
	    }

	    crc2 = crc2 * 3 + left;
	    crc2 = crc2 * 3 + right;

	    if (flags & JOINT_STEREO) {
		sum = (left + right) >> 1;
		left -= right;
		right = sum;
	    }

	    if (flags & LOSSY_SHAPE) {
		wpc->dc.error [0] = -(left -= wpc->dc.error [0]);
		wpc->dc.error [1] = -(right -= wpc->dc.error [1]);
	    }

	    if ((flags & BYTES_3) && shift < 4)
		for (dpp = wpc->decorr_passes; dpp->term; dpp++) {
		    left -= (dpp->aweight_A = apply_weight24 (dpp->weight_A, dpp->samples_A [m]));
		    right -= (dpp->aweight_B = apply_weight24 (dpp->weight_B, dpp->samples_B [m]));
		}
	    else
		for (dpp = wpc->decorr_passes; dpp->term; dpp++) {
		    left -= (dpp->aweight_A = apply_weight (8, dpp->weight_A, dpp->samples_A [m]));
		    right -= (dpp->aweight_B = apply_weight (8, dpp->weight_B, dpp->samples_B [m]));
		}

	    left = send_word4 (wpc, left, 0);
	    right = send_word4 (wpc, right, 1);

	    while (--dpp >= wpc->decorr_passes) {
		long sam_A = dpp->samples_A [m], sam_B = dpp->samples_B [m];
		int k = (m + dpp->term) & (MAX_TERM - 1);

		update_weight (8, dpp->weight_A, sam_A, left);
		dpp->samples_A [k] = (left += dpp->aweight_A);

		update_weight (8, dpp->weight_B, sam_B, right);
		dpp->samples_B [k] = (right += dpp->aweight_B);
	    }

	    m = (m + 1) & (MAX_TERM - 1);
	    wpc->dc.error [0] += left;
	    wpc->dc.error [1] += right;

	    if (flags & JOINT_STEREO) {
		right = ((sum = (right << 1) | (left & 1)) - (diff = left)) >> 1;
		left = (sum + diff) >> 1;
	    }

	    crc = crc * 3 + left;
	    crc = crc * 3 + right;

	    if (flags & CALC_NOISE) {
		if (flags & BYTES_3) {
		    noise = (double)((left << shift) - GET_24 (bptr-6)) * ((left << shift) - GET_24 (bptr-6));
		    noise += (double)((right << shift) - GET_24 (bptr-3)) * ((right << shift) - GET_24 (bptr-3));
		}
		else {
		    noise = (double)(left - ((short*) bptr) [-2]) * (left - ((short*) bptr) [-2]);
		    noise += (double)(right - ((short*) bptr) [-1]) * (right - ((short*) bptr) [-1]);
		}

		noise_acc += noise /= 2.0;
		wpc->dc.noise_ave = (wpc->dc.noise_ave * 0.99) + (noise * 0.01);

		if (wpc->dc.noise_ave > wpc->dc.noise_max)
		    wpc->dc.noise_max = wpc->dc.noise_ave;
	    }
	}
	else
		DebugBreak();

    memcpy (wpc->dc.sample, sample, sizeof (sample));
    memcpy (wpc->dc.dsample, dsample, sizeof (dsample));
    memcpy (wpc->dc.csample, csample, sizeof (csample));
    memcpy (wpc->dc.weight, weight, sizeof (weight));
    memcpy (wpc->dc.cweight, cweight, sizeof (cweight));

    wpc->dc.m = m;
    wpc->wphdr->crc = crc;
    wpc->wphdr->crc2 = crc2;

    if (flags & CALC_NOISE)
	wpc->dc.noise_sum += noise_acc;

	i = (int)wpc->outbits.ptr;
	i = ((i + 3) & ~3) - i;
	i += 8;//append 2 int to end to make getBit() happy
	while(i)
	{
		if(wpc->outbits.buf >= wpc->outbits.end)
			break;
		*wpc->outbits.ptr++ = 0;
		i--;
	}
}

void wavpack16(void* pDes, unsigned int *nDes, const void *pSrc, unsigned int sample_count, int bits, int isMono)
{
	WavpackContext wpc;
	WavpackHeader *pHdr = (WavpackHeader*)pDes;
	int npacked;

	if(*nDes < sizeof(WavpackHeader))
		DebugBreak();

	memset(pHdr, 0, sizeof(WavpackHeader));
	pHdr->ckSize = sizeof(WavpackHeader);
	pHdr->version = 3;
	pHdr->total_samples = sample_count;
	pHdr->shift = 0;

	bits -= 3;
	if (bits < 21) {
	    pHdr->bits = (bits * 512) + 1374;
	    if (isMono)
			pHdr->bits /= 2;
	}
	else
		DebugBreak();
    pHdr->flags = (NEW_DECORR_FLAG | NEW_HIGH_FLAG | EXTREME_DECORR);
	if(isMono)
		pHdr->flags |= MONO_FLAG;
	else
		pHdr->flags |= JOINT_STEREO | CROSS_DECORR;

	CLEAR(wpc);

	wpc.wphdr = pHdr;
	pack_init(&wpc);

	bs_open_write(&wpc.outbits, (char*)pDes + sizeof(WavpackHeader), *nDes - sizeof(WavpackHeader));

	pack_samples(&wpc, (void*)pSrc, sample_count);

	npacked = wpc.outbits.ptr - wpc.outbits.buf + sizeof(WavpackHeader);
	if(*nDes < npacked)
		DebugBreak();
	*nDes = npacked;
	
	bs_close_write(&wpc.outbits);


	pHdr->flags &= STORED_FLAGS;
}