/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef LWIP_HDR_DEF_H
#define LWIP_HDR_DEF_H

/* arch.h might define NULL already */
#include "lwip/arch.h"
#include "lwip/opt.h"

#if defined (__cplusplus) && __cplusplus
extern "C" {
#endif

/* time unit */
#define MS_PER_SECOND    1000
#define US_PER_MSECOND   1000
#define NS_PER_USECOND   1000

#define LWIP_MAX(x, y)  (((x) > (y)) ? (x) : (y))
#define LWIP_MIN(x, y)  (((x) < (y)) ? (x) : (y))

/* Get the number of entries in an array ('x' must NOT be a pointer!) */
#define LWIP_ARRAYSIZE(x) (sizeof(x)/sizeof((x)[0]))

/** Create u32_t value from bytes */
#define LWIP_MAKEU32(a, b, c, d) (((u32_t)((a) & 0xff) << 24) | \
                               ((u32_t)((b) & 0xff) << 16) | \
                               ((u32_t)((c) & 0xff) << 8)  | \
                                (u32_t)((d) & 0xff))


/* Endianess-optimized shifting of two u8_t to create one u16_t */
#if BYTE_ORDER == LITTLE_ENDIAN
#define LWIP_MAKE_U16(a, b) (((a) << 8) | (b))
#else
#define LWIP_MAKE_U16(a, b) (((b) << 8) | (a))
#endif

#ifndef LWIP_PLATFORM_BYTESWAP
#define LWIP_PLATFORM_BYTESWAP 0
#endif

/* workaround for naming collisions on some platforms */
#ifdef htons
#define lwip_htons htons
#else
#if (BYTE_ORDER == BIG_ENDIAN)
#define lwip_htons(x) ((u16_t)(x))
#else
u16_t lwip_htons(u16_t x);
#endif
#define htons lwip_htons
#endif /* htons */
#ifdef htonl
#define lwip_htonl htonl
#else
#if (BYTE_ORDER == BIG_ENDIAN)
#define lwip_htonl(x) ((u32_t)(x))
#else
u32_t lwip_htonl(u32_t x);
#endif
#define htonl lwip_htonl
#endif /* htonl */
#ifdef ntohs
#define lwip_ntohs ntohs
#else
#if (BYTE_ORDER == BIG_ENDIAN)
#define lwip_ntohs(x) ((u16_t)(x))
#else
u16_t lwip_ntohs(u16_t x);
#endif
#define ntohs lwip_ntohs
#endif /* ntohs */

#ifdef ntohl
#define lwip_ntohl ntohl
#else
#if (BYTE_ORDER == BIG_ENDIAN)
#define lwip_ntohl(x) ((u32_t)(x))
#else
u32_t lwip_ntohl(u32_t x);
#endif
#define ntohl lwip_ntohl
#endif /* ntohl */

#if (BYTE_ORDER == BIG_ENDIAN)
#define PP_HTONS(x) ((u16_t)(x))
#define PP_NTOHS(x) ((u16_t)(x))
#define PP_HTONL(x) ((u32_t)(x))
#define PP_NTOHL(x) ((u32_t)(x))
#else /* BYTE_ORDER != BIG_ENDIAN */


/* These macros should be calculated by the preprocessor and are used
   with compile-time constants only (so that there is no little-endian
   overhead at runtime). */
#define PP_HTONS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))
#define PP_NTOHS(x) PP_HTONS(x)
#define PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                     (((x) & 0x0000ff00UL) <<  8) | \
                     (((x) & 0x00ff0000UL) >>  8) | \
                     (((x) & 0xff000000UL) >> 24))
#define PP_NTOHL(x) PP_HTONL(x)

#endif /* BYTE_ORDER == BIG_ENDIAN */

/* Functions that are not available as standard implementations.
 * In cc.h, you can #define these to implementations available on
 * your platform to save some code bytes if you use these functions
 * in your application, too.
 */

#ifndef lwip_itoa
/* This can be #defined to itoa() or snprintf(result, bufsize, "%d", number) depending on your platform */
void  lwip_itoa(char *result, size_t bufsize, int number);
#endif
#ifndef lwip_strnicmp
/* This can be #defined to strnicmp() or strncasecmp() depending on your platform */
int   lwip_strnicmp(const char *str1, const char *str2, size_t len);
#endif
#ifndef lwip_stricmp
/* This can be #defined to stricmp() or strcasecmp() depending on your platform */
int   lwip_stricmp(const char *str1, const char *str2);
#endif
#ifndef lwip_strnstr
/* This can be #defined to strnstr() depending on your platform */
char *lwip_strnstr(const char *buffer, const char *token, size_t n);
#endif
#if LWIP_SO_DONTROUTE
/* route entry scope, Actually it is not scope, but distance to the destination. */
typedef enum rt_scope {
  RT_SCOPE_UNIVERSAL, /* everywhere in the Universe */
  RT_SCOPE_LINK, /* destinations located on directly attached link, maybe not same IP network */
  RT_SCOPE_HOST /* our local address */
} rt_scope_t;
#endif

#if defined (__cplusplus) && __cplusplus
}
#endif

#endif /* LWIP_HDR_DEF_H */
