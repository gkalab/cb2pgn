// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
// Copyright: (C) 2009-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "m_istream.h"
#include "m_string.h"
#include "m_stdio.h"
#include "m_assert.h"

#include <stdlib.h>
#include <string.h>


#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#ifndef SSIZE_MAX
# define SSIZE_MAX ((ssize_t) (SIZE_MAX / 2))
#endif

#if USE_UNLOCKED_IO
# include "unlocked-io.h"
# define getc_maybe_unlocked(fp)	getc(fp)
#elif !HAVE_FLOCKFILE || !HAVE_FUNLOCKFILE || !HAVE_DECL_GETC_UNLOCKED
# undef flockfile
# undef funlockfile
# define flockfile(x) ((void) 0)
# define funlockfile(x) ((void) 0)
# define getc_maybe_unlocked(fp)	getc(fp)
#else
# define getc_maybe_unlocked(fp)	getc_unlocked(fp)
#endif

using namespace mstl;


istream::istream()
	:m_data(0)
	,m_size(0)
{
}


istream::~istream() throw()
{
	::free(m_data);
}


istream&
istream::get(char& c)
{
	int ch = fgetc(m_fp);

	if (ch == EOF)
		setstate(feof(m_fp) ? failbit | eofbit : badbit);
	else
		c = ch;

	return *this;
}


istream&
istream::getline(char* buf, size_t n)
{
	switch (n)
	{
		case 0:
			setstate(failbit);
			break;

		case 1:
			buf[0] = '\0';

			if (!fgets(buf, n, m_fp))
				setstate(feof(m_fp) ? failbit | eofbit : badbit);
			else if (buf[0] == '\n')
				buf[0] = '\0';
			else
				setstate(failbit);
			break;

		default:
			buf[0] = '\0';
			buf[n - 1] = '\n';

			if (!fgets(buf, n, m_fp))
				setstate(feof(m_fp) ? failbit | eofbit : badbit);
			else if (buf[0] == '\n')
				buf[0] = '\0';
			else if (buf[n - 1] == '\0' && buf[n - 2] != '\n')
				setstate(failbit);
			break;
	}

	return *this;
}

/* Read up to (and including) a DELIMITER from FP into *LINEPTR (and
   NUL-terminate it).  *LINEPTR is a pointer returned from malloc (or
   NULL), pointing to *N characters of space.  It is realloc'ed as
   necessary.  Returns the number of characters read (not including
   the null terminator), or -1 on error or EOF.  */

ssize_t
istream::getdelim (char **lineptr, size_t *n, int delimiter, FILE *fp)
{
  ssize_t result;
  size_t cur_len = 0;

  if (lineptr == NULL || n == NULL || fp == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  flockfile (fp);

  if (*lineptr == NULL || *n == 0)
    {
      char *new_lineptr;
      *n = 120;
      new_lineptr = (char *) realloc (*lineptr, *n);
      if (new_lineptr == NULL)
	{
	  result = -1;
	  goto unlock_return;
	}
      *lineptr = new_lineptr;
    }

  for (;;)
    {
      int i;

      i = getc_maybe_unlocked (fp);
      if (i == EOF)
	{
	  result = -1;
	  break;
	}

      /* Make enough space for len+1 (for final NUL) bytes.  */
      if (cur_len + 1 >= *n)
	{
	  size_t needed_max =
	    SSIZE_MAX < SIZE_MAX ? (size_t) SSIZE_MAX + 1 : SIZE_MAX;
	  size_t needed = 2 * *n + 1;   /* Be generous. */
	  char *new_lineptr;

	  if (needed_max < needed)
	    needed = needed_max;
	  if (cur_len + 1 >= needed)
	    {
	      result = -1;
	      errno = EOVERFLOW;
	      goto unlock_return;
	    }

	  new_lineptr = (char *) realloc (*lineptr, needed);
	  if (new_lineptr == NULL)
	    {
	      result = -1;
	      goto unlock_return;
	    }

	  *lineptr = new_lineptr;
	  *n = needed;
	}

      (*lineptr)[cur_len] = i;
      cur_len++;

      if (i == delimiter)
	break;
    }
  (*lineptr)[cur_len] = '\0';
  result = cur_len ? cur_len : result;

 unlock_return:
  funlockfile (fp); /* doesn't set errno */

  return result;
}

ssize_t
istream::getline (char **lineptr, size_t *n, FILE *stream)
{
  return getdelim (lineptr, n, '\n', stream);
}

istream&
istream::getline(string& buf)
{
	long size = getline(&m_data, &m_size, m_fp);

	if (size == -1)
	{
		setstate(feof(m_fp) ? failbit | eofbit : badbit);
	}
	else if (size == 0)
	{
		buf.clear();
	}
	else if (m_data[size - 1] == '\n')
	{
		m_data[size - 1] = '\0';
		buf.hook(m_data, size - 1);
	}
	else
	{
		buf.hook(m_data, size);
	}

	return *this;
}


istream&
istream::get(string& buf)
{
	long size = getdelim(&m_data, &m_size, '\0', m_fp);

	if (size == -1)
	{
		setstate(feof(m_fp) ? failbit | eofbit : badbit);
	}
	else if (size == 0)
	{
		buf.clear();
	}
	else if (m_data[size - 1] == '\0')
	{
		m_data[size - 1] = '\0';
		buf.hook(m_data, size - 1);
	}
	else
	{
		buf.hook(m_data, size);
	}

	return *this;
}


bool
istream::eof()
{
	if (ios_base::eof())
		return true;

	if (!feof(m_fp))
		return false;

	setstate(eofbit);
	return true;
}


istream&
istream::read(char* buf, size_t n)
{
	size_t bytes_read = fread(buf, 1, n, m_fp);

	if (bytes_read < n)
		setstate(feof(m_fp) ? failbit | eofbit : badbit);

	return *this;
}


size_t
istream::readsome(char* buf, size_t n)
{
	if (eof())
		return 0;

	size_t bytes_read = fread(buf, 1, n, m_fp);

	if (bytes_read < n && !feof(m_fp))
		setstate(badbit);

	return bytes_read;
}


istream&
istream::ignore(unsigned long n, int delim)
{
	if (delim != traits::eof || fseek(m_fp, n, SEEK_CUR) != 0)
	{
		while (n--)
		{
			if (fgetc(m_fp) == delim)
				return *this;
		}
	}

	return *this;
}


int
istream::get()
{
	return fgetc(m_fp);
}


int
istream::peek()
{
	int c = fgetc(m_fp);

	if (c != EOF)
		::ungetc(c, m_fp);

	return c;
}


istream&
istream::putback(char c)
{
	if (::ungetc(c, m_fp) == EOF)
		setstate(failbit);

	return *this;
}


istream&
istream::unget()
{
	if (fseek(m_fp, -1, SEEK_CUR) == -1)
		setstate(failbit);

	return *this;
}


uint64_t
istream::tellg()
{
	return ftell(m_fp);
}


istream&
istream::seekg(uint64_t offset)
{
	if (fseek(m_fp, offset, SEEK_CUR) == -1)
		setstate(failbit);

	return *this;
}


istream&
istream::seekg(int64_t offset, seekdir dir)
{
	if (fseek(m_fp, offset, fdir(dir)) == -1)
		setstate(failbit);

	return *this;
}


int64_t
istream::size() const
{
	return -1;
}


uint64_t
istream::goffset()
{
	return tellg();
}

// vi:set ts=3 sw=3:
