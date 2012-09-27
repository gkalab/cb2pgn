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

#ifndef u_zstream_included
#define u_zstream_included

#include "m_iostream.h"
#include "m_vector.h"
#include "m_string.h"

extern "C"
{
	struct zzip_dir;
	struct zzip_file;
};

namespace util {

class ZStream : public mstl::iostream
{
public:

	enum Type { None, Text, Zip, GZip };

	typedef mstl::vector<mstl::string>	Strings;
	typedef mstl::ios_base::openmode		Mode;

	ZStream(char const* filename, Mode mode = mstl::ios_base::in);
	ZStream(char const* filename, Type type, Mode mode = mstl::ios_base::out);
	~ZStream() throw();

	bool is_open() const;

	// NOTE: in case of GZIP stream the decompressed size is only an estimation
	int64_t size() const override;	// decompressed size
	uint64_t goffset() override;

	Type type() const;

	void open(char const* filename, Mode mode = mstl::ios_base::in);
	void open(char const* filename, Type type, Mode mode = mstl::ios_base::in);
	void close();

	static bool size(char const* filename, int64_t& size, Type* type = 0);
	static void setZipFileSuffixes(Strings const& suffixes);
	static Strings const& zipFileSuffixes();

	// is public due to technical reasons
	struct Handle
	{
		union
		{
			struct
			{
				struct zzip_dir*	dir;
				struct zzip_file*	file;
			};

			void* handle;
		};

		Mode				mode;
		Strings const*	suffixes;

		Handle();
	};

private:

	Handle	m_handle;
	int64_t	m_size;
	Type		m_type;

	static Strings m_suffixes;
};

} // namespace util

#endif //u_zstream_included

// vi:set ts=3 sw=3:
