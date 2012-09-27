// ======================================================================,
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

#ifdef __unix__
# error "should not be included"
#endif

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *	@(#)stdio.h	5.3 (Berkeley) 3/15/86
 */

/*
 * NB: to fit things in six character monocase externals, the
 * stdio code uses the prefix `__s' for stdio objects, typically
 * followed by a three-character attempt at a mnemonic.
 */

#ifndef _STDIO_H_
#define	_STDIO_H_

//#include "_ansi.h"
/* Provide support for both ANSI and non-ANSI environments.  */

/* Some ANSI environments are "broken" in the sense that __STDC__ cannot be
   relied upon to have it's intended meaning.  Therefore we must use our own
   concoction: _HAVE_STDC.  Always use _HAVE_STDC instead of __STDC__ in newlib
   sources!

   To get a strict ANSI C environment, define macro __STRICT_ANSI__.  This will
   "comment out" the non-ANSI parts of the ANSI header files (non-ANSI header
   files aren't affected).  */

#ifndef	_ANSIDECL_H_
#define	_ANSIDECL_H_

/* First try to figure out whether we really are in an ANSI C environment.  */
/* FIXME: This probably needs some work.  Perhaps sys/config.h can be
   prevailed upon to give us a clue.  */

#ifdef __STDC__
#define _HAVE_STDC
#endif

#ifdef _WIN32
#define __CYGWIN__
#endif

#define HAVE_FCNTL 1

/*  ISO C++.  */

#ifdef __cplusplus
#if !(defined(_BEGIN_STD_C) && defined(_END_STD_C))
#ifdef _HAVE_STD_CXX
#define _BEGIN_STD_C namespace std { extern "C" {
#define _END_STD_C  } }
#else
#define _BEGIN_STD_C extern "C" {
#define _END_STD_C  }
#endif
#if defined(__GNUC__) && \
  ( (__GNUC__ >= 4) || \
    ( (__GNUC__ >= 3) && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ >= 3) ) )
#define _NOTHROW __attribute__ ((nothrow))
#else
#define _NOTHROW throw()
#endif
#endif
#else
#define _BEGIN_STD_C
#define _END_STD_C
#define _NOTHROW
#endif

#ifdef _HAVE_STDC
#define	_PTR		void *
#define	_AND		,
#define	_NOARGS		void
#define	_CONST		const
#define	_VOLATILE	volatile
#define	_SIGNED		signed
#define	_DOTS		, ...
#define _VOID void
#ifdef __CYGWIN__
#define	_EXFUN_NOTHROW(name, proto)	__cdecl name proto _NOTHROW
#define	_EXFUN(name, proto)		__cdecl name proto
#define	_EXPARM(name, proto)		(* __cdecl name) proto
#else
#define	_EXFUN_NOTHROW(name, proto)	name proto _NOTHROW
#define	_EXFUN(name, proto)		name proto
#define _EXPARM(name, proto)		(* name) proto
#endif
#define	_DEFUN(name, arglist, args)	name(args)
#define	_DEFUN_VOID(name)		name(_NOARGS)
#define _CAST_VOID (void)
#ifndef _LONG_DOUBLE
#define _LONG_DOUBLE long double
#endif
#ifndef _LONG_LONG_TYPE
#define _LONG_LONG_TYPE long long
#endif
#ifndef _PARAMS
#define _PARAMS(paramlist)		paramlist
#endif
#else
#define	_PTR		char *
#define	_AND		;
#define	_NOARGS
#define	_CONST
#define	_VOLATILE
#define	_SIGNED
#define	_DOTS
#define _VOID void
#define	_EXFUN(name, proto)		name()
#define	_EXFUN_NOTHROW(name, proto)	name()
#define	_DEFUN(name, arglist, args)	name arglist args;
#define	_DEFUN_VOID(name)		name()
#define _CAST_VOID
#define _LONG_DOUBLE double
#define _LONG_LONG_TYPE long
#ifndef _PARAMS
#define _PARAMS(paramlist)		()
#endif
#endif

/* Support gcc's __attribute__ facility.  */

#ifdef __GNUC__
#define _ATTRIBUTE(attrs) __attribute__ (attrs)
#else
#define _ATTRIBUTE(attrs)
#endif

/*  The traditional meaning of 'extern inline' for GCC is not
  to emit the function body unless the address is explicitly
  taken.  However this behaviour is changing to match the C99
  standard, which uses 'extern inline' to indicate that the
  function body *must* be emitted.  If we are using GCC, but do
  not have the new behaviour, we need to use extern inline; if
  we are using a new GCC with the C99-compatible behaviour, or
  a non-GCC compiler (which we will have to hope is C99, since
  there is no other way to achieve the effect of omitting the
  function if it isn't referenced) we just use plain 'inline',
  which c99 defines to mean more-or-less the same as the Gnu C
  'extern inline'.  */
#if defined(__GNUC__) && !defined(__GNUC_STDC_INLINE__)
/* We're using GCC, but without the new C99-compatible behaviour.  */
#define _ELIDABLE_INLINE extern __inline__ _ATTRIBUTE ((__always_inline__))
#else
/* We're using GCC in C99 mode, or an unknown compiler which
  we just have to hope obeys the C99 semantics of inline.  */
#define _ELIDABLE_INLINE __inline__
#endif

#endif /* _ANSIDECL_H_ */

#ifndef __SYS_LOCK_H__
#define __SYS_LOCK_H__

/* dummy lock routines for single-threaded aps */

typedef int _LOCK_T;
typedef int _LOCK_RECURSIVE_T;

//#include <_ansi.h>

#define __LOCK_INIT(class,lock) static int lock = 0;
#define __LOCK_INIT_RECURSIVE(class,lock) static int lock = 0;
#define __lock_init(lock) (_CAST_VOID 0)
#define __lock_init_recursive(lock) (_CAST_VOID 0)
#define __lock_close(lock) (_CAST_VOID 0)
#define __lock_close_recursive(lock) (_CAST_VOID 0)
#define __lock_acquire(lock) (_CAST_VOID 0)
#define __lock_acquire_recursive(lock) (_CAST_VOID 0)
#define __lock_try_acquire(lock) (_CAST_VOID 0)
#define __lock_try_acquire_recursive(lock) (_CAST_VOID 0)
#define __lock_release(lock) (_CAST_VOID 0)
#define __lock_release_recursive(lock) (_CAST_VOID 0)

#endif /* __SYS_LOCK_H__ */

/*
 *  $Id: _types.h,v 1.3 2007/09/07 21:16:25 jjohnstn Exp $
 */

#ifndef _MACHINE__TYPES_H
#define _MACHINE__TYPES_H
//#include <machine/_default_types.h>
/*
 *  $Id: _default_types.h,v 1.2 2008/06/11 22:14:54 jjohnstn Exp $
 */

#ifndef _MACHINE__DEFAULT_TYPES_H
#define _MACHINE__DEFAULT_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Guess on types by examining *_MIN / *_MAX defines.
 */
#if defined(__GNUC__) && ((__GNUC__ >= 4) || (__GNUC__ >= 3 ) \
  && defined(__GNUC_MINOR__) && (__GNUC_MINOR__ > 2 ))
/* GCC >= 3.3.0 has __<val>__ implicitly defined. */
#define __EXP(x) __##x##__
#else
/* Fall back to POSIX versions from <limits.h> */
#define __EXP(x) x
#include <limits.h>
#endif

#if __EXP(SCHAR_MAX) == 0x7f
typedef signed char __int8_t ;
typedef unsigned char __uint8_t ;
#define ___int8_t_defined 1
#endif

#if __EXP(INT_MAX) == 0x7fff
typedef signed int __int16_t;
typedef unsigned int __uint16_t;
#define ___int16_t_defined 1
#elif __EXP(SHRT_MAX) == 0x7fff
typedef signed short __int16_t;
typedef unsigned short __uint16_t;
#define ___int16_t_defined 1
#elif __EXP(SCHAR_MAX) == 0x7fff
typedef signed char __int16_t;
typedef unsigned char __uint16_t;
#define ___int16_t_defined 1
#endif

#if ___int16_t_defined
typedef __int16_t __int_least16_t;
typedef __uint16_t __uint_least16_t;
#define ___int_least16_t_defined 1

#if !___int8_t_defined
typedef __int16_t __int_least8_t;
typedef __uint16_t __uint_least8_t;
#define ___int_least8_t_defined 1
#endif
#endif

#if __EXP(INT_MAX) == 0x7fffffffL
typedef signed int __int32_t;
typedef unsigned int __uint32_t;
#define ___int32_t_defined 1
#elif __EXP(LONG_MAX) == 0x7fffffffL
typedef signed long __int32_t;
typedef unsigned long __uint32_t;
#define ___int32_t_defined 1
#elif __EXP(SHRT_MAX) == 0x7fffffffL
typedef signed short __int32_t;
typedef unsigned short __uint32_t;
#define ___int32_t_defined 1
#elif __EXP(SCHAR_MAX) == 0x7fffffffL
typedef signed char __int32_t;
typedef unsigned char __uint32_t;
#define ___int32_t_defined 1
#endif

#if ___int32_t_defined
typedef __int32_t __int_least32_t;
typedef __uint32_t __uint_least32_t;
#define ___int_least32_t_defined 1

