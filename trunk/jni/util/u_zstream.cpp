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

extern "C" {
#include "extrastdio.h"
}
#include "u_zstream.h"
#include "u_byte_stream.h"
#include "u_misc.h"

#include "m_ifstream.h"
#include "m_stdio.h"
#include "m_assert.h"

#include <zzip.h>
#include <zlib.h>
#include <zip.h>

#include <string.h>

#ifndef MAX
# define MAX(a,b) ((a) < (b) ? b : a)
#endif

using namespace util;


#define ZIP_FILE	static_cast<ZStream::Handle*>(cookie)->file
#define ZIP_DIR	static_cast<ZStream::Handle*>(cookie)->dir
#define HANDLE		static_cast<ZStream::Handle*>(cookie)->handle

#define IS_READABLE	(static_cast<ZStream::Handle*>(cookie)->mode & mstl::ios_base::in)
#define IS_WRITEABLE	(static_cast<ZStream::Handle*>(cookie)->mode & mstl::ios_base::out)


static unsigned char const gzipMagic [2] = { '\037', static_cast<unsigned char>('\213') };
static unsigned char const zzipMagic [4] = { 'P', 'K', '\003', '\004' };

static double const DecompressionFactor = 3.365;


namespace {
namespace zzip {

static bool
match(ZStream::Strings const& suffixes, char const* name)
{
	if (suffixes.empty())
		return true;

	unsigned len = strlen(name);

	for (unsigned i = 0; i < suffixes.size(); ++i)
	{
		unsigned n = suffixes[i].size();

		if (n < len && strncasecmp(name + len - n, suffixes[i], n) == 0)
			return true;
	}

	return false;
}


static struct zzip_file*
fileOpen(void* cookie)
{
	ZZIP_DIRENT entry;

	while (true)
	{
		if (!zzip_dir_read(ZIP_DIR, &entry))
			return 0;

		//M_ASSERT(static_cast<ZStream::Handle*>(cookie)->suffixes);

		if (match(*static_cast<ZStream::Handle*>(cookie)->suffixes, entry.d_name))
			return ::zzip_file_open(ZIP_DIR, entry.d_name, 0);
	}

	return 0;	// satisfies the compiler
}


static ssize_t
read(void* cookie, char* buf, size_t len)
{
	//M_ASSERT(IS_READABLE);

	long n = ZIP_FILE ? zzip_file_read(ZIP_FILE, buf, len) : 0;

	while (n <= 0)
	{
		if (ZIP_FILE)
			::zzip_fclose(ZIP_FILE);

		if (!(ZIP_FILE = fileOpen(cookie)))
			return 0;

		n = ::zzip_file_read(ZIP_FILE, buf, len);
	}

	return n;
}


static int
seek(void* cookie, off_t* pos, int whence)
{
	//M_ASSERT(IS_READABLE);

	*pos = ::zzip_seek(ZIP_FILE, *pos, whence);
	return *pos < 0 ? -1 : 0;
}


static int
close(void* cookie)
{
	//M_ASSERT(ZIP_DIR);

	if (ZIP_FILE)
	{
		::zzip_fclose(ZIP_FILE);
		ZIP_FILE = 0;
	}

	::zzip_dir_close(ZIP_DIR);
	ZIP_DIR = 0;

	return 0;
}


static ssize_t
write(void*, char const*, size_t)
{
	//M_RAISE("unexpected call of %s", __func__);
	return 0;
}


static int64_t
size(ZStream::Strings const& suffixes, ZZIP_DIR* dir)
{
	ZZIP_DIRENT entry;
	int64_t		size	= 0;
	unsigned		count	= 0;

	while (::zzip_dir_read(dir, &entry))
	{
		if (match(suffixes, entry.d_name))
		{
			size += entry.st_size;
			++count;
		}
	}

	::zzip_rewinddir(dir);

	return count ? size : int64_t(-1);
}


static bool
find(ZZIP_DIR* dir, mstl::string const& name)
{
	ZZIP_DIRENT entry;

	while (::zzip_dir_read(dir, &entry))
	{
		if (name == entry.d_name)
		{
			::zzip_rewinddir(dir);
			return true;
		}
	}

	::zzip_rewinddir(dir);
	return false;
}

} // namespace zzip

namespace zip {

bool
openNewFile(void* cookie, char const* filename, mstl::ios_base::openmode mode)
{
	mstl::string basename(::util::misc::file::rootname(::util::misc::file::basename(filename)));
	mstl::string fname(basename);

	mstl::string const& suffix = ZStream::zipFileSuffixes().empty()
											? mstl::string::empty_string
											: ZStream::zipFileSuffixes().front();

	fname += suffix;

	if (mode & mstl::ios_base::app)
	{
		ZZIP_DIR* dir = ::zzip_dir_open(filename, 0);

		if (dir)
		{
			unsigned count = 2;

			while (zzip::find(dir, fname))
			{
				char buf[100];
				sprintf(buf, "-%d", count++);
				fname = basename;
				fname += buf;
				fname += suffix;
			}

			::zzip_dir_close(dir);
		}
	}

	::zip_fileinfo info;
	info.dosDate = 0;
	util::misc::time::tm tm;
	getCurrentTime(tm);
	info.tmz_date.tm_sec  = tm.sec;
	info.tmz_date.tm_min  = tm.min;
	info.tmz_date.tm_hour = tm.hour;
	info.tmz_date.tm_mday = tm.mday;
	info.tmz_date.tm_mon  = tm.mon;
	info.tmz_date.tm_year = tm.year;

	int rc = ::zipOpenNewFileInZip(	HANDLE,
												fname,
												&info,
												0, 0, 0, 0, 0,
												Z_DEFLATED,
												Z_DEFAULT_COMPRESSION);

	return rc == ZIP_OK;
}


static ssize_t
read(void*, char*, size_t)
{
	//M_RAISE("unexpected call of %s", __func__);
	return 0;
}


static int
seek(void*, off_t*, int)
{
	//M_RAISE("unexpected call of %s", __func__);
	return 0;
}


static int
close(void* cookie)
{
	//M_ASSERT(HANDLE);

	::zipCloseFileInZip(HANDLE);
	int rc = ::zipClose(HANDLE, 0) == ZIP_OK ? 0 : -1;
	HANDLE = 0;
	return rc;
}


static ssize_t
write(void* cookie, char const* buf, size_t len)
{
	//M_ASSERT(IS_WRITEABLE);
	return ::zipWriteInFileInZip(HANDLE, buf, len) == ZIP_OK ? len : 0;
}

} // namespace zip

namespace gzip {

static ssize_t
read(void* cookie, char* buf, size_t len)
{
	//M_ASSERT(IS_READABLE);

	int bytesRead = ::gzread(HANDLE, buf, len);
	return bytesRead <= 0 ? 0 : bytesRead;
}


static int
seek(void* cookie, off_t* pos, int whence)
{
	//M_ASSERT(IS_READABLE);

	*pos = ::gzseek(HANDLE, *pos, whence);
	return *pos == -1 ? -1 : 0;
}


static int
close(void* cookie)
{
	return ::gzclose(HANDLE);
}


static ssize_t
write(void* cookie, char const* buf, size_t len)
{
	//M_ASSERT(IS_WRITEABLE);
	return ::gzwrite(HANDLE, buf, len);
}


unsigned
readUncompressedSize(mstl::ifstream& strm)
{
	char buf[4];

	unsigned offs = strm.tellg();
	unsigned size;

	strm.seekg(-4, mstl::ios_base::end);
	strm.read(buf, 4);
	ByteStream bstrm(buf, 4);
	size = bstrm.uint32LE();
	strm.seekg(offs, mstl::ios_base::beg);

	// IMPORTANT NOTE: This is the size modulo 2**32. If the decompressed
	// file has more than 4GB we have to increase the size by 2**32.
	return size;
}

} // namespace gzip
} // namespace


