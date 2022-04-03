// SPDX-License-Identifier: GPL-2.0+
/*
 * Usefuls routines based on the LzmaTest.c file from LZMA SDK 4.65
 *
 * Copyright (C) 2007-2009 Industrie Dial Face S.p.A.
 * Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
 *
 * Copyright (C) 1999-2005 Igor Pavlov
 */

/*
 * LZMA_Alone stream format:
 *
 * uchar   Properties[5]
 * uint64  Uncompressed size
 * uchar   data[*]
 *
 */

#include <hi_boot_rom.h>


#define LZMA_PROPERTIES_OFFSET 0
#define LZMA_SIZE_OFFSET       LZMA_PROPS_SIZE
#define LZMA_DATA_OFFSET       LZMA_SIZE_OFFSET+sizeof(uint64_t)

#include "LzmaDec.h"
#include "LzmaTools.h"

#define IN_BUF_SIZE     (0x1000)
#define OUT_BUF_SIZE    (0x1000)


SRes Decode2(CLzmaDec* state, Byte* in_buf, LZMA_STREAM_S* in_stream, Byte* out_buf, LZMA_STREAM_S* out_stream,
             unsigned int uncompress_len, unsigned int compress_len, ELzmaStatus* status);


int lzmaBuffToBuffDecompress (unsigned char* outStream, SizeT* uncompressedSize,
                              unsigned char* inStream,  SizeT length, ISzAlloc* alloc)
{
    int res = SZ_ERROR_DATA;
    int i;

    SizeT outSizeFull = 0xFFFFFFFF; /* 4GBytes limit */
    SizeT outProcessed;
    SizeT outSize;
    SizeT outSizeHigh;
    ELzmaStatus state;
    SizeT compressedSize = (SizeT)(length - LZMA_PROPS_SIZE);

    (hi_void)memset_s(&state,sizeof(state), 0, sizeof(state), (uintptr_t)(&state) ^ sizeof(state) ^ 0 ^ sizeof(state));

    outSize = 0;
    outSizeHigh = 0;
    /* Read the uncompressed size */
    for (i = 0; i < 8; i++)
    {
        unsigned char b = inStream[LZMA_SIZE_OFFSET + i];

        if (i < 4)
        {
            outSize     += (UInt32)(b) << ((unsigned int)i * 8);
        }
        else
        {
            outSizeHigh += (UInt32)(b) << (((unsigned int)i - 4) * 8);
        }
    }

    outSizeFull = (SizeT)outSize;

    if (sizeof(SizeT) >= 8)
    {
        /*
         * SizeT is a 64 bit uint => We can manage files larger than 4GB!
         *
         */
        outSizeFull |= (((SizeT)outSizeHigh << 16) << 16);
    }
    else if (outSizeHigh != 0 || (UInt32)(SizeT)outSize != outSize)
    {
        /*
         * SizeT is a 32 bit uint => We cannot manage files larger than
         * 4GB!  Assume however that all 0xf values is "unknown size" and
         * not actually a file of 2^64 bits.
         *
         */
        if (outSizeHigh != (SizeT)-1 || outSize != (SizeT)-1) {
            boot_msg0 ("LZMA: 64bit support not enabled.\n");
            return SZ_ERROR_DATA;
        }
    }

    boot_msg1("Uncompresed size: ", outSizeFull);
    boot_msg1("Compresed size: ", compressedSize);

    /* Short-circuit early if we know the buffer can't hold the results. */
    if (outSizeFull != (SizeT) - 1 && *uncompressedSize < outSizeFull)
    { return SZ_ERROR_OUTPUT_EOF; }

    /* Decompress */
    outProcessed = (outSizeFull > *uncompressedSize) ? (*uncompressedSize) : outSizeFull;

    hi_watchdog_feed();
    res = LzmaDecode(
              outStream, &outProcessed,
              inStream + LZMA_DATA_OFFSET, &compressedSize,
              inStream, LZMA_PROPS_SIZE, LZMA_FINISH_END, &state, alloc);
    *uncompressedSize = outProcessed;

    if (res != SZ_OK)
    {
        return res;
    }

    return res;
}


unsigned int LzmaDecode2(const Byte* prop_data, unsigned int prop_size, ELzmaStatus* status, ISzAlloc* alloc,
                LZMA_STREAM_S* in_stream, LZMA_STREAM_S* out_stream, unsigned int uncompress_len, unsigned int compress_len)
{
    CLzmaDec p;
    SRes res;
    Byte* in_buf = HI_NULL;
    Byte* out_buf = HI_NULL;
    unsigned int dic_size = compress_len;

    LzmaDec_Construct(&p);

    res = LzmaDec_AllocateProbs(&p, prop_data, prop_size, alloc);

    if (0 != res)
    {
        return SZ_ERROR_MEM;
    }

    LzmaDec_Init(&p);

    in_buf = IAlloc_Alloc(alloc, IN_BUF_SIZE);

    if (HI_NULL == in_buf)
    {
        return SZ_ERROR_MEM;
    }

    out_buf = IAlloc_Alloc(alloc, OUT_BUF_SIZE);

    if (HI_NULL == out_buf)
    {
        IAlloc_Free(alloc, in_buf);
        in_buf = HI_NULL;
        return SZ_ERROR_MEM;
    }

    (hi_void)memset_s(in_buf, IN_BUF_SIZE, 0, IN_BUF_SIZE, (uintptr_t)in_buf ^ IN_BUF_SIZE ^ 0 ^ IN_BUF_SIZE);
    (hi_void)memset_s(out_buf, OUT_BUF_SIZE, 0, OUT_BUF_SIZE, (uintptr_t)out_buf ^ OUT_BUF_SIZE ^ 0 ^ OUT_BUF_SIZE);

    if (p.prop.dicSize <= compress_len)
    {
        dic_size = p.prop.dicSize;
    }

    p.dic = IAlloc_Alloc(alloc, dic_size);

    if (HI_NULL == p.dic)
    {
        IAlloc_Free(alloc, in_buf);
        in_buf = HI_NULL;

        IAlloc_Free(alloc, out_buf);
        out_buf = HI_NULL;

        return SZ_ERROR_MEM;
    }

    p.dicBufSize = dic_size;

    res = Decode2(&p, in_buf, in_stream, out_buf, out_stream, uncompress_len, compress_len, status);

    if ((res == SZ_OK) && (*status == LZMA_STATUS_NEEDS_MORE_INPUT))
    {
        res = SZ_ERROR_INPUT_EOF;
    }

    IAlloc_Free(alloc, in_buf);
    in_buf = HI_NULL;

    IAlloc_Free(alloc, out_buf);
    out_buf = HI_NULL;

    IAlloc_Free(alloc, p.dic);
    p.dic = HI_NULL;

    LzmaDec_FreeProbs(&p, alloc);

    return res;
}


SRes Decode2(CLzmaDec* state, Byte* in_buf, LZMA_STREAM_S* in_stream, Byte* out_buf, LZMA_STREAM_S* out_stream,
             unsigned int uncompress_len, unsigned int compress_len, ELzmaStatus* status)
{
    int there_is_size = (compress_len != (unsigned int) - 1);
    size_t in_pos = 0, in_size = 0, out_pos = 0;
    unsigned int in_offset = in_stream->offset;
    unsigned int out_offset = out_stream->offset;

    LzmaDec_Init(state);

    for (;;)
    {
        if (in_pos == in_size)
        {
            in_size = (uncompress_len > IN_BUF_SIZE) ? IN_BUF_SIZE : uncompress_len;
            RINOK(in_stream->func(in_offset, in_buf, in_size));
            RINOK(in_stream->func(in_offset, in_buf, 1));       /* Make sure the readings are correct. */
            uncompress_len -= in_size;
            in_pos = 0;
        }

        {
            SRes res;
            size_t in_processed = in_size - in_pos;
            size_t out_processed = OUT_BUF_SIZE - out_pos;

            ELzmaFinishMode finish_mode = LZMA_FINISH_ANY;

            if (there_is_size && out_processed > compress_len)
            {
                out_processed = (size_t)compress_len;
                finish_mode = LZMA_FINISH_END;
            }

            res = LzmaDec_DecodeToBuf(state, out_buf + out_pos, &out_processed,
                                      in_buf + in_pos, &in_processed, finish_mode, status);

            in_pos += in_processed;
            out_pos += out_processed;
            compress_len -= out_processed;

            /* Misjudgment is required. */
            if (out_stream->func(out_offset, out_buf, out_processed) != out_processed)
            {
                return SZ_ERROR_WRITE;
            }

            in_offset += in_processed;
            out_offset += out_processed;

            out_pos = 0;

            if (res != SZ_OK || (there_is_size && compress_len == 0))
            {
                return res;
            }

            if (in_processed == 0 && out_processed == 0)
            {
                if (there_is_size || *status != LZMA_STATUS_FINISHED_WITH_MARK)
                {
                    return SZ_ERROR_DATA;
                }

                return res;
            }
        }
    }
}