#if !___int8_t_defined
typedef __int32_t __int_least8_t;
typedef __uint32_t __uint_least8_t;
#define ___int_least8_t_defined 1
#endif
#if !___int16_t_defined
typedef __int32_t __int_least16_t;
typedef __uint32_t __uint_least16_t;
#define ___int_least16_t_defined 1
#endif
#endif

#if __EXP(LONG_MAX) > 0x7fffffff
typedef signed long __int64_t;
typedef unsigned long __uint64_t;
#define ___int64_t_defined 1

/* GCC has __LONG_LONG_MAX__ */
#elif  defined(__LONG_LONG_MAX__) && (__LONG_LONG_MAX__ > 0x7fffffff)
typedef signed long long __int64_t;
typedef unsigned long long __uint64_t;
#define ___int64_t_defined 1

/* POSIX mandates LLONG_MAX in <limits.h> */
#elif  defined(LLONG_MAX) && (LLONG_MAX > 0x7fffffff)
typedef signed long long __int64_t;
typedef unsigned long long __uint64_t;
#define ___int64_t_defined 1

#elif  __EXP(INT_MAX) > 0x7fffffff
typedef signed int __int64_t;
typedef unsigned int __uint64_t;
#define ___int64_t_defined 1
#endif

#undef __EXP

#ifdef __cplusplus
}
#endif

#endif /* _MACHINE__DEFAULT_TYPES_H */
#endif
/* ANSI C namespace clean utility typedefs */

/* This file defines various typedefs needed by the system calls that support
   the C library.  Basically, they're just the POSIX versions with an '_'
   prepended.  This file lives in the `sys' directory so targets can provide
   their own if desired (or they can put target dependant conditionals here).
*/

#ifndef	_SYS__TYPES_H
#define _SYS__TYPES_H

//#include <machine/_types.h>
//#include <sys/lock.h>

typedef long _off_t;
typedef long off_t;
typedef long ssize_t;

#if defined(__rtems__)
/* device numbers are 32-bit major and and 32-bit minor */
typedef unsigned long long __dev_t;
#else
#ifndef __dev_t_defined
#if __WORDSIZE == 64
typedef unsigned long __dev_t;
#else
typedef unsigned long long __dev_t;
#endif
#endif
#endif

#ifndef __uid_t_defined
typedef unsigned __uid_t;
#endif
#ifndef __gid_t_defined
typedef unsigned __gid_t;
#endif

#ifndef __off64_t_defined
__extension__ typedef long long _off64_t;
#endif

/*
 * We need fpos_t for the following, but it doesn't have a leading "_",
 * so we use _fpos_t instead.
 */
#ifndef __fpos_t_defined
typedef long _fpos_t;		/* XXX must match off_t in <sys/types.h> */
				/* (and must be `long' for now) */
#endif

#ifdef __LARGE64_FILES
#ifndef __fpos64_t_defined
typedef _off64_t _fpos64_t;
#endif
#endif

#ifndef __ssize_t_defined
#if defined(__INT_MAX__) && __INT_MAX__ == 2147483647
typedef int _ssize_t;
#else
typedef long _ssize_t;
#endif
#endif

#define __need_wint_t
#include <stddef.h>

#ifndef __mbstate_t_defined
/* Conversion state information.  */
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    unsigned char __wchb[4];
  } __value;		/* Value so far.  */
} _mbstate_t;
#endif

#ifndef __flock_t_defined
typedef _LOCK_RECURSIVE_T _flock_t;
#endif

#ifndef __iconv_t_defined
/* Iconv descriptor type */
typedef void *_iconv_t;
#endif

#endif	/* _SYS__TYPES_H */

/* This header file provides the reentrancy.  */

/* WARNING: All identifiers here must begin with an underscore.  This file is
   included by stdio.h and others and we therefore must only use identifiers
   in the namespace allotted to us.  */

#ifndef _SYS_REENT_H_
#ifdef __cplusplus
extern "C" {
#endif
#define _SYS_REENT_H_

//#include <_ansi.h>
//#include <sys/_types.h>

#define _NULL 0

#ifndef __Long
#if __LONG_MAX__ == 2147483647L
#define __Long long
typedef unsigned __Long __ULong;
#elif __INT_MAX__ == 2147483647
#define __Long int
typedef unsigned __Long __ULong;
#endif
#endif

#if !defined( __Long)
#include <sys/types.h>
#endif

#ifndef __Long
#define __Long __int32_t
typedef __uint32_t __ULong;
#endif

struct _reent;

/*
 * If _REENT_SMALL is defined, we make struct _reent as small as possible,
 * by having nearly everything possible allocated at first use.
 */

struct _Bigint
{
  struct _Bigint *_next;
  int _k, _maxwds, _sign, _wds;
  __ULong _x[1];
};

/* needed by reentrant structure */
struct __tm
{
  int   __tm_sec;
  int   __tm_min;
  int   __tm_hour;
  int   __tm_mday;
  int   __tm_mon;
  int   __tm_year;
  int   __tm_wday;
  int   __tm_yday;
  int   __tm_isdst;
};

/*
 * atexit() support.
 */

#define	_ATEXIT_SIZE 32	/* must be at least 32 to guarantee ANSI conformance */

struct _on_exit_args {
	void *  _fnargs[_ATEXIT_SIZE];	        /* user fn args */
	void *	_dso_handle[_ATEXIT_SIZE];
	/* Bitmask is set if user function takes arguments.  */
	__ULong _fntypes;           	        /* type of exit routine -
				   Must have at least _ATEXIT_SIZE bits */
	/* Bitmask is set if function was registered via __cxa_atexit.  */
	__ULong _is_cxa;
};

#ifdef _REENT_SMALL
struct _atexit {
	struct	_atexit *_next;			/* next in list */
	int	_ind;				/* next index in this table */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
        struct _on_exit_args * _on_exit_args_ptr;
};
#else
struct _atexit {
	struct	_atexit *_next;			/* next in list */
	int	_ind;				/* next index in this table */
	/* Some entries may already have been called, and will be NULL.  */
	void	(*_fns[_ATEXIT_SIZE])(void);	/* the table itself */
        struct _on_exit_args _on_exit_args;
};
#endif

/*
 * Stdio buffers.
 *
 * This and __FILE are defined here because we need them for struct _reent,
 * but we don't want stdio.h included when stdlib.h is.
 */

struct __sbuf {
	unsigned char *_base;
	int	_size;
};

/*
 * Stdio state variables.
 *
 * The following always hold:
 *
 *	if (_flags&(__SLBF|__SWR)) == (__SLBF|__SWR),
 *		_lbfsize is -_bf._size, else _lbfsize is 0
 *	if _flags&__SRD, _w is 0
 *	if _flags&__SWR, _r is 0
 *
 * This ensures that the getc and putc macros (or inline functions) never
 * try to write or read from a file that is in `read' or `write' mode.
 * (Moreover, they can, and do, automatically switch from read mode to
 * write mode, and back, on "r+" and "w+" files.)
 *
 * _lbfsize is used only to make the inline line-buffered output stream
 * code as compact as possible.
 *
 * _ub, _up, and _ur are used when ungetc() pushes back more characters
 * than fit in the current _bf, or when ungetc() pushes back a character
 * that does not match the previous one in _bf.  When this happens,
 * _ub._base becomes non-nil (i.e., a stream has ungetc() data iff
 * _ub._base!=NULL) and _up and _ur save the current values of _p and _r.
 */

#ifdef _REENT_SMALL
/*
 * struct __sFILE_fake is the start of a struct __sFILE, with only the
 * minimal fields allocated.  In __sinit() we really allocate the 3
 * standard streams, etc., and point away from this fake.
 */
struct __sFILE_fake {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

  struct _reent *_data;
};

/* Following is needed both in libc/stdio and libc/stdlib so we put it
 * here instead of libc/stdio/local.h where it was previously. */

extern _VOID   _EXFUN(__sinit,(struct _reent *));

# define _REENT_SMALL_CHECK_INIT(ptr)		\
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
    }						\
  while (0)
#else
# define _REENT_SMALL_CHECK_INIT(ptr) /* nothing */
#endif

#define _READ_WRITE_RETURN_TYPE _ssize_t

struct __sFILE {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

#ifdef _REENT_SMALL
  struct _reent *_data;
#endif

  /* operations */
  _PTR	_cookie;	/* cookie passed to io functions */

  _READ_WRITE_RETURN_TYPE _EXFUN((*_read),(struct _reent *, _PTR,
					   char *, int));
  _READ_WRITE_RETURN_TYPE _EXFUN((*_write),(struct _reent *, _PTR,
					    const char *, int));
  _fpos_t _EXFUN((*_seek),(struct _reent *, _PTR, _fpos_t, int));
  int _EXFUN((*_close),(struct _reent *, _PTR));

  /* separate buffer for long sequences of ungetc() */
  struct __sbuf _ub;	/* ungetc buffer */
  unsigned char *_up;	/* saved _p when _p is doing ungetc data */
  int	_ur;		/* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
  unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
  unsigned char _nbuf[1];	/* guarantee a getc() buffer */

  /* separate buffer for fgetline() when line crosses buffer boundary */
  struct __sbuf _lb;	/* buffer for fgetline() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
  int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
  int	_offset;	/* current lseek offset */

#ifndef _REENT_SMALL
  struct _reent *_data;	/* Here for binary compatibility? Remove? */
#endif

#ifndef __SINGLE_THREAD__
  _flock_t _lock;	/* for thread-safety locking */
#endif
  _mbstate_t _mbstate;	/* for wide char stdio functions. */
  int   _flags2;        /* for future use */
};

#ifdef __CUSTOM_FILE_IO__

/* Get custom _FILE definition.  */
//#include <sys/custom_file.h>

#else /* !__CUSTOM_FILE_IO__ */
#ifdef __LARGE64_FILES
struct __sFILE64 {
  unsigned char *_p;	/* current position in (some) buffer */
  int	_r;		/* read space left for getc() */
  int	_w;		/* write space left for putc() */
  short	_flags;		/* flags, below; this FILE is free if 0 */
  short	_file;		/* fileno, if Unix descriptor, else -1 */
  struct __sbuf _bf;	/* the buffer (at least 1 byte, if !NULL) */
  int	_lbfsize;	/* 0 or -_bf._size, for inline putc */

  struct _reent *_data;

  /* operations */
  _PTR	_cookie;	/* cookie passed to io functions */

  _READ_WRITE_RETURN_TYPE _EXFUN((*_read),(struct _reent *, _PTR,
					   char *, int));
  _READ_WRITE_RETURN_TYPE _EXFUN((*_write),(struct _reent *, _PTR,
					    const char *, int));
  _fpos_t _EXFUN((*_seek),(struct _reent *, _PTR, _fpos_t, int));
  int _EXFUN((*_close),(struct _reent *, _PTR));

  /* separate buffer for long sequences of ungetc() */
  struct __sbuf _ub;	/* ungetc buffer */
  unsigned char *_up;	/* saved _p when _p is doing ungetc data */
  int	_ur;		/* saved _r when _r is counting ungetc data */

  /* tricks to meet minimum requirements even when malloc() fails */
  unsigned char _ubuf[3];	/* guarantee an ungetc() buffer */
  unsigned char _nbuf[1];	/* guarantee a getc() buffer */

  /* separate buffer for fgetline() when line crosses buffer boundary */
  struct __sbuf _lb;	/* buffer for fgetline() */

  /* Unix stdio files get aligned to block boundaries on fseek() */
  int	_blksize;	/* stat.st_blksize (may be != _bf._size) */
  int   _flags2;        /* for future use */

  _off64_t _offset;     /* current lseek offset */
  _fpos64_t _EXFUN((*_seek64),(struct _reent *, _PTR, _fpos64_t, int));

#ifndef __SINGLE_THREAD__
  _flock_t _lock;	/* for thread-safety locking */
#endif
  _mbstate_t _mbstate;	/* for wide char stdio functions. */
};
typedef struct __sFILE64 __FILE;
#else
typedef struct __sFILE   __FILE;
#endif /* __LARGE64_FILES */
#endif /* !__CUSTOM_FILE_IO__ */

struct _glue
{
  struct _glue *_next;
  int _niobs;
  __FILE *_iobs;
};

/*
 * rand48 family support
 *
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 */
#define        _RAND48_SEED_0  (0x330e)
#define        _RAND48_SEED_1  (0xabcd)
#define        _RAND48_SEED_2  (0x1234)
#define        _RAND48_MULT_0  (0xe66d)
#define        _RAND48_MULT_1  (0xdeec)
#define        _RAND48_MULT_2  (0x0005)
#define        _RAND48_ADD     (0x000b)
struct _rand48 {
  unsigned short _seed[3];
  unsigned short _mult[3];
  unsigned short _add;
#ifdef _REENT_SMALL
  /* Put this in here as well, for good luck.  */
  __extension__ unsigned long long _rand_next;
#endif
};

/* How big the some arrays are.  */
#define _REENT_EMERGENCY_SIZE 25
#define _REENT_ASCTIME_SIZE 26
#define _REENT_SIGNAL_SIZE 24

/*
 * struct _reent
 *
 * This structure contains *all* globals needed by the library.
 * It's raison d'etre is to facilitate threads by making all library routines
 * reentrant.  IE: All state information is contained here.
 */

#ifdef _REENT_SMALL

struct _mprec
{
  /* used by mprec routines */
  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;
};


struct _misc_reent
{
  /* miscellaneous reentrant data */
  char *_strtok_last;
  _mbstate_t _mblen_state;
  _mbstate_t _wctomb_state;
  _mbstate_t _mbtowc_state;
  char _l64a_buf[8];
  int _getdate_err;
  _mbstate_t _mbrlen_state;
  _mbstate_t _mbrtowc_state;
  _mbstate_t _mbsrtowcs_state;
  _mbstate_t _wcrtomb_state;
  _mbstate_t _wcsrtombs_state;
};

/* This version of _reent is layed our with "int"s in pairs, to help
 * ports with 16-bit int's but 32-bit pointers, align nicely.  */
struct _reent
{

  /* FILE is a big struct and may change over time.  To try to achieve binary
     compatibility with future versions, put stdin,stdout,stderr here.
     These are pointers into member __sf defined below.  */
  __FILE *_stdin, *_stdout, *_stderr;	/* XXX */

  int _errno;			/* local copy of errno */

  int  _inc;			/* used by tmpnam */

  char *_emergency;

  int __sdidinit;		/* 1 means stdio has been init'd */

  int _current_category;	/* unused */
  _CONST char *_current_locale;	/* unused */

  struct _mprec *_mp;

  void _EXFUN((*__cleanup),(struct _reent *));

  int _gamma_signgam;

  /* used by some fp conversion routines */
  int _cvtlen;			/* should be size_t */
  char *_cvtbuf;

  struct _rand48 *_r48;
  struct __tm *_localtime_buf;
  char *_asctime_buf;

  /* signal info */
  void (**(_sig_func))(int);

  /* atexit stuff */
  struct _atexit *_atexit;
  struct _atexit _atexit0;

  struct _glue __sglue;			/* root of glue chain */
  __FILE *__sf;			        /* file descriptors */
  struct _misc_reent *_misc;            /* strtok, multibyte states */
  char *_signal_buf;                    /* strsignal */
};

extern const struct __sFILE_fake __sf_fake_stdin;
extern const struct __sFILE_fake __sf_fake_stdout;
extern const struct __sFILE_fake __sf_fake_stderr;

#define _REENT_INIT(var) \
  { (__FILE *)&__sf_fake_stdin, \
    (__FILE *)&__sf_fake_stdout, \
    (__FILE *)&__sf_fake_stderr, \
    0, \
    0, \
    _NULL, \
    0, \
    0, \
    "C", \
    _NULL, \
    _NULL, \
    0, \
    0, \
    _NULL, \
    _NULL, \
    _NULL, \
    _NULL, \
    _NULL, \
    _NULL, \
    {_NULL, 0, {_NULL}, _NULL}, \
    {_NULL, 0, _NULL}, \
    _NULL, \
    _NULL, \
    _NULL \
  }

#define _REENT_INIT_PTR(var) \
  { (var)->_stdin = (__FILE *)&__sf_fake_stdin; \
    (var)->_stdout = (__FILE *)&__sf_fake_stdout; \
    (var)->_stderr = (__FILE *)&__sf_fake_stderr; \
    (var)->_errno = 0; \
    (var)->_inc = 0; \
    (var)->_emergency = _NULL; \
    (var)->__sdidinit = 0; \
    (var)->_current_category = 0; \
    (var)->_current_locale = "C"; \
    (var)->_mp = _NULL; \
    (var)->__cleanup = _NULL; \
    (var)->_gamma_signgam = 0; \
    (var)->_cvtlen = 0; \
    (var)->_cvtbuf = _NULL; \
    (var)->_r48 = _NULL; \
    (var)->_localtime_buf = _NULL; \
    (var)->_asctime_buf = _NULL; \
    (var)->_sig_func = _NULL; \
    (var)->_atexit = _NULL; \
    (var)->_atexit0._next = _NULL; \
    (var)->_atexit0._ind = 0; \
    (var)->_atexit0._fns[0] = _NULL; \
    (var)->_atexit0._on_exit_args_ptr = _NULL; \
    (var)->__sglue._next = _NULL; \
    (var)->__sglue._niobs = 0; \
    (var)->__sglue._iobs = _NULL; \
    (var)->__sf = 0; \
    (var)->_misc = _NULL; \
    (var)->_signal_buf = _NULL; \
  }

/* Only built the assert() calls if we are built with debugging.  */
#if DEBUG
#include <assert.h>
#define __reent_assert(x) assert(x)
#else
#define __reent_assert(x) ((void)0)
#endif

#ifdef __CUSTOM_FILE_IO__
#error Custom FILE I/O and _REENT_SMALL not currently supported.
#endif

/* Generic _REENT check macro.  */
#define _REENT_CHECK(var, what, type, size, init) do { \
  struct _reent *_r = (var); \
  if (_r->what == NULL) { \
    _r->what = (type)malloc(size); \
    __reent_assert(_r->what); \
    init; \
  } \
} while (0)

#define _REENT_CHECK_TM(var) \
  _REENT_CHECK(var, _localtime_buf, struct __tm *, sizeof *((var)->_localtime_buf), \
    /* nothing */)

#define _REENT_CHECK_ASCTIME_BUF(var) \
  _REENT_CHECK(var, _asctime_buf, char *, _REENT_ASCTIME_SIZE, \
    memset((var)->_asctime_buf, 0, _REENT_ASCTIME_SIZE))

/* Handle the dynamically allocated rand48 structure. */
#define _REENT_INIT_RAND48(var) do { \
  struct _reent *_r = (var); \
  _r->_r48->_seed[0] = _RAND48_SEED_0; \
  _r->_r48->_seed[1] = _RAND48_SEED_1; \
  _r->_r48->_seed[2] = _RAND48_SEED_2; \
  _r->_r48->_mult[0] = _RAND48_MULT_0; \
  _r->_r48->_mult[1] = _RAND48_MULT_1; \
  _r->_r48->_mult[2] = _RAND48_MULT_2; \
  _r->_r48->_add = _RAND48_ADD; \
  _r->_r48->_rand_next = 1; \
} while (0)
#define _REENT_CHECK_RAND48(var) \
  _REENT_CHECK(var, _r48, struct _rand48 *, sizeof *((var)->_r48), _REENT_INIT_RAND48((var)))

#define _REENT_INIT_MP(var) do { \
  struct _reent *_r = (var); \
  _r->_mp->_result_k = 0; \
  _r->_mp->_result = _r->_mp->_p5s = _NULL; \
  _r->_mp->_freelist = _NULL; \
} while (0)
#define _REENT_CHECK_MP(var) \
  _REENT_CHECK(var, _mp, struct _mprec *, sizeof *((var)->_mp), _REENT_INIT_MP(var))

#define _REENT_CHECK_EMERGENCY(var) \
  _REENT_CHECK(var, _emergency, char *, _REENT_EMERGENCY_SIZE, /* nothing */)

#define _REENT_INIT_MISC(var) do { \
  struct _reent *_r = (var); \
  _r->_misc->_strtok_last = _NULL; \
  _r->_misc->_mblen_state.__count = 0; \
  _r->_misc->_mblen_state.__value.__wch = 0; \
  _r->_misc->_wctomb_state.__count = 0; \
  _r->_misc->_wctomb_state.__value.__wch = 0; \
  _r->_misc->_mbtowc_state.__count = 0; \
  _r->_misc->_mbtowc_state.__value.__wch = 0; \
  _r->_misc->_mbrlen_state.__count = 0; \
  _r->_misc->_mbrlen_state.__value.__wch = 0; \
  _r->_misc->_mbrtowc_state.__count = 0; \
  _r->_misc->_mbrtowc_state.__value.__wch = 0; \
  _r->_misc->_mbsrtowcs_state.__count = 0; \
  _r->_misc->_mbsrtowcs_state.__value.__wch = 0; \
  _r->_misc->_wcrtomb_state.__count = 0; \
  _r->_misc->_wcrtomb_state.__value.__wch = 0; \
  _r->_misc->_wcsrtombs_state.__count = 0; \
  _r->_misc->_wcsrtombs_state.__value.__wch = 0; \
  _r->_misc->_l64a_buf[0] = '\0'; \
  _r->_misc->_getdate_err = 0; \
} while (0)
#define _REENT_CHECK_MISC(var) \
  _REENT_CHECK(var, _misc, struct _misc_reent *, sizeof *((var)->_misc), _REENT_INIT_MISC(var))

#define _REENT_CHECK_SIGNAL_BUF(var) \
  _REENT_CHECK(var, _signal_buf, char *, _REENT_SIGNAL_SIZE, /* nothing */)

#define _REENT_SIGNGAM(ptr)	((ptr)->_gamma_signgam)
#define _REENT_RAND_NEXT(ptr)	((ptr)->_r48->_rand_next)
#define _REENT_RAND48_SEED(ptr)	((ptr)->_r48->_seed)
#define _REENT_RAND48_MULT(ptr)	((ptr)->_r48->_mult)
#define _REENT_RAND48_ADD(ptr)	((ptr)->_r48->_add)
#define _REENT_MP_RESULT(ptr)	((ptr)->_mp->_result)
#define _REENT_MP_RESULT_K(ptr)	((ptr)->_mp->_result_k)
#define _REENT_MP_P5S(ptr)	((ptr)->_mp->_p5s)
#define _REENT_MP_FREELIST(ptr)	((ptr)->_mp->_freelist)
#define _REENT_ASCTIME_BUF(ptr)	((ptr)->_asctime_buf)
#define _REENT_TM(ptr)		((ptr)->_localtime_buf)
#define _REENT_EMERGENCY(ptr)	((ptr)->_emergency)
#define _REENT_STRTOK_LAST(ptr)	((ptr)->_misc->_strtok_last)
#define _REENT_MBLEN_STATE(ptr)	((ptr)->_misc->_mblen_state)
#define _REENT_MBTOWC_STATE(ptr)((ptr)->_misc->_mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)((ptr)->_misc->_wctomb_state)
#define _REENT_MBRLEN_STATE(ptr) ((ptr)->_misc->_mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr) ((ptr)->_misc->_mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr) ((ptr)->_misc->_mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr) ((ptr)->_misc->_wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr) ((ptr)->_misc->_wcsrtombs_state)
#define _REENT_L64A_BUF(ptr)    ((ptr)->_misc->_l64a_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->_misc->_getdate_err))
#define _REENT_SIGNAL_BUF(ptr)  ((ptr)->_signal_buf)

#else /* !_REENT_SMALL */

struct _reent
{
  int _errno;			/* local copy of errno */

  /* FILE is a big struct and may change over time.  To try to achieve binary
     compatibility with future versions, put stdin,stdout,stderr here.
     These are pointers into member __sf defined below.  */
  __FILE *_stdin, *_stdout, *_stderr;

  int  _inc;			/* used by tmpnam */
  char _emergency[_REENT_EMERGENCY_SIZE];

  int _current_category;	/* used by setlocale */
  _CONST char *_current_locale;

  int __sdidinit;		/* 1 means stdio has been init'd */

  void _EXFUN((*__cleanup),(struct _reent *));

  /* used by mprec routines */
  struct _Bigint *_result;
  int _result_k;
  struct _Bigint *_p5s;
  struct _Bigint **_freelist;

  /* used by some fp conversion routines */
  int _cvtlen;			/* should be size_t */
  char *_cvtbuf;

  union
    {
      struct
        {
          unsigned int _unused_rand;
          char * _strtok_last;
          char _asctime_buf[_REENT_ASCTIME_SIZE];
          struct __tm _localtime_buf;
          int _gamma_signgam;
          __extension__ unsigned long long _rand_next;
          struct _rand48 _r48;
          _mbstate_t _mblen_state;
          _mbstate_t _mbtowc_state;
          _mbstate_t _wctomb_state;
          char _l64a_buf[8];
          char _signal_buf[_REENT_SIGNAL_SIZE];
          int _getdate_err;
          _mbstate_t _mbrlen_state;
          _mbstate_t _mbrtowc_state;
          _mbstate_t _mbsrtowcs_state;
          _mbstate_t _wcrtomb_state;
          _mbstate_t _wcsrtombs_state;
	  int _h_errno;
        } _reent;
  /* Two next two fields were once used by malloc.  They are no longer
     used. They are used to preserve the space used before so as to
     allow addition of new reent fields and keep binary compatibility.   */
      struct
        {
#define _N_LISTS 30
          unsigned char * _nextf[_N_LISTS];
          unsigned int _nmalloc[_N_LISTS];
        } _unused;
    } _new;

  /* atexit stuff */
  struct _atexit *_atexit;	/* points to head of LIFO stack */
  struct _atexit _atexit0;	/* one guaranteed table, required by ANSI */

  /* signal info */
  void (**(_sig_func))(int);

  /* These are here last so that __FILE can grow without changing the offsets
     of the above members (on the off chance that future binary compatibility
     would be broken otherwise).  */
  struct _glue __sglue;		/* root of glue chain */
  __FILE __sf[3];  		/* first three file descriptors */
};

#define _REENT_INIT(var) \
  { 0, \
    &(var).__sf[0], \
    &(var).__sf[1], \
    &(var).__sf[2], \
    0, \
    "", \
    0, \
    "C", \
    0, \
    _NULL, \
    _NULL, \
    0, \
    _NULL, \
    _NULL, \
    0, \
    _NULL, \
    { \
      { \
        0, \
        _NULL, \
        "", \
        {0, 0, 0, 0, 0, 0, 0, 0, 0}, \
        0, \
        1, \
        { \
          {_RAND48_SEED_0, _RAND48_SEED_1, _RAND48_SEED_2}, \
          {_RAND48_MULT_0, _RAND48_MULT_1, _RAND48_MULT_2}, \
          _RAND48_ADD \
        }, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}}, \
        "", \
        "", \
        0, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}}, \
        {0, {0}} \
      } \
    }, \
    _NULL, \
    {_NULL, 0, {_NULL}, {{_NULL}, {_NULL}, 0, 0}}, \
    _NULL, \
    {_NULL, 0, _NULL} \
  }

#define _REENT_INIT_PTR(var) \
  { (var)->_errno = 0; \
    (var)->_stdin = &(var)->__sf[0]; \
    (var)->_stdout = &(var)->__sf[1]; \
    (var)->_stderr = &(var)->__sf[2]; \
    (var)->_inc = 0; \
    memset(&(var)->_emergency, 0, sizeof((var)->_emergency)); \
    (var)->_current_category = 0; \
    (var)->_current_locale = "C"; \
    (var)->__sdidinit = 0; \
    (var)->__cleanup = _NULL; \
    (var)->_result = _NULL; \
    (var)->_result_k = 0; \
    (var)->_p5s = _NULL; \
    (var)->_freelist = _NULL; \
    (var)->_cvtlen = 0; \
    (var)->_cvtbuf = _NULL; \
    (var)->_new._reent._unused_rand = 0; \
    (var)->_new._reent._strtok_last = _NULL; \
    (var)->_new._reent._asctime_buf[0] = 0; \
    memset(&(var)->_new._reent._localtime_buf, 0, sizeof((var)->_new._reent._localtime_buf)); \
    (var)->_new._reent._gamma_signgam = 0; \
    (var)->_new._reent._rand_next = 1; \
    (var)->_new._reent._r48._seed[0] = _RAND48_SEED_0; \
    (var)->_new._reent._r48._seed[1] = _RAND48_SEED_1; \
    (var)->_new._reent._r48._seed[2] = _RAND48_SEED_2; \
    (var)->_new._reent._r48._mult[0] = _RAND48_MULT_0; \
    (var)->_new._reent._r48._mult[1] = _RAND48_MULT_1; \
    (var)->_new._reent._r48._mult[2] = _RAND48_MULT_2; \
    (var)->_new._reent._r48._add = _RAND48_ADD; \
    (var)->_new._reent._mblen_state.__count = 0; \
    (var)->_new._reent._mblen_state.__value.__wch = 0; \
    (var)->_new._reent._mbtowc_state.__count = 0; \
    (var)->_new._reent._mbtowc_state.__value.__wch = 0; \
    (var)->_new._reent._wctomb_state.__count = 0; \
    (var)->_new._reent._wctomb_state.__value.__wch = 0; \
    (var)->_new._reent._mbrlen_state.__count = 0; \
    (var)->_new._reent._mbrlen_state.__value.__wch = 0; \
    (var)->_new._reent._mbrtowc_state.__count = 0; \
    (var)->_new._reent._mbrtowc_state.__value.__wch = 0; \
    (var)->_new._reent._mbsrtowcs_state.__count = 0; \
    (var)->_new._reent._mbsrtowcs_state.__value.__wch = 0; \
    (var)->_new._reent._wcrtomb_state.__count = 0; \
    (var)->_new._reent._wcrtomb_state.__value.__wch = 0; \
    (var)->_new._reent._wcsrtombs_state.__count = 0; \
    (var)->_new._reent._wcsrtombs_state.__value.__wch = 0; \
    (var)->_new._reent._l64a_buf[0] = '\0'; \
    (var)->_new._reent._signal_buf[0] = '\0'; \
    (var)->_new._reent._getdate_err = 0; \
    (var)->_atexit = _NULL; \
    (var)->_atexit0._next = _NULL; \
    (var)->_atexit0._ind = 0; \
    (var)->_atexit0._fns[0] = _NULL; \
    (var)->_atexit0._on_exit_args._fntypes = 0; \
    (var)->_atexit0._on_exit_args._fnargs[0] = _NULL; \
    (var)->_sig_func = _NULL; \
    (var)->__sglue._next = _NULL; \
    (var)->__sglue._niobs = 0; \
    (var)->__sglue._iobs = _NULL; \
    memset(&(var)->__sf, 0, sizeof((var)->__sf)); \
  }

#define _REENT_CHECK_RAND48(ptr)	/* nothing */
#define _REENT_CHECK_MP(ptr)		/* nothing */
#define _REENT_CHECK_TM(ptr)		/* nothing */
#define _REENT_CHECK_ASCTIME_BUF(ptr)	/* nothing */
#define _REENT_CHECK_EMERGENCY(ptr)	/* nothing */
#define _REENT_CHECK_MISC(ptr)	        /* nothing */
#define _REENT_CHECK_SIGNAL_BUF(ptr)	/* nothing */

#define _REENT_SIGNGAM(ptr)	((ptr)->_new._reent._gamma_signgam)
#define _REENT_RAND_NEXT(ptr)	((ptr)->_new._reent._rand_next)
#define _REENT_RAND48_SEED(ptr)	((ptr)->_new._reent._r48._seed)
#define _REENT_RAND48_MULT(ptr)	((ptr)->_new._reent._r48._mult)
#define _REENT_RAND48_ADD(ptr)	((ptr)->_new._reent._r48._add)
#define _REENT_MP_RESULT(ptr)	((ptr)->_result)
#define _REENT_MP_RESULT_K(ptr)	((ptr)->_result_k)
#define _REENT_MP_P5S(ptr)	((ptr)->_p5s)
#define _REENT_MP_FREELIST(ptr)	((ptr)->_freelist)
#define _REENT_ASCTIME_BUF(ptr)	((ptr)->_new._reent._asctime_buf)
#define _REENT_TM(ptr)		(&(ptr)->_new._reent._localtime_buf)
#define _REENT_EMERGENCY(ptr)	((ptr)->_emergency)
#define _REENT_STRTOK_LAST(ptr)	((ptr)->_new._reent._strtok_last)
#define _REENT_MBLEN_STATE(ptr)	((ptr)->_new._reent._mblen_state)
#define _REENT_MBTOWC_STATE(ptr)((ptr)->_new._reent._mbtowc_state)
#define _REENT_WCTOMB_STATE(ptr)((ptr)->_new._reent._wctomb_state)
#define _REENT_MBRLEN_STATE(ptr)((ptr)->_new._reent._mbrlen_state)
#define _REENT_MBRTOWC_STATE(ptr)((ptr)->_new._reent._mbrtowc_state)
#define _REENT_MBSRTOWCS_STATE(ptr)((ptr)->_new._reent._mbsrtowcs_state)
#define _REENT_WCRTOMB_STATE(ptr)((ptr)->_new._reent._wcrtomb_state)
#define _REENT_WCSRTOMBS_STATE(ptr)((ptr)->_new._reent._wcsrtombs_state)
#define _REENT_L64A_BUF(ptr)    ((ptr)->_new._reent._l64a_buf)
#define _REENT_SIGNAL_BUF(ptr)  ((ptr)->_new._reent._signal_buf)
#define _REENT_GETDATE_ERR_P(ptr) (&((ptr)->_new._reent._getdate_err))

#endif /* !_REENT_SMALL */

/*
 * All references to struct _reent are via this pointer.
 * Internally, newlib routines that need to reference it should use _REENT.
 */

#ifndef __ATTRIBUTE_IMPURE_PTR__
#define __ATTRIBUTE_IMPURE_PTR__
#endif

extern struct _reent *_impure_ptr __ATTRIBUTE_IMPURE_PTR__;
extern struct _reent *_CONST _global_impure_ptr __ATTRIBUTE_IMPURE_PTR__;

void _reclaim_reent _PARAMS ((struct _reent *));

/* #define _REENT_ONLY define this to get only reentrant routines */

#ifndef _REENT_ONLY

#if defined(__DYNAMIC_REENT__) && !defined(__SINGLE_THREAD__)
#ifndef __getreent
  struct _reent * _EXFUN(__getreent, (void));
#endif
# define _REENT (__getreent())
#else /* __SINGLE_THREAD__ || !__DYNAMIC_REENT__ */
# define _REENT _impure_ptr
#endif /* __SINGLE_THREAD__ || !__DYNAMIC_REENT__ */

#endif /* !_REENT_ONLY */

#define _GLOBAL_REENT _global_impure_ptr

#ifdef __cplusplus
}
#endif
#endif /* _SYS_REENT_H_ */

#define	_FSTDIO			/* ``function stdio'' */

#define __need_size_t
#include <stddef.h>

#define __need___va_list
#include <stdarg.h>

/*
 * <sys/reent.h> defines __FILE, _fpos_t.
 * They must be defined there because struct _reent needs them (and we don't
 * want reent.h to include this file.
 */

//#include <sys/reent.h>
//#include <sys/types.h>

_BEGIN_STD_C

typedef __FILE FILE;

#ifdef __CYGWIN__
#ifdef __CYGWIN_USE_BIG_TYPES__
typedef _fpos64_t fpos_t;
#else
typedef _fpos_t fpos_t;
#endif
#else
typedef _fpos_t fpos_t;
#ifdef __LARGE64_FILES
typedef _fpos64_t fpos64_t;
#endif
#endif /* !__CYGWIN__ */

//#include <sys/stdio.h>
#ifndef _NEWLIB_STDIO_H
#define _NEWLIB_STDIO_H

//#include <sys/lock.h>
//#include <sys/reent.h>

/* Internal locking macros, used to protect stdio functions.  In the
   general case, expand to nothing. Use __SSTR flag in FILE _flags to
   detect if FILE is private to sprintf/sscanf class of functions; if
   set then do nothing as lock is not initialised. */
#if !defined(_flockfile)
#ifndef __SINGLE_THREAD__
#  define _flockfile(fp) (((fp)->_flags & __SSTR) ? 0 : __lock_acquire_recursive((fp)->_lock))
#else
#  define _flockfile(fp)	(_CAST_VOID 0)
#endif
#endif

#if !defined(_funlockfile)
#ifndef __SINGLE_THREAD__
#  define _funlockfile(fp) (((fp)->_flags & __SSTR) ? 0 : __lock_release_recursive((fp)->_lock))
#else
#  define _funlockfile(fp)	(_CAST_VOID 0)
#endif
#endif

#endif /* _NEWLIB_STDIO_H */
#define	__SLBF	0x0001		/* line buffered */
#define	__SNBF	0x0002		/* unbuffered */
#define	__SRD	0x0004		/* OK to read */
#define	__SWR	0x0008		/* OK to write */
	/* RD and WR are never simultaneously asserted */
#define	__SRW	0x0010		/* open for reading & writing */
#define	__SEOF	0x0020		/* found EOF */
#define	__SERR	0x0040		/* found error */
#define	__SMBF	0x0080		/* _buf is from malloc */
#define	__SAPP	0x0100		/* fdopen()ed in append mode - so must  write to end */
#define	__SSTR	0x0200		/* this is an sprintf/snprintf string */
#define	__SOPT	0x0400		/* do fseek() optimisation */
#define	__SNPT	0x0800		/* do not do fseek() optimisation */
#define	__SOFF	0x1000		/* set iff _offset is in fact correct */
#define	__SORD	0x2000		/* true => stream orientation (byte/wide) decided */
#if defined(__CYGWIN__)
#  define __SCLE  0x4000        /* convert line endings CR/LF <-> NL */
#endif
#define	__SL64	0x8000		/* is 64-bit offset large file */

/* _flags2 flags */
#define	__SWID	0x2000		/* true => stream orientation wide, false => byte, only valid if __SORD in _flags is true */

/*
 * The following three definitions are for ANSI C, which took them
 * from System V, which stupidly took internal interface macros and
 * made them official arguments to setvbuf(), without renaming them.
 * Hence, these ugly _IOxxx names are *supposed* to appear in user code.
 *
 * Although these happen to match their counterparts above, the
 * implementation does not rely on that (so these could be renumbered).
 */
#define	_IOFBF	0		/* setvbuf should set fully buffered */
#define	_IOLBF	1		/* setvbuf should set line buffered */
#define	_IONBF	2		/* setvbuf should set unbuffered */

#ifndef NULL
#define	NULL	0
#endif

#define	EOF	(-1)

#ifdef __BUFSIZ__
#define	BUFSIZ		__BUFSIZ__
#else
#define	BUFSIZ		1024
#endif

#ifdef __FOPEN_MAX__
#define FOPEN_MAX	__FOPEN_MAX__
#else
#define	FOPEN_MAX	20
#endif

#ifdef __FILENAME_MAX__
#define FILENAME_MAX    __FILENAME_MAX__
#else
#define	FILENAME_MAX	1024
#endif

#ifdef __L_tmpnam__
#define L_tmpnam	__L_tmpnam__
#else
#define	L_tmpnam	FILENAME_MAX
#endif

#ifndef __STRICT_ANSI__
#define P_tmpdir        "/tmp"
#endif

#ifndef SEEK_SET
#define	SEEK_SET	0	/* set file offset to offset */
#endif
#ifndef SEEK_CUR
#define	SEEK_CUR	1	/* set file offset to current plus offset */
#endif
#ifndef SEEK_END
#define	SEEK_END	2	/* set file offset to EOF plus offset */
#endif

#define	TMP_MAX		26

#ifndef _REENT_ONLY
#define	stdin	(_REENT->_stdin)
#define	stdout	(_REENT->_stdout)
#define	stderr	(_REENT->_stderr)
#else /* _REENT_ONLY */
#define	stdin	(_impure_ptr->_stdin)
#define	stdout	(_impure_ptr->_stdout)
#define	stderr	(_impure_ptr->_stderr)
#endif /* _REENT_ONLY */

#define _stdin_r(x)	((x)->_stdin)
#define _stdout_r(x)	((x)->_stdout)
#define _stderr_r(x)	((x)->_stderr)

/*
 * Functions defined in ANSI C standard.
 */

#ifndef __VALIST
#ifdef __GNUC__
#define __VALIST __gnuc_va_list
#else
#define __VALIST char*
#endif
#endif

FILE *	_EXFUN(tmpfile, (void));
char *	_EXFUN(tmpnam, (char *));
int	_EXFUN(fclose, (FILE *));
int	_EXFUN(fflush, (FILE *));
int	_EXFUN(fflush_unlocked, (FILE *));
FILE *	_EXFUN(freopen, (const char *, const char *, FILE *));
void	_EXFUN(setbuf, (FILE *, char *));
int	_EXFUN(setvbuf, (FILE *, char *, int, size_t));
int	_EXFUN(fprintf, (FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
int	_EXFUN(fscanf, (FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3))));
int	_EXFUN(printf, (const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 1, 2))));
int	_EXFUN(scanf, (const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 2))));
int	_EXFUN(sscanf, (const char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3))));
int	_EXFUN(vfprintf, (FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(vprintf, (const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 1, 0))));
int	_EXFUN(vsprintf, (char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(fgetc, (FILE *));
int	_EXFUN(fgetc_unlocked, (FILE *));
char *  _EXFUN(fgets, (char *, int, FILE *));
char *  _EXFUN(fgets_unlocked, (char *, int, FILE *));
int	_EXFUN(fputc, (int, FILE *));
int	_EXFUN(fputs, (const char *, FILE *));
int	_EXFUN(fputs_unlocked, (const char *, FILE *));
int	_EXFUN(getc, (FILE *));
int	_EXFUN(getchar, (void));
char *  _EXFUN(gets, (char *));
int	_EXFUN(putc, (int, FILE *));
int	_EXFUN(putchar, (int));
int	_EXFUN(puts, (const char *));
int	_EXFUN(ungetc, (int, FILE *));
size_t	_EXFUN(fread, (_PTR, size_t _size, size_t _n, FILE *));
size_t	_EXFUN(fread_unlocked, (_PTR, size_t _size, size_t _n, FILE *));
size_t	_EXFUN(fwrite, (const _PTR , size_t _size, size_t _n, FILE *));
size_t	_EXFUN(fwrite_unlocked, (const _PTR , size_t _size, size_t _n, FILE *));
#ifdef _COMPILING_NEWLIB
int	_EXFUN(fgetpos, (FILE *, _fpos_t *));
#else
int	_EXFUN(fgetpos, (FILE *, fpos_t *));
#endif
int	_EXFUN(fseek, (FILE *, long, int));
#ifdef _COMPILING_NEWLIB
int	_EXFUN(fsetpos, (FILE *, const _fpos_t *));
#else
int	_EXFUN(fsetpos, (FILE *, const fpos_t *));
#endif
long	_EXFUN(ftell, ( FILE *));
void	_EXFUN(rewind, (FILE *));
void	_EXFUN(clearerr, (FILE *));
int	_EXFUN(feof, (FILE *));
int	_EXFUN(ferror, (FILE *));
void    _EXFUN(perror, (const char *));
#ifndef _REENT_ONLY
FILE *	_EXFUN(fopen, (const char *_name, const char *_type));
int	_EXFUN(sprintf, (char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
int	_EXFUN(remove, (const char *));
int	_EXFUN(rename, (const char *, const char *));
#endif
#ifndef __STRICT_ANSI__
#ifdef _COMPILING_NEWLIB
int	_EXFUN(fseeko, (FILE *, _off_t, int));
_off_t	_EXFUN(ftello, ( FILE *));
#else
int	_EXFUN(fseeko, (FILE *, off_t, int));
off_t	_EXFUN(ftello, ( FILE *));
#endif
#ifndef _REENT_ONLY
int	_EXFUN(asiprintf, (char **, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
char *	_EXFUN(asniprintf, (char *, size_t *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
char *	_EXFUN(asnprintf, (char *, size_t *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(asprintf, (char **, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
#ifndef diprintf
int	_EXFUN(diprintf, (int, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
#endif
int	_EXFUN(fcloseall, (_VOID));
int	_EXFUN(fiprintf, (FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
int	_EXFUN(fiscanf, (FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3))));
int	_EXFUN(iprintf, (const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 1, 2))));
int	_EXFUN(iscanf, (const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 2))));
int	_EXFUN(siprintf, (char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
int	_EXFUN(siscanf, (const char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3))));
int	_EXFUN(snprintf, (char *, size_t, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(sniprintf, (char *, size_t, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
char *	_EXFUN(tempnam, (const char *, const char *));
int	_EXFUN(vasiprintf, (char **, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
char *	_EXFUN(vasniprintf, (char *, size_t *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
char *	_EXFUN(vasnprintf, (char *, size_t *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(vasprintf, (char **, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(vdiprintf, (int, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(vfiprintf, (FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(vfiscanf, (FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0))));
int	_EXFUN(vfscanf, (FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0))));
int	_EXFUN(viprintf, (const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 1, 0))));
int	_EXFUN(viscanf, (const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 0))));
int	_EXFUN(vscanf, (const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 1, 0))));
int	_EXFUN(vsiprintf, (char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(vsiscanf, (const char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0))));
int	_EXFUN(vsniprintf, (char *, size_t, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(vsnprintf, (char *, size_t, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(vsscanf, (const char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0))));
#endif /* !_REENT_ONLY */
#endif /* !__STRICT_ANSI__ */

/*
 * Routines in POSIX 1003.1:2001.
 */

#ifndef __STRICT_ANSI__
#ifndef _REENT_ONLY
FILE *	_EXFUN(fdopen, (int, const char *));
#endif
int	_EXFUN(fileno, (FILE *));
int	_EXFUN(getw, (FILE *));
/*int	_EXFUN(pclose, (FILE *));*/
/*FILE *  _EXFUN(popen, (const char *, const char *));*/
int	_EXFUN(putw, (int, FILE *));
void    _EXFUN(setbuffer, (FILE *, char *, int));
int	_EXFUN(setlinebuf, (FILE *));
int	_EXFUN(getc_unlocked, (FILE *));
int	_EXFUN(getchar_unlocked, (void));
void	_EXFUN(flockfile, (FILE *));
int	_EXFUN(ftrylockfile, (FILE *));
void	_EXFUN(funlockfile, (FILE *));
int	_EXFUN(putc_unlocked, (int, FILE *));
int	_EXFUN(putchar_unlocked, (int));

#endif /* ! __STRICT_ANSI__ */

/*
 * Routines in POSIX 1003.1:200x.
 */

#ifndef __STRICT_ANSI__
# ifndef _REENT_ONLY
#  ifndef dprintf
int	_EXFUN(dprintf, (int, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
#  endif
FILE *	_EXFUN(fmemopen, (void *, size_t, const char *));
/* getdelim - see __getdelim for now */
/* getline - see __getline for now */
FILE *	_EXFUN(open_memstream, (char **, size_t *));
#if defined (__CYGWIN__)
int	_EXFUN(renameat, (int, const char *, int, const char *));
int	_EXFUN(symlinkat, (const char *, int, const char *));
#endif
int	_EXFUN(vdprintf, (int, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
# endif
#endif

/*
 * Recursive versions of the above.
 */

int	_EXFUN(_asiprintf_r, (struct _reent *, char **, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
char *	_EXFUN(_asniprintf_r, (struct _reent *, char *, size_t *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 4, 5))));
char *	_EXFUN(_asnprintf_r, (struct _reent *, char *, size_t *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 4, 5))));
int	_EXFUN(_asprintf_r, (struct _reent *, char **, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_diprintf_r, (struct _reent *, int, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_dprintf_r, (struct _reent *, int, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_fclose_r, (struct _reent *, FILE *));
int	_EXFUN(_fcloseall_r, (struct _reent *));
FILE *	_EXFUN(_fdopen_r, (struct _reent *, int, const char *));
int	_EXFUN(_fflush_r, (struct _reent *, FILE *));
int	_EXFUN(_fgetc_r, (struct _reent *, FILE *));
char *  _EXFUN(_fgets_r, (struct _reent *, char *, int, FILE *));
#ifdef _COMPILING_NEWLIB
int	_EXFUN(_fgetpos_r, (struct _reent *, FILE *, _fpos_t *));
int	_EXFUN(_fsetpos_r, (struct _reent *, FILE *, const _fpos_t *));
#else
int	_EXFUN(_fgetpos_r, (struct _reent *, FILE *, fpos_t *));
int	_EXFUN(_fsetpos_r, (struct _reent *, FILE *, const fpos_t *));
#endif
int	_EXFUN(_fiprintf_r, (struct _reent *, FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_fiscanf_r, (struct _reent *, FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 4))));
FILE *	_EXFUN(_fmemopen_r, (struct _reent *, void *, size_t, const char *));
FILE *	_EXFUN(_fopen_r, (struct _reent *, const char *, const char *));
FILE *	_EXFUN(_freopen_r, (struct _reent *, const char *, const char *, FILE *));
int	_EXFUN(_fprintf_r, (struct _reent *, FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_fputc_r, (struct _reent *, int, FILE *));
int	_EXFUN(_fputs_r, (struct _reent *, const char *, FILE *));
size_t	_EXFUN(_fread_r, (struct _reent *, _PTR, size_t _size, size_t _n, FILE *));
int	_EXFUN(_fscanf_r, (struct _reent *, FILE *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 4))));
int	_EXFUN(_fseek_r, (struct _reent *, FILE *, long, int));
int	_EXFUN(_fseeko_r,(struct _reent *, FILE *, _off_t, int));
long	_EXFUN(_ftell_r, (struct _reent *, FILE *));
_off_t	_EXFUN(_ftello_r,(struct _reent *, FILE *));
void	_EXFUN(_rewind_r, (struct _reent *, FILE *));
size_t	_EXFUN(_fwrite_r, (struct _reent *, const _PTR , size_t _size, size_t _n, FILE *));
int	_EXFUN(_getc_r, (struct _reent *, FILE *));
int	_EXFUN(_getc_unlocked_r, (struct _reent *, FILE *));
int	_EXFUN(_getchar_r, (struct _reent *));
int	_EXFUN(_getchar_unlocked_r, (struct _reent *));
char *	_EXFUN(_gets_r, (struct _reent *, char *));
int	_EXFUN(_iprintf_r, (struct _reent *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
int	_EXFUN(_iscanf_r, (struct _reent *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3))));
FILE *	_EXFUN(_open_memstream_r, (struct _reent *, char **, size_t *));
void	_EXFUN(_perror_r, (struct _reent *, const char *));
int	_EXFUN(_printf_r, (struct _reent *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 2, 3))));
int	_EXFUN(_putc_r, (struct _reent *, int, FILE *));
int	_EXFUN(_putc_unlocked_r, (struct _reent *, int, FILE *));
int	_EXFUN(_putchar_unlocked_r, (struct _reent *, int));
int	_EXFUN(_putchar_r, (struct _reent *, int));
int	_EXFUN(_puts_r, (struct _reent *, const char *));
int	_EXFUN(_remove_r, (struct _reent *, const char *));
int	_EXFUN(_rename_r, (struct _reent *,
			   const char *_old, const char *_new));
int	_EXFUN(_scanf_r, (struct _reent *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 3))));
int	_EXFUN(_siprintf_r, (struct _reent *, char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_siscanf_r, (struct _reent *, const char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 4))));
int	_EXFUN(_sniprintf_r, (struct _reent *, char *, size_t, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 4, 5))));
int	_EXFUN(_snprintf_r, (struct _reent *, char *, size_t, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 4, 5))));
int	_EXFUN(_sprintf_r, (struct _reent *, char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__printf__, 3, 4))));
int	_EXFUN(_sscanf_r, (struct _reent *, const char *, const char *, ...)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 4))));
char *	_EXFUN(_tempnam_r, (struct _reent *, const char *, const char *));
FILE *	_EXFUN(_tmpfile_r, (struct _reent *));
char *	_EXFUN(_tmpnam_r, (struct _reent *, char *));
int	_EXFUN(_ungetc_r, (struct _reent *, int, FILE *));
int	_EXFUN(_vasiprintf_r, (struct _reent *, char **, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
char *	_EXFUN(_vasniprintf_r, (struct _reent*, char *, size_t *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 4, 0))));
char *	_EXFUN(_vasnprintf_r, (struct _reent*, char *, size_t *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 4, 0))));
int	_EXFUN(_vasprintf_r, (struct _reent *, char **, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vdiprintf_r, (struct _reent *, int, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vdprintf_r, (struct _reent *, int, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vfiprintf_r, (struct _reent *, FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vfiscanf_r, (struct _reent *, FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 0))));
int	_EXFUN(_vfprintf_r, (struct _reent *, FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vfscanf_r, (struct _reent *, FILE *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 0))));
int	_EXFUN(_viprintf_r, (struct _reent *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(_viscanf_r, (struct _reent *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0))));
int	_EXFUN(_vprintf_r, (struct _reent *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 2, 0))));
int	_EXFUN(_vscanf_r, (struct _reent *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 2, 0))));
int	_EXFUN(_vsiprintf_r, (struct _reent *, char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vsiscanf_r, (struct _reent *, const char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 0))));
int	_EXFUN(_vsniprintf_r, (struct _reent *, char *, size_t, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 4, 0))));
int	_EXFUN(_vsnprintf_r, (struct _reent *, char *, size_t, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 4, 0))));
int	_EXFUN(_vsprintf_r, (struct _reent *, char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__printf__, 3, 0))));
int	_EXFUN(_vsscanf_r, (struct _reent *, const char *, const char *, __VALIST)
               _ATTRIBUTE ((__format__ (__scanf__, 3, 0))));

ssize_t _EXFUN(__getdelim, (char **, size_t *, int, FILE *));
ssize_t _EXFUN(__getline, (char **, size_t *, FILE *));

#ifdef __LARGE64_FILES
#if !defined(__CYGWIN__) || defined(_COMPILING_NEWLIB)
FILE *	_EXFUN(fdopen64, (int, const char *));
FILE *  _EXFUN(fopen64, (const char *, const char *));
FILE *  _EXFUN(freopen64, (_CONST char *, _CONST char *, FILE *));
_off64_t _EXFUN(ftello64, (FILE *));
_off64_t _EXFUN(fseeko64, (FILE *, _off64_t, int));
int     _EXFUN(fgetpos64, (FILE *, _fpos64_t *));
int     _EXFUN(fsetpos64, (FILE *, const _fpos64_t *));
FILE *  _EXFUN(tmpfile64, (void));

FILE *	_EXFUN(_fdopen64_r, (struct _reent *, int, const char *));
FILE *  _EXFUN(_fopen64_r, (struct _reent *,const char *, const char *));
FILE *  _EXFUN(_freopen64_r, (struct _reent *, _CONST char *, _CONST char *, FILE *));
_off64_t _EXFUN(_ftello64_r, (struct _reent *, FILE *));
_off64_t _EXFUN(_fseeko64_r, (struct _reent *, FILE *, _off64_t, int));
int     _EXFUN(_fgetpos64_r, (struct _reent *, FILE *, _fpos64_t *));
int     _EXFUN(_fsetpos64_r, (struct _reent *, FILE *, const _fpos64_t *));
FILE *  _EXFUN(_tmpfile64_r, (struct _reent *));
#endif /* !__CYGWIN__ */
#endif /* __LARGE64_FILES */

/*
 * Routines internal to the implementation.
 */

int	_EXFUN(__srget_r, (struct _reent *, FILE *));
int	_EXFUN(__swbuf_r, (struct _reent *, int, FILE *));

/*
 * Stdio function-access interface.
 */

#ifndef __STRICT_ANSI__
# ifdef __LARGE64_FILES
FILE	*_EXFUN(funopen,(const _PTR __cookie,
		int (*__readfn)(_PTR __c, char *__buf, int __n),
		int (*__writefn)(_PTR __c, const char *__buf, int __n),
		_fpos64_t (*__seekfn)(_PTR __c, _fpos64_t __off, int __whence),
		int (*__closefn)(_PTR __c)));
FILE	*_EXFUN(_funopen_r,(struct _reent *, const _PTR __cookie,
		int (*__readfn)(_PTR __c, char *__buf, int __n),
		int (*__writefn)(_PTR __c, const char *__buf, int __n),
		_fpos64_t (*__seekfn)(_PTR __c, _fpos64_t __off, int __whence),
		int (*__closefn)(_PTR __c)));
# else
FILE	*_EXFUN(funopen,(const _PTR __cookie,
		int (*__readfn)(_PTR __cookie, char *__buf, int __n),
		int (*__writefn)(_PTR __cookie, const char *__buf, int __n),
		fpos_t (*__seekfn)(_PTR __cookie, fpos_t __off, int __whence),
		int (*__closefn)(_PTR __cookie)));
FILE	*_EXFUN(_funopen_r,(struct _reent *, const _PTR __cookie,
		int (*__readfn)(_PTR __cookie, char *__buf, int __n),
		int (*__writefn)(_PTR __cookie, const char *__buf, int __n),
		fpos_t (*__seekfn)(_PTR __cookie, fpos_t __off, int __whence),
		int (*__closefn)(_PTR __cookie)));
# endif /* !__LARGE64_FILES */

# define	fropen(__cookie, __fn) funopen(__cookie, __fn, (int (*)())0, \
					       (fpos_t (*)())0, (int (*)())0)
# define	fwopen(__cookie, __fn) funopen(__cookie, (int (*)())0, __fn, \
					       (fpos_t (*)())0, (int (*)())0)

typedef ssize_t cookie_read_function_t(void *__cookie, char *__buf, size_t __n);
typedef ssize_t cookie_write_function_t(void *__cookie, const char *__buf,
					size_t __n);
# ifdef __LARGE64_FILES
typedef int cookie_seek_function_t(void *__cookie, _off64_t *__off,
				   int __whence);
# else
typedef int cookie_seek_function_t(void *__cookie, off_t *__off, int __whence);
# endif /* !__LARGE64_FILES */
typedef int cookie_close_function_t(void *__cookie);
typedef struct
{
  /* These four struct member names are dictated by Linux; hopefully,
     they don't conflict with any macros.  */
  cookie_read_function_t  *read;
  cookie_write_function_t *write;
  cookie_seek_function_t  *seek;
  cookie_close_function_t *close;
} cookie_io_functions_t;
FILE *_EXFUN(fopencookie,(void *__cookie,
		const char *__mode, cookie_io_functions_t __functions));
FILE *_EXFUN(_fopencookie_r,(struct _reent *, void *__cookie,
		const char *__mode, cookie_io_functions_t __functions));
#endif /* ! __STRICT_ANSI__ */

#ifndef __CUSTOM_FILE_IO__
/*
 * The __sfoo macros are here so that we can
 * define function versions in the C library.
 */
#define       __sgetc_raw_r(__ptr, __f) (--(__f)->_r < 0 ? __srget_r(__ptr, __f) : (int)(*(__f)->_p++))

#ifdef __SCLE
/*  For a platform with CR/LF, additional logic is required by
  __sgetc_r which would otherwise simply be a macro; therefore we
  use an inlined function.  The function is only meant to be inlined
  in place as used and the function body should never be emitted.

  There are two possible means to this end when compiling with GCC,
  one when compiling with a standard C99 compiler, and for other
  compilers we're just stuck.  At the moment, this issue only
  affects the Cygwin target, so we'll most likely be using GCC. */

_ELIDABLE_INLINE int __sgetc_r(struct _reent *__ptr, FILE *__p);

_ELIDABLE_INLINE int __sgetc_r(struct _reent *__ptr, FILE *__p)
  {
    int __c = __sgetc_raw_r(__ptr, __p);
    if ((__p->_flags & __SCLE) && (__c == '\r'))
      {
      int __c2 = __sgetc_raw_r(__ptr, __p);
      if (__c2 == '\n')
        __c = __c2;
      else
        ungetc(__c2, __p);
      }
    return __c;
  }
#else
#define __sgetc_r(__ptr, __p) __sgetc_raw_r(__ptr, __p)
#endif

#ifdef _never /* __GNUC__ */
/* If this inline is actually used, then systems using coff debugging
   info get hopelessly confused.  21sept93 rich@cygnus.com.  */
_ELIDABLE_INLINE int __sputc_r(struct _reent *_ptr, int _c, FILE *_p) {
	if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n'))
		return (*_p->_p++ = _c);
	else
		return (__swbuf_r(_ptr, _c, _p));
}
#else
/*
 * This has been tuned to generate reasonable code on the vax using pcc
 */
#define       __sputc_raw_r(__ptr, __c, __p) \
	(--(__p)->_w < 0 ? \
		(__p)->_w >= (__p)->_lbfsize ? \
			(*(__p)->_p = (__c)), *(__p)->_p != '\n' ? \
				(int)*(__p)->_p++ : \
				__swbuf_r(__ptr, '\n', __p) : \
			__swbuf_r(__ptr, (int)(__c), __p) : \
		(*(__p)->_p = (__c), (int)*(__p)->_p++))
#ifdef __SCLE
#define __sputc_r(__ptr, __c, __p) \
        ((((__p)->_flags & __SCLE) && ((__c) == '\n')) \
          ? __sputc_raw_r(__ptr, '\r', (__p)) : 0 , \
        __sputc_raw_r((__ptr), (__c), (__p)))
#else
#define __sputc_r(__ptr, __c, __p) __sputc_raw_r(__ptr, __c, __p)
#endif
#endif

#define	__sfeof(p)	(((p)->_flags & __SEOF) != 0)
#define	__sferror(p)	(((p)->_flags & __SERR) != 0)
#define	__sclearerr(p)	((void)((p)->_flags &= ~(__SERR|__SEOF)))
#define	__sfileno(p)	((p)->_file)

#define	feof_unlocked(p)	__sfeof(p)
#define	ferror_unlocked(p)	__sferror(p)
#define	fputc_unlocked(c,p)	__sputc_r(_REENT, c, p)
#define fileno_unlocked(fp)	__sfileno (fp)

#ifndef _REENT_SMALL
#define	feof(p)		__sfeof(p)
#define	ferror(p)	__sferror(p)
#define	clearerr(p)	__sclearerr(p)
#endif

#if 0 /* #ifndef __STRICT_ANSI__ - FIXME: must initialize stdio first, use fn */
#define	fileno(p)	__sfileno(p)
#endif

#ifndef __CYGWIN__
#ifndef lint
#define	getc(fp)	__sgetc_r(_REENT, fp)
#define putc(x, fp)	__sputc_r(_REENT, x, fp)
#endif /* lint */
#endif /* __CYGWIN__ */

#ifndef __STRICT_ANSI__
/* fast always-buffered version, true iff error */
#define	fast_putc(x,p) (--(p)->_w < 0 ? \
	__swbuf_r(_REENT, (int)(x), p) == EOF : (*(p)->_p = (x), (p)->_p++, 0))

#define	L_cuserid	9		/* posix says it goes in stdio.h :( */
#ifdef __CYGWIN__
#define L_ctermid       16
#endif
#endif

#endif /* !__CUSTOM_FILE_IO__ */

#define	getchar()	getc(stdin)
#define	putchar(x)	putc(x, stdout)

_END_STD_C

#endif /* _STDIO_H_ */

// vi:set ts=8 sw=8:
