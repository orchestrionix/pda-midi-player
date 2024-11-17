////////////////////////////////////////////////////////////////////////////
//                           **** WAVPACK ****                            //
//                  Hybrid Lossless Wavefile Compressor                   //
//              Copyright (c) 1998 - 2002 Conifer Software.               //
//                          All Rights Reserved.                          //
//      Distributed under the BSD Software License (see license.txt)      //
////////////////////////////////////////////////////////////////////////////

// bits.c

// This module provides utilities to support the BitStream structure which is
// used to read and write all WavPack audio data streams. Each function is
// provided in both a single-threaded and multi-threaded version, the latter
// providing the option of overlapped file I/O with systems that do not provide
// this as intrinsic (i.e. Win95/98). For some reason this actually hurts
// performance on NT based systems and should not be used. This does not apply
// for bs_restore() which is not usable with overlapped operation.

#include "wavpack.h"
//#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////////
// Open the specified BitStream and associate with the specified file. The  //
// "bufsiz" field of the structure must be preset with the desired buffer   //
// size and the file's read pointer must be set to where the desired bit    //
// data is located.  A return value of TRUE indicates an error in           //
// allocating buffer space.                                                 //
//////////////////////////////////////////////////////////////////////////////

int bs_open_read (Bitstream *bs, void* buf, int len)
{
	bs->buf = (unsigned char*)buf;
	bs->bufsiz = len;
    bs->end = bs->buf + bs->bufsiz;
	bs->ptr = bs->buf;

    bs->sr = bs->bc = 0;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// This function is called to release any resources used by the BitStream   //
// and position the file pointer to the first byte past the read bits.      //
//////////////////////////////////////////////////////////////////////////////

void bs_close_read (Bitstream *bs)
{
	bs->buf = NULL;
}

//////////////////////////////////////////////////////////////////////////////
// Open the specified BitStream and associate with the specified file. The  //
// "bufsiz" field of the structure must be preset with the desired buffer   //
// size and the file's write pointer must be set to where the bit data      //
// should be written. A return value of TRUE indicates an error in          //
// allocating buffer space.                                                 //
//////////////////////////////////////////////////////////////////////////////

int bs_open_write (Bitstream *bs, void* buf, int len)
{
	bs->buf = (unsigned char *) buf;
	bs->bufsiz = len;
    bs->ptr = bs->buf;
    bs->end = bs->buf + bs->bufsiz;
    bs->sr = bs->bc = 0;
    return 0;
}

//////////////////////////////////////////////////////////////////////////////
// This function forces a flushing write of the specified BitStream,        //
// positions the write pointer past the bit data written, and releases any  //
// resources used by the BitStream. The "error" field should be checked     //
// after this to make sure all data was properly written.                   //
//////////////////////////////////////////////////////////////////////////////

void bs_close_write (Bitstream *bs)
{
	bs->buf = NULL;
}
