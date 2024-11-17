////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2002 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// words4.c

// This module provides entropy word encoding and decoding functions using
// a variation on the Rice method. This was introduced in version 3.93
// because it allows splitting the data into a "lossy" stream and a
// "correction" stream in a very efficient manner and is therefore ideal
// for the "hybrid" mode.
//
// Like in classical Rice codes, the value to be transmitted is divided into
// a division part and a modulo part. However, instead of limiting the
// modulo part to a power of two and transmitting the bits literally, in
// this method the modulo part can be any even integer number of codes wide,
// and is calculated as 2/3 of the exponentially decaying average of the
// actual samples. This is very well optimized for transmitting the division
// portion using standard Rice, however the modulo portion can no longer be
// simply transmitted literally. Instead, this data is transmitted as a
// successive binary tree until either the allocated number of bits is used or
// the exact result has been sent. If the allocated bit count is exhausted
// before the exact value is determined, then the decoder simply uses the
// midpoint of the last known span, making it ideal for a lossy mode. If the
// hybrid mode is selected, then the remaining information to define the sample
// exactly is sent to the "correction" file. In the simplest case this would
// simply be a continuation of the binary tree, but a faster and slightly more
// efficient (based on the distribution of the data) method is used.
//
// When operated in lossy mode at a fixed bitrate, this system results in a
// fixed signal to noise ratio, although the "signal" is the decorrelated
// residual, so is only loosely related to the signal to noise ratio of the
// final output. In a very simple version of this system, we could simply
// transmit a fixed number of extra precision bits per sample and the
// quantization noise would modulate directly with the amplitude of the
// residual, filtered only by the relatively fast time constant (32 samples)
// of the modulo "base" calculation.
//
// Unfortunately, there is a problem with this method. There exist
// transient bursts of high residual signal level as the decorrelator
// adjusts to sudden changes in the characteristics of the source audio,
// and these occur over a much longer time constant than 32 samples. This
// could be masked by using a much slower time constant for the "base"
// calculation, but at the expense of bitstream efficiency. The solution
// choosen is to have two different level measurments, one with a fast
// time constant that is used to calculate the "base" value, and one with
// a slower time constant (256 samples) that determines the quantization
// noise level. By comparing these two levels logarithmically, the code
// determines how many extra bits to allocate to the precision to maintain
// the desired noise level during residual transients.
//
// Another disadvantage with the simple method is that it does not allow
// redistribution of bits when the two channels of a stereo signal are not
// balanced, which results in higher than necessary total noise. Again,
// this is accomplished by including the relative levels of the channels
// in the logarithmic calculation of the bitrate.

#include "wavpack.h"
//#include <stdlib.h>
//#include <string.h>

///////////////////////// external table references //////////////////////////

const char nbits_table [] = {
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,	// 0 - 15
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,	// 16 - 31
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,	// 32 - 47
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,	// 48 - 63
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,	// 64 - 79
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,	// 80 - 95
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,	// 96 - 111
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,	// 112 - 127
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 128 - 143
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 144 - 159
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 160 - 175
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 176 - 191
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 192 - 207
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 208 - 223
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,	// 224 - 239
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8	// 240 - 255
};
#if 0
//////////////////////////////////////////////////////////////////////////////
// This function initializes everything required to send or receive words   //
// with this module and must be called BEFORE any other function in this    //
// module. The "bits" field in the WavPack header represents the average    //
// desired total bitrate for the stream in bits/sample * 256. Because this  //
// method transmits an average of 3 bits per sample minimum, we subtract    //
// 3 * 256 from this value (or from 1/2 this value for stereo) to determine //
// the "excess" bitrate per mono sample that we have for transmitting more  //
// precise values.                                                          //
//////////////////////////////////////////////////////////////////////////////
#endif

void init_word4 (WavpackContext *wpc)
{
    CLEAR (wpc->w4);

    if (wpc->wphdr->flags & MONO_FLAG)
	wpc->w4.bitrate = wpc->wphdr->bits - 768;
    else
	wpc->w4.bitrate = (wpc->wphdr->bits / 2) - 768;
}

static int log2_fixed8 (unsigned long avalue);

//////////////////////////////////////////////////////////////////////////////
// This macro counts the number of bits that are required to specify the    //
// unsigned 32-bit value, counting from the LSB to the most significant bit //
// that is set. Return range is 0 - 32.                                     //
//////////////////////////////////////////////////////////////////////////////

#define count_bits(av) ( \
 (av) < (1 << 8) ? nbits_table [av] : \
  ( \
   (av) < (1L << 16) ? nbits_table [(av) >> 8] + 8 : \
   ((av) < (1L << 24) ? nbits_table [(av) >> 16] + 16 : nbits_table [(av) >> 24] + 24) \
  ) \
)

//#ifdef PACK

#define outbits (&wpc->outbits)
#define out2bits (&wpc->out2bits)

//////////////////////////////////////////////////////////////////////////////
// Send the specified word to the "outbits" bitstream (which must have been //
// previously opened with bs_open_write()). Note that the "chan" value must //
// either be always 0 or 1, or alternate 0, 1, 0, 1, 0, 1, etc. The return  //
// value is the actual value that is transmitted onto the "lossy" stream    //
// and is the value that should be fed back into the predictor because it   //
// is the only value that will certainly be available at decode time. If    //
// a correction file is also being generated (to out2bits) then the extra   //
// information required to decode the value exactly is placed onto that     //
// bitstream (although it is not guaranteed that this will be available at  //
// decode time).                                                            //
//////////////////////////////////////////////////////////////////////////////

long send_word4 (WavpackContext *wpc, long value, int chan)
{
    unsigned long base, ones_count, avalue;
    int sign = value < 0, bitcount;
    long low, mid, high;

    // This code determines how much information to allocate to each sample
    // transmitted and is executed once every complete sample (i.e. once every
    // two words in stereo). The bitrate is calculated to 1/256 of a bit and
    // added to an accumulator so that unused partial bits are saved over for
    // subsequent samples.

    if (!chan) {

	// the (3 << 8) factor in these calculations is because the fast and slow
	//  response levels differ by a factor of 8 (log2 (8) = 3) and the log2
	//  values have 8 bits of fractions

	if (wpc->wphdr->flags & MONO_FLAG) {
	    wpc->w4.bits_acc [0] += wpc->w4.bitrate + log2_fixed8 (wpc->w4.fast_level [0]) - log2_fixed8 (wpc->w4.slow_level [0]) + (3 << 8);

	    if (wpc->w4.bits_acc [0] < 0)
		wpc->w4.bits_acc [0] = 0;
	}
	else {
		int slow_log_0, slow_log_1, balance; 
	    slow_log_0 = log2_fixed8 (wpc->w4.slow_level [0]);
	    slow_log_1 = log2_fixed8 (wpc->w4.slow_level [1]);
	    
	    if (wpc->wphdr->flags & JOINT_STEREO)
		balance = (slow_log_1 - slow_log_0 + 257) >> 1;
	    else
		balance = (slow_log_1 - slow_log_0 + 1) >> 1;

	    wpc->w4.bits_acc [0] += wpc->w4.bitrate - balance + log2_fixed8 (wpc->w4.fast_level [0]) - slow_log_0 + (3 << 8);
	    wpc->w4.bits_acc [1] += wpc->w4.bitrate + balance + log2_fixed8 (wpc->w4.fast_level [1]) - slow_log_1 + (3 << 8);

	    if (wpc->w4.bits_acc [0] + wpc->w4.bits_acc [1] < 0)
		wpc->w4.bits_acc [0] = wpc->w4.bits_acc [1] = 0;
	    else if (wpc->w4.bits_acc [0] < 0) {
		wpc->w4.bits_acc [1] += wpc->w4.bits_acc [0];
		wpc->w4.bits_acc [0] = 0;
	    }
	    else if (wpc->w4.bits_acc [1] < 0) {
		wpc->w4.bits_acc [0] += wpc->w4.bits_acc [1];
		wpc->w4.bits_acc [1] = 0;
	    }
	}
	}

    base = (wpc->w4.fast_level [chan] + 48) / 96;   // 1/3 of average sample value
    bitcount = wpc->w4.bits_acc [chan] >> 8;	    // available extra bits this sample
    wpc->w4.bits_acc [chan] &= 0xff;		    // fraction left for next sample

    if (!base) {			    // if base is too small we transmit
	high = low = mid = value;	    //  value using unary bits
	ones_count = value >= 0 ? value : -value;
    }
    else {

	// note that we use ones complement for transmitting negative values

	if (sign)
	    mid = -(((ones_count = ~value / (base * 2)) * 2 + 1) * base);
	else
	    mid = ((ones_count = value / (base * 2)) * 2 + 1) * base;

	low = mid - base;
	high = mid + base - 1;
    }

    // If the ones count is 24 or greater then we transmit exactly 24
    // ones, a zero, and then we switch to a non-unary code to transmit the
    // remaining ones count. This has virtually no statistical significance
    // but does make it impossible for pathological samples to generate
    // millions of bits. This method is similar in concept to, but much more
    // elegant and generally applicable than the method used in words1.c.

    if (ones_count >= 24) {
	int cbits;

	putbits (0xffffff, 25, outbits);
	ones_count -= 24;
	cbits = count_bits (ones_count);

	while (cbits--) {
	    putbit_1 (outbits);
	}

	putbit_0 (outbits);

	while (ones_count > 1) {
	    putbit (ones_count & 1, outbits);
	    ones_count >>= 1;
	}
    }
    else
	putbits ((1 << ones_count) - 1, ones_count + 1, outbits);

    // the only time that we would not transmit the sign bit is when the
    // sample is zero AND we are in the unary mode (i.e. base = 0)

    if (mid)
	putbit (sign, outbits);

    // as long as the exact value has not been transmitted and the available
    // bits have not been exhausted, continue adding precision to the result

    while (high != low && bitcount--)
	if (value < mid) {
	    mid = ((high = mid - 1) + low + 1) >> 1;
	    putbit_0 (outbits);
	}
	else {
	    mid = (high + (low = mid) + 1) >> 1;
	    putbit_1 (outbits);
	}

    // update the fast and slow level values

    wpc->w4.fast_level [chan] -= ((wpc->w4.fast_level [chan] + 0x10) >> 5);
    wpc->w4.fast_level [chan] += (avalue = (mid >= 0 ? mid : -mid));
    wpc->w4.slow_level [chan] -= ((wpc->w4.slow_level [chan] + 0x80) >> 8);
    wpc->w4.slow_level [chan] += avalue;

    return mid;
}

//#endif

//#ifdef UNPACK

#define inbits (&wpc->inbits)
#define in2bits (&wpc->in2bits)

//////////////////////////////////////////////////////////////////////////////
// Get a word from the "inbits" bitstream. Note that the "chan" value must  //
// either be always 0 or 1, or alternate 0, 1, 0, 1, 0, 1, etc. If the      //
// "correction" bitstream is being used (in2bits) then this is also read to //
// determine the actual exact value and the delta from the lossy value is   //
// stored at *correction (assuming the pointer is not NULL).                //
//////////////////////////////////////////////////////////////////////////////

unsigned int getBit(Bitstream *bs)
{
	unsigned int c;
	if(bs->bc == 0)
	{
#ifdef DEBUG_WVPACK
		if(bs->ptr + sizeof(unsigned int) > bs->end)
			return 0;
#endif
		bs->bc = 1;
		bs->sr = *((unsigned int*)bs->ptr);
		bs->ptr += sizeof(unsigned int);
	}
	c = bs->sr & bs->bc;
	bs->bc <<= 1;
	return c;
}

long get_word4 (WavpackContext *wpc, int chan)
{
    unsigned long base, ones_count, avalue;
    long low, mid, high;
    int bitcount;

    // count consecutive ones in bitstream, > 25 indicates error (or EOF)

#ifdef DEBUG_WVPACK
    for (ones_count = 0; ones_count < 25 && getBit (inbits); ++ones_count);
    if (ones_count == 25)
		return WORD_EOF;
#else
    for (ones_count = 0; getBit (inbits); ++ones_count);
#endif

    // if the ones count is exactly 24, then we switch to non-unary method

    if (ones_count == 24) {
		long mask;
		int cbits;
		
#ifdef DEBUG_WVPACK
		for (cbits = 0; cbits < 33 && getBit (inbits); ++cbits);
		
		if (cbits == 33)
			return WORD_EOF;
#else
		for (cbits = 0; getBit (inbits); ++cbits);
#endif
		
		if (cbits < 2)
			ones_count = cbits;
		else
		{
			for (mask = 1, ones_count = 0; --cbits; mask <<= 1)
			{
				if (getBit (inbits))
					ones_count |= mask;
			}
			ones_count |= mask;
		}
		
		ones_count += 24;
    }
	
    // This code determines how much information to allocate to each sample
    // transmitted and is executed once every complete sample (i.e. once every
    // two words in stereo). The bitrate is calculated to 1/256 of a bit and
    // added to an accumulator so that unused partial bits are saved over for
    // subsequent samples.

    if (!chan) {

	// the (3 << 8) factor in these calculations is because the fast and slow
	//  response levels differ by a factor of 8 (log2 (8) = 3) and the log2
	//  values have 8 bits of fractions

#ifndef MONO_ONLY
	if (wpc->wphdr->flags & MONO_FLAG)
#endif 
	{
	    wpc->w4.bits_acc [0] += wpc->w4.bitrate + log2_fixed8 (wpc->w4.fast_level [0]) - log2_fixed8 (wpc->w4.slow_level [0]) + (3 << 8);

	    if (wpc->w4.bits_acc [0] < 0)
		wpc->w4.bits_acc [0] = 0;
	}
#ifndef MONO_ONLY
	else {
		int slow_log_0, slow_log_1, balance; 
	    slow_log_0 = log2_fixed8 (wpc->w4.slow_level [0]);
	    slow_log_1 = log2_fixed8 (wpc->w4.slow_level [1]);
	    
	    if (wpc->wphdr->flags & JOINT_STEREO)
		balance = (slow_log_1 - slow_log_0 + 257) >> 1;
	    else
		balance = (slow_log_1 - slow_log_0 + 1) >> 1;

	    wpc->w4.bits_acc [0] += wpc->w4.bitrate - balance + log2_fixed8 (wpc->w4.fast_level [0]) - slow_log_0 + (3 << 8);
	    wpc->w4.bits_acc [1] += wpc->w4.bitrate + balance + log2_fixed8 (wpc->w4.fast_level [1]) - slow_log_1 + (3 << 8);

	    if (wpc->w4.bits_acc [0] + wpc->w4.bits_acc [1] < 0)
		wpc->w4.bits_acc [0] = wpc->w4.bits_acc [1] = 0;
	    else if (wpc->w4.bits_acc [0] < 0) {
		wpc->w4.bits_acc [1] += wpc->w4.bits_acc [0];
		wpc->w4.bits_acc [0] = 0;
	    }
	    else if (wpc->w4.bits_acc [1] < 0) {
		wpc->w4.bits_acc [0] += wpc->w4.bits_acc [1];
		wpc->w4.bits_acc [1] = 0;
	    }
	}
#endif 
   }

    base = (wpc->w4.fast_level [chan] + 48) / 96;   // 1/3 of average sample value
    bitcount = wpc->w4.bits_acc [chan] >> 8;	    // available extra bits this sample
    wpc->w4.bits_acc [chan] &= 0xff;		    // fraction left for next sample

    if (!base) {	// if base is too small, just get value using unary
	if (ones_count)
	    high = low = mid = (getBit (inbits)) ? -ones_count : ones_count;
	else
	    high = low = mid = 0;
    }
    else {
	mid = (ones_count * 2 + 1) * base;
	if (getBit (inbits)) mid = -mid;
	low = mid - base;
	high = mid + base - 1;

	// as long as the exact value has not been decoded and the available
	// bits have are not exhausted, continue adding precision to the result

	while (bitcount--) {
	    if (getBit (inbits))
		mid = (high + (low = mid) + 1) >> 1;
	    else
		mid = ((high = mid - 1) + low + 1) >> 1;

	    if (high == low)
		break;
	}
    }

    // update the fast and slow level values

    wpc->w4.fast_level [chan] -= ((wpc->w4.fast_level [chan] + 0x10) >> 5);
    wpc->w4.fast_level [chan] += (avalue = (mid < 0)?-mid:mid);
    wpc->w4.slow_level [chan] -= ((wpc->w4.slow_level [chan] + 0x80) >> 8);
    wpc->w4.slow_level [chan] += avalue;

    return mid;
}

//#endif

//////////////////////////////////////////////////////////////////////////////
// This function calculates an approximate base-2 logarithm (with 8 bits of //
// fraction) from the supplied value. Using logarithms makes comparing      //
// signal level values and calculating fractional bitrates much easier.     //
//////////////////////////////////////////////////////////////////////////////

static int log2_fixed8 (unsigned long avalue)
{
    int dbits;

    if ((avalue += avalue >> 9) <= ((1 << 8) - 1)) {
	dbits = nbits_table [avalue];
	return (dbits << 8) + ((avalue << (9 - dbits)));
    }
    else {
	if (avalue < (1L << 16))
	    dbits = nbits_table [avalue >> 8] + 8;
	else if (avalue < (1L << 24))
	    dbits = nbits_table [avalue >> 16] + 16;
	else
	    dbits = nbits_table [avalue >> 24] + 24;

	return (dbits << 8) + ((avalue >> (dbits - 9)));
    }
}
