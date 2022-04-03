/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Usefuls routines based on the LzmaTest.c file from LZMA SDK 4.65
 *
 * Copyright (C) 2007-2008 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
 *
 * Copyright (C) 1999-2005 Igor Pavlov
 */

#ifndef __LZMA_TOOL_H__
#define __LZMA_TOOL_H__


/* The return value of the function is the actual read/write length. Note that the return value is not success/fail */
typedef unsigned int (*LZMA_STREAM_FCT)(unsigned int offset, unsigned char* buffer, unsigned int size);

typedef struct
{
    unsigned int offset;
    LZMA_STREAM_FCT func;
} LZMA_STREAM_S;

extern int lzmaBuffToBuffDecompress (unsigned char* outStream, unsigned int* uncompressedSize,
                                     unsigned char* inStream, unsigned int length, ISzAlloc* alloc);

extern unsigned int LzmaDecode2(const Byte* prop_data, unsigned int prop_size, ELzmaStatus* status,
	                                   ISzAlloc* alloc, LZMA_STREAM_S* in_stream, LZMA_STREAM_S* out_stream,
	                                   unsigned int uncompress_len, unsigned int compress_len);

#endif

