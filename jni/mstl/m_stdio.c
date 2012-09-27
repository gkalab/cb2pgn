// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

/*
NOT IMPLEMENTED: pclose, popen
*/

#ifndef __unix__

#ifdef IO_NOT_MTSAFE
# define __SINGLE_THREAD__
#endif

#include "m_stdio_internal.h"

#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#define register

typedef unsigned char u_char;
typedef unsigned long u_long;

static void*
_malloc_r (struct _reent *ptr, size_t size)
{
  return malloc (size);
}

static void
_free_r (struct _reent *ptr, void *addr)
{
  free (addr);
}

// ### HEADERS ##################################################################################

/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
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
 *	%W% (UofMD/Berkeley) %G%
 */

/*
 * Information local to this implementation of stdio,
 * in particular, macros and private variables.
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#ifdef __SCLE
# include <io.h>
#endif
#endif

/* Called by the main entry point fns to ensure stdio has been initialized.  */

#ifdef _REENT_SMALL
#define CHECK_INIT(ptr, fp) \
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
      if ((fp) == (FILE *)&__sf_fake_stdin)	\
	(fp) = _stdin_r(ptr);			\
      else if ((fp) == (FILE *)&__sf_fake_stdout) \
	(fp) = _stdout_r(ptr);			\
      else if ((fp) == (FILE *)&__sf_fake_stderr) \
	(fp) = _stderr_r(ptr);			\
    }						\
  while (0)
#else /* !_REENT_SMALL   */
#define CHECK_INIT(ptr, fp) \
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
    }						\
  while (0)
#endif /* !_REENT_SMALL  */

#define CHECK_STD_INIT(ptr) \
  do						\
    {						\
      if ((ptr) && !(ptr)->__sdidinit)		\
	__sinit (ptr);				\
    }						\
  while (0)

/* Return true iff the given FILE cannot be written now.  */

#define	cantwrite(ptr, fp)                                     \
  ((((fp)->_flags & __SWR) == 0 || (fp)->_bf._base == NULL) && \
   __swsetup_r(ptr, fp))

/* Test whether the given stdio file has an active ungetc buffer;
   release such a buffer, without restoring ordinary unread data.  */

#define	HASUB(fp) ((fp)->_ub._base != NULL)
#define	FREEUB(ptr, fp) {                    \
	if ((fp)->_ub._base != (fp)->_ubuf) \
		_free_r(ptr, (char *)(fp)->_ub._base); \
	(fp)->_ub._base = NULL; \
}

/* Test for an fgetline() buffer.  */

#define	HASLB(fp) ((fp)->_lb._base != NULL)
#define	FREELB(ptr, fp) { _free_r(ptr,(char *)(fp)->_lb._base); \
      (fp)->_lb._base = NULL; }

/*
 * Set the orientation for a stream. If o > 0, the stream has wide-
 * orientation. If o < 0, the stream has byte-orientation.
 */
#define ORIENT(fp,ori)					\
  do								\
    {								\
      if (!((fp)->_flags & __SORD))	\
	{							\
	  (fp)->_flags |= __SORD;				\
	  if (ori > 0)						\
	    (fp)->_flags2 |= __SWID;				\
	  else							\
	    (fp)->_flags2 &= ~__SWID;				\
	}							\
    }								\
  while (0)

#ifdef __SINGLE_THREAD__

#define __sfp_lock_acquire()
#define __sfp_lock_release()
#define __sinit_lock_acquire()
#define __sinit_lock_release()

#else

#ifdef __WIN32__

static CRITICAL_SECTION __sfp_lock;
static CRITICAL_SECTION __sinit_lock;

InitializeCriticalSection(&__sfp_lock);
InitializeCriticalSection(&__sinit_lock);

static _VOID
_DEFUN_VOID(__sfp_lock_acquire)
{
  EnterCriticalSection (&__sfp_lock);
}

static _VOID
_DEFUN_VOID(__sfp_lock_release)
{
  LeaveCriticalSection (__sfp_lock);
}

static _VOID
_DEFUN_VOID(__sinit_lock_acquire)
{
  EnterCriticalSection (__sinit_lock);
}

static _VOID
_DEFUN_VOID(__sinit_lock_release)
{
  LeaveCriticalSection (&__sinit_lock);
}

static int
_DEFUN_VOID(__lock_try_acquire_recursive)
{
  return TryEnterCriticalSection (&__sinit_lock) ? 1 : 0;
}

#else

# error "unsupported architecture"

#endif
#endif

/* Types used in positional argument support in vfprinf/vfwprintf.
   The implementation is char/wchar_t dependent but the class and state
   tables are only defined once in vfprintf.c. */
typedef enum {
  ZERO,   /* '0' */
  DIGIT,  /* '1-9' */
  DOLLAR, /* '$' */
  MODFR,  /* spec modifier */
  SPEC,   /* format specifier */
  DOT,    /* '.' */
  STAR,   /* '*' */
  FLAG,   /* format flag */
  OTHER,  /* all other chars */
  MAX_CH_CLASS /* place-holder */
} __CH_CLASS;

typedef enum {
  START,  /* start */
  SFLAG,  /* seen a flag */
  WDIG,   /* seen digits in width area */
  WIDTH,  /* processed width */
  SMOD,   /* seen spec modifier */
  SDOT,   /* seen dot */
  VARW,   /* have variable width specifier */
  VARP,   /* have variable precision specifier */
  PREC,   /* processed precision */
  VWDIG,  /* have digits in variable width specification */
  VPDIG,  /* have digits in variable precision specification */
  DONE,   /* done */
  MAX_STATE, /* place-holder */
} __STATE;

typedef enum {
  NOOP,  /* do nothing */
  NUMBER, /* build a number from digits */
  SKIPNUM, /* skip over digits */
  GETMOD,  /* get and process format modifier */
  GETARG,  /* get and process argument */
  GETPW,   /* get variable precision or width */
  GETPWB,  /* get variable precision or width and pushback fmt char */
  GETPOS,  /* get positional parameter value */
  PWPOS,   /* get positional parameter value for variable width or precision */
} __ACTION;


static int
_DEFUN(__fwalk, (ptr, function),
       struct _reent *ptr _AND
       register int (*function) (FILE *))
{
  register FILE *fp;
  register int n, ret = 0;
  register struct _glue *g;

  for (g = &ptr->__sglue; g != NULL; g = g->_next)
    for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
      if (fp->_flags != 0)
        {
          if (fp->_flags != 0 && fp->_file != -1)
            ret |= (*function) (fp);
        }

  return ret;
}

/* Special version of __fwalk where the function pointer is a reentrant
   I/O function (e.g. _fclose_r).  */
static int
_DEFUN(__fwalk_reent, (ptr, reent_function),
       struct _reent *ptr _AND
       register int (*reent_function) (struct _reent *, FILE *))
{
  register FILE *fp;
  register int n, ret = 0;
  register struct _glue *g;

  for (g = &ptr->__sglue; g != NULL; g = g->_next)
    for (fp = g->_iobs, n = g->_niobs; --n >= 0; fp++)
      if (fp->_flags != 0)
        {
          if (fp->_flags != 0 && fp->_file != -1)
            ret |= (*reent_function) (ptr, fp);
        }

  return ret;
}

static int
_DEFUN(_fwalk, (ptr, function),
       struct _reent *ptr _AND
       register int (*function)(FILE *))
{
  register int ret = 0;

  __sfp_lock_acquire ();

  /* Must traverse given list for streams.  Note that _GLOBAL_REENT
     only walked once in exit().  */
  ret |= __fwalk (ptr, function);

  __sfp_lock_release ();

  return ret;
}

/* Special version of _fwalk which handles a function pointer to a
   reentrant I/O function (e.g. _fclose_r).  */
static int
_DEFUN(_fwalk_reent, (ptr, reent_function),
       struct _reent *ptr _AND
       register int (*reent_function) (struct _reent *, FILE *))
{
  register int ret = 0;

  __sfp_lock_acquire ();

  /* Must traverse given list for streams.  Note that _GLOBAL_REENT
     only walked once in exit().  */
  ret |= __fwalk_reent (ptr, reent_function);

  __sfp_lock_release ();

  return ret;
}

/* Special version of _fwalk which handles a function pointer to a
   reentrant I/O function (e.g. _fclose_r).  */
static int
_DEFUN(_fwalk_reent_unlocked, (ptr, reent_function),
       struct _reent *ptr _AND
       register int (*reent_function) (struct _reent *, FILE *))
{
  register int ret = 0;

  /* Must traverse given list for streams.  Note that _GLOBAL_REENT
     only walked once in exit().  */
  ret |= __fwalk_reent (ptr, reent_function);

  return ret;
}

static _VOID
_DEFUN(_cleanup_r, (ptr),
       struct _reent *ptr)
{
  _CAST_VOID _fwalk(ptr, fclose);
  /* _CAST_VOID _fwalk (ptr, fflush); */	/* `cheating' */
}

_ssize_t
_DEFUN (_read_r, (ptr, fd, buf, cnt),
     struct _reent *ptr _AND
     int fd _AND
     _PTR buf _AND
     size_t cnt)
{
  _ssize_t ret;

  errno = 0;
  if ((ret = (_ssize_t)_read (fd, buf, cnt)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

static _READ_WRITE_RETURN_TYPE
_DEFUN(__sread, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       char *buf _AND
       int n)
{
  register FILE *fp = (FILE *) cookie;
  register int ret;

#ifdef __SCLE
  int oldmode = 0;
  if (fp->_flags & __SCLE)
    oldmode = setmode (fp->_file, O_BINARY);
#endif

  ret = _read_r (ptr, fp->_file, buf, n);

#ifdef __SCLE
  if (oldmode)
    setmode (fp->_file, oldmode);
#endif

  /* If the read succeeded, update the current offset.  */

  if (ret >= 0)
    fp->_offset += ret;
  else
    fp->_flags &= ~__SOFF;	/* paranoia */
  return ret;
}

_off_t
_DEFUN (_lseek_r, (ptr, fd, pos, whence),
     struct _reent *ptr _AND
     int fd _AND
     _off_t pos _AND
     int whence)
{
  _off_t ret;

  errno = 0;
  if ((ret = _lseek (fd, pos, whence)) == (_off_t) -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

_ssize_t
_DEFUN (_write_r, (ptr, fd, buf, cnt),
     struct _reent *ptr _AND
     int fd _AND
     _CONST _PTR buf _AND
     size_t cnt)
{
  _ssize_t ret;

  errno = 0;
  if ((ret = (_ssize_t)_write (fd, buf, cnt)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

static _READ_WRITE_RETURN_TYPE
_DEFUN(__swrite, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       char const *buf _AND
       int n)
{
  register FILE *fp = (FILE *) cookie;
  int w;
#ifdef __SCLE
  int oldmode=0;
#endif

  if (fp->_flags & __SAPP)
    _lseek_r (ptr, fp->_file, (_off_t) 0, SEEK_END);
  fp->_flags &= ~__SOFF;	/* in case O_APPEND mode is set */

#ifdef __SCLE
  if (fp->_flags & __SCLE)
    oldmode = setmode (fp->_file, O_BINARY);
#endif

  w = _write_r (ptr, fp->_file, buf, n);

#ifdef __SCLE
  if (oldmode)
    setmode (fp->_file, oldmode);
#endif

  return w;
}

static _fpos_t
_DEFUN(__sseek, (ptr, cookie, offset, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos_t offset _AND
       int whence)
{
  register FILE *fp = (FILE *) cookie;
  register _off_t ret;

  ret = _lseek_r (ptr, fp->_file, (_off_t) offset, whence);
  if (ret == -1L)
    fp->_flags &= ~__SOFF;
  else
    {
      fp->_flags |= __SOFF;
      fp->_offset = ret;
    }
  return ret;
}

static int
_DEFUN(_close_r, (ptr, fd),
     struct _reent *ptr _AND
     int fd)
{
  int ret;

  errno = 0;
  if ((ret = _close (fd)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

static int
_DEFUN(__sclose, (ptr, cookie),
       struct _reent *ptr _AND
       void *cookie)
{
  FILE *fp = (FILE *) cookie;

  return _close_r (ptr, fp->_file);
}

static _VOID
_DEFUN(_std, (ptr, flags, file, data),
    FILE *ptr _AND
    int flags _AND
    int file  _AND
    struct _reent *data)
{
  ptr->_p = 0;
  ptr->_r = 0;
  ptr->_w = 0;
  ptr->_flags = flags;
  ptr->_flags2 = 0;
  ptr->_file = file;
  ptr->_bf._base = 0;
  ptr->_bf._size = 0;
  ptr->_lbfsize = 0;
  memset (&ptr->_mbstate, 0, sizeof (_mbstate_t));
  ptr->_cookie = ptr;
  ptr->_read = __sread;
#ifndef __LARGE64_FILES
  ptr->_write = __swrite;
#else /* __LARGE64_FILES */
  ptr->_write = __swrite64;
  ptr->_seek64 = __sseek64;
  ptr->_flags |= __SL64;
#endif /* __LARGE64_FILES */
  ptr->_seek = __sseek;
  ptr->_close = __sclose;
#if !defined(__SINGLE_THREAD__) && !defined(_REENT_SMALL)
  __lock_init_recursive (ptr->_lock);
  /*
   * #else
   * lock is already initialized in __sfp
   */
#endif

#ifdef __SCLE
  if (__stextmode (ptr->_file))
    ptr->_flags |= __SCLE;
#endif
}

static _VOID
_DEFUN(__sinit, (s),
       struct _reent *s)
{
  __sinit_lock_acquire ();

  if (s->__sdidinit)
    {
      __sinit_lock_release ();
      return;
    }

  /* make sure we clean up on exit */
  s->__cleanup = _cleanup_r;	/* conservative */
  s->__sdidinit = 1;

  s->__sglue._next = NULL;
#ifndef _REENT_SMALL
  s->__sglue._niobs = 3;
  s->__sglue._iobs = &s->__sf[0];
#else
  s->__sglue._niobs = 0;
  s->__sglue._iobs = NULL;
  s->_stdin = __sfp(s);
  s->_stdout = __sfp(s);
  s->_stderr = __sfp(s);
#endif

  _std (s->_stdin,  __SRD, 0, s);

  /* On platforms that have true file system I/O, we can verify
     whether stdout is an interactive terminal or not, as part of
     __smakebuf on first use of the stream.  For all other platforms,
     we will default to line buffered mode here.  Technically, POSIX
     requires both stdin and stdout to be line-buffered, but tradition
     leaves stdin alone on systems without fcntl.  */
#ifdef HAVE_FCNTL
  _std (s->_stdout, __SWR, 1, s);
#else
  _std (s->_stdout, __SWR | __SLBF, 1, s);
#endif

  /* POSIX requires stderr to be opened for reading and writing, even
     when the underlying fd 2 is write-only.  */
  _std (s->_stderr, __SRW | __SNBF, 2, s);

  __sinit_lock_release ();
}

static int
_DEFUN(_isatty_r, (ptr, fd),
     struct _reent *ptr _AND
     int fd)
{
  int ret;

  errno = 0;
  if ((ret = _isatty (fd)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

static int
_DEFUN(_fstat_r, (ptr, fd, pstat),
     struct _reent *ptr _AND
     int fd _AND
     struct stat *pstat)
{
  int ret;

  errno = 0;
  if ((ret = _fstat (fd, pstat)) == -1 && errno != 0)
    ptr->_errno = errno;
  return ret;
}

static _VOID
_DEFUN(__smakebuf_r, (ptr, fp),
       struct _reent *ptr _AND
       register FILE *fp)
{
  register size_t size, couldbetty;
  register _PTR p;
#ifdef __USE_INTERNAL_STAT64
  struct stat64 st;
#else
  struct stat st;
#endif

  if (fp->_flags & __SNBF)
    {
      fp->_bf._base = fp->_p = fp->_nbuf;
      fp->_bf._size = 1;
      return;
    }
#ifdef __USE_INTERNAL_STAT64
  if (fp->_file < 0 || _fstat64_r (ptr, fp->_file, &st) < 0)
#else
  if (fp->_file < 0 || _fstat_r (ptr, fp->_file, &st) < 0)
#endif
    {
      couldbetty = 0;
      /* Check if we are be called by asprintf family for initial buffer.  */
      if (fp->_flags & __SMBF)
        size = _DEFAULT_ASPRINTF_BUFSIZE;
      else
        size = BUFSIZ;
      /* do not try to optimise fseek() */
      fp->_flags |= __SNPT;
    }
  else
    {
      couldbetty = (st.st_mode & S_IFMT) == S_IFCHR;
#ifdef HAVE_BLKSIZE
      size = st.st_blksize <= 0 ? BUFSIZ : st.st_blksize;
#else
      size = BUFSIZ;
#endif
      /*
       * Optimize fseek() only if it is a regular file.
       * (The test for __sseek is mainly paranoia.)
       */
      if ((st.st_mode & S_IFMT) == S_IFREG && fp->_seek == __sseek)
	{
	  fp->_flags |= __SOPT;
#ifdef HAVE_BLKSIZE
	  fp->_blksize = st.st_blksize;
#else
	  fp->_blksize = 1024;
#endif
	}
      else
	fp->_flags |= __SNPT;
    }
  if ((p = _malloc_r (ptr, size)) == NULL)
    {
      if (!(fp->_flags & __SSTR))
	{
	  fp->_flags |= __SNBF;
	  fp->_bf._base = fp->_p = fp->_nbuf;
	  fp->_bf._size = 1;
	}
    }
  else
    {
      ptr->__cleanup = _cleanup_r;
      fp->_flags |= __SMBF;
      fp->_bf._base = fp->_p = (unsigned char *) p;
      fp->_bf._size = size;
      if (couldbetty && _isatty_r (ptr, fp->_file))
	fp->_flags |= __SLBF;
    }
}

/*
 * Refill a stdio buffer.
 * Return EOF on eof or error, 0 otherwise.
 */

static int
_DEFUN(__srefill_r, (ptr, fp),
       struct _reent * ptr _AND
       register FILE * fp)
{
  /* make sure stdio is set up */

  CHECK_INIT (ptr, fp);

  ORIENT (fp, -1);

  fp->_r = 0;			/* largely a convenience for callers */

#ifndef __CYGWIN__
  /* SysV does not make this test; take it out for compatibility */
  if (fp->_flags & __SEOF)
    return EOF;
#endif

  /* if not already reading, have to be reading and writing */
  if ((fp->_flags & __SRD) == 0)
    {
      if ((fp->_flags & __SRW) == 0)
	{
	  ptr->_errno = EBADF;
	  fp->_flags |= __SERR;
	  return EOF;
	}
      /* switch to reading */
      if (fp->_flags & __SWR)
	{
	  if (_fflush_r (ptr, fp))
	    return EOF;
	  fp->_flags &= ~__SWR;
	  fp->_w = 0;
	  fp->_lbfsize = 0;
	}
      fp->_flags |= __SRD;
    }
  else
    {
      /*
       * We were reading.  If there is an ungetc buffer,
       * we must have been reading from that.  Drop it,
       * restoring the previous buffer (if any).  If there
       * is anything in that buffer, return.
       */
      if (HASUB (fp))
	{
	  FREEUB (ptr, fp);
	  if ((fp->_r = fp->_ur) != 0)
	    {
	      fp->_p = fp->_up;
	      return 0;
	    }
	}
    }

  if (fp->_bf._base == NULL)
    __smakebuf_r (ptr, fp);

  /*
   * Before reading from a line buffered or unbuffered file,
   * flush all line buffered output files, per the ANSI C
   * standard.
   */

  if (fp->_flags & (__SLBF | __SNBF))
    _CAST_VOID _fwalk (_GLOBAL_REENT, lflush);
  fp->_p = fp->_bf._base;
  fp->_r = fp->_read (ptr, fp->_cookie, (char *) fp->_p, fp->_bf._size);
#ifndef __CYGWIN__
  if (fp->_r <= 0)
#else
  if (fp->_r > 0)
    fp->_flags &= ~__SEOF;
  else
#endif
    {
      if (fp->_r == 0)
	fp->_flags |= __SEOF;
      else
	{
	  fp->_r = 0;
	  fp->_flags |= __SERR;
	}
      return EOF;
    }
  return 0;
}

#define MAXEXP       4932 /* 308 */
#define MAXFRACT     39
#define BUF          (MAXEXP+MAXFRACT+3)
#define BufferEmpty  (fp->_r <= 0 && __srefill(fp))

typedef union
{
  char c[16] __attribute__ ((__aligned__ (16)));
  short h[8];
  long l[4];
  int i[4];
  float f[4];
} vec_union;

static int
_DEFUN(__svfscanf_r, (rptr, fp, fmt0, ap),
     struct _reent *rptr _AND
     register FILE *fp _AND
     char _CONST *fmt0 _AND
     va_list ap)
{
  register u_char *fmt = (u_char *) fmt0;
  register int c;		/* character from format, or conversion */
  register int type;		/* conversion type */
  register size_t width;	/* field width, or 0 */
  register char *p;		/* points into all kinds of strings */
  register int n;		/* handy integer */
  register int flags;		/* flags as defined above */
  register char *p0;		/* saves original value of p when necessary */
  int orig_flags;               /* saved flags used when processing vector */
  int int_width;                /* tmp area to store width when processing int */
  int nassigned;		/* number of fields assigned */
  int nread;			/* number of characters consumed from fp */
  int base = 0;			/* base argument to strtol/strtoul */
  int nbytes = 1;               /* number of bytes read from fmt string */
  wchar_t wc;                   /* wchar to use to read format string */
  char vec_sep;                 /* vector separator char */
  char last_space_char;         /* last white-space char eaten - needed for vec support */
  int vec_read_count;           /* number of vector items to read separately */
  int looped;                   /* has vector processing looped */
  u_long (*ccfn) () = 0;	/* conversion function (strtol/strtoul) */
  char ccltab[256];		/* character class table for %[...] */
  char buf[BUF];		/* buffer for numeric conversions */
  vec_union vec_buf;
  char *lptr;                   /* literal pointer */
#ifdef _MB_CAPABLE
  mbstate_t state;                /* value to keep track of multibyte state */
#endif

  char *ch_dest;
  short *sp;
  int *ip;
  float *flp;
  _LONG_DOUBLE *ldp;
  double *dp;
  long *lp;
#ifndef _NO_LONGLONG
  long long *llp;
#else
	u_long _uquad;
#endif

  /* `basefix' is used to avoid `if' tests in the integer scanner */
  static _CONST short basefix[17] =
    {10, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

  nassigned = 0;
  nread = 0;
  for (;;)
    {
#ifndef _MB_CAPABLE
      wc = *fmt;
#else
      memset (&state, '\0', sizeof (state));
      nbytes = _mbtowc_r (rptr, &wc, fmt, MB_CUR_MAX, &state);
#endif
      fmt += nbytes;
      if (wc == 0)
	return nassigned;
      if (nbytes == 1 && isspace (wc))
	{
	  for (;;)
	    {
	      if (BufferEmpty)
		return nassigned;
	      if (!isspace (*fp->_p))
		break;
	      nread++, fp->_r--, fp->_p++;
	    }
	  continue;
	}
      if (wc != '%')
	goto literal;
      width = 0;
      flags = 0;
      vec_sep = ' ';
      vec_read_count = 0;
      looped = 0;

      /*
       * switch on the format.  continue if done; break once format
       * type is derived.
       */

    again:
      c = *fmt++;

      switch (c)
	{
	case '%':
	literal:
          lptr = fmt - nbytes;
          for (n = 0; n < nbytes; ++n)
            {
	      if (BufferEmpty)
	        goto input_failure;
	      if (*fp->_p != *lptr)
	        goto match_failure;
	      fp->_r--, fp->_p++;
	      nread++;
              ++lptr;
            }
	  continue;

	case '*':
	  flags |= SUPPRESS;
	  goto again;
	case ',':
	case ';':
	case ':':
	case '_':
	  if (flags == SUPPRESS || flags == 0)
	    vec_sep = c;
	  goto again;
	case 'l':
	  if (flags & SHORT)
	    continue; /* invalid format, don't process any further */
	  if (flags & LONG)
	    {
	      flags &= ~LONG;
	      flags &= ~VECTOR;
	      flags |= LONGDBL;
	    }
	  else
	    {
	      flags |= LONG;
	      if (flags & VECTOR)
		vec_read_count = 4;
	    }
	  goto again;
	case 'L':
	  flags |= LONGDBL;
	  flags &= ~VECTOR;
	  goto again;
	case 'h':
	  flags |= SHORT;
	  if (flags & LONG)
	    continue;  /* invalid format, don't process any further */
	  if (flags & VECTOR)
	    vec_read_count = 8;
	  goto again;
#ifdef __ALTIVEC__
	case 'v':
	  flags |= VECTOR;
	  vec_read_count = (flags & SHORT) ? 8 : ((flags & LONG) ? 4 : 16);
	  goto again;
#endif
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	  width = width * 10 + c - '0';
	  goto again;

	  /*
	   * Conversions. Those marked `compat' are for
	   * 4.[123]BSD compatibility.
	   *
	   * (According to ANSI, E and X formats are supposed to
	   * the same as e and x.  Sorry about that.)
	   */

	case 'D':		/* compat */
	  flags |= LONG;
	  /* FALLTHROUGH */
	case 'd':
	  type = CT_INT;
	  ccfn = (u_long (*)())_strtol_r;
	  base = 10;
	  break;

	case 'i':
	  type = CT_INT;
	  ccfn = (u_long (*)())_strtol_r;
	  base = 0;
	  break;

	case 'O':		/* compat */
	  flags |= LONG;
	  /* FALLTHROUGH */
	case 'o':
	  type = CT_INT;
	  ccfn = _strtoul_r;
	  base = 8;
	  break;

	case 'u':
	  type = CT_INT;
	  ccfn = _strtoul_r;
	  base = 10;
	  break;

	case 'X':		/* compat   XXX */
	case 'x':
	  flags |= PFXOK;	/* enable 0x prefixing */
	  type = CT_INT;
	  ccfn = _strtoul_r;
	  base = 16;
	  break;

#ifdef FLOATING_POINT
	case 'E':		/* compat   XXX */
	case 'G':		/* compat   XXX */
/* ANSI says that E,G and X behave the same way as e,g,x */
	  /* FALLTHROUGH */
	case 'e':
	case 'f':
	case 'g':
	  type = CT_FLOAT;
	  if (flags & VECTOR)
	    vec_read_count = 4;
	  break;

# ifdef __SPE__
	  /* treat fixed-point like %f floating point */
        case 'r':
	  flags |= SIGNED;
	  /* fallthrough */
        case 'R':
          flags |= FIXEDPOINT;
	  type = CT_FLOAT;
          break;
# endif
#endif

	case 's':
	  flags &= ~VECTOR;
	  type = CT_STRING;
	  break;

	case '[':
	  fmt = __sccl (ccltab, fmt);
	  flags |= NOSKIP;
	  flags &= ~VECTOR;
	  type = CT_CCL;
	  break;

	case 'c':
	  flags |= NOSKIP;
	  type = CT_CHAR;
	  if (flags & VECTOR)
	    {
	      /* not allowed to have h or l with c specifier */
	      if (flags & (LONG | SHORT))
		continue;  /* invalid format don't process any further */
	      width = 0;
	      vec_read_count = 16;
	    }
	  break;

	case 'p':		/* pointer format is like hex */
	  flags |= POINTER | PFXOK;
	  type = CT_INT;
	  ccfn = _strtoul_r;
	  base = 16;
	  break;

	case 'n':
	  if (flags & SUPPRESS)	/* ??? */
	    continue;
	  flags &= ~VECTOR;
	  if (flags & SHORT)
	    {
	      sp = va_arg (ap, short *);
	      *sp = nread;
	    }
	  else if (flags & LONG)
	    {
	      lp = va_arg (ap, long *);
	      *lp = nread;
	    }
#ifndef _NO_LONGLONG
	  else if (flags & LONGDBL)
	    {
	      llp = va_arg (ap, long long*);
	      *llp = nread;
	    }
#endif
	  else
	    {
	      ip = va_arg (ap, int *);
	      *ip = nread;
	    }
	  continue;

	  /*
	   * Disgusting backwards compatibility hacks.	XXX
	   */
	case '\0':		/* compat */
	  return EOF;

	default:		/* compat */
	  if (isupper (c))
	    flags |= LONG;
	  type = CT_INT;
	  ccfn = (u_long (*)())_strtol_r;
	  base = 10;
	  break;
	}

    process:
      /*
       * We have a conversion that requires input.
       */
      if (BufferEmpty)
	goto input_failure;

      /*
       * Consume leading white space, except for formats that
       * suppress this.
       */
      last_space_char = '\0';

      if ((flags & NOSKIP) == 0)
	{
	  while (isspace (*fp->_p))
	    {
	      last_space_char = *fp->_p;
	      nread++;
	      if (--fp->_r > 0)
		fp->_p++;
	      else
#ifndef CYGNUS_NEC
	      if (__srefill (fp))
#endif
		goto input_failure;
	    }
	  /*
	   * Note that there is at least one character in the
	   * buffer, so conversions that do not set NOSKIP ca
	   * no longer result in an input failure.
	   */
	}

      /* for vector formats process separator characters after first loop */
      if (looped && (flags & VECTOR))
	{
	  flags = orig_flags;
	  /* all formats other than default char have a separator char */
	  if (vec_sep != ' ' || type != CT_CHAR)
	    {
	      if (vec_sep == ' ' && last_space_char != ' ' ||
		  vec_sep != ' ' && *fp->_p != vec_sep)
		goto match_failure;
	      if (vec_sep != ' ')
		{
		  nread++;
		  if (--fp->_r > 0)
		    fp->_p++;
		  else
#ifndef CYGNUS_NEC
		    if (__srefill (fp))
#endif
		      goto input_failure;
		}
	    }
	  /* after eating the separator char, we must eat any white-space
	     after the separator char that precedes the data to convert */
	  if ((flags & NOSKIP) == 0)
	    {
	      while (isspace (*fp->_p))
		{
		  last_space_char = *fp->_p;
		  nread++;
		  if (--fp->_r > 0)
		    fp->_p++;
		  else
#ifndef CYGNUS_NEC
		    if (__srefill (fp))
#endif
		      goto input_failure;
		}
	    }

 	}
      else /* save to counter-act changes made to flags when processing */
	orig_flags = flags;

      /*
       * Do the conversion.
       */
      switch (type)
	{

	case CT_CHAR:
	  /* scan arbitrary characters (sets NOSKIP) */
	  if (width == 0)
	    width = 1;
	  if (flags & SUPPRESS)
	    {
	      size_t sum = 0;

	      for (;;)
		{
		  if ((n = fp->_r) < (int)width)
		    {
		      sum += n;
		      width -= n;
		      fp->_p += n;
#ifndef CYGNUS_NEC
		      if (__srefill (fp))
			{
#endif
			  if (sum == 0)
			    goto input_failure;
			  break;
#ifndef CYGNUS_NEC
			}
#endif
		    }
		  else
		    {
		      sum += width;
		      fp->_r -= width;
		      fp->_p += width;
		      break;
		    }
		}
	      nread += sum;
	    }
	  else
	    {
	      int n = width;
	      if (!looped)
		{
		  if (flags & VECTOR)
		    ch_dest = vec_buf.c;
		  else
		    ch_dest = va_arg (ap, char *);
		}
#ifdef CYGNUS_NEC
	      /* Kludge city for the moment */
	      if (fp->_r == 0)
		goto input_failure;

	      while (n && fp->_r)
		{
		  *ch_dest++ = *(fp->_p++);
		  n--;
		  fp->_r--;
		  nread++;
		}
#else
	      size_t r = fread (ch_dest, 1, width, fp);

	      if (r == 0)
		goto input_failure;
	      nread += r;
	      ch_dest += r;
#endif
	      if (!(flags & VECTOR))
		nassigned++;
	    }
	  break;

	case CT_CCL:
	  /* scan a (nonempty) character class (sets NOSKIP) */
	  if (width == 0)
	    width = ~0;		/* `infinity' */
	  /* take only those things in the class */
	  if (flags & SUPPRESS)
	    {
	      n = 0;
	      while (ccltab[*fp->_p])
		{
		  n++, fp->_r--, fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    {
		      if (n == 0)
			goto input_failure;
		      break;
		    }
		}
	      if (n == 0)
		goto match_failure;
	    }
	  else
	    {
	      p0 = p = va_arg (ap, char *);
	      while (ccltab[*fp->_p])
		{
		  fp->_r--;
		  *p++ = *fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    {
		      if (p == p0)
			goto input_failure;
		      break;
		    }
		}
	      n = p - p0;
	      if (n == 0)
		goto match_failure;
	      *p = 0;
	      nassigned++;
	    }
	  nread += n;
	  break;

	case CT_STRING:
	  /* like CCL, but zero-length string OK, & no NOSKIP */
	  if (width == 0)
	    width = ~0;
	  if (flags & SUPPRESS)
	    {
	      n = 0;
	      while (!isspace (*fp->_p))
		{
		  n++, fp->_r--, fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    break;
		}
	      nread += n;
	    }
	  else
	    {
	      p0 = p = va_arg (ap, char *);
	      while (!isspace (*fp->_p))
		{
		  fp->_r--;
		  *p++ = *fp->_p++;
		  if (--width == 0)
		    break;
		  if (BufferEmpty)
		    break;
		}
	      *p = 0;
	      nread += p - p0;
	      nassigned++;
	    }
	  continue;

	case CT_INT:
	  {
	  unsigned int_width_left = 0;
	  int skips = 0;
	  int_width = width;
#ifdef hardway
	  if (int_width == 0 || int_width > sizeof (buf) - 1)
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (int_width - 1 > sizeof (buf) - 2)
#endif
	    {
	      int_width_left = width - (sizeof (buf) - 1);
	      int_width = sizeof (buf) - 1;
	    }
	  flags |= SIGNOK | NDIGITS | NZDIGITS | NNZDIGITS;
	  for (p = buf; int_width; int_width--)
	    {
	      c = *fp->_p;
	      /*
	       * Switch on the character; `goto ok' if we
	       * accept it as a part of number.
	       */
	      switch (c)
		{
		  /*
		   * The digit 0 is always legal, but is special.
		   * For %i conversions, if no digits (zero or nonzero)
		   * have been scanned (only signs), we will have base==0.
		   * In that case, we should set it to 8 and enable 0x
		   * prefixing. Also, if we have not scanned zero digits
		   * before this, do not turn off prefixing (someone else
		   * will turn it off if we have scanned any nonzero digits).
		   */
		case '0':
		  if (! (flags & NNZDIGITS))
		    goto ok;
		  if (base == 0)
		    {
		      base = 8;
		      flags |= PFXOK;
		    }
		  if (flags & NZDIGITS)
		    {
		      flags &= ~(SIGNOK | NZDIGITS | NDIGITS);
		      goto ok;
		    }
		  flags &= ~(SIGNOK | PFXOK | NDIGITS);
		  if (int_width_left)
		    {
		      int_width_left--;
		      int_width++;
		    }
		  ++skips;
		  goto skip;

		  /* 1 through 7 always legal */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		  base = basefix[base];
		  flags &= ~(SIGNOK | PFXOK | NDIGITS | NNZDIGITS);
		  goto ok;

		  /* digits 8 and 9 ok iff decimal or hex */
		case '8':
		case '9':
		  base = basefix[base];
		  if (base <= 8)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS | NNZDIGITS);
		  goto ok;

		  /* letters ok iff hex */
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
		  /* no need to fix base here */
		  if (base <= 10)
		    break;	/* not legal here */
		  flags &= ~(SIGNOK | PFXOK | NDIGITS | NNZDIGITS);
		  goto ok;

		  /* sign ok only as first character */
		case '+':
		case '-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto ok;
		    }
		  break;

		  /* x ok iff flag still set & 2nd char */
		case 'x':
		case 'X':
		  if (flags & PFXOK && p == buf + 1)
		    {
		      base = 16;/* if %i */
		      flags &= ~PFXOK;
		      /* We must reset the NZDIGITS and NDIGITS
		         flags that would have been unset by seeing
		         the zero that preceded the X or x.  */
		      flags |= NZDIGITS | NDIGITS;
		      goto ok;
		    }
		  break;
		}

	      /*
	       * If we got here, c is not a legal character
	       * for a number.  Stop accumulating digits.
	       */
	      break;
	    ok:
	      /*
	       * c is legal: store it and look at the next.
	       */
	      *p++ = c;
	    skip:
	      if (--fp->_r > 0)
		fp->_p++;
	      else
#ifndef CYGNUS_NEC
	      if (__srefill (fp))
#endif
		break;		/* EOF */
	    }
	  /*
	   * If we had only a sign, it is no good; push back the sign.
	   * If the number ends in `x', it was [sign] '0' 'x', so push back
	   * the x and treat it as [sign] '0'.
	   */
	  if (flags & NDIGITS)
	    {
	      if (p > buf)
		_CAST_VOID ungetc (*(u_char *)-- p, fp);
	      goto match_failure;
	    }
	  c = ((u_char *) p)[-1];
	  if (c == 'x' || c == 'X')
	    {
	      --p;
	      /*(void)*/ ungetc (c, fp);
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
	      u_long res;

	      *p = 0;
	      res = (*ccfn) (rptr, buf, (char **) NULL, base);
	      if ((flags & POINTER) && !(flags & VECTOR))
		*(va_arg (ap, _PTR *)) = (_PTR) (unsigned _POINTER_INT) res;
	      else if (flags & SHORT)
		{
		  if (!(flags & VECTOR))
		    sp = va_arg (ap, short *);
		  else if (!looped)
		    sp = vec_buf.h;
		  *sp++ = res;
		}
	      else if (flags & LONG)
		{
		  if (!(flags & VECTOR))
		    lp = va_arg (ap, long *);
		  else if (!looped)
		    lp = vec_buf.l;
		  *lp++ = res;
		}
#ifndef _NO_LONGLONG
	      else if (flags & LONGDBL)
		{
		  u_long_long resll;
		  if (ccfn == _strtoul_r)
		    resll = _strtoull_r (rptr, buf, (char **) NULL, base);
		  else
		    resll = _strtoll_r (rptr, buf, (char **) NULL, base);
		  llp = va_arg (ap, long long*);
		  *llp = resll;
		}
#endif
	      else
		{
		  if (!(flags & VECTOR))
		    {
		      ip = va_arg (ap, int *);
		      *ip++ = res;
		    }
		  else
		    {
		      if (!looped)
			ch_dest = vec_buf.c;
		      *ch_dest++ = (char)res;
		    }
		}
	      if (!(flags & VECTOR))
		nassigned++;
	    }
	  nread += p - buf + skips;
	  break;
	  }

#ifdef FLOATING_POINT
	case CT_FLOAT:
	{
	  /* scan a floating point number as if by strtod */
	  /* This code used to assume that the number of digits is reasonable.
	     However, ANSI / ISO C makes no such stipulation; we have to get
	     exact results even when there is an unreasonable amount of
	     leading zeroes.  */
	  long leading_zeroes = 0;
	  long zeroes, exp_adjust;
	  char *exp_start = NULL;
	  unsigned fl_width = width;
	  unsigned width_left = 0;
#ifdef hardway
	  if (fl_width == 0 || fl_width > sizeof (buf) - 1)
#else
	  /* size_t is unsigned, hence this optimisation */
	  if (fl_width - 1 > sizeof (buf) - 2)
#endif
	    {
	      width_left = fl_width - (sizeof (buf) - 1);
	      fl_width = sizeof (buf) - 1;
	    }
	  flags |= SIGNOK | NDIGITS | DPTOK | EXPOK;
	  zeroes = 0;
	  exp_adjust = 0;
	  for (p = buf; fl_width; )
	    {
	      c = *fp->_p;
	      /*
	       * This code mimicks the integer conversion
	       * code, but is much simpler.
	       */
	      switch (c)
		{

		case '0':
		  if (flags & NDIGITS)
		    {
		      flags &= ~SIGNOK;
		      zeroes++;
		      if (width_left)
			{
			  width_left--;
			  fl_width++;
			}
		      goto fskip;
		    }
		  /* Fall through.  */
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		  flags &= ~(SIGNOK | NDIGITS);
		  goto fok;

		case '+':
		case '-':
		  if (flags & SIGNOK)
		    {
		      flags &= ~SIGNOK;
		      goto fok;
		    }
		  break;
		case '.':
		  if (flags & DPTOK)
		    {
		      flags &= ~(SIGNOK | DPTOK);
		      leading_zeroes = zeroes;
		      goto fok;
		    }
		  break;
		case 'e':
		case 'E':
		  /* no exponent without some digits */
		  if ((flags & (NDIGITS | EXPOK)) == EXPOK
		      || ((flags & EXPOK) && zeroes))
		    {
		      if (! (flags & DPTOK))
			{
			  exp_adjust = zeroes - leading_zeroes;
			  exp_start = p;
			}
		      flags =
			(flags & ~(EXPOK | DPTOK)) |
			SIGNOK | NDIGITS;
		      zeroes = 0;
		      goto fok;
		    }
		  break;
		}
	      break;
	    fok:
	      *p++ = c;
	    fskip:
	      fl_width--;
              ++nread;
	      if (--fp->_r > 0)
		fp->_p++;
	      else
#ifndef CYGNUS_NEC
	      if (__srefill (fp))
#endif
		break;		/* EOF */
	    }
	  if (zeroes)
	    flags &= ~NDIGITS;
	  /*
	   * If no digits, might be missing exponent digits
	   * (just give back the exponent) or might be missing
	   * regular digits, but had sign and/or decimal point.
	   */
	  if (flags & NDIGITS)
	    {
	      if (flags & EXPOK)
		{
		  /* no digits at all */
		  while (p > buf)
                    {
		      ungetc (*(u_char *)-- p, fp);
                      --nread;
                    }
		  goto match_failure;
		}
	      /* just a bad exponent (e and maybe sign) */
	      c = *(u_char *)-- p;
              --nread;
	      if (c != 'e' && c != 'E')
		{
		  _CAST_VOID ungetc (c, fp);	/* sign */
		  c = *(u_char *)-- p;
                  --nread;
		}
	      _CAST_VOID ungetc (c, fp);
	    }
	  if ((flags & SUPPRESS) == 0)
	    {
#ifdef _NO_LONGDBL
	      double res;
#else  /* !_NO_LONG_DBL */
	      long double res;
#endif /* !_NO_LONG_DBL */
	      long new_exp = 0;

	      *p = 0;
	      if ((flags & (DPTOK | EXPOK)) == EXPOK)
		{
		  exp_adjust = zeroes - leading_zeroes;
		  new_exp = -exp_adjust;
		  exp_start = p;
		}
	      else if (exp_adjust)
                new_exp = _strtol_r (rptr, (exp_start + 1), NULL, 10) - exp_adjust;
	      if (exp_adjust)
		{

		  /* If there might not be enough space for the new exponent,
		     truncate some trailing digits to make room.  */
		  if (exp_start >= buf + sizeof (buf) - MAX_LONG_LEN)
		    exp_start = buf + sizeof (buf) - MAX_LONG_LEN - 1;
                 sprintf (exp_start, "e%ld", new_exp);
		}
#ifdef __SPE__
	      if (flags & FIXEDPOINT)
		{
		  __uint64_t ufix64;
		  if (flags & SIGNED)
		    ufix64 = (__uint64_t)_strtosfix64_r (rptr, buf, NULL);
                  else
		    ufix64 = _strtoufix64_r (rptr, buf, NULL);
		  if (flags & SHORT)
		    {
		      __uint16_t *sp = va_arg (ap, __uint16_t *);
		      *sp = (__uint16_t)(ufix64 >> 48);
		    }
		  else if (flags & LONG)
		    {
		      __uint64_t *llp = va_arg (ap, __uint64_t *);
		      *llp = ufix64;
		    }
		  else
		    {
		      __uint32_t *lp = va_arg (ap, __uint32_t *);
		      *lp = (__uint32_t)(ufix64 >> 32);
		    }
		  nassigned++;
		  break;
		}

#endif /* __SPE__ */
#ifdef _NO_LONGDBL
	      res = _strtod_r (rptr, buf, NULL);
#else  /* !_NO_LONGDBL */
	      res = _strtold (buf, NULL);
#endif /* !_NO_LONGDBL */
	      if (flags & LONG)
		{
		  dp = va_arg (ap, double *);
		  *dp = res;
		}
	      else if (flags & LONGDBL)
		{
		  ldp = va_arg (ap, _LONG_DOUBLE *);
		  *ldp = res;
		}
	      else
		{
		  if (!(flags & VECTOR))
		    flp = va_arg (ap, float *);
		  else if (!looped)
		    flp = vec_buf.f;
		  *flp++ = res;
		}
	      if (!(flags & VECTOR))
		nassigned++;
	    }
	  break;
	}
#endif /* FLOATING_POINT */
	}
      if (vec_read_count-- > 1)
	{
	  looped = 1;
	  goto process;
	}
      if (flags & VECTOR)
	{
	  int i;
	  unsigned long *vp = va_arg (ap, unsigned long *);
	  for (i = 0; i < 4; ++i)
	    *vp++ = vec_buf.l[i];
	  nassigned++;
	}
    }
input_failure:
  return nassigned ? nassigned : -1;
match_failure:
  return nassigned;
}

#undef BUF

static FILE *
_DEFUN (__sfp, (d),
	struct _reent *d)
{
  int i;
  for (i = 0; i < SPE_FOPEN_MAX; i++) {
    if (!__fp[i]._fp) {
      return &__fp[i];
    }
  }
  d->_errno = EMFILE;
  return NULL;
}

static int
_DEFUN(__sflags, (ptr, mode, optr),
       struct _reent *ptr  _AND
       register char *mode _AND
       int *optr)
{
  register int ret, m, o;

  switch (mode[0])
    {
    case 'r':			/* open for reading */
      ret = __SRD;
      m = O_RDONLY;
      o = 0;
      break;

    case 'w':			/* open for writing */
      ret = __SWR;
      m = O_WRONLY;
      o = O_CREAT | O_TRUNC;
      break;

    case 'a':			/* open for appending */
      ret = __SWR | __SAPP;
      m = O_WRONLY;
      o = O_CREAT | O_APPEND;
      break;
    default:			/* illegal mode */
      ptr->_errno = EINVAL;
      return (0);
    }
  if (mode[1] && (mode[1] == '+' || mode[2] == '+'))
    {
      ret = (ret & ~(__SRD | __SWR)) | __SRW;
      m = O_RDWR;
    }
  if (mode[1] && (mode[1] == 'b' || mode[2] == 'b'))
    {
#ifdef O_BINARY
      m |= O_BINARY;
#endif
    }
#ifdef __CYGWIN__
  else if (mode[1] && (mode[1] == 't' || mode[2] == 't'))
#else
  else
#endif
    {
#ifdef O_TEXT
      m |= O_TEXT;
#endif
    }
  *optr = m | o;
  return ret;
}

static int
_DEFUN(lflush, (fp),
       FILE *fp)
{
  if ((fp->_flags & (__SLBF | __SWR)) == (__SLBF | __SWR))
    return fflush (fp);
  return 0;
}

#ifdef __SCLE
static int
_DEFUN(__stextmode, (fd),
       int fd)
{
#ifdef __CYGWIN__
  extern int _cygwin_istext_for_stdio (int);
  return _cygwin_istext_for_stdio (fd);
#else
  return 0;
#endif
}
#endif

void
_DEFUN(flockfile, (filehandle), FILE *filehandle)
{
  _flockfile(filehandle);
}

int
_DEFUN(ftrylockfile, (filehandle), FILE *filehandle)
{
#ifdef __SINGLE_THREAD__
  return 0;
#else
  if (filehandle->_flags & __SSTR)
    return 0;
  return __lock_try_acquire_recursive(filehandle->_lock);
#endif
}

void
_DEFUN(funlockfile, (filehandle), FILE *filehandle)
{
  _funlockfile(filehandle);
}

// ### SOURCES ##################################################################################

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
 */

/*
FUNCTION
<<fclose>>---close a file

INDEX
	fclose
INDEX
	_fclose_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fclose(FILE *<[fp]>);
	int _fclose_r(struct _reent *<[reent]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fclose(<[fp]>)
	FILE *<[fp]>;

	int fclose(<[fp]>)
        struct _reent *<[reent]>
	FILE *<[fp]>;

DESCRIPTION
If the file or stream identified by <[fp]> is open, <<fclose>> closes
it, after first ensuring that any pending data is written (by calling
<<fflush(<[fp]>)>>).

The alternate function <<_fclose_r>> is a reentrant version.
The extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<fclose>> returns <<0>> if successful (including when <[fp]> is
<<NULL>> or not an open file); otherwise, it returns <<EOF>>.

PORTABILITY
<<fclose>> is required by ANSI C.

Required OS subroutines: <<close>>, <<fstat>>, <<isatty>>, <<lseek>>,
<<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/lock.h>
#include "local.h"
#endif

int
_DEFUN(_fclose_r, (rptr, fp),
      struct _reent *rptr _AND
      register FILE * fp)
{
  int r;

  if (fp == NULL)
    return (0);			/* on NULL */

  __sfp_lock_acquire ();

  CHECK_INIT (rptr, fp);

  _flockfile (fp);

  if (fp->_flags == 0)		/* not open! */
    {
      _funlockfile (fp);
      __sfp_lock_release ();
      return (0);
    }
  /* Unconditionally flush to allow special handling for seekable read
     files to reposition file to last byte processed as opposed to
     last byte read ahead into the buffer.  */
  r = _fflush_r (rptr, fp);
  if (fp->_close != NULL && fp->_close (rptr, fp->_cookie) < 0)
    r = EOF;
  if (fp->_flags & __SMBF)
    _free_r (rptr, (char *) fp->_bf._base);
  if (HASUB (fp))
    FREEUB (rptr, fp);
  if (HASLB (fp))
    FREELB (rptr, fp);
  fp->_flags = 0;		/* release this FILE for reuse */
  _funlockfile (fp);
#ifndef __SINGLE_THREAD__
  __lock_close_recursive (fp->_lock);
#endif

  __sfp_lock_release ();

  return (r);
}

#ifndef _REENT_ONLY

int
_DEFUN(fclose, (fp),
       register FILE * fp)
{
  return _fclose_r(_REENT, fp);
}

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
 */

/*
FUNCTION
<<feof>>---test for end of file

INDEX
	feof

ANSI_SYNOPSIS
	#include <stdio.h>
	int feof(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int feof(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
<<feof>> tests whether or not the end of the file identified by <[fp]>
has been reached.

RETURNS
<<feof>> returns <<0>> if the end of file has not yet been reached; if
at end of file, the result is nonzero.

PORTABILITY
<<feof>> is required by ANSI C.

No supporting OS subroutines are required.
*/

#if 0
#include <stdio.h>
#include "local.h"
#endif

/* A subroutine version of the macro feof.  */

#undef feof

int
_DEFUN(feof, (fp),
       FILE * fp)
{
  int result;
  CHECK_INIT(_REENT, fp);
  _flockfile (fp);
  result = __sfeof (fp);
  _funlockfile (fp);
  return result;
}

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
 */

/*
FUNCTION
<<fflush>>---flush buffered file output

INDEX
	fflush
INDEX
	_fflush_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fflush(FILE *<[fp]>);

	int _fflush_r(struct _reent *<[reent]>, FILE *<[fp]>);

DESCRIPTION
The <<stdio>> output functions can buffer output before delivering it
to the host system, in order to minimize the overhead of system calls.

Use <<fflush>> to deliver any such pending output (for the file
or stream identified by <[fp]>) to the host system.

If <[fp]> is <<NULL>>, <<fflush>> delivers pending output from all
open files.

Additionally, if <[fp]> is a seekable input stream visiting a file
descriptor, set the position of the file descriptor to match next
unread byte, useful for obeying POSIX semantics when ending a process
without consuming all input from the stream.

The alternate function <<_fflush_r>> is a reentrant version, where the
extra argument <[reent]> is a pointer to a reentrancy structure, and
<[fp]> must not be NULL.

RETURNS
<<fflush>> returns <<0>> unless it encounters a write error; in that
situation, it returns <<EOF>>.

PORTABILITY
ANSI C requires <<fflush>>.  The behavior on input streams is only
specified by POSIX, and not all implementations follow POSIX rules.

No supporting OS subroutines are required.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"
#endif

/* Flush a single file, or (if fp is NULL) all files.  */

int
_DEFUN(_fflush_r, (ptr, fp),
       struct _reent *ptr _AND
       register FILE * fp)
{
  register unsigned char *p;
  register int n, t;

#ifdef _REENT_SMALL
  /* For REENT_SMALL platforms, it is possible we are being
     called for the first time on a std stream.  This std
     stream can belong to a reentrant struct that is not
     _REENT.  If CHECK_INIT gets called below based on _REENT,
     we will end up changing said file pointers to the equivalent
     std stream off of _REENT.  This causes unexpected behavior if
     there is any data to flush on the _REENT std stream.  There
     are two alternatives to fix this:  1) make a reentrant fflush
     or 2) simply recognize that this file has nothing to flush
     and return immediately before performing a CHECK_INIT.  Choice
     2 is implemented here due to its simplicity.  */
  if (fp->_bf._base == NULL)
    return 0;
#endif /* _REENT_SMALL  */

  CHECK_INIT (ptr, fp);

  _flockfile (fp);

  t = fp->_flags;
  if ((t & __SWR) == 0)
    {
      /* For a read stream, an fflush causes the next seek to be
         unoptimized (i.e. forces a system-level seek).  This conforms
         to the POSIX and SUSv3 standards.  */
      fp->_flags |= __SNPT;

      /* For a seekable stream with buffered read characters, we will attempt
         a seek to the current position now.  A subsequent read will then get
         the next byte from the file rather than the buffer.  This conforms
         to the POSIX and SUSv3 standards.  Note that the standards allow
         this seek to be deferred until necessary, but we choose to do it here
         to make the change simpler, more contained, and less likely
         to miss a code scenario.  */
      if ((fp->_r > 0 || fp->_ur > 0) && fp->_seek != NULL)
	{
	  int tmp;
#ifdef __LARGE64_FILES
	  _fpos64_t curoff;
#else
	  _fpos_t curoff;
#endif

	  /* Get the physical position we are at in the file.  */
	  if (fp->_flags & __SOFF)
	    curoff = fp->_offset;
	  else
	    {
	      /* We don't know current physical offset, so ask for it.
		 Only ESPIPE is ignorable.  */
#ifdef __LARGE64_FILES
	      if (fp->_flags & __SL64)
		curoff = fp->_seek64 (ptr, fp->_cookie, 0, SEEK_CUR);
	      else
#endif
		curoff = fp->_seek (ptr, fp->_cookie, 0, SEEK_CUR);
	      if (curoff == -1L)
		{
		  int result = EOF;
		  if (ptr->_errno == ESPIPE)
		    result = 0;
		  else
		    fp->_flags |= __SERR;
		  _funlockfile (fp);
		  return result;
		}
            }
          if (fp->_flags & __SRD)
            {
              /* Current offset is at end of buffer.  Compensate for
                 characters not yet read.  */
              curoff -= fp->_r;
              if (HASUB (fp))
                curoff -= fp->_ur;
            }
	  /* Now physically seek to after byte last read.  */
#ifdef __LARGE64_FILES
	  if (fp->_flags & __SL64)
	    tmp = (fp->_seek64 (ptr, fp->_cookie, curoff, SEEK_SET) == curoff);
	  else
#endif
	    tmp = (fp->_seek (ptr, fp->_cookie, curoff, SEEK_SET) == curoff);
	  if (tmp)
	    {
	      /* Seek successful.  We can clear read buffer now.  */
	      fp->_flags &= ~__SNPT;
	      fp->_r = 0;
	      fp->_p = fp->_bf._base;
	      if (fp->_flags & __SOFF)
		fp->_offset = curoff;
	      if (HASUB (fp))
		FREEUB (ptr, fp);
	    }
	  else
	    {
	      fp->_flags |= __SERR;
	      _funlockfile (fp);
	      return EOF;
	    }
	}
      _funlockfile (fp);
      return 0;
    }
  if ((p = fp->_bf._base) == NULL)
    {
      /* Nothing to flush.  */
      _funlockfile (fp);
      return 0;
    }
  n = fp->_p - p;		/* write this much */

  /*
   * Set these immediately to avoid problems with longjmp
   * and to allow exchange buffering (via setvbuf) in user
   * write function.
   */
  fp->_p = p;
  fp->_w = t & (__SLBF | __SNBF) ? 0 : fp->_bf._size;

  while (n > 0)
    {
      t = fp->_write (ptr, fp->_cookie, (char *) p, n);
      if (t <= 0)
	{
          fp->_flags |= __SERR;
          _funlockfile (fp);
          return EOF;
	}
      p += t;
      n -= t;
    }
  _funlockfile (fp);
  return 0;
}

int
_DEFUN(_fflush_r_unlocked, (ptr, fp),
       struct _reent *ptr _AND
       register FILE * fp)
{
  register unsigned char *p;
  register int n, t;

#ifdef _REENT_SMALL
  /* For REENT_SMALL platforms, it is possible we are being
     called for the first time on a std stream.  This std
     stream can belong to a reentrant struct that is not
     _REENT.  If CHECK_INIT gets called below based on _REENT,
     we will end up changing said file pointers to the equivalent
     std stream off of _REENT.  This causes unexpected behavior if
     there is any data to flush on the _REENT std stream.  There
     are two alternatives to fix this:  1) make a reentrant fflush
     or 2) simply recognize that this file has nothing to flush
     and return immediately before performing a CHECK_INIT.  Choice
     2 is implemented here due to its simplicity.  */
  if (fp->_bf._base == NULL)
    return 0;
#endif /* _REENT_SMALL  */

  CHECK_INIT (ptr, fp);

  t = fp->_flags;
  if ((t & __SWR) == 0)
    {
      /* For a read stream, an fflush causes the next seek to be
         unoptimized (i.e. forces a system-level seek).  This conforms
         to the POSIX and SUSv3 standards.  */
      fp->_flags |= __SNPT;

      /* For a seekable stream with buffered read characters, we will attempt
         a seek to the current position now.  A subsequent read will then get
         the next byte from the file rather than the buffer.  This conforms
         to the POSIX and SUSv3 standards.  Note that the standards allow
         this seek to be deferred until necessary, but we choose to do it here
         to make the change simpler, more contained, and less likely
         to miss a code scenario.  */
      if ((fp->_r > 0 || fp->_ur > 0) && fp->_seek != NULL)
	{
	  int tmp;
#ifdef __LARGE64_FILES
	  _fpos64_t curoff;
#else
	  _fpos_t curoff;
#endif

	  /* Get the physical position we are at in the file.  */
	  if (fp->_flags & __SOFF)
	    curoff = fp->_offset;
	  else
	    {
	      /* We don't know current physical offset, so ask for it.
		 Only ESPIPE is ignorable.  */
#ifdef __LARGE64_FILES
	      if (fp->_flags & __SL64)
		curoff = fp->_seek64 (ptr, fp->_cookie, 0, SEEK_CUR);
	      else
#endif
		curoff = fp->_seek (ptr, fp->_cookie, 0, SEEK_CUR);
	      if (curoff == -1L)
		{
		  int result = EOF;
		  if (ptr->_errno == ESPIPE)
		    result = 0;
		  else
		    fp->_flags |= __SERR;
		  return result;
		}
            }
          if (fp->_flags & __SRD)
            {
              /* Current offset is at end of buffer.  Compensate for
                 characters not yet read.  */
              curoff -= fp->_r;
              if (HASUB (fp))
                curoff -= fp->_ur;
            }
	  /* Now physically seek to after byte last read.  */
#ifdef __LARGE64_FILES
	  if (fp->_flags & __SL64)
	    tmp = (fp->_seek64 (ptr, fp->_cookie, curoff, SEEK_SET) == curoff);
	  else
#endif
	    tmp = (fp->_seek (ptr, fp->_cookie, curoff, SEEK_SET) == curoff);
	  if (tmp)
	    {
	      /* Seek successful.  We can clear read buffer now.  */
	      fp->_flags &= ~__SNPT;
	      fp->_r = 0;
	      fp->_p = fp->_bf._base;
	      if (fp->_flags & __SOFF)
		fp->_offset = curoff;
	      if (HASUB (fp))
		FREEUB (ptr, fp);
	    }
	  else
	    {
	      fp->_flags |= __SERR;
	      return EOF;
	    }
	}
      return 0;
    }
  if ((p = fp->_bf._base) == NULL)
    {
      /* Nothing to flush.  */
      return 0;
    }
  n = fp->_p - p;		/* write this much */

  /*
   * Set these immediately to avoid problems with longjmp
   * and to allow exchange buffering (via setvbuf) in user
   * write function.
   */
  fp->_p = p;
  fp->_w = t & (__SLBF | __SNBF) ? 0 : fp->_bf._size;

  while (n > 0)
    {
      t = fp->_write (ptr, fp->_cookie, (char *) p, n);
      if (t <= 0)
	{
          fp->_flags |= __SERR;
          return EOF;
	}
      p += t;
      n -= t;
    }
  return 0;
}

#ifndef _REENT_ONLY

int
_DEFUN(fflush, (fp),
       register FILE * fp)
{
  if (fp == NULL)
    return _fwalk_reent (_GLOBAL_REENT, _fflush_r);

  return _fflush_r (_REENT, fp);
}

int
_DEFUN(fflush_unlocked, (fp),
       register FILE * fp)
{
  if (fp == NULL)
    return _fwalk_reent_unlocked (_GLOBAL_REENT, _fflush_r_unlocked);

  return _fflush_r_unlocked (_REENT, fp);
}

#endif /* _REENT_ONLY */
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
 */

/*
FUNCTION
<<fgetc>>---get a character from a file or stream

INDEX
	fgetc
INDEX
	_fgetc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fgetc(FILE *<[fp]>);

	#include <stdio.h>
	int _fgetc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fgetc(<[fp]>)
	FILE *<[fp]>;

	#include <stdio.h>
	int _fgetc_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

DESCRIPTION
Use <<fgetc>> to get the next single character from the file or stream
identified by <[fp]>.  As a side effect, <<fgetc>> advances the file's
current position indicator.

For a macro version of this function, see <<getc>>.

The function <<_fgetc_r>> is simply a reentrant version of
<<fgetc>> that is passed the additional reentrant structure
pointer argument: <[ptr]>.

RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<fgetc>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
ANSI C requires <<fgetc>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

int
_DEFUN(_fgetc_r, (ptr, fp),
       struct _reent * ptr _AND
       FILE * fp)
{
  int result;
  CHECK_INIT(ptr, fp);
  _flockfile (fp);
  result = __sgetc_r (ptr, fp);
  _funlockfile (fp);
  return result;
}

#ifndef _REENT_ONLY

int
_DEFUN(fgetc, (fp),
       FILE * fp)
{
#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  int result;
  CHECK_INIT(_REENT, fp);
  __sfp_lock_acquire ();
  _flockfile (fp);
  result = __sgetc_r (_REENT, fp);
  _funlockfile (fp);
  __sfp_lock_release ();
  return result;
#else
  return _fgetc_r (_REENT, fp);
#endif
}

int
_DEFUN(fgetc_unlocked, (fp),
       FILE * fp)
{
#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  CHECK_INIT(_REENT, fp);
  return __sgetc_r (_REENT, fp);
#else
  return _fgetc_r (_REENT, fp);
#endif
}

#endif /* !_REENT_ONLY */

/* Copyright (C) 2007 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
<<fmemopen>>---open a stream around a fixed-length string

INDEX
	fmemopen

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *fmemopen(void *restrict <[buf]>, size_t <[size]>,
		       const char *restrict <[mode]>);

DESCRIPTION
<<fmemopen>> creates a seekable <<FILE>> stream that wraps a
fixed-length buffer of <[size]> bytes starting at <[buf]>.  The stream
is opened with <[mode]> treated as in <<fopen>>, where append mode
starts writing at the first NUL byte.  If <[buf]> is NULL, then
<[size]> bytes are automatically provided as if by <<malloc>>, with
the initial size of 0, and <[mode]> must contain <<+>> so that data
can be read after it is written.

The stream maintains a current position, which moves according to
bytes read or written, and which can be one past the end of the array.
The stream also maintains a current file size, which is never greater
than <[size]>.  If <[mode]> starts with <<r>>, the position starts at
<<0>>, and file size starts at <[size]> if <[buf]> was provided.  If
<[mode]> starts with <<w>>, the position and file size start at <<0>>,
and if <[buf]> was provided, the first byte is set to NUL.  If
<[mode]> starts with <<a>>, the position and file size start at the
location of the first NUL byte, or else <[size]> if <[buf]> was
provided.

When reading, NUL bytes have no significance, and reads cannot exceed
the current file size.  When writing, the file size can increase up to
<[size]> as needed, and NUL bytes may be embedded in the stream (see
<<open_memstream>> for an alternative that automatically enlarges the
buffer).  When the stream is flushed or closed after a write that
changed the file size, a NUL byte is written at the current position
if there is still room; if the stream is not also open for reading, a
NUL byte is additionally written at the last byte of <[buf]> when the
stream has exceeded <[size]>, so that a write-only <[buf]> is always
NUL-terminated when the stream is flushed or closed (and the initial
<[size]> should take this into account).  It is not possible to seek
outside the bounds of <[size]>.  A NUL byte written during a flush is
restored to its previous value when seeking elsewhere in the string.

RETURNS
The return value is an open FILE pointer on success.  On error,
<<NULL>> is returned, and <<errno>> will be set to EINVAL if <[size]>
is zero or <[mode]> is invalid, ENOMEM if <[buf]> was NULL and memory
could not be allocated, or EMFILE if too many streams are already
open.

PORTABILITY
This function is being added to POSIX 200x, but is not in POSIX 2001.

Supporting OS subroutines required: <<sbrk>>.
*/

#if 0
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/lock.h>
#include "local.h"
#endif

/* Describe details of an open memstream.  */
typedef struct fmemcookie {
  void *storage; /* storage to free on close */
  char *buf; /* buffer start */
  size_t pos; /* current position */
  size_t eof; /* current file size */
  size_t max; /* maximum file size */
  char append; /* nonzero if appending */
  char writeonly; /* 1 if write-only */
  char saved; /* saved character that lived at pos before write-only NUL */
} fmemcookie;

/* Read up to non-zero N bytes into BUF from stream described by
   COOKIE; return number of bytes read (0 on EOF).  */
static _READ_WRITE_RETURN_TYPE
_DEFUN(fmemreader, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       char *buf _AND
       int n)
{
  fmemcookie *c = (fmemcookie *) cookie;
  /* Can't read beyond current size, but EOF condition is not an error.  */
  if (c->pos > c->eof)
    return 0;
  if (n >= c->eof - c->pos)
    n = c->eof - c->pos;
  memcpy (buf, c->buf + c->pos, n);
  c->pos += n;
  return n;
}

/* Write up to non-zero N bytes of BUF into the stream described by COOKIE,
   returning the number of bytes written or EOF on failure.  */
static _READ_WRITE_RETURN_TYPE
_DEFUN(fmemwriter, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       const char *buf _AND
       int n)
{
  fmemcookie *c = (fmemcookie *) cookie;
  int adjust = 0; /* true if at EOF, but still need to write NUL.  */

  /* Append always seeks to eof; otherwise, if we have previously done
     a seek beyond eof, ensure all intermediate bytes are NUL.  */
  if (c->append)
    c->pos = c->eof;
  else if (c->pos > c->eof)
    memset (c->buf + c->eof, '\0', c->pos - c->eof);
  /* Do not write beyond EOF; saving room for NUL on write-only stream.  */
  if (c->pos + n > c->max - c->writeonly)
    {
      adjust = c->writeonly;
      n = c->max - c->pos;
    }
  /* Now n is the number of bytes being modified, and adjust is 1 if
     the last byte is NUL instead of from buf.  Write a NUL if
     write-only; or if read-write, eof changed, and there is still
     room.  When we are within the file contents, remember what we
     overwrite so we can restore it if we seek elsewhere later.  */
  if (c->pos + n > c->eof)
    {
      c->eof = c->pos + n;
      if (c->eof - adjust < c->max)
	c->saved = c->buf[c->eof - adjust] = '\0';
    }
  else if (c->writeonly)
    {
      if (n)
	{
	  c->saved = c->buf[c->pos + n - adjust];
	  c->buf[c->pos + n - adjust] = '\0';
	}
      else
	adjust = 0;
    }
  c->pos += n;
  if (n - adjust)
    memcpy (c->buf + c->pos - n, buf, n - adjust);
  else
    {
      ptr->_errno = ENOSPC;
      return EOF;
    }
  return n;
}

/* Seek to position POS relative to WHENCE within stream described by
   COOKIE; return resulting position or fail with EOF.  */
static _fpos_t
_DEFUN(fmemseeker, (ptr, cookie, pos, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos_t pos _AND
       int whence)
{
  fmemcookie *c = (fmemcookie *) cookie;
#ifndef __LARGE64_FILES
  off_t offset = (off_t) pos;
#else /* __LARGE64_FILES */
  _off64_t offset = (_off64_t) pos;
#endif /* __LARGE64_FILES */

  if (whence == SEEK_CUR)
    offset += c->pos;
  else if (whence == SEEK_END)
    offset += c->eof;
  if (offset < 0)
    {
      ptr->_errno = EINVAL;
      offset = -1;
    }
  else if (offset > c->max)
    {
      ptr->_errno = ENOSPC;
      offset = -1;
    }
#ifdef __LARGE64_FILES
  else if ((_fpos_t) offset != offset)
    {
      ptr->_errno = EOVERFLOW;
      offset = -1;
    }
#endif /* __LARGE64_FILES */
  else
    {
      if (c->writeonly && c->pos < c->eof)
	{
	  c->buf[c->pos] = c->saved;
	  c->saved = '\0';
	}
      c->pos = offset;
      if (c->writeonly && c->pos < c->eof)
	{
	  c->saved = c->buf[c->pos];
	  c->buf[c->pos] = '\0';
	}
    }
  return (_fpos_t) offset;
}

/* Seek to position POS relative to WHENCE within stream described by
   COOKIE; return resulting position or fail with EOF.  */
#ifdef __LARGE64_FILES
static _fpos64_t
_DEFUN(fmemseeker64, (ptr, cookie, pos, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos64_t pos _AND
       int whence)
{
  _off64_t offset = (_off64_t) pos;
  fmemcookie *c = (fmemcookie *) cookie;
  if (whence == SEEK_CUR)
    offset += c->pos;
  else if (whence == SEEK_END)
    offset += c->eof;
  if (offset < 0)
    {
      ptr->_errno = EINVAL;
      offset = -1;
    }
  else if (offset > c->max)
    {
      ptr->_errno = ENOSPC;
      offset = -1;
    }
  else
    {
      if (c->writeonly && c->pos < c->eof)
	{
	  c->buf[c->pos] = c->saved;
	  c->saved = '\0';
	}
      c->pos = offset;
      if (c->writeonly && c->pos < c->eof)
	{
	  c->saved = c->buf[c->pos];
	  c->buf[c->pos] = '\0';
	}
    }
  return (_fpos64_t) offset;
}
#endif /* __LARGE64_FILES */

/* Reclaim resources used by stream described by COOKIE.  */
static int
_DEFUN(fmemcloser, (ptr, cookie),
       struct _reent *ptr _AND
       void *cookie)
{
  fmemcookie *c = (fmemcookie *) cookie;
  _free_r (ptr, c->storage);
  return 0;
}

/* Open a memstream around buffer BUF of SIZE bytes, using MODE.
   Return the new stream, or fail with NULL.  */
FILE *
_DEFUN(_fmemopen_r, (ptr, buf, size, mode),
       struct _reent *ptr _AND
       void *buf _AND
       size_t size _AND
       const char *mode)
{
  FILE *fp;
  fmemcookie *c;
  int flags;
  int dummy;

  if ((flags = __sflags (ptr, mode, &dummy)) == 0)
    return NULL;
  if (!size || !(buf || flags & __SAPP))
    {
      ptr->_errno = EINVAL;
      return NULL;
    }
  if ((fp = __sfp (ptr)) == NULL)
    return NULL;
  if ((c = (fmemcookie *) _malloc_r (ptr, sizeof *c + (buf ? 0 : size)))
      == NULL)
    {
      __sfp_lock_acquire ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      return NULL;
    }

  c->storage = c;
  c->max = size;
  /* 9 modes to worry about.  */
  /* w/a, buf or no buf: Guarantee a NUL after any file writes.  */
  c->writeonly = (flags & __SWR) != 0;
  c->saved = '\0';
  if (!buf)
    {
      /* r+/w+/a+, and no buf: file starts empty.  */
      c->buf = (char *) (c + 1);
      *(char *) buf = '\0';
      c->pos = c->eof = 0;
      c->append = (flags & __SAPP) != 0;
    }
  else
    {
      c->buf = (char *) buf;
      switch (*mode)
	{
	case 'a':
	  /* a/a+ and buf: position and size at first NUL.  */
	  buf = memchr (c->buf, '\0', size);
	  c->eof = c->pos = buf ? (char *) buf - c->buf : size;
	  if (!buf && c->writeonly)
	    /* a: guarantee a NUL within size even if no writes.  */
	    c->buf[size - 1] = '\0';
	  c->append = 1;
	  break;
	case 'r':
	  /* r/r+ and buf: read at beginning, full size available.  */
	  c->pos = c->append = 0;
	  c->eof = size;
	  break;
	case 'w':
	  /* w/w+ and buf: write at beginning, truncate to empty.  */
	  c->pos = c->append = c->eof = 0;
	  *c->buf = '\0';
	  break;
	default:
	  abort ();
	}
    }

  _flockfile (fp);
  fp->_file = -1;
  fp->_flags = flags;
  fp->_cookie = c;
  fp->_read = flags & (__SRD | __SRW) ? fmemreader : NULL;
  fp->_write = flags & (__SWR | __SRW) ? fmemwriter : NULL;
  fp->_seek = fmemseeker;
#ifdef __LARGE64_FILES
  fp->_seek64 = fmemseeker64;
  fp->_flags |= __SL64;
#endif
  fp->_close = fmemcloser;
  _funlockfile (fp);
  return fp;
}

#ifndef _REENT_ONLY
FILE *
_DEFUN(fmemopen, (buf, size, mode),
       void *buf _AND
       size_t size _AND
       const char *mode)
{
  return _fmemopen_r (_REENT, buf, size, mode);
}
#endif /* !_REENT_ONLY */
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
 */

/*
FUNCTION
<<fopen>>---open a file

INDEX
	fopen
INDEX
	_fopen_r

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *fopen(const char *<[file]>, const char *<[mode]>);

	FILE *_fopen_r(struct _reent *<[reent]>,
                       const char *<[file]>, const char *<[mode]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	FILE *fopen(<[file]>, <[mode]>)
	char *<[file]>;
	char *<[mode]>;

	FILE *_fopen_r(<[reent]>, <[file]>, <[mode]>)
	struct _reent *<[reent]>;
	char *<[file]>;
	char *<[mode]>;

DESCRIPTION
<<fopen>> initializes the data structures needed to read or write a
file.  Specify the file's name as the string at <[file]>, and the kind
of access you need to the file with the string at <[mode]>.

The alternate function <<_fopen_r>> is a reentrant version.
The extra argument <[reent]> is a pointer to a reentrancy structure.

Three fundamental kinds of access are available: read, write, and append.
<<*<[mode]>>> must begin with one of the three characters `<<r>>',
`<<w>>', or `<<a>>', to select one of these:

o+
o r
Open the file for reading; the operation will fail if the file does
not exist, or if the host system does not permit you to read it.

o w
Open the file for writing @emph{from the beginning} of the file:
effectively, this always creates a new file.  If the file whose name you
specified already existed, its old contents are discarded.

o a
Open the file for appending data, that is writing from the end of
file.  When you open a file this way, all data always goes to the
current end of file; you cannot change this using <<fseek>>.
o-

Some host systems distinguish between ``binary'' and ``text'' files.
Such systems may perform data transformations on data written to, or
read from, files opened as ``text''.
If your system is one of these, then you can append a `<<b>>' to any
of the three modes above, to specify that you are opening the file as
a binary file (the default is to open the file as a text file).

`<<rb>>', then, means ``read binary''; `<<wb>>', ``write binary''; and
`<<ab>>', ``append binary''.

To make C programs more portable, the `<<b>>' is accepted on all
systems, whether or not it makes a difference.

Finally, you might need to both read and write from the same file.
You can also append a `<<+>>' to any of the three modes, to permit
this.  (If you want to append both `<<b>>' and `<<+>>', you can do it
in either order: for example, <<"rb+">> means the same thing as
<<"r+b">> when used as a mode string.)

Use <<"r+">> (or <<"rb+">>) to permit reading and writing anywhere in
an existing file, without discarding any data; <<"w+">> (or <<"wb+">>)
to create a new file (or begin by discarding all data from an old one)
that permits reading and writing anywhere in it; and <<"a+">> (or
<<"ab+">>) to permit reading anywhere in an existing file, but writing
only at the end.

RETURNS
<<fopen>> returns a file pointer which you can use for other file
operations, unless the file you requested could not be opened; in that
situation, the result is <<NULL>>.  If the reason for failure was an
invalid string at <[mode]>, <<errno>> is set to <<EINVAL>>.

PORTABILITY
<<fopen>> is required by ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<open>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <errno.h>
#include <sys/lock.h>
#ifdef __CYGWIN__
#include <fcntl.h>
#endif
#include "local.h"
#endif

FILE *
_DEFUN(_fopen_r, (ptr, file, mode),
       struct _reent *ptr _AND
       _CONST char *file _AND
       _CONST char *mode)
{
  register FILE *fp;
  register int f;
  int flags, oflags;

  if ((flags = __sflags (ptr, mode, &oflags)) == 0)
    return NULL;
  if ((fp = __sfp (ptr)) == NULL)
    return NULL;

  if ((f = _open_r (ptr, file, oflags, 0666)) < 0)
    {
      __sfp_lock_acquire ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      return NULL;
    }

  _flockfile (fp);

  fp->_file = f;
  fp->_flags = flags;
  fp->_cookie = (_PTR) fp;
  fp->_read = __sread;
  fp->_write = __swrite;
  fp->_seek = __sseek;
  fp->_close = __sclose;

  if (fp->_flags & __SAPP)
    _fseek_r (ptr, fp, 0, SEEK_END);

#ifdef __SCLE
  if (__stextmode (fp->_file))
    fp->_flags |= __SCLE;
#endif

  _funlockfile (fp);
  return fp;
}

#ifndef _REENT_ONLY

FILE *
_DEFUN(fopen, (file, mode),
       _CONST char *file _AND
       _CONST char *mode)
{
  return _fopen_r (_REENT, file, mode);
}

#endif
/* Copyright (C) 2007 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
<<fopencookie>>---open a stream with custom callbacks

INDEX
	fopencookie

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *fopencookie(const void *<[cookie]>, const char *<[mode]>,
			  cookie_io_functions_t <[functions]>);

DESCRIPTION
<<fopencookie>> creates a <<FILE>> stream where I/O is performed using
custom callbacks.  The callbacks are registered via the structure:

	typedef ssize_t (*cookie_read_function_t)(void *_cookie, char *_buf,
						  size_t _n);
	typedef ssize_t (*cookie_write_function_t)(void *_cookie,
						   const char *_buf, size_t _n);
	typedef int (*cookie_seek_function_t)(void *_cookie, off_t *_off,
					      int _whence);
	typedef int (*cookie_close_function_t)(void *_cookie);

.	typedef struct
.	{
.		cookie_read_function_t	*read;
.		cookie_write_function_t *write;
.		cookie_seek_function_t	*seek;
.		cookie_close_function_t *close;
.	} cookie_io_functions_t;

The stream is opened with <[mode]> treated as in <<fopen>>.  The
callbacks <[functions.read]> and <[functions.write]> may only be NULL
when <[mode]> does not require them.

<[functions.read]> should return -1 on failure, or else the number of
bytes read (0 on EOF).  It is similar to <<read>>, except that
<[cookie]> will be passed as the first argument.

<[functions.write]> should return -1 on failure, or else the number of
bytes written.  It is similar to <<write>>, except that <[cookie]>
will be passed as the first argument.

<[functions.seek]> should return -1 on failure, and 0 on success, with
*<[_off]> set to the current file position.  It is a cross between
<<lseek>> and <<fseek>>, with the <[_whence]> argument interpreted in
the same manner.  A NULL <[functions.seek]> makes the stream behave
similarly to a pipe in relation to stdio functions that require
positioning.

<[functions.close]> should return -1 on failure, or 0 on success.  It
is similar to <<close>>, except that <[cookie]> will be passed as the
first argument.  A NULL <[functions.close]> merely flushes all data
then lets <<fclose>> succeed.  A failed close will still invalidate
the stream.

Read and write I/O functions are allowed to change the underlying
buffer on fully buffered or line buffered streams by calling
<<setvbuf>>.  They are also not required to completely fill or empty
the buffer.  They are not, however, allowed to change streams from
unbuffered to buffered or to change the state of the line buffering
flag.  They must also be prepared to have read or write calls occur on
buffers other than the one most recently specified.

RETURNS
The return value is an open FILE pointer on success.  On error,
<<NULL>> is returned, and <<errno>> will be set to EINVAL if a
function pointer is missing or <[mode]> is invalid, ENOMEM if the
stream cannot be created, or EMFILE if too many streams are already
open.

PORTABILITY
This function is a newlib extension, copying the prototype from Linux.
It is not portable.  See also the <<funopen>> interface from BSD.

Supporting OS subroutines required: <<sbrk>>.
*/

#include <stdio.h>
#include <errno.h>
#include <sys/lock.h>
#include "local.h"

typedef struct fccookie {
  void *cookie;
  FILE *fp;
  cookie_read_function_t *readfn;
  cookie_write_function_t *writefn;
  cookie_seek_function_t *seekfn;
  cookie_close_function_t *closefn;
} fccookie;

static _READ_WRITE_RETURN_TYPE
_DEFUN(fcreader, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       char *buf _AND
       int n)
{
  int result;
  fccookie *c = (fccookie *) cookie;
  errno = 0;
  if ((result = c->readfn (c->cookie, buf, n)) < 0 && errno)
    ptr->_errno = errno;
  return result;
}

static _READ_WRITE_RETURN_TYPE
_DEFUN(fcwriter, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       const char *buf _AND
       int n)
{
  int result;
  fccookie *c = (fccookie *) cookie;
  if (c->fp->_flags & __SAPP && c->fp->_seek)
    {
#ifdef __LARGE64_FILES
      c->fp->_seek64 (ptr, cookie, 0, SEEK_END);
#else
      c->fp->_seek (ptr, cookie, 0, SEEK_END);
#endif
    }
  errno = 0;
  if ((result = c->writefn (c->cookie, buf, n)) < 0 && errno)
    ptr->_errno = errno;
  return result;
}

static _fpos_t
_DEFUN(fcseeker, (ptr, cookie, pos, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos_t pos _AND
       int whence)
{
  fccookie *c = (fccookie *) cookie;
#ifndef __LARGE64_FILES
  off_t offset = (off_t) pos;
#else /* __LARGE64_FILES */
  _off64_t offset = (_off64_t) pos;
#endif /* __LARGE64_FILES */

  errno = 0;
  if (c->seekfn (c->cookie, &offset, whence) < 0 && errno)
    ptr->_errno = errno;
#ifdef __LARGE64_FILES
  else if ((_fpos_t)offset != offset)
    {
      ptr->_errno = EOVERFLOW;
      offset = -1;
    }
#endif /* __LARGE64_FILES */
  return (_fpos_t) offset;
}

#ifdef __LARGE64_FILES
static _fpos64_t
_DEFUN(fcseeker64, (ptr, cookie, pos, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos64_t pos _AND
       int whence)
{
  _off64_t offset;
  fccookie *c = (fccookie *) cookie;
  errno = 0;
  if (c->seekfn (c->cookie, &offset, whence) < 0 && errno)
    ptr->_errno = errno;
  return (_fpos64_t) offset;
}
#endif /* __LARGE64_FILES */

static int
_DEFUN(fccloser, (ptr, cookie),
       struct _reent *ptr _AND
       void *cookie)
{
  int result = 0;
  fccookie *c = (fccookie *) cookie;
  if (c->closefn)
    {
      errno = 0;
      if ((result = c->closefn (c->cookie)) < 0 && errno)
	ptr->_errno = errno;
    }
  _free_r (ptr, c);
  return result;
}

FILE *
_DEFUN(_fopencookie_r, (ptr, cookie, mode, functions),
       struct _reent *ptr _AND
       void *cookie _AND
       const char *mode _AND
       cookie_io_functions_t functions)
{
  FILE *fp;
  fccookie *c;
  int flags;
  int dummy;

  if ((flags = __sflags (ptr, mode, &dummy)) == 0)
    return NULL;
  if (((flags & (__SRD | __SRW)) && !functions.read)
      || ((flags & (__SWR | __SRW)) && !functions.write))
    {
      ptr->_errno = EINVAL;
      return NULL;
    }
  if ((fp = __sfp (ptr)) == NULL)
    return NULL;
  if ((c = (fccookie *) _malloc_r (ptr, sizeof *c)) == NULL)
    {
      __sfp_lock_acquire ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      return NULL;
    }

  _flockfile (fp);
  fp->_file = -1;
  fp->_flags = flags;
  c->cookie = cookie;
  c->fp = fp;
  fp->_cookie = c;
  c->readfn = functions.read;
  fp->_read = fcreader;
  c->writefn = functions.write;
  fp->_write = fcwriter;
  c->seekfn = functions.seek;
  fp->_seek = functions.seek ? fcseeker : NULL;
#ifdef __LARGE64_FILES
  fp->_seek64 = functions.seek ? fcseeker64 : NULL;
  fp->_flags |= __SL64;
#endif
  c->closefn = functions.close;
  fp->_close = fccloser;
  _funlockfile (fp);
  return fp;
}

#ifndef _REENT_ONLY
FILE *
_DEFUN(fopencookie, (cookie, mode, functions),
       void *cookie _AND
       const char *mode _AND
       cookie_io_functions_t functions)
{
  return _fopencookie_r (_REENT, cookie, mode, functions);
}
#endif /* !_REENT_ONLY */
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
 */

/*
FUNCTION
<<fgetc>>---get a character from a file or stream

INDEX
	fgetc
INDEX
	_fgetc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fgetc(FILE *<[fp]>);

	#include <stdio.h>
	int _fgetc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fgetc(<[fp]>)
	FILE *<[fp]>;

	#include <stdio.h>
	int _fgetc_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

DESCRIPTION
Use <<fgetc>> to get the next single character from the file or stream
identified by <[fp]>.  As a side effect, <<fgetc>> advances the file's
current position indicator.

For a macro version of this function, see <<getc>>.

The function <<_fgetc_r>> is simply a reentrant version of
<<fgetc>> that is passed the additional reentrant structure
pointer argument: <[ptr]>.

RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<fgetc>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
ANSI C requires <<fgetc>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

int
_DEFUN(_fgetc_r, (ptr, fp),
       struct _reent * ptr _AND
       FILE * fp)
{
  int result;
  CHECK_INIT(ptr, fp);
  _flockfile (fp);
  result = __sgetc_r (ptr, fp);
  _funlockfile (fp);
  return result;
}

#ifndef _REENT_ONLY

int
_DEFUN(fgetc, (fp),
       FILE * fp)
{
#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  int result;
  CHECK_INIT(_REENT, fp);
  __sfp_lock_acquire ();
  _flockfile (fp);
  result = __sgetc_r (_REENT, fp);
  _funlockfile (fp);
  __sfp_lock_release ();
  return result;
#else
  return _fgetc_r (_REENT, fp);
#endif
}

#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<fgets>>---get character string from a file or stream

INDEX
	fgets
INDEX
	_fgets_r

ANSI_SYNOPSIS
        #include <stdio.h>
	char *fgets(char *<[buf]>, int <[n]>, FILE *<[fp]>);

        #include <stdio.h>
	char *_fgets_r(struct _reent *<[ptr]>, char *<[buf]>, int <[n]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	char *fgets(<[buf]>,<[n]>,<[fp]>)
        char *<[buf]>;
	int <[n]>;
	FILE *<[fp]>;

	#include <stdio.h>
	char *_fgets_r(<[ptr]>, <[buf]>,<[n]>,<[fp]>)
	struct _reent *<[ptr]>;
        char *<[buf]>;
	int <[n]>;
	FILE *<[fp]>;

DESCRIPTION
	Reads at most <[n-1]> characters from <[fp]> until a newline
	is found. The characters including to the newline are stored
	in <[buf]>. The buffer is terminated with a 0.

	The <<_fgets_r>> function is simply the reentrant version of
	<<fgets>> and is passed an additional reentrancy structure
	pointer: <[ptr]>.

RETURNS
	<<fgets>> returns the buffer passed to it, with the data
	filled in. If end of file occurs with some data already
	accumulated, the data is returned with no other indication. If
	no data are read, NULL is returned instead.

PORTABILITY
	<<fgets>> should replace all uses of <<gets>>. Note however
	that <<fgets>> returns all of the data, while <<gets>> removes
	the trailing newline (with no indication that it has done so.)

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include "local.h"
#endif

/*
 * Read at most n-1 characters from the given file.
 * Stop when a newline has been read, or the count runs out.
 * Return first argument, or NULL if no characters were read.
 */

char *
_DEFUN(_fgets_r, (ptr, buf, n, fp),
       struct _reent * ptr _AND
       char *buf _AND
       int n     _AND
       FILE * fp)
{
  size_t len;
  char *s;
  unsigned char *p, *t;

  if (n < 2)			/* sanity check */
    return 0;

  s = buf;

  CHECK_INIT(ptr, fp);

  __sfp_lock_acquire ();
  _flockfile (fp);
#ifdef __SCLE
  if (fp->_flags & __SCLE)
    {
      int c;
      /* Sorry, have to do it the slow way */
      while (--n > 0 && (c = __sgetc_r (ptr, fp)) != EOF)
	{
	  *s++ = c;
	  if (c == '\n')
	    break;
	}
      if (c == EOF && s == buf)
        {
          _funlockfile (fp);
	  __sfp_lock_release ();
          return NULL;
        }
      *s = 0;
      _funlockfile (fp);
      __sfp_lock_release ();
      return buf;
    }
#endif

  n--;				/* leave space for NUL */
  do
    {
      /*
       * If the buffer is empty, refill it.
       */
      if ((len = fp->_r) <= 0)
	{
	  if (__srefill_r (ptr, fp))
	    {
	      /* EOF: stop with partial or no line */
	      if (s == buf)
                {
                  _funlockfile (fp);
		  __sfp_lock_release ();
                  return 0;
                }
	      break;
	    }
	  len = fp->_r;
	}
      p = fp->_p;

      /*
       * Scan through at most n bytes of the current buffer,
       * looking for '\n'.  If found, copy up to and including
       * newline, and stop.  Otherwise, copy entire chunk
       * and loop.
       */
      if (len > n)
	len = n;
      t = (unsigned char *) memchr ((_PTR) p, '\n', len);
      if (t != 0)
	{
	  len = ++t - p;
	  fp->_r -= len;
	  fp->_p = t;
	  _CAST_VOID memcpy ((_PTR) s, (_PTR) p, len);
	  s[len] = 0;
          _funlockfile (fp);
	  __sfp_lock_release ();
	  return (buf);
	}
      fp->_r -= len;
      fp->_p += len;
      _CAST_VOID memcpy ((_PTR) s, (_PTR) p, len);
      s += len;
    }
  while ((n -= len) != 0);
  *s = 0;
  _funlockfile (fp);
  __sfp_lock_release ();
  return buf;
}

char *
_DEFUN(_fgets_r_unlocked, (ptr, buf, n, fp),
       struct _reent * ptr _AND
       char *buf _AND
       int n     _AND
       FILE * fp)
{
  size_t len;
  char *s;
  unsigned char *p, *t;

  if (n < 2)			/* sanity check */
    return 0;

  s = buf;

  CHECK_INIT(ptr, fp);

#ifdef __SCLE
  if (fp->_flags & __SCLE)
    {
      int c;
      /* Sorry, have to do it the slow way */
      while (--n > 0 && (c = __sgetc_r (ptr, fp)) != EOF)
	{
	  *s++ = c;
	  if (c == '\n')
	    break;
	}
      if (c == EOF && s == buf)
        {
          return NULL;
        }
      *s = 0;
      return buf;
    }
#endif

  n--;				/* leave space for NUL */
  do
    {
      /*
       * If the buffer is empty, refill it.
       */
      if ((len = fp->_r) <= 0)
	{
	  if (__srefill_r (ptr, fp))
	    {
	      /* EOF: stop with partial or no line */
	      if (s == buf)
                {
                  return 0;
                }
	      break;
	    }
	  len = fp->_r;
	}
      p = fp->_p;

      /*
       * Scan through at most n bytes of the current buffer,
       * looking for '\n'.  If found, copy up to and including
       * newline, and stop.  Otherwise, copy entire chunk
       * and loop.
       */
      if (len > n)
	len = n;
      t = (unsigned char *) memchr ((_PTR) p, '\n', len);
      if (t != 0)
	{
	  len = ++t - p;
	  fp->_r -= len;
	  fp->_p = t;
	  _CAST_VOID memcpy ((_PTR) s, (_PTR) p, len);
	  s[len] = 0;
	  return (buf);
	}
      fp->_r -= len;
      fp->_p += len;
      _CAST_VOID memcpy ((_PTR) s, (_PTR) p, len);
      s += len;
    }
  while ((n -= len) != 0);
  *s = 0;
  return buf;
}

#ifndef _REENT_ONLY

char *
_DEFUN(fgets, (buf, n, fp),
       char *buf _AND
       int n     _AND
       FILE * fp)
{
  return _fgets_r (_REENT, buf, n, fp);
}

char *
_DEFUN(fgets_unlocked, (buf, n, fp),
       char *buf _AND
       int n     _AND
       FILE * fp)
{
  return _fgets_r_unlocked (_REENT, buf, n, fp);
}

#endif /* !_REENT_ONLY */
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
 */

/*
FUNCTION
<<fileno>>---return file descriptor associated with stream

INDEX
	fileno

ANSI_SYNOPSIS
	#include <stdio.h>
	int fileno(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fileno(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
You can use <<fileno>> to return the file descriptor identified by <[fp]>.

RETURNS
<<fileno>> returns a non-negative integer when successful.
If <[fp]> is not an open stream, <<fileno>> returns -1.

PORTABILITY
<<fileno>> is not part of ANSI C.
POSIX requires <<fileno>>.

Supporting OS subroutines required: none.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

int
_DEFUN(fileno, (f),
       FILE * f)
{
  int result;
  CHECK_INIT (_REENT, f);
  _flockfile (f);
  result = __sfileno (f);
  _funlockfile (f);
  return result;
}

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
 */

/*
FUNCTION
<<fputc>>---write a character on a stream or file

INDEX
	fputc
INDEX
	_fputc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fputc(int <[ch]>, FILE *<[fp]>);

	#include <stdio.h>
	int _fputc_r(struct _rent *<[ptr]>, int <[ch]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fputc(<[ch]>, <[fp]>)
	int <[ch]>;
	FILE *<[fp]>;

	#include <stdio.h>
	int _fputc_r(<[ptr]>, <[ch]>, <[fp]>)
	struct _reent *<[ptr]>;
	int <[ch]>;
	FILE *<[fp]>;

DESCRIPTION
<<fputc>> converts the argument <[ch]> from an <<int>> to an
<<unsigned char>>, then writes it to the file or stream identified by
<[fp]>.

If the file was opened with append mode (or if the stream cannot
support positioning), then the new character goes at the end of the
file or stream.  Otherwise, the new character is written at the
current value of the position indicator, and the position indicator
oadvances by one.

For a macro version of this function, see <<putc>>.

The <<_fputc_r>> function is simply a reentrant version of <<fputc>>
that takes an additional reentrant structure argument: <[ptr]>.

RETURNS
If successful, <<fputc>> returns its argument <[ch]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(<[fp]>)>>' to
query for errors.

PORTABILITY
<<fputc>> is required by ANSI C.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

int
_DEFUN(_fputc_r, (ptr, ch, file),
       struct _reent *ptr _AND
       int ch _AND
       FILE * file)
{
  int result;
  CHECK_INIT(ptr, file);
   _flockfile (file);
  result = _putc_r (ptr, ch, file);
  _funlockfile (file);
  return result;
}

#ifndef _REENT_ONLY
int
_DEFUN(fputc, (ch, file),
       int ch _AND
       FILE * file)
{
#if !defined(__OPTIMIZE_SIZE__) && !defined(PREFER_SIZE_OVER_SPEED)
  int result;
  CHECK_INIT(_REENT, file);
   _flockfile (file);
  result = _putc_r (_REENT, ch, file);
  _funlockfile (file);
  return result;
#else
  return _fputc_r (_REENT, ch, file);
#endif
}

#endif /* !_REENT_ONLY */

/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
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
 */

/*
FUNCTION
<<fread>>---read array elements from a file

INDEX
	fread
INDEX
	_fread_r

ANSI_SYNOPSIS
	#include <stdio.h>
	size_t fread(void *<[buf]>, size_t <[size]>, size_t <[count]>,
		     FILE *<[fp]>);

	#include <stdio.h>
	size_t _fread_r(struct _reent *<[ptr]>, void *<[buf]>,
	                size_t <[size]>, size_t <[count]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	size_t fread(<[buf]>, <[size]>, <[count]>, <[fp]>)
	char *<[buf]>;
	size_t <[size]>;
	size_t <[count]>;
	FILE *<[fp]>;

	#include <stdio.h>
	size_t _fread_r(<[ptr]>, <[buf]>, <[size]>, <[count]>, <[fp]>)
	struct _reent *<[ptr]>;
	char *<[buf]>;
	size_t <[size]>;
	size_t <[count]>;
	FILE *<[fp]>;

DESCRIPTION
<<fread>> attempts to copy, from the file or stream identified by
<[fp]>, <[count]> elements (each of size <[size]>) into memory,
starting at <[buf]>.   <<fread>> may copy fewer elements than
<[count]> if an error, or end of file, intervenes.

<<fread>> also advances the file position indicator (if any) for
<[fp]> by the number of @emph{characters} actually read.

<<_fread_r>> is simply the reentrant version of <<fread>> that
takes an additional reentrant structure pointer argument: <[ptr]>.

RETURNS
The result of <<fread>> is the number of elements it succeeded in
reading.

PORTABILITY
ANSI C requires <<fread>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "local.h"
#endif

#ifdef __SCLE
static size_t
_DEFUN(crlf_r, (ptr, fp, buf, count, eof),
       struct _reent * ptr _AND
       FILE * fp _AND
       char * buf _AND
       size_t count _AND
       int eof)
{
  int r;
  char *s, *d, *e;

  if (count == 0)
    return 0;

  e = buf + count;
  for (s=d=buf; s<e-1; s++)
    {
      if (*s == '\r' && s[1] == '\n')
        s++;
      *d++ = *s;
    }
  if (s < e)
    {
      if (*s == '\r')
        {
          int c = __sgetc_raw_r (ptr, fp);
          if (c == '\n')
            *s = '\n';
          else
            ungetc (c, fp);
        }
      *d++ = *s++;
    }


  while (d < e)
    {
      r = _getc_r (ptr, fp);
      if (r == EOF)
        return count - (e-d);
      *d++ = r;
    }

  return count;

}

#endif

size_t
_DEFUN(_fread_r, (ptr, buf, size, count, fp),
       struct _reent * ptr _AND
       _PTR buf _AND
       size_t size _AND
       size_t count _AND
       FILE * fp)
{
  register size_t resid;
  register char *p;
  register int r;
  size_t total;

  if ((resid = count * size) == 0)
    return 0;

  CHECK_INIT(ptr, fp);

  __sfp_lock_acquire ();
  _flockfile (fp);
  ORIENT (fp, -1);
  if (fp->_r < 0)
    fp->_r = 0;
  total = resid;
  p = buf;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)

  /* Optimize unbuffered I/O.  */
  if (fp->_flags & __SNBF)
    {
      /* First copy any available characters from ungetc buffer.  */
      int copy_size = resid > fp->_r ? fp->_r : resid;
      _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, (size_t) copy_size);
      fp->_p += copy_size;
      fp->_r -= copy_size;
      p += copy_size;
      resid -= copy_size;

      /* If still more data needed, free any allocated ungetc buffer.  */
      if (HASUB (fp) && resid > 0)
	FREEUB (ptr, fp);

      /* Finally read directly into user's buffer if needed.  */
      while (resid > 0)
	{
	  int rc = 0;
	  /* save fp buffering state */
	  void *old_base = fp->_bf._base;
	  void * old_p = fp->_p;
	  int old_size = fp->_bf._size;
	  /* allow __refill to use user's buffer */
	  fp->_bf._base = (unsigned char *) p;
	  fp->_bf._size = resid;
	  fp->_p = (unsigned char *) p;
	  rc = __srefill_r (ptr, fp);
	  /* restore fp buffering back to original state */
	  fp->_bf._base = old_base;
	  fp->_bf._size = old_size;
	  fp->_p = old_p;
	  resid -= fp->_r;
	  p += fp->_r;
	  fp->_r = 0;
	  if (rc)
	    {
#ifdef __SCLE
              if (fp->_flags & __SCLE)
	        {
	          _funlockfile (fp);
		  __sfp_lock_release ();
	          return crlf_r (ptr, fp, buf, total-resid, 1) / size;
	        }
#endif
	      _funlockfile (fp);
	      __sfp_lock_release ();
	      return (total - resid) / size;
	    }
	}
    }
  else
#endif /* !PREFER_SIZE_OVER_SPEED && !__OPTIMIZE_SIZE__ */
    {
      while (resid > (r = fp->_r))
	{
	  _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, (size_t) r);
	  fp->_p += r;
	  /* fp->_r = 0 ... done in __srefill */
	  p += r;
	  resid -= r;
	  if (__srefill_r (ptr, fp))
	    {
	      /* no more input: return partial result */
#ifdef __SCLE
	      if (fp->_flags & __SCLE)
		{
		  _funlockfile (fp);
		  __sfp_lock_release ();
		  return crlf_r (ptr, fp, buf, total-resid, 1) / size;
		}
#endif
	      _funlockfile (fp);
	      __sfp_lock_release ();
	      return (total - resid) / size;
	    }
	}
      _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, resid);
      fp->_r -= resid;
      fp->_p += resid;
    }

  /* Perform any CR/LF clean-up if necessary.  */
#ifdef __SCLE
  if (fp->_flags & __SCLE)
    {
      _funlockfile (fp);
      __sfp_lock_release ();
      return crlf_r(ptr, fp, buf, total, 0) / size;
    }
#endif
  _funlockfile (fp);
  __sfp_lock_release ();
  return count;
}

size_t
_DEFUN(_fread_r_unlocked, (ptr, buf, size, count, fp),
       struct _reent * ptr _AND
       _PTR buf _AND
       size_t size _AND
       size_t count _AND
       FILE * fp)
{
  register size_t resid;
  register char *p;
  register int r;
  size_t total;

  if ((resid = count * size) == 0)
    return 0;

  CHECK_INIT(ptr, fp);

  ORIENT (fp, -1);
  if (fp->_r < 0)
    fp->_r = 0;
  total = resid;
  p = buf;

#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)

  /* Optimize unbuffered I/O.  */
  if (fp->_flags & __SNBF)
    {
      /* First copy any available characters from ungetc buffer.  */
      int copy_size = resid > fp->_r ? fp->_r : resid;
      _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, (size_t) copy_size);
      fp->_p += copy_size;
      fp->_r -= copy_size;
      p += copy_size;
      resid -= copy_size;

      /* If still more data needed, free any allocated ungetc buffer.  */
      if (HASUB (fp) && resid > 0)
	FREEUB (ptr, fp);

      /* Finally read directly into user's buffer if needed.  */
      while (resid > 0)
	{
	  int rc = 0;
	  /* save fp buffering state */
	  void *old_base = fp->_bf._base;
	  void * old_p = fp->_p;
	  int old_size = fp->_bf._size;
	  /* allow __refill to use user's buffer */
	  fp->_bf._base = (unsigned char *) p;
	  fp->_bf._size = resid;
	  fp->_p = (unsigned char *) p;
	  rc = __srefill_r (ptr, fp);
	  /* restore fp buffering back to original state */
	  fp->_bf._base = old_base;
	  fp->_bf._size = old_size;
	  fp->_p = old_p;
	  resid -= fp->_r;
	  p += fp->_r;
	  fp->_r = 0;
	  if (rc)
	    {
#ifdef __SCLE
              if (fp->_flags & __SCLE)
	        {
	          return crlf_r (ptr, fp, buf, total-resid, 1) / size;
	        }
#endif
	      return (total - resid) / size;
	    }
	}
    }
  else
#endif /* !PREFER_SIZE_OVER_SPEED && !__OPTIMIZE_SIZE__ */
    {
      while (resid > (r = fp->_r))
	{
	  _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, (size_t) r);
	  fp->_p += r;
	  /* fp->_r = 0 ... done in __srefill */
	  p += r;
	  resid -= r;
	  if (__srefill_r (ptr, fp))
	    {
	      /* no more input: return partial result */
#ifdef __SCLE
	      if (fp->_flags & __SCLE)
		{
		  return crlf_r (ptr, fp, buf, total-resid, 1) / size;
		}
#endif
	      return (total - resid) / size;
	    }
	}
      _CAST_VOID memcpy ((_PTR) p, (_PTR) fp->_p, resid);
      fp->_r -= resid;
      fp->_p += resid;
    }

  /* Perform any CR/LF clean-up if necessary.  */
#ifdef __SCLE
  if (fp->_flags & __SCLE)
    {
      return crlf_r(ptr, fp, buf, total, 0) / size;
    }
#endif
  return count;
}

#ifndef _REENT_ONLY
size_t
_DEFUN(fread, (buf, size, count, fp),
       _PTR buf _AND
       size_t size _AND
       size_t count _AND
       FILE * fp)
{
   return _fread_r (_REENT, buf, size, count, fp);
}

size_t
_DEFUN(fread_unlocked, (buf, size, count, fp),
       _PTR buf _AND
       size_t size _AND
       size_t count _AND
       FILE * fp)
{
   return _fread_r_unlocked (_REENT, buf, size, count, fp);
}
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
 */

/*
FUNCTION
<<fseek>>, <<fseeko>>---set file position

INDEX
	fseek
INDEX
	fseeko
INDEX
	_fseek_r
INDEX
	_fseeko_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fseek(FILE *<[fp]>, long <[offset]>, int <[whence]>)
	int fseeko(FILE *<[fp]>, off_t <[offset]>, int <[whence]>)
	int _fseek_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	             long <[offset]>, int <[whence]>)
	int _fseeko_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	             off_t <[offset]>, int <[whence]>)

TRAD_SYNOPSIS
	#include <stdio.h>
	int fseek(<[fp]>, <[offset]>, <[whence]>)
	FILE *<[fp]>;
	long <[offset]>;
	int <[whence]>;

	int fseeko(<[fp]>, <[offset]>, <[whence]>)
	FILE *<[fp]>;
	off_t <[offset]>;
	int <[whence]>;

	int _fseek_r(<[ptr]>, <[fp]>, <[offset]>, <[whence]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	long <[offset]>;
	int <[whence]>;

	int _fseeko_r(<[ptr]>, <[fp]>, <[offset]>, <[whence]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	off_t <[offset]>;
	int <[whence]>;

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fseek>>/<<fseeko>> to set the position for the file identified by
<[fp]>.  The value of <[offset]> determines the new position, in one
of three ways selected by the value of <[whence]> (defined as macros
in `<<stdio.h>>'):

<<SEEK_SET>>---<[offset]> is the absolute file position (an offset
from the beginning of the file) desired.  <[offset]> must be positive.

<<SEEK_CUR>>---<[offset]> is relative to the current file position.
<[offset]> can meaningfully be either positive or negative.

<<SEEK_END>>---<[offset]> is relative to the current end of file.
<[offset]> can meaningfully be either positive (to increase the size
of the file) or negative.

See <<ftell>>/<<ftello>> to determine the current file position.

RETURNS
<<fseek>>/<<fseeko>> return <<0>> when successful.  On failure, the
result is <<EOF>>.  The reason for failure is indicated in <<errno>>:
either <<ESPIPE>> (the stream identified by <[fp]> doesn't support
repositioning) or <<EINVAL>> (invalid file position).

PORTABILITY
ANSI C requires <<fseek>>.

<<fseeko>> is defined by the Single Unix specification.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include "local.h"
#endif

#define	POS_ERR	(-(_fpos_t)1)

/*
 * Seek the given file to the given offset.
 * `Whence' must be one of the three SEEK_* macros.
 */

int
_DEFUN(_fseek_r, (ptr, fp, offset, whence),
       struct _reent *ptr _AND
       register FILE *fp  _AND
       long offset        _AND
       int whence)
{
  _fpos_t _EXFUN((*seekfn), (struct _reent *, _PTR, _fpos_t, int));
  _fpos_t target;
  _fpos_t curoff = 0;
  size_t n;
#ifdef __USE_INTERNAL_STAT64
  struct stat64 st;
#else
  struct stat st;
#endif
  int havepos;

  /* Make sure stdio is set up.  */

  CHECK_INIT (ptr, fp);

  __sfp_lock_acquire ();
  _flockfile (fp);

  /* If we've been doing some writing, and we're in append mode
     then we don't really know where the filepos is.  */

  if (fp->_flags & __SAPP && fp->_flags & __SWR)
    {
      /* So flush the buffer and seek to the end.  */
      _fflush_r (ptr, fp);
    }

  /* Have to be able to seek.  */

  if ((seekfn = fp->_seek) == NULL)
    {
      ptr->_errno = ESPIPE;	/* ??? */
      _funlockfile (fp);
      __sfp_lock_release ();
      return EOF;
    }

  /*
   * Change any SEEK_CUR to SEEK_SET, and check `whence' argument.
   * After this, whence is either SEEK_SET or SEEK_END.
   */

  switch (whence)
    {
    case SEEK_CUR:
      /*
       * In order to seek relative to the current stream offset,
       * we have to first find the current stream offset a la
       * ftell (see ftell for details).
       */
      _fflush_r (ptr, fp);   /* may adjust seek offset on append stream */
      if (fp->_flags & __SOFF)
	curoff = fp->_offset;
      else
	{
	  curoff = seekfn (ptr, fp->_cookie, (_fpos_t) 0, SEEK_CUR);
	  if (curoff == -1L)
	    {
	      _funlockfile (fp);
	      __sfp_lock_release ();
	      return EOF;
	    }
	}
      if (fp->_flags & __SRD)
	{
	  curoff -= fp->_r;
	  if (HASUB (fp))
	    curoff -= fp->_ur;
	}
      else if (fp->_flags & __SWR && fp->_p != NULL)
	curoff += fp->_p - fp->_bf._base;

      offset += curoff;
      whence = SEEK_SET;
      havepos = 1;
      break;

    case SEEK_SET:
    case SEEK_END:
      havepos = 0;
      break;

    default:
      ptr->_errno = EINVAL;
      _funlockfile (fp);
      __sfp_lock_release ();
      return (EOF);
    }

  /*
   * Can only optimise if:
   *	reading (and not reading-and-writing);
   *	not unbuffered; and
   *	this is a `regular' Unix file (and hence seekfn==__sseek).
   * We must check __NBF first, because it is possible to have __NBF
   * and __SOPT both set.
   */

  if (fp->_bf._base == NULL)
    __smakebuf_r (ptr, fp);
  if (fp->_flags & (__SWR | __SRW | __SNBF | __SNPT))
    goto dumb;
  if ((fp->_flags & __SOPT) == 0)
    {
      if (seekfn != __sseek
	  || fp->_file < 0
#ifdef __USE_INTERNAL_STAT64
	  || _fstat64_r (ptr, fp->_file, &st)
#else
	  || _fstat_r (ptr, fp->_file, &st)
#endif
	  || (st.st_mode & S_IFMT) != S_IFREG)
	{
	  fp->_flags |= __SNPT;
	  goto dumb;
	}
#ifdef	HAVE_BLKSIZE
      fp->_blksize = st.st_blksize;
#else
      fp->_blksize = 1024;
#endif
      fp->_flags |= __SOPT;
    }

  /*
   * We are reading; we can try to optimise.
   * Figure out where we are going and where we are now.
   */

  if (whence == SEEK_SET)
    target = offset;
  else
    {
#ifdef __USE_INTERNAL_STAT64
      if (_fstat64_r (ptr, fp->_file, &st))
#else
      if (_fstat_r (ptr, fp->_file, &st))
#endif
	goto dumb;
      target = st.st_size + offset;
    }
  if ((long)target != target)
    {
      ptr->_errno = EOVERFLOW;
      _funlockfile (fp);
      __sfp_lock_release ();
      return EOF;
    }

  if (!havepos)
    {
      if (fp->_flags & __SOFF)
	curoff = fp->_offset;
      else
	{
	  curoff = seekfn (ptr, fp->_cookie, 0L, SEEK_CUR);
	  if (curoff == POS_ERR)
	    goto dumb;
	}
      curoff -= fp->_r;
      if (HASUB (fp))
	curoff -= fp->_ur;
    }

  /*
   * Compute the number of bytes in the input buffer (pretending
   * that any ungetc() input has been discarded).  Adjust current
   * offset backwards by this count so that it represents the
   * file offset for the first byte in the current input buffer.
   */

  if (HASUB (fp))
    {
      curoff += fp->_r;       /* kill off ungetc */
      n = fp->_up - fp->_bf._base;
      curoff -= n;
      n += fp->_ur;
    }
  else
    {
      n = fp->_p - fp->_bf._base;
      curoff -= n;
      n += fp->_r;
    }

  /*
   * If the target offset is within the current buffer,
   * simply adjust the pointers, clear EOF, undo ungetc(),
   * and return.
   */

  if (target >= curoff && target < curoff + n)
    {
      register int o = target - curoff;

      fp->_p = fp->_bf._base + o;
      fp->_r = n - o;
      if (HASUB (fp))
	FREEUB (ptr, fp);
      fp->_flags &= ~__SEOF;
      memset (&fp->_mbstate, 0, sizeof (_mbstate_t));
      _funlockfile (fp);
      __sfp_lock_release ();
      return 0;
    }

  /*
   * The place we want to get to is not within the current buffer,
   * but we can still be kind to the kernel copyout mechanism.
   * By aligning the file offset to a block boundary, we can let
   * the kernel use the VM hardware to map pages instead of
   * copying bytes laboriously.  Using a block boundary also
   * ensures that we only read one block, rather than two.
   */

  curoff = target & ~(fp->_blksize - 1);
  if (seekfn (ptr, fp->_cookie, curoff, SEEK_SET) == POS_ERR)
    goto dumb;
  fp->_r = 0;
  fp->_p = fp->_bf._base;
  if (HASUB (fp))
    FREEUB (ptr, fp);
  fp->_flags &= ~__SEOF;
  n = target - curoff;
  if (n)
    {
      if (__srefill_r (ptr, fp) || fp->_r < n)
	goto dumb;
      fp->_p += n;
      fp->_r -= n;
    }
  memset (&fp->_mbstate, 0, sizeof (_mbstate_t));
  _funlockfile (fp);
  __sfp_lock_release ();
  return 0;

  /*
   * We get here if we cannot optimise the seek ... just
   * do it.  Allow the seek function to change fp->_bf._base.
   */

dumb:
  if (_fflush_r (ptr, fp)
      || seekfn (ptr, fp->_cookie, offset, whence) == POS_ERR)
    {
      _funlockfile (fp);
      __sfp_lock_release ();
      return EOF;
    }
  /* success: clear EOF indicator and discard ungetc() data */
  if (HASUB (fp))
    FREEUB (ptr, fp);
  fp->_p = fp->_bf._base;
  fp->_r = 0;
  /* fp->_w = 0; *//* unnecessary (I think...) */
  fp->_flags &= ~__SEOF;
  /* Reset no-optimization flag after successful seek.  The
     no-optimization flag may be set in the case of a read
     stream that is flushed which by POSIX/SUSv3 standards,
     means that a corresponding seek must not optimize.  The
     optimization is then allowed if no subsequent flush
     is performed.  */
  fp->_flags &= ~__SNPT;
  memset (&fp->_mbstate, 0, sizeof (_mbstate_t));
  _funlockfile (fp);
  __sfp_lock_release ();
  return 0;
}

#ifndef _REENT_ONLY

int
_DEFUN(fseek, (fp, offset, whence),
       register FILE *fp _AND
       long offset       _AND
       int whence)
{
  return _fseek_r (_REENT, fp, offset, whence);
}

#endif /* !_REENT_ONLY */
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
 */

/*
FUNCTION
<<ftell>>, <<ftello>>---return position in a stream or file

INDEX
	ftell
INDEX
	ftello
INDEX
	_ftell_r
INDEX
	_ftello_r

ANSI_SYNOPSIS
	#include <stdio.h>
	long ftell(FILE *<[fp]>);
	off_t ftello(FILE *<[fp]>);
	long _ftell_r(struct _reent *<[ptr]>, FILE *<[fp]>);
	off_t _ftello_r(struct _reent *<[ptr]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	long ftell(<[fp]>)
	FILE *<[fp]>;

	off_t ftello(<[fp]>)
	FILE *<[fp]>;

	long _ftell_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

	off_t _ftello_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

The result of <<ftell>>/<<ftello>> is the current position for a file
identified by <[fp]>.  If you record this result, you can later
use it with <<fseek>>/<<fseeko>> to return the file to this
position.  The difference between <<ftell>> and <<ftello>> is that
<<ftell>> returns <<long>> and <<ftello>> returns <<off_t>>.

In the current implementation, <<ftell>>/<<ftello>> simply uses a character
count to represent the file position; this is the same number that
would be recorded by <<fgetpos>>.

RETURNS
<<ftell>>/<<ftello>> return the file position, if possible.  If they cannot do
this, they return <<-1L>>.  Failure occurs on streams that do not support
positioning; the global <<errno>> indicates this condition with the
value <<ESPIPE>>.

PORTABILITY
<<ftell>> is required by the ANSI C standard, but the meaning of its
result (when successful) is not specified beyond requiring that it be
acceptable as an argument to <<fseek>>.  In particular, other
conforming C implementations may return a different result from
<<ftell>> than what <<fgetpos>> records.

<<ftello>> is defined by the Single Unix specification.

No supporting OS subroutines are required.
*/

/*
 * ftell: return current offset.
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"
#endif

long
_DEFUN(_ftell_r, (ptr, fp),
       struct _reent *ptr _AND
       register FILE * fp)
{
  _fpos_t pos;

  /* Ensure stdio is set up.  */

  CHECK_INIT (ptr, fp);

  _flockfile (fp);

  if (fp->_seek == NULL)
    {
      ptr->_errno = ESPIPE;
      _funlockfile (fp);
      return -1L;
    }

  /* Find offset of underlying I/O object, then adjust for buffered
     bytes.  Flush a write stream, since the offset may be altered if
     the stream is appending.  Do not flush a read stream, since we
     must not lose the ungetc buffer.  */
  if (fp->_flags & __SWR)
    _fflush_r (ptr, fp);
  if (fp->_flags & __SOFF)
    pos = fp->_offset;
  else
    {
      pos = fp->_seek (ptr, fp->_cookie, (_fpos_t) 0, SEEK_CUR);
      if (pos == -1L)
        {
          _funlockfile (fp);
          return pos;
        }
    }
  if (fp->_flags & __SRD)
    {
      /*
       * Reading.  Any unread characters (including
       * those from ungetc) cause the position to be
       * smaller than that in the underlying object.
       */
      pos -= fp->_r;
      if (HASUB (fp))
	pos -= fp->_ur;
    }
  else if ((fp->_flags & __SWR) && fp->_p != NULL)
    {
      /*
       * Writing.  Any buffered characters cause the
       * position to be greater than that in the
       * underlying object.
       */
      pos += fp->_p - fp->_bf._base;
    }

  _funlockfile (fp);
  if ((long)pos != pos)
    {
      pos = -1;
      ptr->_errno = EOVERFLOW;
    }
  return pos;
}

#ifndef _REENT_ONLY

long
_DEFUN(ftell, (fp),
       register FILE * fp)
{
  return _ftell_r (_REENT, fp);
}

#endif /* !_REENT_ONLY */
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
 */

/*
FUNCTION
<<fwrite>>---write array elements

INDEX
	fwrite
INDEX
	_fwrite_r

ANSI_SYNOPSIS
	#include <stdio.h>
	size_t fwrite(const void *<[buf]>, size_t <[size]>,
		      size_t <[count]>, FILE *<[fp]>);

	#include <stdio.h>
	size_t _fwrite_r(struct _reent *<[ptr]>, const void *<[buf]>, size_t <[size]>,
		      size_t <[count]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	size_t fwrite(<[buf]>, <[size]>, <[count]>, <[fp]>)
	char *<[buf]>;
	size_t <[size]>;
	size_t <[count]>;
	FILE *<[fp]>;

	#include <stdio.h>
	size_t _fwrite_r(<[ptr]>, <[buf]>, <[size]>, <[count]>, <[fp]>)
	struct _reent *<[ptr]>;
	char *<[buf]>;
	size_t <[size]>;
	size_t <[count]>;
	FILE *<[fp]>;

DESCRIPTION
<<fwrite>> attempts to copy, starting from the memory location
<[buf]>, <[count]> elements (each of size <[size]>) into the file or
stream identified by <[fp]>.  <<fwrite>> may copy fewer elements than
<[count]> if an error intervenes.

<<fwrite>> also advances the file position indicator (if any) for
<[fp]> by the number of @emph{characters} actually written.

<<_fwrite_r>> is simply the reentrant version of <<fwrite>> that
takes an additional reentrant structure argument: <[ptr]>.

RETURNS
If <<fwrite>> succeeds in writing all the elements you specify, the
result is the same as the argument <[count]>.  In any event, the
result is the number of complete elements that <<fwrite>> copied to
the file.

PORTABILITY
ANSI C requires <<fwrite>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#if 0
#include <sys/stdc.h>
#endif
#include "local.h"
#if 1
#include "fvwrite.h"
#endif
#endif

/*
 * Write `count' objects (each size `size') from memory to the given file.
 * Return the number of whole objects written.
 */

size_t
_DEFUN(_fwrite_r, (ptr, buf, size, count, fp),
       struct _reent * ptr _AND
       _CONST _PTR buf _AND
       size_t size     _AND
       size_t count    _AND
       FILE * fp)
{
  size_t n;
  struct __suio uio;
  struct __siov iov;

  iov.iov_base = buf;
  uio.uio_resid = iov.iov_len = n = count * size;
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;

  /*
   * The usual case is success (__sfvwrite_r returns 0);
   * skip the divide if this happens, since divides are
   * generally slow and since this occurs whenever size==0.
   */

  CHECK_INIT(ptr, fp);

  _flockfile (fp);
  ORIENT (fp, -1);
  if (__sfvwrite_r (ptr, fp, &uio) == 0)
    {
      _funlockfile (fp);
      return count;
    }
  _funlockfile (fp);
  return (n - uio.uio_resid) / size;
}

size_t
_DEFUN(_fwrite_r_unlocked, (ptr, buf, size, count, fp),
       struct _reent * ptr _AND
       _CONST _PTR buf _AND
       size_t size     _AND
       size_t count    _AND
       FILE * fp)
{
  size_t n;
  struct __suio uio;
  struct __siov iov;

  iov.iov_base = buf;
  uio.uio_resid = iov.iov_len = n = count * size;
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;

  /*
   * The usual case is success (__sfvwrite_r returns 0);
   * skip the divide if this happens, since divides are
   * generally slow and since this occurs whenever size==0.
   */

  CHECK_INIT(ptr, fp);

  ORIENT (fp, -1);
  if (__sfvwrite_r (ptr, fp, &uio) == 0)
    {
      return count;
    }
  return (n - uio.uio_resid) / size;
}

#ifndef _REENT_ONLY
size_t
_DEFUN(fwrite, (buf, size, count, fp),
       _CONST _PTR buf _AND
       size_t size     _AND
       size_t count    _AND
       FILE * fp)
{
  return _fwrite_r (_REENT, buf, size, count, fp);
}

size_t
_DEFUN(fwrite_unlocked, (buf, size, count, fp),
       _CONST _PTR buf _AND
       size_t size     _AND
       size_t count    _AND
       FILE * fp)
{
  return _fwrite_r_unlocked (_REENT, buf, size, count, fp);
}
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
 */

/*
FUNCTION
<<setvbuf>>---specify file or stream buffering

INDEX
	setvbuf

ANSI_SYNOPSIS
	#include <stdio.h>
	int setvbuf(FILE *<[fp]>, char *<[buf]>,
	            int <[mode]>, size_t <[size]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int setvbuf(<[fp]>, <[buf]>, <[mode]>, <[size]>)
	FILE *<[fp]>;
	char *<[buf]>;
	int <[mode]>;
	size_t <[size]>;

DESCRIPTION
Use <<setvbuf>> to specify what kind of buffering you want for the
file or stream identified by <[fp]>, by using one of the following
values (from <<stdio.h>>) as the <[mode]> argument:

o+
o _IONBF
Do not use a buffer: send output directly to the host system for the
file or stream identified by <[fp]>.

o _IOFBF
Use full output buffering: output will be passed on to the host system
only when the buffer is full, or when an input operation intervenes.

o _IOLBF
Use line buffering: pass on output to the host system at every
newline, as well as when the buffer is full, or when an input
operation intervenes.
o-

Use the <[size]> argument to specify how large a buffer you wish.  You
can supply the buffer itself, if you wish, by passing a pointer to a
suitable area of memory as <[buf]>.  Otherwise, you may pass <<NULL>>
as the <[buf]> argument, and <<setvbuf>> will allocate the buffer.

WARNINGS
You may only use <<setvbuf>> before performing any file operation other
than opening the file.

If you supply a non-null <[buf]>, you must ensure that the associated
storage continues to be available until you close the stream
identified by <[fp]>.

RETURNS
A <<0>> result indicates success, <<EOF>> failure (invalid <[mode]> or
<[size]> can cause failure).

PORTABILITY
Both ANSI C and the System V Interface Definition (Issue 2) require
<<setvbuf>>. However, they differ on the meaning of a <<NULL>> buffer
pointer: the SVID issue 2 specification says that a <<NULL>> buffer
pointer requests unbuffered output.  For maximum portability, avoid
<<NULL>> buffer pointers.

Both specifications describe the result on failure only as a
nonzero value.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include "local.h"
#endif

/*
 * Set one of the three kinds of buffering, optionally including a buffer.
 */

int
_DEFUN(setvbuf, (fp, buf, mode, size),
       register FILE * fp _AND
       char *buf          _AND
       register int mode  _AND
       register size_t size)
{
  int ret = 0;

  CHECK_INIT (_REENT, fp);

  _flockfile (fp);

  /*
   * Verify arguments.  The `int' limit on `size' is due to this
   * particular implementation.
   */

  if ((mode != _IOFBF && mode != _IOLBF && mode != _IONBF) || (int)(_POINTER_INT) size < 0)
    {
      _funlockfile (fp);
      return (EOF);
    }

  /*
   * Write current buffer, if any; drop read count, if any.
   * Make sure putc() will not think fp is line buffered.
   * Free old buffer if it was from malloc().  Clear line and
   * non buffer flags, and clear malloc flag.
   */

  _fflush_r (_REENT, fp);
  fp->_r = 0;
  fp->_lbfsize = 0;
  if (fp->_flags & __SMBF)
    _free_r (_REENT, (_PTR) fp->_bf._base);
  fp->_flags &= ~(__SLBF | __SNBF | __SMBF);

  if (mode == _IONBF)
    goto nbf;

  /*
   * Allocate buffer if needed. */
  if (buf == NULL)
    {
      /* we need this here because malloc() may return a pointer
	 even if size == 0 */
      if (!size) size = BUFSIZ;
      if ((buf = malloc (size)) == NULL)
	{
	  ret = EOF;
	  /* Try another size... */
	  buf = malloc (BUFSIZ);
	  size = BUFSIZ;
	}
      if (buf == NULL)
        {
          /* Can't allocate it, let's try another approach */
nbf:
          fp->_flags |= __SNBF;
          fp->_w = 0;
          fp->_bf._base = fp->_p = fp->_nbuf;
          fp->_bf._size = 1;
          _funlockfile (fp);
          return (ret);
        }
      fp->_flags |= __SMBF;
    }
  /*
   * Now put back whichever flag is needed, and fix _lbfsize
   * if line buffered.  Ensure output flush on exit if the
   * stream will be buffered at all.
   * If buf is NULL then make _lbfsize 0 to force the buffer
   * to be flushed and hence malloced on first use
   */

  switch (mode)
    {
    case _IOLBF:
      fp->_flags |= __SLBF;
      fp->_lbfsize = buf ? -size : 0;
      /* FALLTHROUGH */

    case _IOFBF:
      /* no flag */
      _REENT->__cleanup = _cleanup_r;
      fp->_bf._base = fp->_p = (unsigned char *) buf;
      fp->_bf._size = size;
      break;
    }

  /*
   * Patch up write count if necessary.
   */

  if (fp->_flags & __SWR)
    fp->_w = fp->_flags & (__SLBF | __SNBF) ? 0 : size;

  _funlockfile (fp);
  return 0;
}
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
 */
/*
FUNCTION
<<ungetc>>---push data back into a stream

INDEX
	ungetc
INDEX
	_ungetc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int ungetc(int <[c]>, FILE *<[stream]>);

	int _ungetc_r(struct _reent *<[reent]>, int <[c]>, FILE *<[stream]>);

DESCRIPTION
<<ungetc>> is used to return bytes back to <[stream]> to be read again.
If <[c]> is EOF, the stream is unchanged.  Otherwise, the unsigned
char <[c]> is put back on the stream, and subsequent reads will see
the bytes pushed back in reverse order.  Pushed byes are lost if the
stream is repositioned, such as by <<fseek>>, <<fsetpos>>, or
<<rewind>>.

The underlying file is not changed, but it is possible to push back
something different than what was originally read.  Ungetting a
character will clear the end-of-stream marker, and decrement the file
position indicator.  Pushing back beyond the beginning of a file gives
unspecified behavior.

The alternate function <<_ungetc_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
The character pushed back, or <<EOF>> on error.

PORTABILITY
ANSI C requires <<ungetc>>, but only requires a pushback buffer of one
byte; although this implementation can handle multiple bytes, not all
can.  Pushing back a signed char is a common application bug.

Supporting OS subroutines required: <<sbrk>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "local.h"
#endif

/*
 * Expand the ungetc buffer `in place'.  That is, adjust fp->_p when
 * the buffer moves, so that it points the same distance from the end,
 * and move the bytes in the buffer around as necessary so that they
 * are all at the end (stack-style).
 */

/*static*/
int
_DEFUN(__submore, (rptr, fp),
       struct _reent *rptr _AND
       register FILE *fp)
{
  register int i;
  register unsigned char *p;

  if (fp->_ub._base == fp->_ubuf)
    {
      /*
       * Get a new buffer (rather than expanding the old one).
       */
      if ((p = (unsigned char *) _malloc_r (rptr, (size_t) BUFSIZ)) == NULL)
	return EOF;
      fp->_ub._base = p;
      fp->_ub._size = BUFSIZ;
      p += BUFSIZ - sizeof (fp->_ubuf);
      for (i = sizeof (fp->_ubuf); --i >= 0;)
	p[i] = fp->_ubuf[i];
      fp->_p = p;
      return 0;
    }
  i = fp->_ub._size;
  p = (unsigned char *) _realloc_r (rptr, (_PTR) (fp->_ub._base), i << 1);
  if (p == NULL)
    return EOF;
  _CAST_VOID memcpy ((_PTR) (p + i), (_PTR) p, (size_t) i);
  fp->_p = p + i;
  fp->_ub._base = p;
  fp->_ub._size = i << 1;
  return 0;
}

int
_DEFUN(_ungetc_r, (rptr, c, fp),
       struct _reent *rptr _AND
       int c               _AND
       register FILE *fp)
{
  if (c == EOF)
    return (EOF);

  /* Ensure stdio has been initialized.
     ??? Might be able to remove this as some other stdio routine should
     have already been called to get the char we are un-getting.  */

  CHECK_INIT (rptr, fp);

  _flockfile (fp);

  ORIENT (fp, -1);

  /* After ungetc, we won't be at eof anymore */
  fp->_flags &= ~__SEOF;

  if ((fp->_flags & __SRD) == 0)
    {
      /*
       * Not already reading: no good unless reading-and-writing.
       * Otherwise, flush any current write stuff.
       */
      if ((fp->_flags & __SRW) == 0)
        {
          _funlockfile (fp);
          return EOF;
        }
      if (fp->_flags & __SWR)
	{
	  if (_fflush_r (rptr, fp))
            {
              _funlockfile (fp);
              return EOF;
            }
	  fp->_flags &= ~__SWR;
	  fp->_w = 0;
	  fp->_lbfsize = 0;
	}
      fp->_flags |= __SRD;
    }
  c = (unsigned char) c;

  /*
   * If we are in the middle of ungetc'ing, just continue.
   * This may require expanding the current ungetc buffer.
   */

  if (HASUB (fp))
    {
      if (fp->_r >= fp->_ub._size && __submore (rptr, fp))
        {
          _funlockfile (fp);
          return EOF;
        }
      *--fp->_p = c;
      fp->_r++;
      _funlockfile (fp);
      return c;
    }

  /*
   * If we can handle this by simply backing up, do so,
   * but never replace the original character.
   * (This makes sscanf() work when scanning `const' data.)
   */

  if (fp->_bf._base != NULL && fp->_p > fp->_bf._base && fp->_p[-1] == c)
    {
      fp->_p--;
      fp->_r++;
      _funlockfile (fp);
      return c;
    }

  /*
   * Create an ungetc buffer.
   * Initially, we will use the `reserve' buffer.
   */

  fp->_ur = fp->_r;
  fp->_up = fp->_p;
  fp->_ub._base = fp->_ubuf;
  fp->_ub._size = sizeof (fp->_ubuf);
  fp->_ubuf[sizeof (fp->_ubuf) - 1] = c;
  fp->_p = &fp->_ubuf[sizeof (fp->_ubuf) - 1];
  fp->_r = 1;
  _funlockfile (fp);
  return c;
}

#ifndef _REENT_ONLY
int
_DEFUN(ungetc, (c, fp),
       int c               _AND
       register FILE *fp)
{
  return _ungetc_r (_REENT, c, fp);
}
#endif /* !_REENT_ONLY */

/* Copyright 2002, Red Hat Inc. - all rights reserved */
/*
FUNCTION
<<getline>>---read a line from a file

INDEX
        getline

ANSI_SYNOPSIS
        #include <stdio.h>
        ssize_t getline(char **<[bufptr]>, size_t *<[n]>, FILE *<[fp]>);

TRAD_SYNOPSIS
        #include <stdio.h>
        ssize_t getline(<[bufptr]>, <[n]>, <[fp]>)
        char **<[bufptr]>;
        size_t *<[n]>;
        FILE *<[fp]>;

DESCRIPTION
<<getline>> reads a file <[fp]> up to and possibly including the
newline character.  The line is read into a buffer pointed to
by <[bufptr]> and designated with size *<[n]>.  If the buffer is
not large enough, it will be dynamically grown by <<getdelim>>.
As the buffer is grown, the pointer to the size <[n]> will be
updated.

<<getline>> is equivalent to getdelim(bufptr, n, '\n', fp);

RETURNS
<<getline>> returns <<-1>> if no characters were successfully read,
otherwise, it returns the number of bytes successfully read.
at end of file, the result is nonzero.

PORTABILITY
<<getline>> is a glibc extension.

No supporting OS subroutines are directly required.
*/

#include <_ansi.h>
#include <stdio.h>

extern ssize_t _EXFUN(__getdelim, (char **, size_t *, int, FILE *));

ssize_t
_DEFUN(__getline, (lptr, n, fp),
       char **lptr _AND
       size_t *n   _AND
       FILE *fp)
{
  return __getdelim (lptr, n, '\n', fp);
}

/* Copyright 2002, Red Hat Inc. - all rights reserved */
/*
FUNCTION
<<getdelim>>---read a line up to a specified line delimiter

INDEX
	getdelim

ANSI_SYNOPSIS
	#include <stdio.h>
	int getdelim(char **<[bufptr]>, size_t *<[n]>,
                     int <[delim]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int getdelim(<[bufptr]>, <[n]>, <[delim]>, <[fp]>)
	char **<[bufptr]>;
	size_t *<[n]>;
	int <[delim]>;
	FILE *<[fp]>;

DESCRIPTION
<<getdelim>> reads a file <[fp]> up to and possibly including a specified
delimiter <[delim]>.  The line is read into a buffer pointed to
by <[bufptr]> and designated with size *<[n]>.  If the buffer is
not large enough, it will be dynamically grown by <<getdelim>>.
As the buffer is grown, the pointer to the size <[n]> will be
updated.

RETURNS
<<getdelim>> returns <<-1>> if no characters were successfully read;
otherwise, it returns the number of bytes successfully read.
At end of file, the result is nonzero.

PORTABILITY
<<getdelim>> is a glibc extension.

No supporting OS subroutines are directly required.
*/

#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "local.h"

#define MIN_LINE_SIZE 4
#define DEFAULT_LINE_SIZE 128

ssize_t
_DEFUN(__getdelim, (bufptr, n, delim, fp),
       char **bufptr _AND
       size_t *n     _AND
       int delim     _AND
       FILE *fp)
{
  char *buf;
  char *ptr;
  size_t newsize, numbytes;
  int pos;
  int ch;
  int cont;

  if (fp == NULL || bufptr == NULL || n == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  buf = *bufptr;
  if (buf == NULL || *n < MIN_LINE_SIZE)
    {
      buf = (char *)realloc (*bufptr, DEFAULT_LINE_SIZE);
      if (buf == NULL)
        {
	  return -1;
        }
      *bufptr = buf;
      *n = DEFAULT_LINE_SIZE;
    }

  CHECK_INIT (_REENT, fp);

  __sfp_lock_acquire ();
  _flockfile (fp);

  numbytes = *n;
  ptr = buf;

  cont = 1;

  while (cont)
    {
      /* fill buffer - leaving room for nul-terminator */
      while (--numbytes > 0)
        {
          if ((ch = getc_unlocked (fp)) == EOF)
            {
	      cont = 0;
              break;
            }
	  else
            {
              *ptr++ = ch;
              if (ch == delim)
                {
                  cont = 0;
                  break;
                }
            }
        }

      if (cont)
        {
          /* Buffer is too small so reallocate a larger buffer.  */
          pos = ptr - buf;
          newsize = (*n << 1);
          buf = realloc (buf, newsize);
          if (buf == NULL)
            {
              cont = 0;
              break;
            }

          /* After reallocating, continue in new buffer */
          *bufptr = buf;
          *n = newsize;
          ptr = buf + pos;
          numbytes = newsize - pos;
        }
    }

  _funlockfile (fp);
  __sfp_lock_release ();

  /* if no input data, return failure */
  if (ptr == buf)
    return -1;

  /* otherwise, nul-terminate and return number of bytes read */
  *ptr = '\0';
  return (ssize_t)(ptr - buf);
}

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
 */

/*
FUNCTION
<<clearerr>>---clear file or stream error indicator

INDEX
	clearerr

ANSI_SYNOPSIS
	#include <stdio.h>
	void clearerr(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void clearerr(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
The <<stdio>> functions maintain an error indicator with each file
pointer <[fp]>, to record whether any read or write errors have
occurred on the associated file or stream.  Similarly, it maintains an
end-of-file indicator to record whether there is no more data in the
file.

Use <<clearerr>> to reset both of these indicators.

See <<ferror>> and <<feof>> to query the two indicators.


RETURNS
<<clearerr>> does not return a result.

PORTABILITY
ANSI C requires <<clearerr>>.

No supporting OS subroutines are required.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

/* A subroutine version of the macro clearerr.  */

#undef	clearerr

_VOID
_DEFUN(clearerr, (fp),
       FILE * fp)
{
  CHECK_INIT(_REENT, fp);
  _flockfile (fp);
  __sclearerr (fp);
  _funlockfile (fp);
}

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
 */

/*
FUNCTION
<<fdopen>>---turn open file into a stream

INDEX
	fdopen
INDEX
	_fdopen_r

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *fdopen(int <[fd]>, const char *<[mode]>);
	FILE *_fdopen_r(struct _reent *<[reent]>,
                        int <[fd]>, const char *<[mode]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	FILE *fdopen(<[fd]>, <[mode]>)
	int <[fd]>;
	char *<[mode]>;

	FILE *_fdopen_r(<[reent]>, <[fd]>, <[mode]>)
	struct _reent *<[reent]>;
        int <[fd]>;
	char *<[mode]>);

DESCRIPTION
<<fdopen>> produces a file descriptor of type <<FILE *>>, from a
descriptor for an already-open file (returned, for example, by the
system subroutine <<open>> rather than by <<fopen>>).
The <[mode]> argument has the same meanings as in <<fopen>>.

RETURNS
File pointer or <<NULL>>, as for <<fopen>>.

PORTABILITY
<<fdopen>> is ANSI.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdio.h>
#include <errno.h>
#include "local.h"
#include <_syslist.h>
#endif

FILE *
_DEFUN(_fdopen_r, (ptr, fd, mode),
       struct _reent *ptr _AND
       int fd             _AND
       _CONST char *mode)
{
  register FILE *fp;
  int flags, oflags;
#ifdef HAVE_FCNTL
  int fdflags, fdmode;
#endif

  if ((flags = __sflags (ptr, mode, &oflags)) == 0)
    return 0;

  /* make sure the mode the user wants is a subset of the actual mode */
#ifdef HAVE_FCNTL
  if ((fdflags = _fcntl_r (ptr, fd, F_GETFL, 0)) < 0)
    return 0;
  fdmode = fdflags & O_ACCMODE;
  if (fdmode != O_RDWR && (fdmode != (oflags & O_ACCMODE)))
    {
      ptr->_errno = EBADF;
      return 0;
    }
#endif

  if ((fp = __sfp (ptr)) == 0)
    return 0;

  _flockfile (fp);

  fp->_flags = flags;
  /* POSIX recommends setting the O_APPEND bit on fd to match append
     streams.  Someone may later clear O_APPEND on fileno(fp), but the
     stream must still remain in append mode.  Rely on __sflags
     setting __SAPP properly.  */
#ifdef HAVE_FCNTL
  if ((oflags & O_APPEND) && !(fdflags & O_APPEND))
    _fcntl_r (ptr, fd, F_SETFL, fdflags | O_APPEND);
#endif
  fp->_file = fd;
  fp->_cookie = (_PTR) fp;

#undef _read
#undef _write
#undef _seek
#undef _close

  fp->_read = __sread;
  fp->_write = __swrite;
  fp->_seek = __sseek;
  fp->_close = __sclose;

#ifdef __SCLE
  /* Explicit given mode results in explicit setting mode on fd */
  if (oflags & O_BINARY)
    setmode (fp->_file, O_BINARY);
  else if (oflags & O_TEXT)
    setmode (fp->_file, O_TEXT);
  if (__stextmode (fp->_file))
    fp->_flags |= __SCLE;
#endif

  _funlockfile (fp);
  return fp;
}

#ifndef _REENT_ONLY

FILE *
_DEFUN(fdopen, (fd, mode),
       int fd _AND
       _CONST char *mode)
{
  return _fdopen_r (_REENT, fd, mode);
}

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
 */

/*
FUNCTION
<<ferror>>---test whether read/write error has occurred

INDEX
	ferror

ANSI_SYNOPSIS
	#include <stdio.h>
	int ferror(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int ferror(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
The <<stdio>> functions maintain an error indicator with each file
pointer <[fp]>, to record whether any read or write errors have
occurred on the associated file or stream.
Use <<ferror>> to query this indicator.

See <<clearerr>> to reset the error indicator.

RETURNS
<<ferror>> returns <<0>> if no errors have occurred; it returns a
nonzero value otherwise.

PORTABILITY
ANSI C requires <<ferror>>.

No supporting OS subroutines are required.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

/* A subroutine version of the macro ferror.  */

#undef ferror

int
_DEFUN(ferror, (fp),
       FILE * fp)
{
  int result;
  CHECK_INIT(_REENT, fp);
  _flockfile (fp);
  result = __sferror (fp);
  _funlockfile (fp);
  return result;
}

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
 */

/*
FUNCTION
<<fgetpos>>---record position in a stream or file

INDEX
	fgetpos
INDEX
	_fgetpos_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fgetpos(FILE *<[fp]>, fpos_t *<[pos]>);
	int _fgetpos_r(struct _reent *<[ptr]>, FILE *<[fp]>, fpos_t *<[pos]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fgetpos(<[fp]>, <[pos]>)
	FILE *<[fp]>;
	fpos_t *<[pos]>;

	int _fgetpos_r(<[ptr]>, <[fp]>, <[pos]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	fpos_t *<[pos]>;

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fgetpos>> to report on the current position for a file
identified by <[fp]>; <<fgetpos>> will write a value
representing that position at <<*<[pos]>>>.  Later, you can
use this value with <<fsetpos>> to return the file to this
position.

In the current implementation, <<fgetpos>> simply uses a character
count to represent the file position; this is the same number that
would be returned by <<ftell>>.

RETURNS
<<fgetpos>> returns <<0>> when successful.  If <<fgetpos>> fails, the
result is <<1>>.  Failure occurs on streams that do not support
positioning; the global <<errno>> indicates this condition with the
value <<ESPIPE>>.

PORTABILITY
<<fgetpos>> is required by the ANSI C standard, but the meaning of the
value it records is not specified beyond requiring that it be
acceptable as an argument to <<fsetpos>>.  In particular, other
conforming C implementations may return a different result from
<<ftell>> than what <<fgetpos>> writes at <<*<[pos]>>>.

No supporting OS subroutines are required.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#endif

int
_DEFUN(_fgetpos_r, (ptr, fp, pos),
       struct _reent * ptr _AND
       FILE * fp           _AND
       _fpos_t * pos)
{
  *pos = _ftell_r (ptr, fp);

  if (*pos != -1)
    {
      return 0;
    }
  return 1;
}

#ifndef _REENT_ONLY

int
_DEFUN(fgetpos, (fp, pos),
       FILE * fp _AND
       _fpos_t * pos)
{
  return _fgetpos_r (_REENT, fp, pos);
}

#endif /* !_REENT_ONLY */

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
 */
/* doc in sprintf.c */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdarg.h>
#endif

int
_DEFUN(_fprintf_r, (ptr, fp, fmt),
       struct _reent *ptr _AND
       FILE *fp _AND
       const char *fmt _DOTS)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = _vfprintf_r (ptr, fp, fmt, ap);
  va_end (ap);
  return ret;
}

#ifndef _REENT_ONLY

int
_DEFUN(fprintf, (fp, fmt),
       FILE *fp _AND
       const char *fmt _DOTS)
{
  int ret;
  va_list ap;

  va_start (ap, fmt);
  ret = _vfprintf_r (_REENT, fp, fmt, ap);
  va_end (ap);
  return ret;
}

#endif /* ! _REENT_ONLY */

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
 */

/*
FUNCTION
<<fputs>>---write a character string in a file or stream

INDEX
	fputs
INDEX
	_fputs_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fputs(const char *<[s]>, FILE *<[fp]>);

	#include <stdio.h>
	int _fputs_r(struct _reent *<[ptr]>, const char *<[s]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fputs(<[s]>, <[fp]>)
	char *<[s]>;
	FILE *<[fp]>;

	#include <stdio.h>
	int _fputs_r(<[ptr]>, <[s]>, <[fp]>)
	struct _reent *<[ptr]>;
	char *<[s]>;
	FILE *<[fp]>;

DESCRIPTION
<<fputs>> writes the string at <[s]> (but without the trailing null)
to the file or stream identified by <[fp]>.

<<_fputs_r>> is simply the reentrant version of <<fputs>> that takes
an additional reentrant struct pointer argument: <[ptr]>.

RETURNS
If successful, the result is <<0>>; otherwise, the result is <<EOF>>.

PORTABILITY
ANSI C requires <<fputs>>, but does not specify that the result on
success must be <<0>>; any non-negative value is permitted.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <string.h>
#include "fvwrite.h"
#include "local.h"
#endif

/*
 * Write the given string to the given file.
 */

int
_DEFUN(_fputs_r, (ptr, s, fp),
       struct _reent * ptr _AND
       char _CONST * s _AND
       FILE * fp)
{
  int result;
  struct __suio uio;
  struct __siov iov;

  iov.iov_base = s;
  iov.iov_len = uio.uio_resid = strlen (s);
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;

  CHECK_INIT(ptr, fp);

  _flockfile (fp);
  ORIENT (fp, -1);
  result = __sfvwrite_r (ptr, fp, &uio);
  _funlockfile (fp);
  return result;
}

int
_DEFUN(_fputs_r_unlocked, (ptr, s, fp),
       struct _reent * ptr _AND
       char _CONST * s _AND
       FILE * fp)
{
  int result;
  struct __suio uio;
  struct __siov iov;

  iov.iov_base = s;
  iov.iov_len = uio.uio_resid = strlen (s);
  uio.uio_iov = &iov;
  uio.uio_iovcnt = 1;

  ORIENT (fp, -1);
  return __sfvwrite_r (ptr, fp, &uio);
}

#ifndef _REENT_ONLY
int
_DEFUN(fputs, (s, fp),
       char _CONST * s _AND
       FILE * fp)
{
  return _fputs_r (_REENT, s, fp);
}

int
_DEFUN(fputs_unlocked, (s, fp),
       char _CONST * s _AND
       FILE * fp)
{
  return _fputs_r_unlocked (_REENT, s, fp);
}
#endif /* !_REENT_ONLY */

/*
 * Copyright (c) 1990, 2007 The Regents of the University of California.
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
 */

/*
FUNCTION
<<freopen>>---open a file using an existing file descriptor

INDEX
	freopen
INDEX
	_freopen_r

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *freopen(const char *<[file]>, const char *<[mode]>,
		      FILE *<[fp]>);
	FILE *_freopen_r(struct _reent *<[ptr]>, const char *<[file]>,
		      const char *<[mode]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	FILE *freopen(<[file]>, <[mode]>, <[fp]>)
	char *<[file]>;
	char *<[mode]>;
	FILE *<[fp]>;

	FILE *_freopen_r(<[ptr]>, <[file]>, <[mode]>, <[fp]>)
	struct _reent *<[ptr]>;
	char *<[file]>;
	char *<[mode]>;
	FILE *<[fp]>;

DESCRIPTION
Use this variant of <<fopen>> if you wish to specify a particular file
descriptor <[fp]> (notably <<stdin>>, <<stdout>>, or <<stderr>>) for
the file.

If <[fp]> was associated with another file or stream, <<freopen>>
closes that other file or stream (but ignores any errors while closing
it).

<[file]> and <[mode]> are used just as in <<fopen>>.

If <[file]> is <<NULL>>, the underlying stream is modified rather than
closed.  The file cannot be given a more permissive access mode (for
example, a <[mode]> of "w" will fail on a read-only file descriptor),
but can change status such as append or binary mode.  If modification
is not possible, failure occurs.

RETURNS
If successful, the result is the same as the argument <[fp]>.  If the
file cannot be opened as specified, the result is <<NULL>>.

PORTABILITY
ANSI C requires <<freopen>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<open>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/lock.h>
#include "local.h"
#endif

/*
 * Re-direct an existing, open (probably) file to some other file.
 */

FILE *
_DEFUN(_freopen_r, (ptr, file, mode, fp),
       struct _reent *ptr _AND
       const char *file _AND
       const char *mode _AND
       register FILE *fp)
{
  register int f;
  int flags, oflags;
  int e = 0;

  __sfp_lock_acquire ();

  CHECK_INIT (ptr, fp);

  _flockfile (fp);

  if ((flags = __sflags (ptr, mode, &oflags)) == 0)
    {
      _funlockfile (fp);
      _fclose_r (ptr, fp);
      __sfp_lock_release ();
      return NULL;
    }

  /*
   * Remember whether the stream was open to begin with, and
   * which file descriptor (if any) was associated with it.
   * If it was attached to a descriptor, defer closing it,
   * so that, e.g., freopen("/dev/stdin", "r", stdin) works.
   * This is unnecessary if it was not a Unix file.
   */

  if (fp->_flags == 0)
    fp->_flags = __SEOF;	/* hold on to it */
  else
    {
      if (fp->_flags & __SWR)
	_fflush_r (ptr, fp);
      /*
       * If close is NULL, closing is a no-op, hence pointless.
       * If file is NULL, the file should not be closed.
       */
      if (fp->_close != NULL && file != NULL)
	fp->_close (ptr, fp->_cookie);
    }

  /*
   * Now get a new descriptor to refer to the new file, or reuse the
   * existing file descriptor if file is NULL.
   */

  if (file != NULL)
    {
      f = _open_r (ptr, (char *) file, oflags, 0666);
      e = ptr->_errno;
    }
  else
    {
#ifdef HAVE_FCNTL
      int oldflags;
      /*
       * Reuse the file descriptor, but only if the new access mode is
       * equal or less permissive than the old.  F_SETFL correctly
       * ignores creation flags.
       */
      f = fp->_file;
      if ((oldflags = _fcntl_r (ptr, f, F_GETFL, 0)) == -1
	  || ! ((oldflags & O_ACCMODE) == O_RDWR
		|| ((oldflags ^ oflags) & O_ACCMODE) == 0)
	  || _fcntl_r (ptr, f, F_SETFL, oflags) == -1)
	f = -1;
#else
      /* We cannot modify without fcntl support.  */
      f = -1;
#endif

#ifdef __SCLE
      /*
       * F_SETFL doesn't change textmode.  Don't mess with modes of ttys.
       */
      if (0 <= f && ! _isatty_r (ptr, f)
	  && setmode (f, oflags & (O_BINARY | O_TEXT)) == -1)
	f = -1;
#endif

      if (f < 0)
	{
	  e = EBADF;
	  if (fp->_close != NULL)
	    fp->_close (ptr, fp->_cookie);
	}
    }

  /*
   * Finish closing fp.  Even if the open succeeded above,
   * we cannot keep fp->_base: it may be the wrong size.
   * This loses the effect of any setbuffer calls,
   * but stdio has always done this before.
   */

  if (fp->_flags & __SMBF)
    _free_r (ptr, (char *) fp->_bf._base);
  fp->_w = 0;
  fp->_r = 0;
  fp->_p = NULL;
  fp->_bf._base = NULL;
  fp->_bf._size = 0;
  fp->_lbfsize = 0;
  if (HASUB (fp))
    FREEUB (ptr, fp);
  fp->_ub._size = 0;
  if (HASLB (fp))
    FREELB (ptr, fp);
  fp->_lb._size = 0;
  fp->_flags & ~__SORD;
  fp->_flags2 = 0;
  memset (&fp->_mbstate, 0, sizeof (_mbstate_t));

  if (f < 0)
    {				/* did not get it after all */
      fp->_flags = 0;		/* set it free */
      ptr->_errno = e;		/* restore in case _close clobbered */
      _funlockfile (fp);
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      return NULL;
    }

  fp->_flags = flags;
  fp->_file = f;
  fp->_cookie = (_PTR) fp;
  fp->_read = __sread;
  fp->_write = __swrite;
  fp->_seek = __sseek;
  fp->_close = __sclose;

#ifdef __SCLE
  if (__stextmode (fp->_file))
    fp->_flags |= __SCLE;
#endif

  _funlockfile (fp);
  __sfp_lock_release ();
  return fp;
}

#ifndef _REENT_ONLY

FILE *
_DEFUN(freopen, (file, mode, fp),
       _CONST char *file _AND
       _CONST char *mode _AND
       register FILE *fp)
{
  return _freopen_r (_REENT, file, mode, fp);
}

#endif /*!_REENT_ONLY */

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
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "local.h"
#endif

#ifndef _REENT_ONLY

int
#ifdef _HAVE_STDC
fscanf(FILE *fp, _CONST char *fmt, ...)
#else
fscanf(FILE *fp, fmt, va_alist)
       FILE *fp;
       char *fmt;
       va_dcl
#endif
{
  int ret;
  va_list ap;

#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = __svfscanf_r (_REENT, fp, fmt, ap);
  va_end (ap);
  return ret;
}

#endif /* !_REENT_ONLY */

int
#ifdef _HAVE_STDC
_fscanf_r(struct _reent *ptr, FILE *fp, _CONST char *fmt, ...)
#else
_fscanf_r(ptr, FILE *fp, fmt, va_alist)
          struct _reent *ptr;
          FILE *fp;
          char *fmt;
          va_dcl
#endif
{
  int ret;
  va_list ap;

#ifdef _HAVE_STDC
  va_start (ap, fmt);
#else
  va_start (ap);
#endif
  ret = __svfscanf_r (ptr, fp, fmt, ap);
  va_end (ap);
  return (ret);
}

/*
 * Copyright (c) 2002, Red Hat Inc.
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
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#endif

int
_DEFUN(_fseeko_r, (ptr, fp, offset, whence),
       struct _reent *ptr _AND
       register FILE *fp  _AND
       _off_t offset      _AND
       int whence)
{
  return _fseek_r (ptr, fp, (long)offset, whence);
}

#ifndef _REENT_ONLY

int
_DEFUN(fseeko, (fp, offset, whence),
       register FILE *fp _AND
       _off_t offset     _AND
       int whence)
{
  /* for now we simply cast since off_t should be long */
  return _fseek_r (_REENT, fp, (long)offset, whence);
}

#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<fsetpos>>---restore position of a stream or file

INDEX
	fsetpos
INDEX
	_fsetpos_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int fsetpos(FILE *<[fp]>, const fpos_t *<[pos]>);
	int _fsetpos_r(struct _reent *<[ptr]>, FILE *<[fp]>,
	               const fpos_t *<[pos]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int fsetpos(<[fp]>, <[pos]>)
	FILE *<[fp]>;
	fpos_t *<[pos]>;

	int _fsetpos_r(<[ptr]>, <[fp]>, <[pos]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;
	fpos_t *<[pos]>;

DESCRIPTION
Objects of type <<FILE>> can have a ``position'' that records how much
of the file your program has already read.  Many of the <<stdio>> functions
depend on this position, and many change it as a side effect.

You can use <<fsetpos>> to return the file identified by <[fp]> to a previous
position <<*<[pos]>>> (after first recording it with <<fgetpos>>).

See <<fseek>> for a similar facility.

RETURNS
<<fgetpos>> returns <<0>> when successful.  If <<fgetpos>> fails, the
result is <<1>>.  The reason for failure is indicated in <<errno>>:
either <<ESPIPE>> (the stream identified by <[fp]> doesn't support
repositioning) or <<EINVAL>> (invalid file position).

PORTABILITY
ANSI C requires <<fsetpos>>, but does not specify the nature of
<<*<[pos]>>> beyond identifying it as written by <<fgetpos>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#endif

int
_DEFUN(_fsetpos_r, (ptr, iop, pos),
       struct _reent * ptr _AND
       FILE * iop          _AND
       _CONST _fpos_t * pos)
{
  int x = _fseek_r (ptr, iop, *pos, SEEK_SET);

  if (x != 0)
    return 1;
  return 0;
}

#ifndef _REENT_ONLY

int
_DEFUN(fsetpos, (iop, pos),
       FILE * iop _AND
       _CONST _fpos_t * pos)
{
  return _fsetpos_r (_REENT, iop, pos);
}

#endif /* !_REENT_ONLY */

/*
 * Copyright (c) 2002, Red Hat Inc.
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
 */

#include <_ansi.h>
#include <reent.h>
#include <stdio.h>

_off_t
_DEFUN(_ftello_r, (ptr, fp),
       struct _reent * ptr _AND
       register FILE * fp)
{
  /* for now we simply cast since off_t should be long */
  return (_off_t)_ftell_r (ptr, fp);
}

#ifndef _REENT_ONLY

_off_t
_DEFUN(ftello, (fp),
       register FILE * fp)
{
  return (_off_t)_ftell_r (_REENT, fp);
}

#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<getc>>---read a character (macro)

INDEX
	getc
INDEX
	_getc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int getc(FILE *<[fp]>);

	#include <stdio.h>
	int _getc_r(struct _reent *<[ptr]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int getc(<[fp]>)
	FILE *<[fp]>;

	#include <stdio.h>
	int _getc_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

DESCRIPTION
<<getc>> is a macro, defined in <<stdio.h>>.  You can use <<getc>>
to get the next single character from the file or stream
identified by <[fp]>.  As a side effect, <<getc>> advances the file's
current position indicator.

For a subroutine version of this macro, see <<fgetc>>.

The <<_getc_r>> function is simply the reentrant version of <<getc>>
which passes an additional reentrancy structure pointer argument: <[ptr]>.

RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<getc>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using the <<ferror>> and <<feof>> functions.

PORTABILITY
ANSI C requires <<getc>>; it suggests, but does not require, that
<<getc>> be implemented as a macro.  The standard explicitly permits
macro implementations of <<getc>> to use the argument more than once;
therefore, in a portable program, you should not use an expression
with side effects as the <<getc>> argument.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

/*
 * A subroutine version of the macro getc.
 */

#undef getc

int
_DEFUN(_getc_r, (ptr, fp),
       struct _reent *ptr _AND
       register FILE *fp)
{
  int result;
  CHECK_INIT (ptr, fp);
  __sfp_lock_acquire ();
  _flockfile (fp);
  result = __sgetc_r (ptr, fp);
  _funlockfile (fp);
  __sfp_lock_release ();
  return result;
}

#ifndef _REENT_ONLY

int
_DEFUN(getc, (fp),
       register FILE *fp)
{
  int result;
  CHECK_INIT (_REENT, fp);
  __sfp_lock_acquire ();
  _flockfile (fp);
  result = __sgetc_r (_REENT, fp);
  _funlockfile (fp);
  __sfp_lock_release ();
  return result;
}

#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<getc_unlocked>>---non-thread-safe version of getc (macro)

INDEX
	getc_unlocked
INDEX
	_getc_unlocked_r

SYNOPSIS
	#include <stdio.h>
	int getc_unlocked(FILE *<[fp]>);

	#include <stdio.h>
	int _getc_unlocked_r(FILE *<[fp]>);

DESCRIPTION
<<getc_unlocked>> is a non-thread-safe version of <<getc>> declared in
<<stdio.h>>.  <<getc_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  These
functions may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the ( FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<getc_unlocked>> is equivalent to <<getc>>.

The <<_getc_unlocked_r>> function is simply the reentrant version of
<<get_unlocked>> which passes an additional reentrancy structure pointer
argument: <[ptr]>.

RETURNS
See <<getc>>.

PORTABILITY
POSIX 1003.1 requires <<getc_unlocked>>.  <<getc_unlocked>> may be
implemented as a macro, so arguments should not have side-effects.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.  */

#if 0
#include <_ansi.h>
#include <stdio.h>
#endif

/*
 * A subroutine version of the macro getc_unlocked.
 */

#undef getc_unlocked

int
_DEFUN(_getc_unlocked_r, (ptr, fp),
       struct _reent *ptr _AND
       register FILE *fp)
{
  /* CHECK_INIT is called (eventually) by __srefill_r.  */

  return __sgetc_r (ptr, fp);
}

#ifndef _REENT_ONLY

int
_DEFUN(getc_unlocked, (fp),
       register FILE *fp)
{
  return __sgetc_r (_REENT, fp);
}

#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<getchar>>---read a character (macro)

INDEX
	getchar
INDEX
	_getchar_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int getchar(void);

	int _getchar_r(struct _reent *<[reent]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int getchar();

	int _getchar_r(<[reent]>)
	char * <[reent]>;

DESCRIPTION
<<getchar>> is a macro, defined in <<stdio.h>>.  You can use <<getchar>>
to get the next single character from the standard input stream.
As a side effect, <<getchar>> advances the standard input's
current position indicator.

The alternate function <<_getchar_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.


RETURNS
The next character (read as an <<unsigned char>>, and cast to
<<int>>), unless there is no more data, or the host system reports a
read error; in either of these situations, <<getchar>> returns <<EOF>>.

You can distinguish the two situations that cause an <<EOF>> result by
using `<<ferror(stdin)>>' and `<<feof(stdin)>>'.

PORTABILITY
ANSI C requires <<getchar>>; it suggests, but does not require, that
<<getchar>> be implemented as a macro.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

/*
 * A subroutine version of the macro getchar.
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include "local.h"
#endif

#undef getchar

int
_DEFUN(_getchar_r, (reent),
       struct _reent *reent)
{
  _REENT_SMALL_CHECK_INIT (reent);
  return _getc_r (reent, _stdin_r (reent));
}

#ifndef _REENT_ONLY

int
_DEFUN_VOID(getchar)
{
  /* CHECK_INIT is called (eventually) by __srefill_r.  */
  _REENT_SMALL_CHECK_INIT (_REENT);
  return _getc_r (_REENT, _stdin_r (_REENT));
}

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
 */

/*
FUNCTION
<<getchar_unlocked>>---non-thread-safe version of getchar (macro)

INDEX
	getchar_unlocked
INDEX
	_getchar_unlocked_r

SYNOPSIS
	#include <stdio.h>
	int getchar_unlocked();

	#include <stdio.h>
	int _getchar_unlocked_r(struct _reent *<[ptr]>);

DESCRIPTION
<<getchar_unlocked>> is a non-thread-safe version of <<getchar>>
declared in <<stdio.h>>.  <<getchar_unlocked>> may only safely be used
within a scope protected by flockfile() (or ftrylockfile()) and
funlockfile().  These functions may safely be used in a multi-threaded
program if and only if they are called while the invoking thread owns
the ( FILE *) object, as is the case after a successful call to the
flockfile() or ftrylockfile() functions.  If threads are disabled,
then <<getchar_unlocked>> is equivalent to <<getchar>>.

The <<_getchar_unlocked_r>> function is simply the reentrant version of
<<getchar_unlocked>> which passes an addtional reentrancy structure pointer
argument: <[ptr]>.

RETURNS
See <<getchar>>.

PORTABILITY
POSIX 1003.1 requires <<getchar_unlocked>>.  <<getchar_unlocked>> may
be implemented as a macro.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.  */

/*
 * A subroutine version of the macro getchar_unlocked.
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#endif

#undef getchar_unlocked

int
_DEFUN(_getchar_unlocked_r, (ptr),
       struct _reent *ptr)
{
  return _getc_unlocked_r (ptr, _stdin_r (ptr));
}

#ifndef _REENT_ONLY

int
_DEFUN_VOID(getchar_unlocked)
{
  /* CHECK_INIT is called (eventually) by __srefill_r.  */

  return _getc_unlocked_r (_REENT, _stdin_r (_REENT));
}

#endif

/* Copyright 2002, Red Hat Inc. - all rights reserved */
/*
FUNCTION
<<getdelim>>---read a line up to a specified line delimiter

INDEX
	getdelim

ANSI_SYNOPSIS
	#include <stdio.h>
	int getdelim(char **<[bufptr]>, size_t *<[n]>,
                     int <[delim]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int getdelim(<[bufptr]>, <[n]>, <[delim]>, <[fp]>)
	char **<[bufptr]>;
	size_t *<[n]>;
	int <[delim]>;
	FILE *<[fp]>;

DESCRIPTION
<<getdelim>> reads a file <[fp]> up to and possibly including a specified
delimiter <[delim]>.  The line is read into a buffer pointed to
by <[bufptr]> and designated with size *<[n]>.  If the buffer is
not large enough, it will be dynamically grown by <<getdelim>>.
As the buffer is grown, the pointer to the size <[n]> will be
updated.

RETURNS
<<getdelim>> returns <<-1>> if no characters were successfully read;
otherwise, it returns the number of bytes successfully read.
At end of file, the result is nonzero.

PORTABILITY
<<getdelim>> is a glibc extension.

No supporting OS subroutines are directly required.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "local.h"
#endif

#define MIN_LINE_SIZE 4
#define DEFAULT_LINE_SIZE 128

ssize_t
_DEFUN(__getdelim, (bufptr, n, delim, fp),
       char **bufptr _AND
       size_t *n     _AND
       int delim     _AND
       FILE *fp)
{
  char *buf;
  char *ptr;
  size_t newsize, numbytes;
  int pos;
  int ch;
  int cont;

  if (fp == NULL || bufptr == NULL || n == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  buf = *bufptr;
  if (buf == NULL || *n < MIN_LINE_SIZE)
    {
      buf = (char *)realloc (*bufptr, DEFAULT_LINE_SIZE);
      if (buf == NULL)
        {
	  return -1;
        }
      *bufptr = buf;
      *n = DEFAULT_LINE_SIZE;
    }

  CHECK_INIT (_REENT, fp);

  __sfp_lock_acquire ();
  _flockfile (fp);

  numbytes = *n;
  ptr = buf;

  cont = 1;

  while (cont)
    {
      /* fill buffer - leaving room for nul-terminator */
      while (--numbytes > 0)
        {
          if ((ch = getc_unlocked (fp)) == EOF)
            {
	      cont = 0;
              break;
            }
	  else
            {
              *ptr++ = ch;
              if (ch == delim)
                {
                  cont = 0;
                  break;
                }
            }
        }

      if (cont)
        {
          /* Buffer is too small so reallocate a larger buffer.  */
          pos = ptr - buf;
          newsize = (*n << 1);
          buf = realloc (buf, newsize);
          if (buf == NULL)
            {
              cont = 0;
              break;
            }

          /* After reallocating, continue in new buffer */
          *bufptr = buf;
          *n = newsize;
          ptr = buf + pos;
          numbytes = newsize - pos;
        }
    }

  _funlockfile (fp);
  __sfp_lock_release ();

  /* if no input data, return failure */
  if (ptr == buf)
    return -1;

  /* otherwise, nul-terminate and return number of bytes read */
  *ptr = '\0';
  return (ssize_t)(ptr - buf);
}

/* Copyright 2002, Red Hat Inc. - all rights reserved */
/*
FUNCTION
<<getline>>---read a line from a file

INDEX
        getline

ANSI_SYNOPSIS
        #include <stdio.h>
        ssize_t getline(char **<[bufptr]>, size_t *<[n]>, FILE *<[fp]>);

TRAD_SYNOPSIS
        #include <stdio.h>
        ssize_t getline(<[bufptr]>, <[n]>, <[fp]>)
        char **<[bufptr]>;
        size_t *<[n]>;
        FILE *<[fp]>;

DESCRIPTION
<<getline>> reads a file <[fp]> up to and possibly including the
newline character.  The line is read into a buffer pointed to
by <[bufptr]> and designated with size *<[n]>.  If the buffer is
not large enough, it will be dynamically grown by <<getdelim>>.
As the buffer is grown, the pointer to the size <[n]> will be
updated.

<<getline>> is equivalent to getdelim(bufptr, n, '\n', fp);

RETURNS
<<getline>> returns <<-1>> if no characters were successfully read,
otherwise, it returns the number of bytes successfully read.
at end of file, the result is nonzero.

PORTABILITY
<<getline>> is a glibc extension.

No supporting OS subroutines are directly required.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#endif

extern ssize_t _EXFUN(__getdelim, (char **, size_t *, int, FILE *));

ssize_t
_DEFUN(__getline, (lptr, n, fp),
       char **lptr _AND
       size_t *n   _AND
       FILE *fp)
{
  return __getdelim (lptr, n, '\n', fp);
}

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
 */

/*
FUNCTION
<<getw>>---read a word (int)

INDEX
	getw

ANSI_SYNOPSIS
	#include <stdio.h>
	int getw(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int getw(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
<<getw>> is a function, defined in <<stdio.h>>.  You can use <<getw>>
to get the next word from the file or stream identified by <[fp]>.  As
a side effect, <<getw>> advances the file's current position
indicator.

RETURNS
The next word (read as an <<int>>), unless there is no more
data or the host system reports a read error; in either of these
situations, <<getw>> returns <<EOF>>.  Since <<EOF>> is a valid
<<int>>, you must use <<ferror>> or <<feof>> to distinguish these
situations.

PORTABILITY
<<getw>> is a remnant of K&R C; it is not part of any ISO C Standard.
<<fread>> should be used instead.  In fact, this implementation of
<<getw>> is based upon <<fread>>.

Supporting OS subroutines required: <<fread>>.  */

#if 0
#include <_ansi.h>
#include <stdio.h>
#endif

int
_DEFUN(getw, (fp),
       register FILE *fp)
{
  int result;
  if (fread ((char*)&result, sizeof (result), 1, fp) != 1)
    return EOF;
  return result;
}

/*-
 * Copyright (c) 2002 Tim J. Robbins.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<getwchar>>---read a wide character from standard input

INDEX
	getwchar
INDEX
	_getwchar_r

ANSI_SYNOPSIS
	#include <wchar.h>
	wint_t getwchar(void);

	wint_t _getwchar_r(struct _reent *<[reent]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	wint_t getwchar();

	wint_t _getwchar_r(<[reent]>)
	char * <[reent]>;

DESCRIPTION
<<getwchar>> function or macro is the wide character equivalent of
the <<getchar>> function.  You can use <<getwchar>> to get the next
wide character from the standard input stream.  As a side effect,
<<getwchar>> advances the standard input's current position indicator.

The alternate function <<_getwchar_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
The next wide character cast to <<wint_t>>, unless there is no more
data, or the host system reports a read error; in either of these
situations, <<getwchar>> returns <<WEOF>>.

You can distinguish the two situations that cause an <<WEOF>> result by
using `<<ferror(stdin)>>' and `<<feof(stdin)>>'.

PORTABILITY
C99
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include "local.h"
#endif

#undef getwchar

wint_t
_DEFUN (_getwchar_r, (ptr),
	struct _reent *ptr)
{
  return _fgetwc_r (ptr, stdin);
}

/*
 * Synonym for fgetwc(stdin).
 */
wint_t
_DEFUN_VOID (getwchar)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return fgetwc (stdin);
}

/* Copyright (C) 2007 Eric Blake
 * Permission to use, copy, modify, and distribute this software
 * is freely granted, provided that this notice is preserved.
 */

/*
FUNCTION
<<open_memstream>>, <<open_wmemstream>>---open a write stream around an arbitrary-length string

INDEX
	open_memstream
INDEX
	open_wmemstream

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *open_memstream(char **restrict <[buf]>,
			     size_t *restrict <[size]>);

	#include <wchar.h>
	FILE *open_wmemstream(wchar_t **restrict <[buf]>,
			      size_t *restrict <[size]>);

DESCRIPTION
<<open_memstream>> creates a seekable, byte-oriented <<FILE>> stream that
wraps an arbitrary-length buffer, created as if by <<malloc>>.  The current
contents of *<[buf]> are ignored; this implementation uses *<[size]>
as a hint of the maximum size expected, but does not fail if the hint
was wrong.  The parameters <[buf]> and <[size]> are later stored
through following any call to <<fflush>> or <<fclose>>, set to the
current address and usable size of the allocated string; although
after fflush, the pointer is only valid until another stream operation
that results in a write.  Behavior is undefined if the user alters
either *<[buf]> or *<[size]> prior to <<fclose>>.

<<open_wmemstream>> is like <<open_memstream>> just with the associated
stream being wide-oriented.  The size set in <[size]> in subsequent
operations is the number of wide characters.

The stream is write-only, since the user can directly read *<[buf]>
after a flush; see <<fmemopen>> for a way to wrap a string with a
readable stream.  The user is responsible for calling <<free>> on
the final *<[buf]> after <<fclose>>.

Any time the stream is flushed, a NUL byte is written at the current
position (but is not counted in the buffer length), so that the string
is always NUL-terminated after at most *<[size]> bytes (or wide characters
in case of <<open_wmemstream>>).  However, data previously written beyond
the current stream offset is not lost, and the NUL value written during a
flush is restored to its previous value when seeking elsewhere in the string.

RETURNS
The return value is an open FILE pointer on success.  On error,
<<NULL>> is returned, and <<errno>> will be set to EINVAL if <[buf]>
or <[size]> is NULL, ENOMEM if memory could not be allocated, or
EMFILE if too many streams are already open.

PORTABILITY
POSIX.1-2008

Supporting OS subroutines required: <<sbrk>>.
*/

#if 0
#include <stdio.h>
#include <wchar.h>
#include <errno.h>
#include <string.h>
#include <sys/lock.h>
#include <stdint.h>
#include "local.h"
#endif

#ifndef __LARGE64_FILES
# define OFF_T off_t
#else
# define OFF_T _off64_t
#endif

/* Describe details of an open memstream.  */
typedef struct memstream {
  void *storage; /* storage to free on close */
  char **pbuf; /* pointer to the current buffer */
  size_t *psize; /* pointer to the current size, smaller of pos or eof */
  size_t pos; /* current position */
  size_t eof; /* current file size */
  size_t max; /* current malloc buffer size, always > eof */
  union {
    char c;
    wchar_t w;
  } saved; /* saved character that lived at *psize before NUL */
  int8_t wide; /* wide-oriented (>0) or byte-oriented (<0) */
} memstream;

/* Write up to non-zero N bytes of BUF into the stream described by COOKIE,
   returning the number of bytes written or EOF on failure.  */
static _READ_WRITE_RETURN_TYPE
_DEFUN(memwriter, (ptr, cookie, buf, n),
       struct _reent *ptr _AND
       void *cookie _AND
       const char *buf _AND
       int n)
{
  memstream *c = (memstream *) cookie;
  char *cbuf = *c->pbuf;

  /* size_t is unsigned, but off_t is signed.  Don't let stream get so
     big that user cannot do ftello.  */
  if (sizeof (OFF_T) == sizeof (size_t) && (ssize_t) (c->pos + n) < 0)
    {
      ptr->_errno = EFBIG;
      return EOF;
    }
  /* Grow the buffer, if necessary.  Choose a geometric growth factor
     to avoid quadratic realloc behavior, but use a rate less than
     (1+sqrt(5))/2 to accomodate malloc overhead.  Overallocate, so
     that we can add a trailing \0 without reallocating.  The new
     allocation should thus be max(prev_size*1.5, c->pos+n+1). */
  if (c->pos + n >= c->max)
    {
      size_t newsize = c->max * 3 / 2;
      if (newsize < c->pos + n + 1)
	newsize = c->pos + n + 1;
      cbuf = _realloc_r (ptr, cbuf, newsize);
      if (! cbuf)
	return EOF; /* errno already set to ENOMEM */
      *c->pbuf = cbuf;
      c->max = newsize;
    }
  /* If we have previously done a seek beyond eof, ensure all
     intermediate bytes are NUL.  */
  if (c->pos > c->eof)
    memset (cbuf + c->eof, '\0', c->pos - c->eof);
  memcpy (cbuf + c->pos, buf, n);
  c->pos += n;
  /* If the user has previously written further, remember what the
     trailing NUL is overwriting.  Otherwise, extend the stream.  */
  if (c->pos > c->eof)
    c->eof = c->pos;
  else if (c->wide > 0)
    c->saved.w = *(wchar_t *)(cbuf + c->pos);
  else
    c->saved.c = cbuf[c->pos];
  cbuf[c->pos] = '\0';
  *c->psize = (c->wide > 0) ? c->pos / sizeof (wchar_t) : c->pos;
  return n;
}

/* Seek to position POS relative to WHENCE within stream described by
   COOKIE; return resulting position or fail with EOF.  */
static _fpos_t
_DEFUN(memseeker, (ptr, cookie, pos, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos_t pos _AND
       int whence)
{
  memstream *c = (memstream *) cookie;
  OFF_T offset = (OFF_T) pos;

  if (whence == SEEK_CUR)
    offset += c->pos;
  else if (whence == SEEK_END)
    offset += c->eof;
  if (offset < 0)
    {
      ptr->_errno = EINVAL;
      offset = -1;
    }
  else if ((size_t) offset != offset)
    {
      ptr->_errno = ENOSPC;
      offset = -1;
    }
#ifdef __LARGE64_FILES
  else if ((_fpos_t) offset != offset)
    {
      ptr->_errno = EOVERFLOW;
      offset = -1;
    }
#endif /* __LARGE64_FILES */
  else
    {
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    *(wchar_t *)((*c->pbuf) + c->pos) = c->saved.w;
	  else
	    (*c->pbuf)[c->pos] = c->saved.c;
	  c->saved.w = L'\0';
	}
      c->pos = offset;
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    {
	      c->saved.w = *(wchar_t *)((*c->pbuf) + c->pos);
	      *(wchar_t *)((*c->pbuf) + c->pos) = L'\0';
	      *c->psize = c->pos / sizeof (wchar_t);
	    }
	  else
	    {
	      c->saved.c = (*c->pbuf)[c->pos];
	      (*c->pbuf)[c->pos] = '\0';
	      *c->psize = c->pos;
	    }
	}
      else if (c->wide > 0)
	*c->psize = c->eof / sizeof (wchar_t);
      else
	*c->psize = c->eof;
    }
  return (_fpos_t) offset;
}

/* Seek to position POS relative to WHENCE within stream described by
   COOKIE; return resulting position or fail with EOF.  */
#ifdef __LARGE64_FILES
static _fpos64_t
_DEFUN(memseeker64, (ptr, cookie, pos, whence),
       struct _reent *ptr _AND
       void *cookie _AND
       _fpos64_t pos _AND
       int whence)
{
  _off64_t offset = (_off64_t) pos;
  memstream *c = (memstream *) cookie;

  if (whence == SEEK_CUR)
    offset += c->pos;
  else if (whence == SEEK_END)
    offset += c->eof;
  if (offset < 0)
    {
      ptr->_errno = EINVAL;
      offset = -1;
    }
  else if ((size_t) offset != offset)
    {
      ptr->_errno = ENOSPC;
      offset = -1;
    }
  else
    {
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    *(wchar_t *)((*c->pbuf) + c->pos) = c->saved.w;
	  else
	    (*c->pbuf)[c->pos] = c->saved.c;
	  c->saved.w = L'\0';
	}
      c->pos = offset;
      if (c->pos < c->eof)
	{
	  if (c->wide > 0)
	    {
	      c->saved.w = *(wchar_t *)((*c->pbuf) + c->pos);
	      *(wchar_t *)((*c->pbuf) + c->pos) = L'\0';
	      *c->psize = c->pos / sizeof (wchar_t);
	    }
	  else
	    {
	      c->saved.c = (*c->pbuf)[c->pos];
	      (*c->pbuf)[c->pos] = '\0';
	      *c->psize = c->pos;
	    }
	}
      else if (c->wide > 0)
	*c->psize = c->eof / sizeof (wchar_t);
      else
	*c->psize = c->eof;
    }
  return (_fpos64_t) offset;
}
#endif /* __LARGE64_FILES */

/* Reclaim resources used by stream described by COOKIE.  */
static int
_DEFUN(memcloser, (ptr, cookie),
       struct _reent *ptr _AND
       void *cookie)
{
  memstream *c = (memstream *) cookie;
  char *buf;

  /* Be nice and try to reduce any unused memory.  */
  buf = _realloc_r (ptr, *c->pbuf,
		    c->wide > 0 ? (*c->psize + 1) * sizeof (wchar_t)
				: *c->psize + 1);
  if (buf)
    *c->pbuf = buf;
  _free_r (ptr, c->storage);
  return 0;
}

/* Open a memstream that tracks a dynamic buffer in BUF and SIZE.
   Return the new stream, or fail with NULL.  */
static FILE *
_DEFUN(internal_open_memstream_r, (ptr, buf, size, wide),
       struct _reent *ptr _AND
       char **buf _AND
       size_t *size _AND
       int wide)
{
  FILE *fp;
  memstream *c;

  if (!buf || !size)
    {
      ptr->_errno = EINVAL;
      return NULL;
    }
  if ((fp = __sfp (ptr)) == NULL)
    return NULL;
  if ((c = (memstream *) _malloc_r (ptr, sizeof *c)) == NULL)
    {
      __sfp_lock_acquire ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      return NULL;
    }
  /* Use *size as a hint for initial sizing, but bound the initial
     malloc between 64 bytes (same as asprintf, to avoid frequent
     mallocs on small strings) and 64k bytes (to avoid overusing the
     heap if *size was garbage).  */
  c->max = *size;
  if (wide == 1)
    c->max *= sizeof(wchar_t);
  if (c->max < 64)
    c->max = 64;
  else if (c->max > 64 * 1024)
    c->max = 64 * 1024;
  *size = 0;
  *buf = _malloc_r (ptr, c->max);
  if (!*buf)
    {
      __sfp_lock_acquire ();
      fp->_flags = 0;		/* release */
#ifndef __SINGLE_THREAD__
      __lock_close_recursive (fp->_lock);
#endif
      __sfp_lock_release ();
      _free_r (ptr, c);
      return NULL;
    }
  if (wide == 1)
    **((wchar_t **)buf) = L'\0';
  else
    **buf = '\0';

  c->storage = c;
  c->pbuf = buf;
  c->psize = size;
  c->eof = 0;
  c->saved.w = L'\0';
  c->wide = (int8_t) wide;

  _flockfile (fp);
  fp->_file = -1;
  fp->_flags = __SWR;
  fp->_cookie = c;
  fp->_read = NULL;
  fp->_write = memwriter;
  fp->_seek = memseeker;
#ifdef __LARGE64_FILES
  fp->_seek64 = memseeker64;
  fp->_flags |= __SL64;
#endif
  fp->_close = memcloser;
  ORIENT (fp, wide);
  _funlockfile (fp);
  return fp;
}

FILE *
_DEFUN(_open_memstream_r, (ptr, buf, size),
       struct _reent *ptr _AND
       char **buf _AND
       size_t *size)
{
  return internal_open_memstream_r (ptr, buf, size, -1);
}

FILE *
_DEFUN(_open_wmemstream_r, (ptr, buf, size),
       struct _reent *ptr _AND
       wchar_t **buf _AND
       size_t *size)
{
  return internal_open_memstream_r (ptr, (char **)buf, size, 1);
}

#ifndef _REENT_ONLY
FILE *
_DEFUN(open_memstream, (buf, size),
       char **buf _AND
       size_t *size)
{
  return _open_memstream_r (_REENT, buf, size);
}

FILE *
_DEFUN(open_wmemstream, (buf, size),
       wchar_t **buf _AND
       size_t *size)
{
  return _open_wmemstream_r (_REENT, buf, size);
}
#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<putc>>---write a character (macro)

INDEX
	putc
INDEX
	_putc_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int putc(int <[ch]>, FILE *<[fp]>);

	#include <stdio.h>
	int _putc_r(struct _reent *<[ptr]>, int <[ch]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int putc(<[ch]>, <[fp]>)
	int <[ch]>;
	FILE *<[fp]>;

	#include <stdio.h>
	int _putc_r(<[ptr]>, <[ch]>, <[fp]>)
	struct _reent *<[ptr]>;
	int <[ch]>;
	FILE *<[fp]>;

DESCRIPTION
<<putc>> is a macro, defined in <<stdio.h>>.  <<putc>>
writes the argument <[ch]> to the file or stream identified by
<[fp]>, after converting it from an <<int>> to an <<unsigned char>>.

If the file was opened with append mode (or if the stream cannot
support positioning), then the new character goes at the end of the
file or stream.  Otherwise, the new character is written at the
current value of the position indicator, and the position indicator
advances by one.

For a subroutine version of this macro, see <<fputc>>.

The <<_putc_r>> function is simply the reentrant version of
<<putc>> that takes an additional reentrant structure argument: <[ptr]>.

RETURNS
If successful, <<putc>> returns its argument <[ch]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(<[fp]>)>>' to
query for errors.

PORTABILITY
ANSI C requires <<putc>>; it suggests, but does not require, that
<<putc>> be implemented as a macro.  The standard explicitly permits
macro implementations of <<putc>> to use the <[fp]> argument more than once;
therefore, in a portable program, you should not use an expression
with side effects as this argument.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

/*
 * A subroutine version of the macro putc.
 */

#undef putc

int
_DEFUN(_putc_r, (ptr, c, fp),
       struct _reent *ptr _AND
       int c _AND
       register FILE *fp)
{
  int result;
  CHECK_INIT (ptr, fp);
  _flockfile (fp);
  result = __sputc_r (ptr, c, fp);
  _funlockfile (fp);
  return result;
}

#ifndef _REENT_ONLY
int
_DEFUN(putc, (c, fp),
       int c _AND
       register FILE *fp)
{
#if !defined(PREFER_SIZE_OVER_SPEED) && !defined(__OPTIMIZE_SIZE__)
  int result;
  CHECK_INIT (_REENT, fp);
  _flockfile (fp);
  result = __sputc_r (_REENT, c, fp);
  _funlockfile (fp);
  return result;
#else
  return _putc_r (_REENT, c, fp);
#endif
}
#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<putc_unlocked>>---non-thread-safe version of putc (macro)

INDEX
	putc_unlocked
INDEX
	_putc_unlocked_r

SYNOPSIS
	#include <stdio.h>
	int putc_unlocked(int <[ch]>, FILE *<[fp]>);

	#include <stdio.h>
	int _putc_unlocked_r(struct _reent *<[ptr]>, int <[ch]>, FILE *<[fp]>);

DESCRIPTION
<<putc_unlocked>> is a non-thread-safe version of <<putc>> declared in
<<stdio.h>>.  <<putc_unlocked>> may only safely be used within a scope
protected by flockfile() (or ftrylockfile()) and funlockfile().  These
functions may safely be used in a multi-threaded program if and only
if they are called while the invoking thread owns the ( FILE *)
object, as is the case after a successful call to the flockfile() or
ftrylockfile() functions.  If threads are disabled, then
<<putc_unlocked>> is equivalent to <<putc>>.

The function <<_putc_unlocked_r>> is simply the reentrant version of
<<putc_unlocked>> that takes an additional reentrant structure pointer
argument: <[ptr]>.

RETURNS
See <<putc>>.

PORTABILITY
POSIX 1003.1 requires <<putc_unlocked>>.  <<putc_unlocked>> may be
implemented as a macro, so arguments should not have side-effects.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#endif

/*
 * A subroutine version of the macro putc_unlocked.
 */

#undef putc_unlocked

int
_DEFUN(_putc_unlocked_r, (ptr, c, fp),
       struct _reent *ptr _AND
       int c _AND
       register FILE *fp)
{
  /* CHECK_INIT is (eventually) called by __swbuf.  */

  return __sputc_r (ptr, c, fp);
}

#ifndef _REENT_ONLY
int
_DEFUN(putc_unlocked, (c, fp),
       int c _AND
       register FILE *fp)
{
  /* CHECK_INIT is (eventually) called by __swbuf.  */

  return __sputc_r (_REENT, c, fp);
}
#endif /* !_REENT_ONLY */

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
 */

/*
FUNCTION
<<putchar>>---write a character (macro)

INDEX
	putchar
INDEX
	_putchar_r

ANSI_SYNOPSIS
	#include <stdio.h>
	int putchar(int <[ch]>);

	int _putchar_r(struct _reent *<[reent]>, int <[ch]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int putchar(<[ch]>)
	int <[ch]>;

	int _putchar_r(<[reent]>, <[ch]>)
	struct _reent *<[reent]>;
	int <[ch]>;

DESCRIPTION
<<putchar>> is a macro, defined in <<stdio.h>>.  <<putchar>>
writes its argument to the standard output stream,
after converting it from an <<int>> to an <<unsigned char>>.

The alternate function <<_putchar_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
If successful, <<putchar>> returns its argument <[ch]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(stdin)>>' to
query for errors.

PORTABILITY
ANSI C requires <<putchar>>; it suggests, but does not require, that
<<putchar>> be implemented as a macro.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

/*
 * A subroutine version of the macro putchar.
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include "local.h"
#endif

#undef putchar

int
_DEFUN(_putchar_r, (ptr, c),
       struct _reent *ptr _AND
       int c)
{
  _REENT_SMALL_CHECK_INIT (ptr);
  return _putc_r (ptr, c, _stdout_r (ptr));
}

#ifndef _REENT_ONLY

int
_DEFUN(putchar, (c),
       int c)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return _putc_r (_REENT, c, _stdout_r (_REENT));
}

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
 */

/*
FUNCTION
<<putchar_unlocked>>---non-thread-safe version of putchar (macro)

INDEX
	putchar_unlocked

SYNOPSIS
	#include <stdio.h>
	int putchar_unlocked(int <[ch]>);

DESCRIPTION
<<putchar_unlocked>> is a non-thread-safe version of <<putchar>>
declared in <<stdio.h>>.  <<putchar_unlocked>> may only safely be used
within a scope protected by flockfile() (or ftrylockfile()) and
funlockfile().  These functions may safely be used in a multi-threaded
program if and only if they are called while the invoking thread owns
the ( FILE *) object, as is the case after a successful call to the
flockfile() or ftrylockfile() functions.  If threads are disabled,
then <<putchar_unlocked>> is equivalent to <<putchar>>.

RETURNS
See <<putchar>>.

PORTABILITY
POSIX 1003.1 requires <<putchar_unlocked>>.  <<putchar_unlocked>> may
be implemented as a macro.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.  */

/*
 * A subroutine version of the macro putchar_unlocked.
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#endif

#undef putchar_unlocked

int
_DEFUN(_putchar_unlocked_r, (ptr, c),
       struct _reent *ptr _AND
       int c)
{
  return putc_unlocked (c, _stdout_r (ptr));
}

#ifndef _REENT_ONLY

int
_DEFUN(putchar_unlocked, (c),
       int c)
{
  /* CHECK_INIT is (eventually) called by __swbuf.  */

  return _putchar_unlocked_r (_REENT, c);
}

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
 */

/*
FUNCTION
<<putw>>---write a word (int)

INDEX
	putw

ANSI_SYNOPSIS
	#include <stdio.h>
	int putw(int <[w]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	int putw(<w>, <[fp]>)
	int <w>;
	FILE *<[fp]>;

DESCRIPTION
<<putw>> is a function, defined in <<stdio.h>>.  You can use <<putw>>
to write a word to the file or stream identified by <[fp]>.  As a side
effect, <<putw>> advances the file's current position indicator.

RETURNS
Zero on success, <<EOF>> on failure.

PORTABILITY
<<putw>> is a remnant of K&R C; it is not part of any ISO C Standard.
<<fwrite>> should be used instead.  In fact, this implementation of
<<putw>> is based upon <<fwrite>>.

Supporting OS subroutines required: <<fwrite>>.
*/

#if 0
#include <stdio.h>
#endif

int
_DEFUN(putw, (w, fp),
       int w _AND
       register FILE *fp)
{
  if (fwrite ((_CONST char*)&w, sizeof (w), 1, fp) != 1)
    return EOF;
  return 0;
}

/*-
 * Copyright (c) 2002 Tim J. Robbins.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<putwchar>>---write a wide character to standard output

INDEX
	putwchar
INDEX
	_putwchar_r

ANSI_SYNOPSIS
	#include <wchar.h>
	wint_t putwchar(wchar_t <[wc]>);

	wint_t _putwchar_r(struct _reent *<[reent]>, wchar_t <[wc]>);

TRAD_SYNOPSIS
	#include <wchar.h>
	wint_t putwchar(<[wc]>)
	wchar_t <[wc]>;

	wint_t _putwchar_r(<[reent]>, <[wc]>)
	struct _reent *<[reent]>;
	wchar_t <[wc]>;

DESCRIPTION
The <<putwchar>> function or macro is the wide-character equivalent of
the <<putchar>> function. It writes the wide character wc to stdout.

The alternate function <<_putwchar_r>> is a reentrant version.  The
extra argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
If successful, <<putwchar>> returns its argument <[wc]>.  If an error
intervenes, the result is <<EOF>>.  You can use `<<ferror(stdin)>>' to
query for errors.

PORTABILITY
C99
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <wchar.h>
#include "local.h"
#endif

#undef putwchar

wint_t
_DEFUN(_putwchar_r, (ptr, wc),
	struct _reent *ptr _AND
	wchar_t wc)
{
  return _fputwc_r (ptr, wc, stdout);
}

/*
 * Synonym for fputwc(wc, stdout).
 */
wint_t
_DEFUN(putwchar, (wc),
	wchar_t wc)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return fputwc (wc, stdout);
}

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
 */

/*
FUNCTION
<<rewind>>---reinitialize a file or stream

INDEX
	rewind
INDEX
	_rewind_r

ANSI_SYNOPSIS
	#include <stdio.h>
	void rewind(FILE *<[fp]>);
	void _rewind_r(struct _reent *<[ptr]>, FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void rewind(<[fp]>)
	FILE *<[fp]>;

	void _rewind_r(<[ptr]>, <[fp]>)
	struct _reent *<[ptr]>;
	FILE *<[fp]>;

DESCRIPTION
<<rewind>> returns the file position indicator (if any) for the file
or stream identified by <[fp]> to the beginning of the file.  It also
clears any error indicator and flushes any pending output.

RETURNS
<<rewind>> does not return a result.

PORTABILITY
ANSI C requires <<rewind>>.

No supporting OS subroutines are required.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#endif

_VOID
_DEFUN(_rewind_r, (ptr, fp),
       struct _reent * ptr _AND
       register FILE * fp)
{
  _CAST_VOID _fseek_r (ptr, fp, 0L, SEEK_SET);
  clearerr (fp);
}

#ifndef _REENT_ONLY

_VOID
_DEFUN(rewind, (fp),
       register FILE * fp)
{
  _rewind_r (_REENT, fp);
}

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
 */

/*
FUNCTION
<<setbuf>>---specify full buffering for a file or stream

INDEX
	setbuf

ANSI_SYNOPSIS
	#include <stdio.h>
	void setbuf(FILE *<[fp]>, char *<[buf]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void setbuf(<[fp]>, <[buf]>)
	FILE *<[fp]>;
	char *<[buf]>;

DESCRIPTION
<<setbuf>> specifies that output to the file or stream identified by <[fp]>
should be fully buffered.  All output for this file will go to a
buffer (of size <<BUFSIZ>>, specified in `<<stdio.h>>').  Output will
be passed on to the host system only when the buffer is full, or when
an input operation intervenes.

You may, if you wish, supply your own buffer by passing a pointer to
it as the argument <[buf]>.  It must have size <<BUFSIZ>>.  You can
also use <<NULL>> as the value of <[buf]>, to signal that the
<<setbuf>> function is to allocate the buffer.

WARNINGS
You may only use <<setbuf>> before performing any file operation other
than opening the file.

If you supply a non-null <[buf]>, you must ensure that the associated
storage continues to be available until you close the stream
identified by <[fp]>.

RETURNS
<<setbuf>> does not return a result.

PORTABILITY
Both ANSI C and the System V Interface Definition (Issue 2) require
<<setbuf>>.  However, they differ on the meaning of a <<NULL>> buffer
pointer: the SVID issue 2 specification says that a <<NULL>> buffer
pointer requests unbuffered output.  For maximum portability, avoid
<<NULL>> buffer pointers.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

_VOID
_DEFUN(setbuf, (fp, buf),
       FILE * fp _AND
       char *buf)
{
  _CAST_VOID setvbuf (fp, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

#endif /* !_REENT_ONLY */

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
 */
/*
Modified copy of setbuf.c to support the setbuffer function
defined as part of BSD.
Modifications by Gareth Pearce, 2001.
*/

/*
FUNCTION
<<setbuffer>>---specify full buffering for a file or stream with size

INDEX
	setbuffer

ANSI_SYNOPSIS
	#include <stdio.h>
	void setbuffer(FILE *<[fp]>, char *<[buf]>, int <[size]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void setbuffer(<[fp]>, <[buf]>, <[size]>)
	FILE *<[fp]>;
	char *<[buf]>;
	int <[size]>;

DESCRIPTION
<<setbuffer>> specifies that output to the file or stream identified by
<[fp]> should be fully buffered.  All output for this file will go to a
buffer (of size <[size]>).  Output will be passed on to the host system
only when the buffer is full, or when an input operation intervenes.

You may, if you wish, supply your own buffer by passing a pointer to
it as the argument <[buf]>.  It must have size <[size]>.  You can
also use <<NULL>> as the value of <[buf]>, to signal that the
<<setbuffer>> function is to allocate the buffer.

WARNINGS
You may only use <<setbuffer>> before performing any file operation
other than opening the file.

If you supply a non-null <[buf]>, you must ensure that the associated
storage continues to be available until you close the stream
identified by <[fp]>.

RETURNS
<<setbuffer>> does not return a result.

PORTABILITY
This function comes from BSD not ANSI or POSIX.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

_VOID
_DEFUN(setbuffer, (fp, buf, size),
       FILE * fp _AND
       char *buf _AND
       int size)
{
  _CAST_VOID setvbuf (fp, buf, buf ? _IOFBF : _IONBF, (size_t) size);
}

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
 */
/*
Modified copy of setbuf.c to support setlinebuf function
defined as part of BSD.
Modifications by Gareth Pearce, 2001.
*/

/*
FUNCTION
<<setlinebuf>>---specify line buffering for a file or stream

INDEX
	setlinebuf

ANSI_SYNOPSIS
	#include <stdio.h>
	void setlinebuf(FILE *<[fp]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	void setlinebuf(<[fp]>)
	FILE *<[fp]>;

DESCRIPTION
<<setlinebuf>> specifies that output to the file or stream identified by
<[fp]> should be line buffered.  This causes the file or stream to pass
on output to the host system at every newline, as well as when the
buffer is full, or when an input operation intervenes.

WARNINGS
You may only use <<setlinebuf>> before performing any file operation
other than opening the file.

RETURNS
<<setlinebuf>> returns as per setvbuf.

PORTABILITY
This function comes from BSD not ANSI or POSIX.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

#if 0
#include <_ansi.h>
#include <stdio.h>
#include "local.h"
#endif

int
_DEFUN(setlinebuf, (fp),
       FILE * fp)
{
  return (setvbuf (fp, (char *) NULL, _IOLBF, (size_t) 0));
}

/*
FUNCTION
<<tmpfile>>---create a temporary file

INDEX
	tmpfile
INDEX
	_tmpfile_r

ANSI_SYNOPSIS
	#include <stdio.h>
	FILE *tmpfile(void);

	FILE *_tmpfile_r(struct _reent *<[reent]>);

TRAD_SYNOPSIS
	#include <stdio.h>
	FILE *tmpfile();

	FILE *_tmpfile_r(<[reent]>)
	struct _reent *<[reent]>;

DESCRIPTION
Create a temporary file (a file which will be deleted automatically),
using a name generated by <<tmpnam>>.  The temporary file is opened with
the mode <<"wb+">>, permitting you to read and write anywhere in it
as a binary file (without any data transformations the host system may
perform for text files).

The alternate function <<_tmpfile_r>> is a reentrant version.  The
argument <[reent]> is a pointer to a reentrancy structure.

RETURNS
<<tmpfile>> normally returns a pointer to the temporary file.  If no
temporary file could be created, the result is NULL, and <<errno>>
records the reason for failure.

PORTABILITY
Both ANSI C and the System V Interface Definition (Issue 2) require
<<tmpfile>>.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<getpid>>,
<<isatty>>, <<lseek>>, <<open>>, <<read>>, <<sbrk>>, <<write>>.

<<tmpfile>> also requires the global pointer <<environ>>.
*/

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

#ifndef O_BINARY
# define O_BINARY 0
#endif

FILE *
_DEFUN(_tmpfile_r, (ptr),
       struct _reent *ptr)
{
  FILE *fp;
  int e;
  char *f;
  char buf[L_tmpnam];
  int fd;

  do
    {
      if ((f = _tmpnam_r (ptr, buf)) == NULL)
	return NULL;
      fd = _open_r (ptr, f, O_RDWR | O_CREAT | O_EXCL | O_BINARY,
		    S_IRUSR | S_IWUSR);
    }
  while (fd < 0 && ptr->_errno == EEXIST);
  if (fd < 0)
    return NULL;
  fp = _fdopen_r (ptr, fd, "wb+");
  e = ptr->_errno;
  if (!fp)
    _close_r (ptr, fd);
  _CAST_VOID _remove_r (ptr, f);
  ptr->_errno = e;
  return fp;
}

#ifndef _REENT_ONLY

FILE *
_DEFUN_VOID(tmpfile)
{
  return _tmpfile_r (_REENT);
}

#endif

/*
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

/*
FUNCTION
<<vfprintf>>, <<vprintf>>, <<vsprintf>>, <<vsnprintf>>, <<vasprintf>>, <<vasnprintf>>---format argument list

INDEX
	vfprintf
INDEX
	_vfprintf_r
INDEX
	vprintf
INDEX
	_vprintf_r
INDEX
	vsprintf
INDEX
	_vsprintf_r
INDEX
	vsnprintf
INDEX
	_vsnprintf_r
INDEX
	vasprintf
INDEX
	_vasprintf_r
INDEX
	vasnprintf
INDEX
	_vasnprintf_r

ANSI_SYNOPSIS
	#include <stdio.h>
	#include <stdarg.h>
	int vprintf(const char *<[fmt]>, va_list <[list]>);
	int vfprintf(FILE *<[fp]>, const char *<[fmt]>, va_list <[list]>);
	int vsprintf(char *<[str]>, const char *<[fmt]>, va_list <[list]>);
	int vsnprintf(char *<[str]>, size_t <[size]>, const char *<[fmt]>,
                      va_list <[list]>);
	int vasprintf(char **<[strp]>, const char *<[fmt]>, va_list <[list]>);
	char *vasnprintf(char *<[str]>, size_t *<[size]>, const char *<[fmt]>,
                         va_list <[list]>);

	int _vprintf_r(struct _reent *<[reent]>, const char *<[fmt]>,
                        va_list <[list]>);
	int _vfprintf_r(struct _reent *<[reent]>, FILE *<[fp]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vsprintf_r(struct _reent *<[reent]>, char *<[str]>,
                        const char *<[fmt]>, va_list <[list]>);
	int _vasprintf_r(struct _reent *<[reent]>, char **<[str]>,
                         const char *<[fmt]>, va_list <[list]>);
	int _vsnprintf_r(struct _reent *<[reent]>, char *<[str]>,
                         size_t <[size]>, const char *<[fmt]>, va_list <[list]>);
	char *_vasnprintf_r(struct _reent *<[reent]>, char *<[str]>,
                            size_t *<[size]>, const char *<[fmt]>, va_list <[list]>);

DESCRIPTION
<<vprintf>>, <<vfprintf>>, <<vasprintf>>, <<vsprintf>>, <<vsnprintf>>,
and <<vasnprintf>> are (respectively) variants of <<printf>>,
<<fprintf>>, <<asprintf>>, <<sprintf>>, <<snprintf>>, and
<<asnprintf>>.  They differ only in allowing their caller to pass the
variable argument list as a <<va_list>> object (initialized by
<<va_start>>) rather than directly accepting a variable number of
arguments.  The caller is responsible for calling <<va_end>>.

<<_vprintf_r>>, <<_vfprintf_r>>, <<_vasprintf_r>>, <<_vsprintf_r>>,
<<_vsnprintf_r>>, and <<_vasnprintf_r>> are reentrant versions of the
above.

RETURNS
The return values are consistent with the corresponding functions.

PORTABILITY
ANSI C requires <<vprintf>>, <<vfprintf>>, <<vsprintf>>, and
<<vsnprintf>>.  The remaining functions are newlib extensions.

Supporting OS subroutines required: <<close>>, <<fstat>>, <<isatty>>,
<<lseek>>, <<read>>, <<sbrk>>, <<write>>.
*/

/*
 * Actual printf innards.
 *
 * This code is large and complicated...
 */
#if 0
#include <newlib.h>
#endif

#ifdef INTEGER_ONLY
# define VFPRINTF vfiprintf
# ifdef STRING_ONLY
#   define _VFPRINTF_R _svfiprintf_r
# else
#   define _VFPRINTF_R _vfiprintf_r
# endif
#else
# define VFPRINTF vfprintf
# ifdef STRING_ONLY
#   define _VFPRINTF_R _svfprintf_r
# else
#   define _VFPRINTF_R _vfprintf_r
# endif
# ifndef NO_FLOATING_POINT
#  define FLOATING_POINT
# endif
#endif

#define _NO_POS_ARGS
#ifdef _WANT_IO_POS_ARGS
# undef _NO_POS_ARGS
#endif

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>
#include <wchar.h>
#include <sys/lock.h>
#include <stdarg.h>
#include "local.h"
#include "fvwrite.h"
#include "vfieeefp.h"
#endif

/* Currently a test is made to see if long double processing is warranted.
   This could be changed in the future should the _ldtoa_r code be
   preferred over _dtoa_r.  */
#define _NO_LONGDBL
#if defined _WANT_IO_LONG_DOUBLE && (LDBL_MANT_DIG > DBL_MANT_DIG)
#undef _NO_LONGDBL
#endif

#define _NO_LONGLONG
#if defined _WANT_IO_LONG_LONG \
	&& (defined __GNUC__ || __STDC_VERSION__ >= 199901L)
# undef _NO_LONGLONG
#endif

#ifdef STRING_ONLY
#define __SPRINT __ssprint_r
#else
#define __SPRINT __sprint_r
#endif

/* The __sprint_r/__ssprint_r functions are shared between all versions of
   vfprintf and vfwprintf.  They must only be defined once, which we do in
   the INTEGER_ONLY versions here. */
#ifdef STRING_ONLY
#ifdef INTEGER_ONLY
int
_DEFUN(__ssprint_r, (ptr, fp, uio),
       struct _reent *ptr _AND
       FILE *fp _AND
       register struct __suio *uio)
{
	register size_t len;
	register int w;
	register struct __siov *iov;
	register _CONST char *p = NULL;

	iov = uio->uio_iov;
	len = 0;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}

        do {
		while (len == 0) {
			p = iov->iov_base;
			len = iov->iov_len;
			iov++;
		}
		w = fp->_w;
		if (len >= w && fp->_flags & (__SMBF | __SOPT)) {
			/* must be asprintf family */
			unsigned char *str;
			int curpos = (fp->_p - fp->_bf._base);
			/* Choose a geometric growth factor to avoid
		 	 * quadratic realloc behavior, but use a rate less
			 * than (1+sqrt(5))/2 to accomodate malloc
		 	 * overhead. asprintf EXPECTS us to overallocate, so
		 	 * that it can add a trailing \0 without
		 	 * reallocating.  The new allocation should thus be
		 	 * max(prev_size*1.5, curpos+len+1). */
			int newsize = fp->_bf._size * 3 / 2;
			if (newsize < curpos + len + 1)
				newsize = curpos + len + 1;
			if (fp->_flags & __SOPT)
			{
				/* asnprintf leaves original buffer alone.  */
				str = (unsigned char *)_malloc_r (ptr, newsize);
				if (!str)
				{
					ptr->_errno = ENOMEM;
					goto err;
				}
				memcpy (str, fp->_bf._base, curpos);
				fp->_flags = (fp->_flags & ~__SOPT) | __SMBF;
			}
			else
			{
				str = (unsigned char *)_realloc_r (ptr, fp->_bf._base,
						newsize);
				if (!str) {
					/* Free unneeded buffer.  */
					_free_r (ptr, fp->_bf._base);
					/* Ensure correct errno, even if free
					 * changed it.  */
					ptr->_errno = ENOMEM;
					goto err;
				}
			}
			fp->_bf._base = str;
			fp->_p = str + curpos;
			fp->_bf._size = newsize;
			w = len;
			fp->_w = newsize - curpos;
		}
		if (len < w)
			w = len;
		(void)memmove ((_PTR) fp->_p, (_PTR) p, (size_t) (w));
		fp->_w -= w;
		fp->_p += w;
		w = len;          /* pretend we copied all */
		p += w;
		len -= w;
        } while ((uio->uio_resid -= w) != 0);

	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return 0;

err:
  fp->_flags |= __SERR;
  uio->uio_resid = 0;
  uio->uio_iovcnt = 0;
  return EOF;
}
#else /* !INTEGER_ONLY */
int __ssprint_r (struct _reent *, FILE *, register struct __suio *);
#endif /* !INTEGER_ONLY */

#else /* !STRING_ONLY */
#ifdef INTEGER_ONLY
/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
int
_DEFUN(__sprint_r, (ptr, fp, uio),
       struct _reent *ptr _AND
       FILE *fp _AND
       register struct __suio *uio)
{
	register int err = 0;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	if (fp->_flags2 & __SWID) {
		struct __siov *iov;
		wchar_t *p;
		int i, len;

		iov = uio->uio_iov;
		for (; uio->uio_resid != 0;
		     uio->uio_resid -= len * sizeof (wchar_t), iov++) {
			p = (wchar_t *) iov->iov_base;
			len = iov->iov_len / sizeof (wchar_t);
			for (i = 0; i < len; i++) {
				if (_fputwc_r (ptr, p[i], fp) == WEOF) {
					err = -1;
					goto out;
				}
			}
		}
	} else
		err = __sfvwrite_r(ptr, fp, uio);
out:
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}
#else /* !INTEGER_ONLY */
int __sprint_r (struct _reent *, FILE *, register struct __suio *);
#endif /* !INTEGER_ONLY */

/*
 * Helper function for `fprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
_DEFUN(__sbprintf, (rptr, fp, fmt, ap),
       struct _reent *rptr _AND
       register FILE *fp   _AND
       _CONST char *fmt  _AND
       va_list ap)
{
	int ret;
	FILE fake;
	unsigned char buf[BUFSIZ];

	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._flags2 = fp->_flags2;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;

	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof (buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */
#ifndef __SINGLE_THREAD__
	__lock_init_recursive (fake._lock);
#endif

	/* do the work, then copy any error status */
	ret = _VFPRINTF_R (rptr, &fake, fmt, ap);
	if (ret >= 0 && _fflush_r (rptr, &fake))
		ret = EOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;

#ifndef __SINGLE_THREAD__
	__lock_close_recursive (fake._lock);
#endif
	return (ret);
}
#endif /* !STRING_ONLY */


#ifdef FLOATING_POINT
# include <locale.h>
# include <math.h>

/* For %La, an exponent of 15 bits occupies the exponent character, a
   sign, and up to 5 digits.  */
# define MAXEXPLEN		7
# define DEFPREC		6

# ifdef _NO_LONGDBL

extern char *_dtoa_r _PARAMS((struct _reent *, double, int,
			      int, int *, int *, char **));

#  define _PRINTF_FLOAT_TYPE double
#  define _DTOA_R _dtoa_r
#  define FREXP frexp

# else /* !_NO_LONGDBL */

extern char *_ldtoa_r _PARAMS((struct _reent *, _LONG_DOUBLE, int,
			      int, int *, int *, char **));

extern int _EXFUN(_ldcheck,(_LONG_DOUBLE *));

#  define _PRINTF_FLOAT_TYPE _LONG_DOUBLE
#  define _DTOA_R _ldtoa_r
/* FIXME - frexpl is not yet supported; and cvt infloops if (double)f
   converts a finite value into infinity.  */
/* #  define FREXP frexpl */
#  define FREXP(f,e) ((_LONG_DOUBLE) frexp ((double)f, e))
# endif /* !_NO_LONGDBL */

static char *cvt(struct _reent *, _PRINTF_FLOAT_TYPE, int, int, char *, int *,
                 int, int *, char *);

static int exponent(char *, int, int);

#endif /* FLOATING_POINT */

/* BUF must be big enough for the maximum %#llo (assuming long long is
   at most 64 bits, this would be 23 characters), the maximum
   multibyte character %C, and the maximum default precision of %La
   (assuming long double is at most 128 bits with 113 bits of
   mantissa, this would be 29 characters).  %e, %f, and %g use
   reentrant storage shared with mprec.  All other formats that use
   buf get by with fewer characters.  Making BUF slightly bigger
   reduces the need for malloc in %.*a and %S, when large precision or
   long strings are processed.  */
#define	BUF		40
#if defined _MB_CAPABLE && MB_LEN_MAX > BUF
# undef BUF
# define BUF MB_LEN_MAX
#endif

#ifndef _NO_LONGLONG
# define quad_t long long
# define u_quad_t unsigned long long
#else
# define quad_t long
# define u_quad_t unsigned long
#endif

typedef quad_t * quad_ptr_t;
typedef _PTR     void_ptr_t;
typedef char *   char_ptr_t;
typedef long *   long_ptr_t;
typedef int  *   int_ptr_t;
typedef short *  short_ptr_t;

#ifndef _NO_POS_ARGS
# ifdef NL_ARGMAX
#  define MAX_POS_ARGS NL_ARGMAX
# else
#  define MAX_POS_ARGS 32
# endif

union arg_val
{
  int val_int;
  u_int val_u_int;
  long val_long;
  u_long val_u_long;
  float val_float;
  double val_double;
  _LONG_DOUBLE val__LONG_DOUBLE;
  int_ptr_t val_int_ptr_t;
  short_ptr_t val_short_ptr_t;
  long_ptr_t val_long_ptr_t;
  char_ptr_t val_char_ptr_t;
  quad_ptr_t val_quad_ptr_t;
  void_ptr_t val_void_ptr_t;
  quad_t val_quad_t;
  u_quad_t val_u_quad_t;
  wint_t val_wint_t;
};

static union arg_val *
_EXFUN(get_arg, (struct _reent *data, int n, char *fmt,
                 va_list *ap, int *numargs, union arg_val *args,
                 int *arg_type, char **last_fmt));
#endif /* !_NO_POS_ARGS */

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit (c) <= 9)
#define	to_char(n)	((n) + '0')

/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double */
#define	LONGINT		0x010		/* long integer */
#ifndef _NO_LONGLONG
# define QUADINT	0x020		/* quad integer */
#else /* ifdef _NO_LONGLONG, make QUADINT equivalent to LONGINT, so
	 that %lld behaves the same as %ld, not as %d, as expected if:
	 sizeof (long long) = sizeof long > sizeof int  */
# define QUADINT	LONGINT
#endif
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */
#ifdef _WANT_IO_C99_FORMATS
# define CHARINT	0x200		/* char as integer */
#else /* define as 0, to make SARG and UARG occupy fewer instructions  */
# define CHARINT	0
#endif

int _EXFUN(_VFPRINTF_R, (struct _reent *, FILE *, _CONST char *, va_list));

#ifndef STRING_ONLY
int
_DEFUN(VFPRINTF, (fp, fmt0, ap),
       FILE * fp         _AND
       _CONST char *fmt0 _AND
       va_list ap)
{
  int result;
  result = _VFPRINTF_R (_REENT, fp, fmt0, ap);
  return result;
}
#endif /* STRING_ONLY */

int
_DEFUN(_VFPRINTF_R, (data, fp, fmt0, ap),
       struct _reent *data _AND
       FILE * fp           _AND
       _CONST char *fmt0   _AND
       va_list ap)
{
	register char *fmt;	/* format string */
	register int ch;	/* character from fmt */
	register int n, m;	/* handy integers (short term usage) */
	register char *cp;	/* handy char pointer (short term usage) */
	register struct __siov *iovp;/* for PRINT macro */
	register int flags;	/* flags as above */
	char *fmt_anchor;       /* current format spec being processed */
#ifndef _NO_POS_ARGS
	int N;                  /* arg number */
	int arg_index;          /* index into args processed directly */
	int numargs;            /* number of varargs read */
	char *saved_fmt;        /* saved fmt pointer */
	union arg_val args[MAX_POS_ARGS];
	int arg_type[MAX_POS_ARGS];
	int is_pos_arg;         /* is current format positional? */
	int old_is_pos_arg;     /* is current format positional? */
#endif
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	char sign;		/* sign prefix (' ', '+', '-', or \0) */
#ifdef FLOATING_POINT
	char *decimal_point = _localeconv_r (data)->decimal_point;
	char softsign;		/* temporary negative sign for floats */
	union { int i; _PRINTF_FLOAT_TYPE fp; } _double_ = {0};
# define _fpvalue (_double_.fp)
	int expt;		/* integer value of exponent */
	int expsize = 0;	/* character count for expstr */
	int ndig = 0;		/* actual number of digits returned by cvt */
	char expstr[MAXEXPLEN];	/* buffer for exponent string */
#endif /* FLOATING_POINT */
	u_quad_t _uquad;	/* integer arguments %[diouxX] */
	enum { OCT, DEC, HEX } base;/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int realsz;		/* field size expanded by dprec */
	int size;		/* size of converted field or string */
	char *xdigs = NULL;	/* digits for [xX] conversion */
#define NIOV 8
	struct __suio uio;	/* output information: summary */
	struct __siov iov[NIOV];/* ... and individual io vectors */
	char buf[BUF];		/* space for %c, %S, %[diouxX], %[aA] */
	char ox[2];		/* space for 0x hex-prefix */
#ifdef _MB_CAPABLE
	wchar_t wc;
	mbstate_t state;        /* mbtowc calls from library must not change state */
#endif
	char *malloc_buf = NULL;/* handy pointer for malloced buffers */

	/*
	 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
	 * fields occur frequently, increase PADSIZE and make the initialisers
	 * below longer.
	 */
#define	PADSIZE	16		/* pad chunk size */
	static _CONST char blanks[PADSIZE] =
	 {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
	static _CONST char zeroes[PADSIZE] =
	 {'0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};

#ifdef _MB_CAPABLE
	memset (&state, '\0', sizeof (state));
#endif
	/*
	 * BEWARE, these `goto error' on error, and PAD uses `n'.
	 */
#define	PRINT(ptr, len) { \
	iovp->iov_base = (ptr); \
	iovp->iov_len = (len); \
	uio.uio_resid += (len); \
	iovp++; \
	if (++uio.uio_iovcnt >= NIOV) { \
		if (__SPRINT(data, fp, &uio)) \
			goto error; \
		iovp = iov; \
	} \
}
#define	PAD(howmany, with) { \
	if ((n = (howmany)) > 0) { \
		while (n > PADSIZE) { \
			PRINT (with, PADSIZE); \
			n -= PADSIZE; \
		} \
		PRINT (with, n); \
	} \
}
#define	FLUSH() { \
	if (uio.uio_resid && __SPRINT(data, fp, &uio)) \
		goto error; \
	uio.uio_iovcnt = 0; \
	iovp = iov; \
}

	/* Macros to support positional arguments */
#ifndef _NO_POS_ARGS
# define GET_ARG(n, ap, type)						\
	(is_pos_arg							\
	 ? (n < numargs							\
	    ? args[n].val_##type					\
	    : get_arg (data, n, fmt_anchor, &ap, &numargs, args,	\
		       arg_type, &saved_fmt)->val_##type)		\
	 : (arg_index++ < numargs					\
	    ? args[n].val_##type					\
	    : (numargs < MAX_POS_ARGS					\
	       ? args[numargs++].val_##type = va_arg (ap, type)		\
	       : va_arg (ap, type))))
#else
# define GET_ARG(n, ap, type) (va_arg (ap, type))
#endif

	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#ifndef _NO_LONGLONG
#define	SARG() \
	(flags&QUADINT ? GET_ARG (N, ap, quad_t) : \
	    flags&LONGINT ? GET_ARG (N, ap, long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (long)(signed char)GET_ARG (N, ap, int) : \
	    (long)GET_ARG (N, ap, int))
#define	UARG() \
	(flags&QUADINT ? GET_ARG (N, ap, u_quad_t) : \
	    flags&LONGINT ? GET_ARG (N, ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (u_long)(unsigned char)GET_ARG (N, ap, int) : \
	    (u_long)GET_ARG (N, ap, u_int))
#else
#define	SARG() \
	(flags&LONGINT ? GET_ARG (N, ap, long) : \
	    flags&SHORTINT ? (long)(short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (long)(signed char)GET_ARG (N, ap, int) : \
	    (long)GET_ARG (N, ap, int))
#define	UARG() \
	(flags&LONGINT ? GET_ARG (N, ap, u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GET_ARG (N, ap, int) : \
	    flags&CHARINT ? (u_long)(unsigned char)GET_ARG (N, ap, int) : \
	    (u_long)GET_ARG (N, ap, u_int))
#endif

#ifndef STRING_ONLY
	/* Initialize std streams if not dealing with sprintf family.  */
	CHECK_INIT (data, fp);
	_flockfile (fp);

	ORIENT(fp, -1);

	/* sorry, fprintf(read_only_file, "") returns EOF, not 0 */
	if (cantwrite (data, fp)) {
		_funlockfile (fp);
		return (EOF);
	}

	/* optimise fprintf(stderr) (and other unbuffered Unix files) */
	if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
	    fp->_file >= 0) {
		_funlockfile (fp);
		return (__sbprintf (data, fp, fmt0, ap));
	}
#else /* STRING_ONLY */
        /* Create initial buffer if we are called by asprintf family.  */
        if (fp->_flags & __SMBF && !fp->_bf._base)
        {
		fp->_bf._base = fp->_p = _malloc_r (data, 64);
		if (!fp->_p)
		{
			data->_errno = ENOMEM;
			return EOF;
		}
		fp->_bf._size = 64;
        }
#endif /* STRING_ONLY */

	fmt = (char *)fmt0;
	uio.uio_iov = iovp = iov;
	uio.uio_resid = 0;
	uio.uio_iovcnt = 0;
	ret = 0;
#ifndef _NO_POS_ARGS
	arg_index = 0;
	saved_fmt = NULL;
	arg_type[0] = -1;
	numargs = 0;
	is_pos_arg = 0;
#endif

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
	        cp = fmt;
#ifdef _MB_CAPABLE
	        while ((n = _mbtowc_r (data, &wc, fmt, MB_CUR_MAX, &state)) > 0) {
                    if (wc == '%')
                        break;
                    fmt += n;
		}
#else
                while (*fmt != '\0' && *fmt != '%')
                    fmt += 1;
#endif
		if ((m = fmt - cp) != 0) {
			PRINT (cp, m);
			ret += m;
		}
#ifdef _MB_CAPABLE
		if (n <= 0)
                    goto done;
#else
                if (*fmt == '\0')
                    goto done;
#endif
		fmt_anchor = fmt;
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = '\0';
#ifndef _NO_POS_ARGS
		N = arg_index;
		is_pos_arg = 0;
#endif

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
#ifdef _WANT_IO_C99_FORMATS
		case '\'':
		  /* The ' flag is required by POSIX, but not C99.
		     In the C locale, LC_NUMERIC requires
		     thousands_sep to be the empty string.  And since
		     no other locales are supported (yet), this flag
		     is currently a no-op.  */
		  goto rflag;
#endif
		case ' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = ' ';
			goto rflag;
		case '#':
			flags |= ALT;
			goto rflag;
		case '*':
#ifndef _NO_POS_ARGS
			/* we must check for positional arg used for dynamic width */
			n = N;
			old_is_pos_arg = is_pos_arg;
			is_pos_arg = 0;
			if (is_digit (*fmt)) {
				char *old_fmt = fmt;

				n = 0;
				ch = *fmt++;
				do {
					n = 10 * n + to_digit (ch);
					ch = *fmt++;
				} while (is_digit (ch));

				if (ch == '$') {
					if (n <= MAX_POS_ARGS) {
						n -= 1;
						is_pos_arg = 1;
					}
					else
						goto error;
				}
				else {
					fmt = old_fmt;
					goto rflag;
				}
			}
#endif /* !_NO_POS_ARGS */

			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			width = GET_ARG (n, ap, int);
#ifndef _NO_POS_ARGS
			is_pos_arg = old_is_pos_arg;
#endif
			if (width >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case '-':
			flags |= LADJUST;
			goto rflag;
		case '+':
			sign = '+';
			goto rflag;
		case '.':
			if ((ch = *fmt++) == '*') {
#ifndef _NO_POS_ARGS
				/* we must check for positional arg used for dynamic width */
				n = N;
				old_is_pos_arg = is_pos_arg;
				is_pos_arg = 0;
				if (is_digit (*fmt)) {
					char *old_fmt = fmt;

					n = 0;
					ch = *fmt++;
					do {
						n = 10 * n + to_digit (ch);
						ch = *fmt++;
					} while (is_digit (ch));

					if (ch == '$') {
						if (n <= MAX_POS_ARGS) {
							n -= 1;
							is_pos_arg = 1;
						}
						else
							goto error;
					}
					else {
						fmt = old_fmt;
						goto rflag;
					}
				}
#endif /* !_NO_POS_ARGS */
				prec = GET_ARG (n, ap, int);
#ifndef _NO_POS_ARGS
				is_pos_arg = old_is_pos_arg;
#endif
				if (prec < 0)
					prec = -1;
				goto rflag;
			}
			n = 0;
			while (is_digit (ch)) {
				n = 10 * n + to_digit (ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case '0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9':
			n = 0;
			do {
				n = 10 * n + to_digit (ch);
				ch = *fmt++;
			} while (is_digit (ch));
#ifndef _NO_POS_ARGS
			if (ch == '$') {
				if (n <= MAX_POS_ARGS) {
					N = n - 1;
					is_pos_arg = 1;
					goto rflag;
				}
				else
					goto error;
			}
#endif /* !_NO_POS_ARGS */
			width = n;
			goto reswitch;
#ifdef FLOATING_POINT
		case 'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case 'h':
#ifdef _WANT_IO_C99_FORMATS
			if (*fmt == 'h') {
				fmt++;
				flags |= CHARINT;
			} else
#endif
				flags |= SHORTINT;
			goto rflag;
		case 'l':
#if defined _WANT_IO_C99_FORMATS || !defined _NO_LONGLONG
			if (*fmt == 'l') {
				fmt++;
				flags |= QUADINT;
			} else
#endif
				flags |= LONGINT;
			goto rflag;
		case 'q': /* extension */
			flags |= QUADINT;
			goto rflag;
#ifdef _WANT_IO_C99_FORMATS
		case 'j':
		  if (sizeof (intmax_t) == sizeof (long))
		    flags |= LONGINT;
		  else
		    flags |= QUADINT;
		  goto rflag;
		case 'z':
		  if (sizeof (size_t) < sizeof (int))
		    /* POSIX states size_t is 16 or more bits, as is short.  */
		    flags |= SHORTINT;
		  else if (sizeof (size_t) == sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (size_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support size_t no wider than
		       long, but that means other environments can
		       have size_t as wide as long long.  */
		    flags |= QUADINT;
		  goto rflag;
		case 't':
		  if (sizeof (ptrdiff_t) < sizeof (int))
		    /* POSIX states ptrdiff_t is 16 or more bits, as
		       is short.  */
		    flags |= SHORTINT;
		  else if (sizeof (ptrdiff_t) == sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (ptrdiff_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support ptrdiff_t no wider than
		       long, but that means other environments can
		       have ptrdiff_t as wide as long long.  */
		    flags |= QUADINT;
		  goto rflag;
		case 'C':
#endif /* _WANT_IO_C99_FORMATS */
		case 'c':
			cp = buf;
#ifdef _MB_CAPABLE
			if (ch == 'C' || (flags & LONGINT)) {
				mbstate_t ps;

				memset ((_PTR)&ps, '\0', sizeof (mbstate_t));
				if ((size = (int)_wcrtomb_r (data, cp,
					       (wchar_t)GET_ARG (N, ap, wint_t),
						&ps)) == -1) {
					fp->_flags |= __SERR;
					goto error;
				}
			}
			else
#endif /* _MB_CAPABLE */
			{
				*cp = GET_ARG (N, ap, int);
				size = 1;
			}
			sign = '\0';
			break;
		case 'D':  /* extension */
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'd':
		case 'i':
			_uquad = SARG ();
#ifndef _NO_LONGLONG
			if ((quad_t)_uquad < 0)
#else
			if ((long) _uquad < 0)
#endif
			{

				_uquad = -_uquad;
				sign = '-';
			}
			base = DEC;
			goto number;
#ifdef FLOATING_POINT
# ifdef _WANT_IO_C99_FORMATS
		case 'a':
		case 'A':
		case 'F':
# endif
		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
# ifdef _NO_LONGDBL
			if (flags & LONGDBL) {
				_fpvalue = (double) GET_ARG (N, ap, _LONG_DOUBLE);
			} else {
				_fpvalue = GET_ARG (N, ap, double);
			}

			/* do this before tricky precision changes

			   If the output is infinite or NaN, leading
			   zeros are not permitted.  Otherwise, scanf
			   could not read what printf wrote.
			 */
			if (isinf (_fpvalue)) {
				if (_fpvalue < 0)
					sign = '-';
				if (ch <= 'G') /* 'A', 'E', 'F', or 'G' */
					cp = "INF";
				else
					cp = "inf";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
			if (isnan (_fpvalue)) {
				if (ch <= 'G') /* 'A', 'E', 'F', or 'G' */
					cp = "NAN";
				else
					cp = "nan";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}

# else /* !_NO_LONGDBL */

			if (flags & LONGDBL) {
				_fpvalue = GET_ARG (N, ap, _LONG_DOUBLE);
			} else {
				_fpvalue = (_LONG_DOUBLE)GET_ARG (N, ap, double);
			}

			/* do this before tricky precision changes */
			expt = _ldcheck (&_fpvalue);
			if (expt == 2) {
				if (_fpvalue < 0)
					sign = '-';
				if (ch <= 'G') /* 'A', 'E', 'F', or 'G' */
					cp = "INF";
				else
					cp = "inf";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
			if (expt == 1) {
				if (ch <= 'G') /* 'A', 'E', 'F', or 'G' */
					cp = "NAN";
				else
					cp = "nan";
				size = 3;
				flags &= ~ZEROPAD;
				break;
			}
# endif /* !_NO_LONGDBL */

# ifdef _WANT_IO_C99_FORMATS
			if (ch == 'a' || ch == 'A') {
				ox[0] = '0';
				ox[1] = ch == 'a' ? 'x' : 'X';
				flags |= HEXPREFIX;
				if (prec >= BUF)
				  {
				    if ((malloc_buf =
					 (char *)_malloc_r (data, prec + 1))
					== NULL)
				      {
					fp->_flags |= __SERR;
					goto error;
				      }
				    cp = malloc_buf;
				  }
				else
				  cp = buf;
			} else
# endif /* _WANT_IO_C99_FORMATS */
			if (prec == -1) {
				prec = DEFPREC;
			} else if ((ch == 'g' || ch == 'G') && prec == 0) {
				prec = 1;
			}

			flags |= FPT;

			cp = cvt (data, _fpvalue, prec, flags, &softsign,
				  &expt, ch, &ndig, cp);

			if (ch == 'g' || ch == 'G') {
				if (expt <= -4 || expt > prec)
					ch -= 2; /* 'e' or 'E' */
				else
					ch = 'g';
			}
# ifdef _WANT_IO_C99_FORMATS
			else if (ch == 'F')
				ch = 'f';
# endif
			if (ch <= 'e') {	/* 'a', 'A', 'e', or 'E' fmt */
				--expt;
				expsize = exponent (expstr, expt, ch);
				size = expsize + ndig;
				if (ndig > 1 || flags & ALT)
					++size;
			} else if (ch == 'f') {		/* f fmt */
				if (expt > 0) {
					size = expt;
					if (prec || flags & ALT)
						size += prec + 1;
				} else	/* "0.X" */
					size = (prec || flags & ALT)
						  ? prec + 2
						  : 1;
			} else if (expt >= ndig) {	/* fixed g fmt */
				size = expt;
				if (flags & ALT)
					++size;
			} else
				size = ndig + (expt > 0 ?
					1 : 2 - expt);

			if (softsign)
				sign = '-';
			break;
#endif /* FLOATING_POINT */
		case 'n':
#ifndef _NO_LONGLONG
			if (flags & QUADINT)
				*GET_ARG (N, ap, quad_ptr_t) = ret;
			else
#endif
			if (flags & LONGINT)
				*GET_ARG (N, ap, long_ptr_t) = ret;
			else if (flags & SHORTINT)
				*GET_ARG (N, ap, short_ptr_t) = ret;
#ifdef _WANT_IO_C99_FORMATS
			else if (flags & CHARINT)
				*GET_ARG (N, ap, char_ptr_t) = ret;
#endif
			else
				*GET_ARG (N, ap, int_ptr_t) = ret;
			continue;	/* no output */
		case 'O': /* extension */
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'o':
			_uquad = UARG ();
			base = OCT;
			goto nosign;
		case 'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
			/* NOSTRICT */
			_uquad = (uintptr_t) GET_ARG (N, ap, void_ptr_t);
			base = HEX;
			xdigs = "0123456789abcdef";
			flags |= HEXPREFIX;
			ox[0] = '0';
			ox[1] = ch = 'x';
			goto nosign;
		case 's':
#ifdef _WANT_IO_C99_FORMATS
		case 'S':
#endif
			sign = '\0';
			cp = GET_ARG (N, ap, char_ptr_t);
#ifndef __OPTIMIZE_SIZE__
			/* Behavior is undefined if the user passed a
			   NULL string when precision is not 0.
			   However, if we are not optimizing for size,
			   we might as well mirror glibc behavior.  */
			if (cp == NULL) {
				cp = "(null)";
				size = ((unsigned) prec > 6U) ? 6 : prec;
			}
			else
#endif /* __OPTIMIZE_SIZE__ */
#ifdef _MB_CAPABLE
			if (ch == 'S' || (flags & LONGINT)) {
				mbstate_t ps;
				_CONST wchar_t *wcp;

				wcp = (_CONST wchar_t *)cp;
				size = m = 0;
				memset ((_PTR)&ps, '\0', sizeof (mbstate_t));

				/* Count number of bytes needed for multibyte
				   string that will be produced from widechar
				   string.  */
				if (prec >= 0) {
					while (1) {
						if (wcp[m] == L'\0')
							break;
						if ((n = (int)_wcrtomb_r (data,
						     buf, wcp[m], &ps)) == -1) {
							fp->_flags |= __SERR;
							goto error;
						}
						if (n + size > prec)
							break;
						m += 1;
						size += n;
						if (size == prec)
							break;
					}
				}
				else {
					if ((size = (int)_wcsrtombs_r (data,
						   NULL, &wcp, 0, &ps)) == -1) {
						fp->_flags |= __SERR;
						goto error;
					}
					wcp = (_CONST wchar_t *)cp;
				}

				if (size == 0)
					break;

				if (size >= BUF) {
					if ((malloc_buf =
					     (char *)_malloc_r (data, size + 1))
					    == NULL) {
						fp->_flags |= __SERR;
						goto error;
					}
					cp = malloc_buf;
				} else
					cp = buf;

				/* Convert widechar string to multibyte string. */
				memset ((_PTR)&ps, '\0', sizeof (mbstate_t));
				if (_wcsrtombs_r (data, cp, &wcp, size, &ps)
				    != size) {
					fp->_flags |= __SERR;
					goto error;
				}
				cp[size] = '\0';
			}
			else
#endif /* _MB_CAPABLE */
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen () will go further.
				 */
				char *p = memchr (cp, 0, prec);

				if (p != NULL) {
					size = p - cp;
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = strlen (cp);

			break;
		case 'U': /* extension */
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case 'u':
			_uquad = UARG ();
			base = DEC;
			goto nosign;
		case 'X':
			xdigs = "0123456789ABCDEF";
			goto hex;
		case 'x':
			xdigs = "0123456789abcdef";
hex:			_uquad = UARG ();
			base = HEX;
			/* leading 0x/X only if non-zero */
			if (flags & ALT && _uquad != 0) {
				ox[0] = '0';
				ox[1] = ch;
				flags |= HEXPREFIX;
			}

			/* unsigned conversions */
nosign:			sign = '\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (_uquad != 0 || prec != 0) {
				/*
				 * Unsigned mod is hard, and unsigned mod
				 * by a constant is easier than that by
				 * a variable; hence this switch.
				 */
				switch (base) {
				case OCT:
					do {
						*--cp = to_char (_uquad & 7);
						_uquad >>= 3;
					} while (_uquad);
					/* handle octal leading 0 */
					if (flags & ALT && *cp != '0')
						*--cp = '0';
					break;

				case DEC:
					/* many numbers are 1 digit */
					while (_uquad >= 10) {
						*--cp = to_char (_uquad % 10);
						_uquad /= 10;
					}
					*--cp = to_char (_uquad);
					break;

				case HEX:
					do {
						*--cp = xdigs[_uquad & 15];
						_uquad >>= 4;
					} while (_uquad);
					break;

				default:
					cp = "bug in vfprintf: bad base";
					size = strlen (cp);
					goto skipsize;
				}
			}
                       /*
			* ...result is to be converted to an 'alternate form'.
			* For o conversion, it increases the precision to force
			* the first digit of the result to be a zero."
			*     -- ANSI X3J11
			*
			* To demonstrate this case, compile and run:
                        *    printf ("%#.0o",0);
			*/
                       else if (base == OCT && (flags & ALT))
                         *--cp = '0';

			size = buf + BUF - cp;
		skipsize:
			break;
		default:	/* "%?" prints ?, unless ? is NUL */
			if (ch == '\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = '\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 * If flags&FPT, ch must be in [aAeEfg].
		 *
		 * Compute actual size, so we know how much to pad.
		 * size excludes decimal prec; realsz includes it.
		 */
		realsz = dprec > size ? dprec : size;
		if (sign)
			realsz++;
		if (flags & HEXPREFIX)
			realsz+= 2;

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			PAD (width - realsz, blanks);

		/* prefix */
		if (sign)
			PRINT (&sign, 1);
		if (flags & HEXPREFIX)
			PRINT (ox, 2);

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			PAD (width - realsz, zeroes);

		/* leading zeroes from decimal precision */
		PAD (dprec - size, zeroes);

		/* the string or number proper */
#ifdef FLOATING_POINT
		if ((flags & FPT) == 0) {
			PRINT (cp, size);
		} else {	/* glue together f_p fragments */
			if (ch >= 'f') {	/* 'f' or 'g' */
				if (_fpvalue == 0) {
					/* kludge for __dtoa irregularity */
					PRINT ("0", 1);
					if (expt < ndig || flags & ALT) {
						PRINT (decimal_point, 1);
						PAD (ndig - 1, zeroes);
					}
				} else if (expt <= 0) {
					PRINT ("0", 1);
					if (expt || ndig || flags & ALT) {
						PRINT (decimal_point, 1);
						PAD (-expt, zeroes);
						PRINT (cp, ndig);
					}
				} else if (expt >= ndig) {
					PRINT (cp, ndig);
					PAD (expt - ndig, zeroes);
					if (flags & ALT)
						PRINT (decimal_point, 1);
				} else {
					PRINT (cp, expt);
					cp += expt;
					PRINT (decimal_point, 1);
					PRINT (cp, ndig - expt);
				}
			} else {	/* 'a', 'A', 'e', or 'E' */
				if (ndig > 1 || flags & ALT) {
					PRINT (cp, 1);
					cp++;
					PRINT (decimal_point, 1);
					if (_fpvalue) {
						PRINT (cp, ndig - 1);
					} else	/* 0.[0..] */
						/* __dtoa irregularity */
						PAD (ndig - 1, zeroes);
				} else	/* XeYYY */
					PRINT (cp, 1);
				PRINT (expstr, expsize);
			}
		}
#else /* !FLOATING_POINT */
		PRINT (cp, size);
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			PAD (width - realsz, blanks);

		/* finally, adjust ret */
		ret += width > realsz ? width : realsz;

		FLUSH ();	/* copy out the I/O vectors */

                if (malloc_buf != NULL) {
			_free_r (data, malloc_buf);
			malloc_buf = NULL;
		}
	}
done:
	FLUSH ();
error:
	if (malloc_buf != NULL)
		_free_r (data, malloc_buf);
#ifndef STRING_ONLY
	_funlockfile (fp);
#endif
	return (__sferror (fp) ? EOF : ret);
	/* NOTREACHED */
}

#ifdef FLOATING_POINT

/* Using reentrant DATA, convert finite VALUE into a string of digits
   with no decimal point, using NDIGITS precision and FLAGS as guides
   to whether trailing zeros must be included.  Set *SIGN to nonzero
   if VALUE was negative.  Set *DECPT to the exponent plus one.  Set
   *LENGTH to the length of the returned string.  CH must be one of
   [aAeEfFgG]; if it is [aA], then the return string lives in BUF,
   otherwise the return value shares the mprec reentrant storage.  */
static char *
cvt(struct _reent *data, _PRINTF_FLOAT_TYPE value, int ndigits, int flags,
    char *sign, int *decpt, int ch, int *length, char *buf)
{
	int mode, dsgn;
	char *digits, *bp, *rve;
# ifdef _NO_LONGDBL
	union double_union tmp;

	tmp.d = value;
	if (word0 (tmp) & Sign_bit) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = '-';
	} else
		*sign = '\000';
# else /* !_NO_LONGDBL */
	union
	{
	  struct ldieee ieee;
	  _LONG_DOUBLE val;
	} ld;

	ld.val = value;
	if (ld.ieee.sign) { /* this will check for < 0 and -0.0 */
		value = -value;
		*sign = '-';
	} else
		*sign = '\000';
# endif /* !_NO_LONGDBL */

# ifdef _WANT_IO_C99_FORMATS
	if (ch == 'a' || ch == 'A') {
		/* This code assumes FLT_RADIX is a power of 2.  The initial
		   division ensures the digit before the decimal will be less
		   than FLT_RADIX (unless it is rounded later).	 There is no
		   loss of precision in these calculations.  */
		value = FREXP (value, decpt) / 8;
		if (!value)
			*decpt = 1;
		digits = ch == 'a' ? "0123456789abcdef" : "0123456789ABCDEF";
		bp = buf;
		do {
			value *= 16;
			mode = (int) value;
			value -= mode;
			*bp++ = digits[mode];
		} while (ndigits-- && value);
		if (value > 0.5 || (value == 0.5 && mode & 1)) {
			/* round to even */
			rve = bp;
			while (*--rve == digits[0xf]) {
				*rve = '0';
			}
			*rve = *rve == '9' ? digits[0xa] : *rve + 1;
		} else {
			while (ndigits-- >= 0) {
				*bp++ = '0';
			}
		}
		*length = bp - buf;
		return buf;
	}
# endif /* _WANT_IO_C99_FORMATS */
	if (ch == 'f' || ch == 'F') {
		mode = 3;		/* ndigits after the decimal point */
	} else {
		/* To obtain ndigits after the decimal point for the 'e'
		 * and 'E' formats, round to ndigits + 1 significant
		 * figures.
		 */
		if (ch == 'e' || ch == 'E') {
			ndigits++;
		}
		mode = 2;		/* ndigits significant digits */
	}

	digits = _DTOA_R (data, value, mode, ndigits, decpt, &dsgn, &rve);

	if ((ch != 'g' && ch != 'G') || flags & ALT) {	/* Print trailing zeros */
		bp = digits + ndigits;
		if (ch == 'f' || ch == 'F') {
			if (*digits == '0' && value)
				*decpt = -ndigits + 1;
			bp += *decpt;
		}
		if (value == 0)	/* kludge for __dtoa irregularity */
			rve = bp;
		while (rve < bp)
			*rve++ = '0';
	}
	*length = rve - digits;
	return (digits);
}

static int
exponent(char *p0, int exp, int fmtch)
{
	register char *p, *t;
	char expbuf[MAXEXPLEN];
# ifdef _WANT_IO_C99_FORMATS
	int isa = fmtch == 'a' || fmtch == 'A';
# else
#  define isa 0
# endif

	p = p0;
	*p++ = isa ? 'p' - 'a' + fmtch : fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = '-';
	}
	else
		*p++ = '+';
	t = expbuf + MAXEXPLEN;
	if (exp > 9) {
		do {
			*--t = to_char (exp % 10);
		} while ((exp /= 10) > 9);
		*--t = to_char (exp);
		for (; t < expbuf + MAXEXPLEN; *p++ = *t++);
	}
	else {
		if (!isa)
			*p++ = '0';
		*p++ = to_char (exp);
	}
	return (p - p0);
}
#endif /* FLOATING_POINT */


#ifndef _NO_POS_ARGS

/* Positional argument support.
   Written by Jeff Johnston

   Copyright (c) 2002 Red Hat Incorporated.
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

      Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

      The name of Red Hat Incorporated may not be used to endorse
      or promote products derived from this software without specific
      prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED.  IN NO EVENT SHALL RED HAT INCORPORATED BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

/* The below constant state tables are shared between all versions of
   vfprintf and vfwprintf.  They must only be defined once, which we do in
   the STRING_ONLY/INTEGER_ONLY versions here. */
#if defined (STRING_ONLY) && defined(INTEGER_ONLY)

_CONST __CH_CLASS __chclass[256] = {
  /* 00-07 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 08-0f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 10-17 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 18-1f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 20-27 */  FLAG,    OTHER,   OTHER,   FLAG,    DOLLAR,  OTHER,   OTHER,   FLAG,
  /* 28-2f */  OTHER,   OTHER,   STAR,    FLAG,    OTHER,   FLAG,    DOT,     OTHER,
  /* 30-37 */  ZERO,    DIGIT,   DIGIT,   DIGIT,   DIGIT,   DIGIT,   DIGIT,   DIGIT,
  /* 38-3f */  DIGIT,   DIGIT,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 40-47 */  OTHER,   SPEC,    OTHER,   SPEC,    SPEC,    SPEC,    SPEC,    SPEC,
  /* 48-4f */  OTHER,   OTHER,   OTHER,   OTHER,   MODFR,   OTHER,   OTHER,   SPEC,
  /* 50-57 */  OTHER,   OTHER,   OTHER,   SPEC,    OTHER,   SPEC,    OTHER,   OTHER,
  /* 58-5f */  SPEC,    OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 60-67 */  OTHER,   SPEC,    OTHER,   SPEC,    SPEC,    SPEC,    SPEC,    SPEC,
  /* 68-6f */  MODFR,   SPEC,    MODFR,   OTHER,   MODFR,   OTHER,   SPEC,    SPEC,
  /* 70-77 */  SPEC,    MODFR,   OTHER,   SPEC,    MODFR,   SPEC,    OTHER,   OTHER,
  /* 78-7f */  SPEC,    OTHER,   MODFR,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 80-87 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 88-8f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 90-97 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* 98-9f */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* a0-a7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* a8-af */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* b0-b7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* b8-bf */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* c0-c7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* c8-cf */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* d0-d7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* d8-df */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* e0-e7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* e8-ef */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* f0-f7 */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
  /* f8-ff */  OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,   OTHER,
};

_CONST __STATE __state_table[MAX_STATE][MAX_CH_CLASS] = {
  /*             '0'     '1-9'     '$'     MODFR    SPEC    '.'     '*'    FLAG    OTHER */
  /* START */  { SFLAG,   WDIG,    DONE,   SMOD,    DONE,   SDOT,  VARW,   SFLAG,  DONE },
  /* SFLAG */  { SFLAG,   WDIG,    DONE,   SMOD,    DONE,   SDOT,  VARW,   SFLAG,  DONE },
  /* WDIG  */  { DONE,    DONE,    WIDTH,  SMOD,    DONE,   SDOT,  DONE,   DONE,   DONE },
  /* WIDTH */  { DONE,    DONE,    DONE,   SMOD,    DONE,   SDOT,  DONE,   DONE,   DONE },
  /* SMOD  */  { DONE,    DONE,    DONE,   DONE,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* SDOT  */  { SDOT,    PREC,    DONE,   SMOD,    DONE,   DONE,  VARP,   DONE,   DONE },
  /* VARW  */  { DONE,    VWDIG,   DONE,   SMOD,    DONE,   SDOT,  DONE,   DONE,   DONE },
  /* VARP  */  { DONE,    VPDIG,   DONE,   SMOD,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* PREC  */  { DONE,    DONE,    DONE,   SMOD,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* VWDIG */  { DONE,    DONE,    WIDTH,  DONE,    DONE,   DONE,  DONE,   DONE,   DONE },
  /* VPDIG */  { DONE,    DONE,    PREC,   DONE,    DONE,   DONE,  DONE,   DONE,   DONE },
};

_CONST __ACTION __action_table[MAX_STATE][MAX_CH_CLASS] = {
  /*             '0'     '1-9'     '$'     MODFR    SPEC    '.'     '*'    FLAG    OTHER */
  /* START */  { NOOP,    NUMBER,  NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* SFLAG */  { NOOP,    NUMBER,  NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* WDIG  */  { NOOP,    NOOP,    GETPOS, GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* WIDTH */  { NOOP,    NOOP,    NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* SMOD  */  { NOOP,    NOOP,    NOOP,   NOOP,    GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* SDOT  */  { NOOP,    SKIPNUM, NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* VARW  */  { NOOP,    NUMBER,  NOOP,   GETPW,   GETPWB, GETPW, NOOP,   NOOP,   NOOP },
  /* VARP  */  { NOOP,    NUMBER,  NOOP,   GETPW,   GETPWB, NOOP,  NOOP,   NOOP,   NOOP },
  /* PREC  */  { NOOP,    NOOP,    NOOP,   GETMOD,  GETARG, NOOP,  NOOP,   NOOP,   NOOP },
  /* VWDIG */  { NOOP,    NOOP,    PWPOS,  NOOP,    NOOP,   NOOP,  NOOP,   NOOP,   NOOP },
  /* VPDIG */  { NOOP,    NOOP,    PWPOS,  NOOP,    NOOP,   NOOP,  NOOP,   NOOP,   NOOP },
};

#endif /* STRING_ONLY && INTEGER_ONLY */

/* function to get positional parameter N where n = N - 1 */
static union arg_val *
_DEFUN(get_arg, (data, n, fmt, ap, numargs_p, args, arg_type, last_fmt),
       struct _reent *data _AND
       int n               _AND
       char *fmt           _AND
       va_list *ap         _AND
       int *numargs_p      _AND
       union arg_val *args _AND
       int *arg_type       _AND
       char **last_fmt)
{
  int ch;
  int number, flags;
  int spec_type;
  int numargs = *numargs_p;
  __CH_CLASS chtype;
  __STATE state, next_state;
  __ACTION action;
  int pos, last_arg;
  int max_pos_arg = n;
  /* Only need types that can be reached via vararg promotions.  */
  enum types { INT, LONG_INT, QUAD_INT, CHAR_PTR, DOUBLE, LONG_DOUBLE, WIDE_CHAR };
# ifdef _MB_CAPABLE
  wchar_t wc;
  mbstate_t wc_state;
  int nbytes;
# endif

  /* if this isn't the first call, pick up where we left off last time */
  if (*last_fmt != NULL)
    fmt = *last_fmt;

# ifdef _MB_CAPABLE
  memset (&wc_state, '\0', sizeof (wc_state));
# endif

  /* we need to process either to end of fmt string or until we have actually
     read the desired parameter from the vararg list. */
  while (*fmt && n >= numargs)
    {
# ifdef _MB_CAPABLE
      while ((nbytes = _mbtowc_r (data, &wc, fmt, MB_CUR_MAX, &wc_state)) > 0)
	{
	  fmt += nbytes;
	  if (wc == '%')
	    break;
	}

      if (nbytes <= 0)
	break;
# else
      while (*fmt != '\0' && *fmt != '%')
	fmt += 1;

      if (*fmt == '\0')
	break;
# endif /* ! _MB_CAPABLE */
      state = START;
      flags = 0;
      pos = -1;
      number = 0;
      spec_type = INT;

      /* Use state/action table to process format specifiers.  We ignore invalid
         formats and we are only interested in information that tells us how to
         read the vararg list. */
      while (state != DONE)
	{
	  ch = *fmt++;
	  chtype = __chclass[ch];
	  next_state = __state_table[state][chtype];
	  action = __action_table[state][chtype];
	  state = next_state;

	  switch (action)
	    {
	    case GETMOD:  /* we have format modifier */
	      switch (ch)
		{
		case 'h':
		  /* No flag needed, since short and char promote to int.  */
		  break;
		case 'L':
		  flags |= LONGDBL;
		  break;
		case 'q':
		  flags |= QUADINT;
		  break;
# ifdef _WANT_IO_C99_FORMATS
		case 'j':
		  if (sizeof (intmax_t) == sizeof (long))
		    flags |= LONGINT;
		  else
		    flags |= QUADINT;
		  break;
		case 'z':
		  if (sizeof (size_t) <= sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (size_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support size_t no wider than
		       long, but that means other environments can
		       have size_t as wide as long long.  */
		    flags |= QUADINT;
		  break;
		case 't':
		  if (sizeof (ptrdiff_t) <= sizeof (int))
		    /* no flag needed */;
		  else if (sizeof (ptrdiff_t) <= sizeof (long))
		    flags |= LONGINT;
		  else
		    /* POSIX states that at least one programming
		       environment must support ptrdiff_t no wider than
		       long, but that means other environments can
		       have ptrdiff_t as wide as long long.  */
		    flags |= QUADINT;
		  break;
# endif /* _WANT_IO_C99_FORMATS */
		case 'l':
		default:
# if defined _WANT_IO_C99_FORMATS || !defined _NO_LONGLONG
		  if (*fmt == 'l')
		    {
		      flags |= QUADINT;
		      ++fmt;
		    }
		  else
# endif
		    flags |= LONGINT;
		  break;
		}
	      break;
	    case GETARG: /* we have format specifier */
	      {
		numargs &= (MAX_POS_ARGS - 1);
		/* process the specifier and translate it to a type to fetch from varargs */
		switch (ch)
		  {
		  case 'd':
		  case 'i':
		  case 'o':
		  case 'x':
		  case 'X':
		  case 'u':
		    if (flags & LONGINT)
		      spec_type = LONG_INT;
# ifndef _NO_LONGLONG
		    else if (flags & QUADINT)
		      spec_type = QUAD_INT;
# endif
		    else
		      spec_type = INT;
		    break;
		  case 'D':
		  case 'U':
		  case 'O':
		    spec_type = LONG_INT;
		    break;
# ifdef _WANT_IO_C99_FORMATS
		  case 'a':
		  case 'A':
		  case 'F':
# endif
		  case 'f':
		  case 'g':
		  case 'G':
		  case 'E':
		  case 'e':
# ifndef _NO_LONGDBL
		    if (flags & LONGDBL)
		      spec_type = LONG_DOUBLE;
		    else
# endif
		      spec_type = DOUBLE;
		    break;
		  case 's':
# ifdef _WANT_IO_C99_FORMATS
		  case 'S':
# endif
		  case 'p':
		  case 'n':
		    spec_type = CHAR_PTR;
		    break;
		  case 'c':
# ifdef _WANT_IO_C99_FORMATS
		    if (flags & LONGINT)
		      spec_type = WIDE_CHAR;
		    else
# endif
		      spec_type = INT;
		    break;
# ifdef _WANT_IO_C99_FORMATS
		  case 'C':
		    spec_type = WIDE_CHAR;
		    break;
# endif
		  }

		/* if we have a positional parameter, just store the type, otherwise
		   fetch the parameter from the vararg list */
		if (pos != -1)
		  arg_type[pos] = spec_type;
		else
		  {
		    switch (spec_type)
		      {
		      case LONG_INT:
			args[numargs++].val_long = va_arg (*ap, long);
			break;
		      case QUAD_INT:
			args[numargs++].val_quad_t = va_arg (*ap, quad_t);
			break;
		      case WIDE_CHAR:
			args[numargs++].val_wint_t = va_arg (*ap, wint_t);
			break;
		      case INT:
			args[numargs++].val_int = va_arg (*ap, int);
			break;
		      case CHAR_PTR:
			args[numargs++].val_char_ptr_t = va_arg (*ap, char *);
			break;
		      case DOUBLE:
			args[numargs++].val_double = va_arg (*ap, double);
			break;
		      case LONG_DOUBLE:
			args[numargs++].val__LONG_DOUBLE = va_arg (*ap, _LONG_DOUBLE);
			break;
		      }
		  }
	      }
	      break;
	    case GETPOS: /* we have positional specifier */
	      if (arg_type[0] == -1)
		memset (arg_type, 0, sizeof (int) * MAX_POS_ARGS);
	      pos = number - 1;
	      max_pos_arg = (max_pos_arg > pos ? max_pos_arg : pos);
	      break;
	    case PWPOS:  /* we have positional specifier for width or precision */
	      if (arg_type[0] == -1)
		memset (arg_type, 0, sizeof (int) * MAX_POS_ARGS);
	      number -= 1;
	      arg_type[number] = INT;
	      max_pos_arg = (max_pos_arg > number ? max_pos_arg : number);
	      break;
	    case GETPWB: /* we require format pushback */
	      --fmt;
	      /* fallthrough */
	    case GETPW:  /* we have a variable precision or width to acquire */
	      args[numargs++].val_int = va_arg (*ap, int);
	      break;
	    case NUMBER: /* we have a number to process */
	      number = (ch - '0');
	      while ((ch = *fmt) != '\0' && is_digit (ch))
		{
		  number = number * 10 + (ch - '0');
		  ++fmt;
		}
	      break;
	    case SKIPNUM: /* we have a number to skip */
	      while ((ch = *fmt) != '\0' && is_digit (ch))
		++fmt;
	      break;
	    case NOOP:
	    default:
	      break; /* do nothing */
	    }
	}
    }

  /* process all arguments up to at least the one we are looking for and if we
     have seen the end of the string, then process up to the max argument needed */
  if (*fmt == '\0')
    last_arg = max_pos_arg;
  else
    last_arg = n;

  while (numargs <= last_arg)
    {
      switch (arg_type[numargs])
	{
	case LONG_INT:
	  args[numargs++].val_long = va_arg (*ap, long);
	  break;
	case QUAD_INT:
	  args[numargs++].val_quad_t = va_arg (*ap, quad_t);
	  break;
	case CHAR_PTR:
	  args[numargs++].val_char_ptr_t = va_arg (*ap, char *);
	  break;
	case DOUBLE:
	  args[numargs++].val_double = va_arg (*ap, double);
	  break;
	case LONG_DOUBLE:
	  args[numargs++].val__LONG_DOUBLE = va_arg (*ap, _LONG_DOUBLE);
	  break;
	case WIDE_CHAR:
	  args[numargs++].val_wint_t = va_arg (*ap, wint_t);
	  break;
	case INT:
	default:
	  args[numargs++].val_int = va_arg (*ap, int);
	  break;
	}
    }

  /* alter the global numargs value and keep a reference to the last bit of the fmt
     string we processed here because the caller will continue processing where we started */
  *numargs_p = numargs;
  *last_fmt = fmt;
  return &args[n];
}
#endif /* !_NO_POS_ARGS */

/*-
 * Code created by modifying scanf.c which has following copyright.
 *
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
 */

#if 0
#include <_ansi.h>
#include <reent.h>
#include <stdio.h>
#ifdef _HAVE_STDC
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#include "local.h"
#endif

#ifndef _REENT_ONLY

int
_DEFUN(vscanf, (fmt, ap),
       _CONST char *fmt _AND
       va_list ap)
{
  _REENT_SMALL_CHECK_INIT (_REENT);
  return __svfscanf_r (_REENT, _stdin_r (_REENT), fmt, ap);
}

#endif /* !_REENT_ONLY */

int
_DEFUN(_vscanf_r, (ptr, fmt, ap),
       struct _reent *ptr _AND
       _CONST char *fmt   _AND
       va_list ap)
{
  _REENT_SMALL_CHECK_INIT (ptr);
  return __svfscanf_r (ptr, _stdin_r (ptr), fmt, ap);
}

#endif // __unix__

// vi:set ts=8 sw=8:
