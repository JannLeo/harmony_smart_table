#ifndef YES
#define YES  1
#endif
#ifndef NO
#define NO  0
#endif

#if defined(__LP64__)
#define _Addr long int
#else
#define _Addr int
#endif
#define _Reg int
#if defined(__LP64__)
#define _Int64 long
#else
#define _Int64 long long
#endif

#if __ARMEB__
#define __BYTE_ORDER 4321
#else
#define __BYTE_ORDER 1234
#endif

#define __LONG_MAX 0x7fffffffL

#if defined(__NEED_va_list) && !defined(__DEFINED_va_list)
typedef __builtin_va_list va_list;
#define __DEFINED_va_list
#endif

#if defined(__NEED___isoc_va_list) && !defined(__DEFINED___isoc_va_list)
typedef __builtin_va_list __isoc_va_list;
#define __DEFINED___isoc_va_list
#endif


#ifndef __cplusplus
#if defined(__NEED_wchar_t) && !defined(__DEFINED_wchar_t)
typedef unsigned wchar_t;
#define __DEFINED_wchar_t
#endif

#endif

#if defined(__NEED_float_t) && !defined(__DEFINED_float_t)
typedef float float_t;
#define __DEFINED_float_t
#endif

#if defined(__NEED_double_t) && !defined(__DEFINED_double_t)
typedef double double_t;
#define __DEFINED_double_t
#endif


#if defined(__NEED_max_align_t) && !defined(__DEFINED_max_align_t)
typedef struct { long long __ll; long double __ld; } max_align_t;
#define __DEFINED_max_align_t
#endif


#if defined(__NEED_time_t) && !defined(__DEFINED_time_t)
typedef _Int64 time_t;
#define __DEFINED_time_t
#endif

#if defined(__NEED_suseconds_t) && !defined(__DEFINED_suseconds_t)
typedef _Int64 suseconds_t;
#define __DEFINED_suseconds_t
#endif

#if defined(__NEED_size_t) && !defined(__DEFINED_size_t) && !defined(size_t)
typedef unsigned _Addr size_t;
#define __DEFINED_size_t
#endif

#if defined(__NEED_struct_cpu_set_t) && !defined(__DEFINED_struct_cpu_set_t)
#if defined(__LP64__)
#undef CPU_SETSIZE
#define CPU_SETSIZE 1024
#else
#define CPU_SETSIZE 32
#endif
typedef struct cpu_set_t { unsigned long __bits[CPU_SETSIZE/(8 * sizeof(unsigned long int))]; } cpu_set_t;
#define __DEFINED_struct_cpu_set_t
#endif

#if defined(__NEED_struct_sched_param) && !defined(__DEFINED_struct_sched_param)
struct sched_param {
  int sched_priority;
};
#define __DEFINED_struct_sched_param
#endif

#if defined(__NEED_pthread_attr_t) && !defined(__DEFINED_pthread_attr_t)
typedef struct __pthread_attr_s {
  unsigned int detachstate;
  unsigned int schedpolicy;
  struct sched_param schedparam;
  unsigned int inheritsched;
  unsigned int scope;
  unsigned int stackaddr_set;
  void* stackaddr;
  unsigned int stacksize_set;
  size_t stacksize;
#if (LOSCFG_KERNEL_SMP == YES)
  cpu_set_t cpuset;
#endif
} pthread_attr_t;
#define __DEFINED_pthread_attr_t
#endif

#if defined(__NEED_uintptr_t) && !defined(__DEFINED_uintptr_t)
typedef unsigned _Addr uintptr_t;
#define __DEFINED_uintptr_t
#endif

#if defined(__NEED_ptrdiff_t) && !defined(__DEFINED_ptrdiff_t)
typedef _Addr ptrdiff_t;
#define __DEFINED_ptrdiff_t
#endif

#if defined(__NEED_ssize_t) && !defined(__DEFINED_ssize_t) && !defined(ssize_t)
typedef _Addr ssize_t;
#define __DEFINED_ssize_t
#endif

#if defined(__NEED_intptr_t) && !defined(__DEFINED_intptr_t)
typedef _Addr intptr_t;
#define __DEFINED_intptr_t
#endif

#if defined(__NEED_regoff_t) && !defined(__DEFINED_regoff_t)
typedef _Addr regoff_t;
#define __DEFINED_regoff_t
#endif

#if defined(__NEED_register_t) && !defined(__DEFINED_register_t)
typedef _Reg register_t;
#define __DEFINED_register_t
#endif


#if defined(__NEED_int8_t) && !defined(__DEFINED_int8_t)
typedef signed char     int8_t;
#define __DEFINED_int8_t
#endif

#if defined(__NEED_int16_t) && !defined(__DEFINED_int16_t)
typedef signed short    int16_t;
#define __DEFINED_int16_t
#endif

#if defined(__NEED_int32_t) && !defined(__DEFINED_int32_t)
typedef signed int      int32_t;
#define __DEFINED_int32_t
#endif

#if defined(__NEED_int64_t) && !defined(__DEFINED_int64_t)
typedef signed _Int64   int64_t;
#define __DEFINED_int64_t
#endif

#if defined(__NEED_intmax_t) && !defined(__DEFINED_intmax_t)
typedef signed _Int64   intmax_t;
#define __DEFINED_intmax_t
#endif

#if defined(__NEED_uint8_t) && !defined(__DEFINED_uint8_t)
typedef unsigned char   uint8_t;
#define __DEFINED_uint8_t
#endif

#if defined(__NEED_uint16_t) && !defined(__DEFINED_uint16_t)
typedef unsigned short  uint16_t;
#define __DEFINED_uint16_t
#endif

#if defined(__NEED_uint32_t) && !defined(__DEFINED_uint32_t)
typedef unsigned int    uint32_t;
#define __DEFINED_uint32_t
#endif

#if defined(__NEED_uint64_t) && !defined(__DEFINED_uint64_t)
typedef unsigned _Int64 uint64_t;
#define __DEFINED_uint64_t
#endif

#if defined(__NEED_u_int64_t) && !defined(__DEFINED_u_int64_t)
typedef unsigned _Int64 u_int64_t;
#define __DEFINED_u_int64_t
#endif

#if defined(__NEED_uintmax_t) && !defined(__DEFINED_uintmax_t)
typedef unsigned _Int64 uintmax_t;
#define __DEFINED_uintmax_t
#endif


#if defined(__NEED_mode_t) && !defined(__DEFINED_mode_t)
typedef unsigned mode_t;
#define __DEFINED_mode_t
#endif

#if defined(__NEED_nlink_t) && !defined(__DEFINED_nlink_t)
typedef unsigned _Reg nlink_t;
#define __DEFINED_nlink_t
#endif

#if defined(__NEED_off_t) && !defined(__DEFINED_off_t)
typedef _Int64 off_t;
typedef long long loff_t;
typedef off_t off64_t;
#define __DEFINED_off_t
#endif

#if defined(__NEED_ino_t) && !defined(__DEFINED_ino_t)
typedef unsigned _Int64 ino_t;
#define __DEFINED_ino_t
#endif

#if defined(__NEED_dev_t) && !defined(__DEFINED_dev_t)
typedef unsigned _Int64 dev_t;
#define __DEFINED_dev_t
#endif

#if defined(__NEED_blksize_t) && !defined(__DEFINED_blksize_t)
typedef long blksize_t;
#define __DEFINED_blksize_t
#endif

#if defined(__NEED_blkcnt_t) && !defined(__DEFINED_blkcnt_t)
typedef _Int64 blkcnt_t;
#define __DEFINED_blkcnt_t
#endif

#if defined(__NEED_fsblkcnt_t) && !defined(__DEFINED_fsblkcnt_t)
typedef unsigned _Int64 fsblkcnt_t;
#define __DEFINED_fsblkcnt_t
#endif

#if defined(__NEED_fsfilcnt_t) && !defined(__DEFINED_fsfilcnt_t)
typedef unsigned _Int64 fsfilcnt_t;
#define __DEFINED_fsfilcnt_t
#endif


#if defined(__NEED_wint_t) && !defined(__DEFINED_wint_t)
typedef unsigned wint_t;
#define __DEFINED_wint_t
#endif

#if defined(__NEED_wctype_t) && !defined(__DEFINED_wctype_t)
typedef unsigned long wctype_t;
#define __DEFINED_wctype_t
#endif


#if defined(__NEED_timer_t) && !defined(__DEFINED_timer_t)
typedef void * timer_t;
#define __DEFINED_timer_t
#endif

#if defined(__NEED_clockid_t) && !defined(__DEFINED_clockid_t)
typedef int clockid_t;
#define __DEFINED_clockid_t
#endif

#if defined(__NEED_clock_t) && !defined(__DEFINED_clock_t)
typedef long clock_t;
#define __DEFINED_clock_t
#endif

#if defined(__NEED_struct_timeval) && !defined(__DEFINED_struct_timeval)
struct timeval { time_t tv_sec; suseconds_t tv_usec; };
#if !defined(__LP64__)
struct timeval64 { _Int64 tv_sec; _Int64 tv_usec; };
#endif
#define __DEFINED_struct_timeval
#endif

#if defined(__NEED_struct_timespec) && !defined(__DEFINED_struct_timespec)
struct timespec { time_t tv_sec; long tv_nsec; };
#if !defined(__LP64__)
struct timespec64 { _Int64 tv_sec; _Int64 tv_nsec; };
#endif
#define __DEFINED_struct_timespec
#endif


#if defined(__NEED_pid_t) && !defined(__DEFINED_pid_t)
typedef int pid_t;
#define __DEFINED_pid_t
#endif

#if defined(__NEED_id_t) && !defined(__DEFINED_id_t)
typedef unsigned id_t;
#define __DEFINED_id_t
#endif

#if defined(__NEED_uid_t) && !defined(__DEFINED_uid_t)
typedef unsigned uid_t;
#define __DEFINED_uid_t
#endif

#if defined(__NEED_gid_t) && !defined(__DEFINED_gid_t)
typedef unsigned gid_t;
#define __DEFINED_gid_t
#endif

#if defined(__NEED_key_t) && !defined(__DEFINED_key_t)
typedef int key_t;
#define __DEFINED_key_t
#endif

#if defined(__NEED_useconds_t) && !defined(__DEFINED_useconds_t)
typedef unsigned useconds_t;
#define __DEFINED_useconds_t
#endif


#if defined(__NEED_pthread_t) && !defined(__DEFINED_pthread_t)
typedef long pthread_t;
#define __DEFINED_pthread_t
#endif

#if defined(__NEED_pthread_once_t) && !defined(__DEFINED_pthread_once_t)
typedef int pthread_once_t;
#define __DEFINED_pthread_once_t
#endif

#if defined(__NEED_pthread_key_t) && !defined(__DEFINED_pthread_key_t)
typedef int pthread_key_t;
#define __DEFINED_pthread_key_t
#endif

#if defined(__NEED_pthread_condattr_t) && !defined(__DEFINED_pthread_condattr_t)
typedef int pthread_condattr_t;
#define __DEFINED_pthread_condattr_t
#endif

#if defined(__NEED_locale_t) && !defined(__DEFINED_locale_t)
typedef struct __locale_struct *locale_t;
#define __DEFINED_locale_t
#endif

#if defined(__NEED_struct__IO_FILE) && !defined(__DEFINED_struct__IO_FILE)
struct _IO_FILE {
  unsigned flags;
  unsigned char *rpos, *rend;
  int (*close)(struct _IO_FILE *);
  unsigned char *wend, *wpos;
  unsigned char *mustbezero_1;
  unsigned char *wbase;
  size_t (*read)(struct _IO_FILE *, unsigned char *, size_t);
  size_t (*write)(struct _IO_FILE *, const unsigned char *, size_t);
  off_t (*seek)(struct _IO_FILE *, off_t, int);
  unsigned char *buf;
  size_t buf_size;
  struct _IO_FILE *prev, *next;
  int fd;
  int pipe_pid;
  int mode;
  void *lock;
  int lbf;
  void *cookie;
  off_t off;
  char *getln_buf;
  void *mustbezero_2;
  unsigned char *shend;
  off_t shlim, shcnt;
  struct __locale_struct *locale;
};
#define __DEFINED_struct__IO_FILE
#endif

#if defined(__NEED_FILE) && !defined(__DEFINED_FILE)
typedef struct _IO_FILE FILE;
#define __DEFINED_FILE
#endif

#if defined(__NEED_mbstate_t) && !defined(__DEFINED_mbstate_t)
typedef struct __mbstate_t { unsigned __opaque1, __opaque2; } mbstate_t;
#define __DEFINED_mbstate_t
#endif

#if defined(__NEED_sigset_t) && !defined(__DEFINED_sigset_t)
typedef unsigned _Int64 sigset_t;
#define __DEFINED_sigset_t
#endif

#if defined(__NEED_struct_iovec) && !defined(__DEFINED_struct_iovec)
struct iovec { void *iov_base; size_t iov_len; };
#define __DEFINED_struct_iovec
#endif


#if defined(__NEED_socklen_t) && !defined(__DEFINED_socklen_t)
typedef unsigned socklen_t;
#define __DEFINED_socklen_t
#endif

#if defined(__NEED_sa_family_t) && !defined(__DEFINED_sa_family_t)
typedef unsigned short sa_family_t;
#define __DEFINED_sa_family_t
#endif


#undef _Addr
#undef _Int64
#undef _Reg