static cookie_io_functions_t m_gzip	= { gzip::read,  gzip::write,  gzip::seek,  gzip::close };
static cookie_io_functions_t m_zzip	= { zzip::read,  zzip::write,  zzip::seek,  zzip::close };
static cookie_io_functions_t m_zip	= { zip::read,   zip::write,   zip::seek,   zip::close };


ZStream::Handle::Handle() : dir(0), file(0), suffixes(0) {}

ZStream::Strings ZStream::m_suffixes;

ZStream::~ZStream() throw()			{ if (m_fp) ::fclose(m_fp); }
bool ZStream::is_open() const			{ return m_fp != 0; }
ZStream::Type ZStream::type() const	{ return m_type; }
int64_t ZStream::size() const			{ return m_size; }


ZStream::ZStream(char const* filename, Mode mode)
	:m_size(-1)
	,m_type(None)
{
	open(filename, mode);
}


ZStream::ZStream(char const* filename, Type type, Mode mode)
	:m_size(-1)
	,m_type(type)
{
	open(filename, type, mode);
}


void
ZStream::setZipFileSuffixes(Strings const& suffixes)
{
	m_suffixes.clear();
	m_suffixes.reserve(suffixes.size());

	for (unsigned i = 0; i < suffixes.size(); ++i)
		m_suffixes.push_back(suffixes[i].front() == '.' ? suffixes[i] : '.' + suffixes[i]);
}


ZStream::Strings const&
ZStream::zipFileSuffixes()
{
	return m_suffixes;
}


uint64_t
ZStream::goffset()
{
	//if (m_type == GZip)
	//	return gzoffset(gzFile(m_handle.handle));

	return tellg();
}


void
ZStream::open(char const* filename, Mode mode)
{
	//M_REQUIRE(!is_open());
	//M_REQUIRE(mode & mstl::ios_base::in);
	//M_REQUIRE(!(mode & mstl::ios_base::out));

	mstl::ifstream strm(filename, mode | mstl::ios_base::binary);

	if (!strm)
		return setstate(failbit);

	unsigned char buffer[MAX(sizeof(gzipMagic), sizeof(zzipMagic))];
	::memset(buffer, 0, sizeof(buffer));

	strm.read(buffer, sizeof(buffer));

	Handle* cookie = &m_handle;
	char fmode[3] = { 'r', '\0', '\0' };

	cookie->mode = mode;
	cookie->suffixes = &m_suffixes;

	if (::memcmp(buffer, gzipMagic, sizeof(gzipMagic)) == 0)
	{
		fmode[1] = 'b';
		HANDLE = ::gzopen(filename, fmode);

		if (!HANDLE)
			return setstate(failbit);

		m_fp = ::fopencookie(cookie, fmode, ::m_gzip);

		if (!m_fp)
			::gzclose(HANDLE);

		m_size = gzip::readUncompressedSize(strm);
		m_type = GZip;
	}
	else if (::memcmp(buffer, zzipMagic, sizeof(zzipMagic)) == 0)
	{
		fmode[1] = 'b';
		ZIP_DIR = ::zzip_dir_open(filename, 0);

		if (!ZIP_DIR)
			return setstate(failbit);

		m_size = zzip::size(m_suffixes, ZIP_DIR);
		m_fp = ::fopencookie(cookie, fmode, ::m_zzip);

		if (!m_fp)
			zzip_dir_close(ZIP_DIR);

		m_type = Zip;
	}
	else
	{
		m_fp = ::fopen(filename, fmode);
		m_size = strm.size();
		m_type = Text;
	}

	strm.close();

	if (!m_fp)
	{
		m_handle.dir = 0;
		m_handle.file = 0;
		m_size = -1;
		m_type = None;
		setstate(failbit);
	}
}


void
ZStream::open(char const* filename, Type type, Mode mode)
{
	//M_REQUIRE(!is_open());
	//M_REQUIRE(mode & mstl::ios_base::out);
	//M_REQUIRE(!(mode & mstl::ios_base::in));

	char		fmode[3]	= { '\0', '\0', '\0' };
	Handle*	cookie	= &m_handle;

	fmode[0] = (mode & mstl::ios_base::app ? 'a' : 'w');
	cookie->mode = mode;

	switch (type)
	{
		case None:
			m_type = Text;
			// fallthru

		case Text:
			m_fp = ::fopen(filename, fmode);
			break;

		case GZip:
			fmode[1] = 'b';
			HANDLE = ::gzopen(filename, fmode);

			if (!HANDLE)
				return setstate(failbit);

			m_fp = ::fopencookie(cookie, fmode, ::m_gzip);

			if (!m_fp)
				::gzclose(HANDLE);
			break;

		case Zip:
			HANDLE = ::zipOpen(
								filename,
								mode & mstl::ios_base::app ? APPEND_STATUS_ADDINZIP : APPEND_STATUS_CREATE);

			if (!HANDLE)
				return setstate(failbit);

			if (!zip::openNewFile(cookie, filename, mode))
			{
				::zipClose(HANDLE, 0);
				return setstate(failbit);
			}

			fmode[1] = 'b';
			m_fp = ::fopencookie(cookie, fmode, ::m_zip);

			if (!m_fp)
				::zipClose(HANDLE, 0);
			break;
	}

	if (!m_fp)
	{
		HANDLE = 0;
		m_type = None;
		setstate(failbit);
	}
	else
	{
		m_size = 0;
	}
}


void
ZStream::close()
{
	int rc = 0;

	if (m_fp)
	{
		rc = ::fclose(m_fp);
		m_fp = 0;
	}

	if (rc != 0)
		setstate(failbit);
}


bool
ZStream::size(char const* filename, int64_t& size, Type* type)
{
	mstl::ifstream strm(filename, mstl::ios_base::in | mstl::ios_base::binary);

	if (!strm)
		return false;

	unsigned char buffer[MAX(sizeof(gzipMagic), sizeof(zzipMagic))];
	::memset(buffer, 0, sizeof(buffer));

	strm.read(buffer, sizeof(buffer));

	if (::memcmp(buffer, gzipMagic, sizeof(gzipMagic)) == 0)
	{
		size = gzip::readUncompressedSize(strm);
		if (type) *type = GZip;
	}
	else if (::memcmp(buffer, zzipMagic, sizeof(zzipMagic)) == 0)
	{
		Handle	handle;
		void*		cookie = &handle;

		ZIP_DIR = ::zzip_dir_open(filename, 0);

		if (!ZIP_DIR)
			return false;

		size = zzip::size(m_suffixes, ZIP_DIR);
		::zzip_dir_close(ZIP_DIR);
		if (type) *type = Zip;
	}
	else
	{
		size = strm.size();
		if (type) *type = Text;
	}

	return size != -1;
}

// vi:set ts=3 sw=3:
