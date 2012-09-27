// ======================================================================
// Author : $Author$
// Version: $Revision$
// Date   : $Date$
// Url    : $URL$
// ======================================================================

// ======================================================================
//    _/|            __
//   // o\         /    )           ,        /    /
//   || ._)    ----\---------__----------__-/----/__-
//   //__\          \      /   '  /    /   /    /   )
//   )___(     _(____/____(___ __/____(___/____(___/_
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

#include "sci_codec.h"
#include "sci_decoder.h"
#include "sci_encoder.h"
#include "sci_consumer.h"
#include "sci_common.h"

#include "db_game_info.h"
#include "db_consumer.h"
#include "db_producer.h"
#include "db_game_data.h"
#include "db_exception.h"

#include "u_byte_stream.h"
#include "u_block_file.h"
#include "u_progress.h"
#include "u_misc.h"

#include "m_string.h"
#include "m_fstream.h"
#include "m_byte_order.h"
#include "m_assert.h"

#include <string.h>

//#define USE_LZO

#ifdef USE_LZO
# include "u_lzo_byte_stream.h"
#endif

using namespace db;
using namespace db::sci;
using namespace util;

typedef ByteStream::uint24_t uint24_t;
typedef ByteStream::uint48_t uint48_t;


namespace
{
#ifdef USE_BITFIELDS // non-portable between different platforms!

	// 56 bytes (Scid: 46 bytes)
	struct IndexEntry
	{
		union
		{
			struct
			{
				uint64_t homePawns;
				//----------------------------- 64 bit
				uint64_t whitePlayer			:24;
				uint64_t blackPlayer			:24;
				uint64_t plyCount				:12;
				uint64_t termination			: 4;
				//----------------------------- 64 bit
				uint32_t positionData;
				//----------------------------- 32 bit
				uint32_t pawnProgress;
				//----------------------------- 32 bit
				uint32_t gameOffset;
				//----------------------------- 32 bit
				uint32_t annotator			:24;
				uint32_t variationCount		: 4;
				uint32_t commentCount		: 4;
				//----------------------------- 32 bit
				uint32_t event					:24;
				uint32_t round					: 8;
				//----------------------------- 32 bit
				uint32_t material				:24;
				uint32_t hpCount				: 4;
				uint32_t blackRatingType	: 3;
				uint32_t underPromotion		: 1;
				//----------------------------- 32 bit
				uint32_t gameFlags			:24;
				uint32_t castling				: 4;
				uint32_t whiteRatingType	: 3;
				uint32_t promotion			: 1;
				//----------------------------- 32 bit
				uint32_t whiteRating			:12;
				uint32_t positionId			:12;
				uint32_t subround				: 8;
				//----------------------------- 32 bit
				uint32_t blackElo				:12;
				uint32_t blackRating			:12;
				uint32_t dateMonth			: 4;
				uint32_t annotationCount	: 4;
				//----------------------------- 32 bit
				uint32_t whiteElo				:12;
				uint32_t dateYear				:10;
				uint32_t dateDay				: 5;
				uint32_t result				: 3;
				uint16_t commentEngFlag		: 1;
				uint16_t commentOthFlag		: 1;
				//----------------------------- 32 bit
			};
			struct
			{
				uint64_t u64[2];

				union
				{
					struct { uint32_t u32[10]; };
					struct { uint16_t u16[20]; };
				};
			};
		};

		void swapBytes(uint16_t idn);
		void swapBytes();
	};

#else

struct IndexEntry
{
	void swapBytes(uint16_t idn);
	void swapBytes();

	union
	{
		struct { uint64_t u64[ 7]; };
		struct { uint32_t u32[14]; };
		struct { uint16_t u16[28]; };
	};
};

#define SET_HOME_PAWNS(item,value)			item->u64[ 0]  = value
#define SET_WHITE_PLAYER(item,value)		item->u32[ 2] |= (value & 0x00ffffff) << 8
#define SET_BLACK_PLAYER(item,value)		item->u32[ 2] |= (value & 0x00ff0000) >> 16, \
														item->u32[ 3] |= (value & 0x0000ffff) << 16
#define SET_PLY_COUNT(item,value)			item->u32[ 3] |= (value & 0x00000fff) << 4
#define SET_TERMINATION(item,value)			item->u32[ 3] |= value & 0x0000000f
#define SET_POSITION_DATA(item,value)		item->u32[ 4]  = value
#define SET_PLY_0(item,value)					item->u16[ 8]  = value
#define SET_PLY_1(item,value)					item->u16[ 9]  = value
#define SET_PAWN_PROGRESS(item,value)		item->u32[ 5]  = value
#define SET_GAME_OFFSET(item,value)			item->u32[ 6]  = value
#define SET_ANNOTATOR(item,value)			item->u32[ 7] |= (value & 0x00ffffff) << 8
#define SET_VARIATION_COUNT(item,value)	item->u32[ 7] |= (value & 0x0000000f) << 4
#define SET_COMMENT_COUNT(item,value)		item->u32[ 7] |= (value & 0x0000000f)
#define SET_EVENT(item,value)					item->u32[ 8] |= (value & 0x00ffffff) << 8
#define SET_ROUND(item,value)					item->u32[ 8] |= value & 0x000000ff
#define SET_MATERIAL(item,value)				item->u32[ 9] |= (value & 0x00ffffff) << 8
#define SET_HP_COUNT(item,value)				item->u32[ 9] |= (value & 0x0000000f) << 4
#define SET_BLACK_RATING_TYPE(item,value)	item->u32[ 9] |= (value & 0x00000007) << 1
#define SET_UNDER_PROMOTION(item,value)	item->u32[ 9] |= value & 0x00000001
#define SET_GAME_FLAGS(item,value)			item->u32[10] |= (value & 0x00ffffff) << 8
#define SET_CASTLING(item,value)				item->u32[10] |= (value & 0x0000000f) << 4
#define SET_WHITE_RATING_TYPE(item,value)	item->u32[10] |= (value & 0x00000007) << 1
#define SET_PROMOTION(item,value)			item->u32[10] |= value & 0x00000001
#define SET_WHITE_RATING(item,value)		item->u32[11] |= (value & 0x00000fff) << 20
#define SET_POSITION_ID(item,value)			item->u32[11] |= (value & 0x00000fff) << 8
#define SET_SUBROUND(item,value)				item->u32[11] |= value & 0x000000ff
#define SET_BLACK_ELO(item,value)			item->u32[12] |= (value & 0x00000fff) << 20
#define SET_BLACK_RATING(item,value)		item->u32[12] |= (value & 0x00000fff) << 8
#define SET_DATE_MONTH(item,value)			item->u32[12] |= (value & 0x0000000f) << 4
#define SET_ANNOTATION_COUNT(item,value)	item->u32[12] |= value & 0x0000000f
#define SET_WHITE_ELO(item,value)			item->u32[13] |= (value & 0x00000fff) << 20
#define SET_DATE_YEAR(item,value)			item->u32[13] |= (value & 0x000003ff) << 10
#define SET_DATE_DAY(item,value)				item->u32[13] |= (value & 0x0000001f) << 5
#define SET_RESULT(item,value)				item->u32[13] |= (value & 0x00000007) << 2
#define SET_COMMENT_ENG_FLAG(item,value)	item->u32[13] |= (value & 0x00000001) << 1
#define SET_COMMENT_OTH_FLAG(item,value)	item->u32[13] |= value & 0x00000001

#define GET_HOME_PAWNS(item)					item->u64[0]
#define GET_WHITE_PLAYER(item)				((item->u32[2] >>  8) & 0x00ffffff)
#define GET_BLACK_PLAYER(item)				((item->u32[2] & 0x000000ff) << 16) | \
														((item->u32[3] >> 16) & 0x0000ffff)
#define GET_PLY_COUNT(item)					((item->u32[3] >>  4) & 0x00000fff)
#define GET_TERMINATION(item)					(item->u32[3] & 0x0000000f)
#define GET_POSITION_DATA(item)				item->u32[4]
#define GET_PLY_0(item)							item->u16[8]
#define GET_PLY_1(item)							item->u16[9]
#define GET_PAWN_PROGRESS(item)				item->u32[5]
#define GET_GAME_OFFSET(item)					item->u32[6]
#define GET_ANNOTATOR(item)					((item->u32[7] >> 8) & 0x00ffffff)
#define GET_VARIATION_COUNT(item)			((item->u32[7] >> 4) & 0x0000000f)
#define GET_COMMENT_COUNT(item)				(item->u32[7] & 0x0000000f)
#define GET_EVENT(item)							((item->u32[8] >> 8) & 0x00ffffff)
#define GET_ROUND(item)							(item->u32[8] & 0x000000ff)
#define GET_MATERIAL(item)						((item->u32[9] >> 8) & 0x00ffffff)
#define GET_HP_COUNT(item)						((item->u32[9] >> 4) & 0x0000000f)
#define GET_BLACK_RATING_TYPE(item)			((item->u32[9] >> 1) & 0x00000007)
#define GET_UNDER_PROMOTION(item)			(item->u32[9] & 0x00000001)
#define GET_GAME_FLAGS(item)					((item->u32[10] >> 8) & 0x00ffffff)
#define GET_CASTLING(item)						((item->u32[10] >> 4) & 0x0000000f)
#define GET_WHITE_RATING_TYPE(item)			((item->u32[10] >> 1) & 0x00000007)
#define GET_PROMOTION(item)					(item->u32[10] & 0x00000001)
#define GET_WHITE_RATING(item)				((item->u32[11] >> 20) & 0x00000fff)
#define GET_POSITION_ID(item)					((item->u32[11] >>  8) & 0x00000fff)
#define GET_SUBROUND(item)						(item->u32[11] & 0x000000ff)
#define GET_BLACK_ELO(item)					((item->u32[12] >> 20) & 0x00000fff)
#define GET_BLACK_RATING(item)				((item->u32[12] >>  8) & 0x00000fff)
#define GET_DATE_MONTH(item)					((item->u32[12] >>  4) & 0x0000000f)
#define GET_ANNOTATION_COUNT(item)			(item->u32[12] & 0x0000000f)
#define GET_WHITE_ELO(item)					((item->u32[13] >> 20) & 0x00000fff)
#define GET_DATE_YEAR(item)					((item->u32[13] >> 10) & 0x000003ff)
#define GET_DATE_DAY(item)						((item->u32[13] >>  5) & 0x0000001f)
#define GET_RESULT(item)						((item->u32[13] >>  2) & 0x00000007)
#define GET_COMMENT_ENG_FLAG(item)			((item->u32[13] >> 1) & 0x00000001)
#define GET_COMMENT_OTH_FLAG(item)			(item->u32[13] & 0x00000001)

#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN

// NOTE dont swap homepawns:			u64[ 0] = mstl::bo::swap(u64[ 0]);
// NOTE dont swap pawn progression:	u32[ 5] = mstl::bo::swap(u32[ 5]);

# define SWAP_BYTES(idn)               \
	u32[ 2] = mstl::bo::swap(u32[ 2]);  \
	u32[ 3] = mstl::bo::swap(u32[ 3]);  \
	u32[ 6] = mstl::bo::swap(u32[ 6]);  \
	u32[ 7] = mstl::bo::swap(u32[ 7]);  \
	u32[ 8] = mstl::bo::swap(u32[ 8]);  \
	u32[ 9] = mstl::bo::swap(u32[ 9]);  \
	u32[10] = mstl::bo::swap(u32[10]);  \
	u32[11] = mstl::bo::swap(u32[11]);  \
	u32[12] = mstl::bo::swap(u32[12]);  \
	u32[13] = mstl::bo::swap(u32[13]);  \
                                       \
	if (idn == 518)                     \
	{                                   \
		u32[4] = mstl::bo::swap(u32[4]); \
	}                                   \
	else                                \
	{                                   \
		u16[8] = mstl::bo::swap(u16[8]); \
		u16[9] = mstl::bo::swap(u16[9]); \
	}                                   \

#else

# define SWAP_BYTES (idn)

#endif

void
IndexEntry::swapBytes(uint16_t idn)
{
	SWAP_BYTES(idn);
};

void
IndexEntry::swapBytes()
{
	SWAP_BYTES(GET_POSITION_ID(this));
};

} // namespace


static mstl::string const MagicIndexFile ("Scidb.i\0", 8);
static mstl::string const MagicGameFile  ("Scidb.g\0", 8);
static mstl::string const MagicNamebase  ("Scidb.n\0", 8);
static mstl::string const Extension("sci");

static uint16_t const FileVersion = 92;

static char const* NamebaseTags[Namebase::Round];

namespace {

struct Init { Init(); };
Init m_init;

Init::Init()
{
	NamebaseTags[Namebase::Player		] = "player\0";
	NamebaseTags[Namebase::Site		] = "site\0\0\0";
	NamebaseTags[Namebase::Event		] = "event\0\0";
	NamebaseTags[Namebase::Annotator	] = "annota\0";
}

} // namespace


inline static NamebaseSite*
getSite(Namebase& base, unsigned index)
{
	if (index >= base.size()) {}
		//IO_RAISE(Index, Corrupted, "corrupted namebase index %u", index);

	return base.siteAt(index);
}


inline static NamebaseEvent*
getEvent(Namebase& base, unsigned index)
{
	if (index >= base.size()){}
		// IO_RAISE(Index, Corrupted, "corrupted namebase index %u", index);

	return base.eventAt(index);
}


inline static NamebasePlayer*
getPlayer(Namebase& base, unsigned index)
{
	if (index >= base.size()){}
		// IO_RAISE(Index, Corrupted, "corrupted namebase index %u", index);

	// XXX we need player with id()==index
	return base.playerAt(index);
}


inline static NamebaseEntry*
getRound(Namebase& base, unsigned index)
{
	// if (index >= base.size())
		// IO_RAISE(Index, Corrupted, "corrupted namebase index %u", index);

	return base.entryAt(index);
}


inline static NamebaseEntry*
getAnnotator(Namebase& base, unsigned index)
{
	// if (index >= base.size())
		// IO_RAISE(Index, Corrupted, "corrupted namebase index %u", index);

	return base.entryAt(index);
}


static unsigned
prefix(char const* s, char const* t)
{
	unsigned count = 0;

	for ( ; *s && *s == *t; ++s, ++t, ++count)
		;

	return count;
}

#ifndef USE_LZO

namespace {

struct ByteIStream : public ByteStream
{
	ByteIStream(mstl::fstream& strm);
	void underflow(unsigned size);
	mstl::istream& m_strm;
};


ByteIStream::ByteIStream(mstl::fstream& strm)
	:ByteStream(strm.bufsize())
	,m_strm(strm)
{
	m_getp = m_endp;	// force underflow
}


void
ByteIStream::underflow(unsigned size)
{
	// //M_ASSERT(size <= capacity());

	unsigned remaining = this->remaining();
	::memmove(m_base, m_getp, remaining);
	m_getp = m_base + remaining;
	m_endp = m_getp + m_strm.readsome(reinterpret_cast<char*>(m_getp), capacity() - remaining);
	m_getp -= remaining;

	// if (__builtin_expect(m_getp >= m_endp, 0))
		// IO_RAISE(Namebase, Corrupted, "unexpected end of stream");
}


struct ByteOStream : public ByteStream
{
	ByteOStream(mstl::ostream& strm, unsigned char* buf, unsigned size);
	void overflow(unsigned size);
	void flush();
	mstl::ostream& m_strm;
};


ByteOStream::ByteOStream(mstl::ostream& strm, unsigned char* buf, unsigned size)
	:ByteStream(buf, size)
	,m_strm(strm)
{
}


void
ByteOStream::overflow(unsigned size)
{
	if (__builtin_expect(!m_strm.write(m_base, m_putp - m_base), 0)){}
		// IO_RAISE(Namebase, Write_Failed, "write failed");

	m_putp = m_base;
}


void
ByteOStream::flush()
{
	if (__builtin_expect(!m_strm.write(m_base, m_putp - m_base), 0)){}
		// IO_RAISE(Namebase, Write_Failed, "write failed");
}

} // namespace

#endif // !USE_LZO


unsigned Codec::maxGameRecordLength() const	{ return (1 << 20) - 1; }
unsigned Codec::maxGameLength() const			{ return (1 << 12) - 1; }
unsigned Codec::maxGameCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxPlayerCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxSiteCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxEventCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxAnnotatorCount() const		{ return (1 << 24) - 1; }
unsigned Codec::minYear() const					{ return Date::MinYear; }
unsigned Codec::maxYear() const					{ return Date::MaxYear; }
unsigned Codec::maxDescriptionLength() const	{ return 109; }
mstl::string const& Codec::extension() const	{ return Extension; }
mstl::string const& Codec::encoding() const	{ return "utf-8"; }
bool Codec::encodingFailed() const				{ return false; }
void Codec::reset()									{}


unsigned
Codec::gameFlags() const
{
	return	GameInfo::Flag_White_Opening
			 | GameInfo::Flag_Black_Opening
			 | GameInfo::Flag_Middle_Game
			 | GameInfo::Flag_End_Game
			 | GameInfo::Flag_Novelty
			 | GameInfo::Flag_Pawn_Structure
			 | GameInfo::Flag_Tactics
			 | GameInfo::Flag_King_Side
			 | GameInfo::Flag_Queen_Side
			 | GameInfo::Flag_Brilliancy
			 | GameInfo::Flag_Blunder
			 | GameInfo::Flag_User
			 | GameInfo::Flag_Best_Game
			 | GameInfo::Flag_Decided_Tournament
			 | GameInfo::Flag_Model_Game
			 | GameInfo::Flag_Strategy
			 | GameInfo::Flag_With_Attack
			 | GameInfo::Flag_Sacrifice
			 | GameInfo::Flag_Defense
			 | GameInfo::Flag_Material
			 | GameInfo::Flag_Piece_Play;
}


Codec::Codec()
	:m_gameData(0)
	,m_asyncReader(0)
	,m_progressReportAfter(0)
	,m_progressCount(0)
{
	static_assert(U_NUMBER_OF(m_lookup) <= Namebase::Round, "index out of range");

	m_magicGameFile = MagicGameFile;
	m_magicGameFile.resize(MagicGameFile.size() + 2);

	ByteStream strm(m_magicGameFile.data(), m_magicGameFile.size());
	strm.advance(MagicGameFile.size());
	strm << uint16_t(FileVersion);
}


Codec::~Codec() throw()
{
	delete m_gameData;
	delete m_asyncReader;
}


bool
Codec::isWriteable() const
{
	return true;
}


bool
Codec::isExtraTag(tag::ID tag)
{
	return Encoder::isExtraTag(tag);
}


bool
Codec::upgradeIndexOnly()
{
	return false;
}


Codec::Format
Codec::format() const
{
	return format::Scidb;
}


void
Codec::setEncoding(mstl::string const& encoding)
{
	// no action
}


void
Codec::filterTag(TagSet& tags, tag::ID tag, Section section) const
{
	bool gameTagsOnly = section == GameTags;

	if (Encoder::skipTag(tag) == gameTagsOnly)
		tags.remove(tag);
}


void
Codec::close()
{
	m_gameData->close();
}


void
Codec::sync()
{
	m_gameData->sync();
}


util::ByteStream
Codec::getGame(GameInfo const& info)
{
	//M_ASSERT(m_gameData);

	ByteStream src;
	getGameRecord(info, *m_gameData, src);
	return src;
}


unsigned
Codec::putGame(ByteStream const& strm)
{
	//M_ASSERT(m_gameData);
	return m_gameData->put(strm);
}


unsigned
Codec::putGame(ByteStream const& strm, unsigned prevOffset, unsigned prevRecordLength)
{
	//M_ASSERT(m_gameData);
	//M_ASSERT(prevRecordLength == 0);

	return m_gameData->put(strm, prevOffset, prevRecordLength);
}


void
Codec::writeNamebases(mstl::ostream& stream)
{
	writeNamebases(stream, 0);
}


void
Codec::save(mstl::string const& rootname, unsigned start, util::Progress& progress, bool attach)
{
	if (!(m_gameStream.mode() & mstl::ios_base::out)){}
		// IO_RAISE(Game, Read_Only, "game file '%s' is read-only", (rootname + ".scg").c_str());

	mstl::string indexFilename(rootname + ".sci");
	if (!attach)
		checkPermissions(indexFilename);

	// if (isReadOnly())
		// IO_RAISE(Index, Read_Only, "index file '%s' is read-only", indexFilename.c_str());

	mstl::string namebaseFilename(rootname + ".scn");
	if (!attach)
		checkPermissions(namebaseFilename);

	// if (isReadOnly())
		// IO_RAISE(Index, Read_Only, "name-base file '%s' is read-only", namebaseFilename.c_str());

	m_gameData->sync();

	mstl::fstream indexStream;
	openFile(indexStream, indexFilename, MagicIndexFile, attach ? Truncate : 0);

	writeNamebases(namebaseFilename);

	if (start > 0 && !indexStream.seekp(start*sizeof(IndexEntry) + 128, mstl::ios_base::beg)){}
		// IO_RAISE(Index, Corrupted, "cannot seek to end of file");
	writeIndex(indexStream, start, progress);
}


void
Codec::save(mstl::string const& rootname, unsigned start, util::Progress& progress)
{
	save(rootname, start, progress, false);
}


void
Codec::attach(mstl::string const& rootname, util::Progress& progress)
{
	static mstl::ios_base::openmode mode =
		mstl::ios_base::in | mstl::ios_base::out | mstl::ios_base::trunc | mstl::ios_base::binary;

	mstl::string gameFilename(rootname + ".scg");
	m_gameStream.set_unbuffered();
	m_gameStream.open(gameFilename, mode);
	progress.message("write-game");
	m_gameData->attach(&m_gameStream, &progress);
	save(rootname, 0, progress, true);
}


void
Codec::update(mstl::string const& rootname)
{
	if (!(m_gameStream.mode() & mstl::ios_base::out)){}
		// IO_RAISE(Game, Read_Only, "game file '%s' is read-only", (rootname + ".scg").c_str());

	mstl::string indexFilename(rootname + ".sci");
	checkPermissions(indexFilename);

	// if (isReadOnly())
		// IO_RAISE(Index, Read_Only, "index file '%s' is read-only", indexFilename.c_str());

	mstl::string namebaseFilename(rootname + ".scn");
	checkPermissions(namebaseFilename);

	// if (isReadOnly())
		// IO_RAISE(Namebase, Read_Only, "name-base file '%s' is read-only", namebaseFilename.c_str());

	mstl::fstream indexStream;

	m_gameData->sync();

	indexStream.open(	indexFilename,
							mstl::ios_base::in | mstl::ios_base::out | mstl::ios_base::binary);

	writeNamebases(namebaseFilename);
	updateIndex(indexStream);
}


void
Codec::update(mstl::string const& rootname, unsigned index, bool updateNamebase)
{
	if (!(m_gameStream.mode() & mstl::ios_base::out)){}
		// IO_RAISE(Game, Read_Only, "game file '%s' is read-only", (rootname + ".scg").c_str());

	mstl::string indexFilename(rootname + ".sci");
	checkPermissions(indexFilename);

	// if (isReadOnly())
		// IO_RAISE(Index, Read_Only, "index file '%s' is read-only", indexFilename.c_str());

	m_gameData->sync();

	mstl::fstream indexStream;
	indexStream.open(	indexFilename,
							mstl::ios_base::in | mstl::ios_base::out | mstl::ios_base::binary);

	if (updateNamebase)
	{
		mstl::string namebaseFilename(rootname + ".scn");
		checkPermissions(namebaseFilename);
		writeNamebases(namebaseFilename);
	}

	GameInfo* info = gameInfoList()[index];

	unsigned char buf[sizeof(IndexEntry)];

	ByteStream bstrm(buf, sizeof(IndexEntry));
	encodeIndex(*info, bstrm);

	if (	!indexStream.seekp(index*sizeof(IndexEntry) + 128)
		|| !indexStream.write(buf, sizeof(IndexEntry)))
	{
		// IO_RAISE(Index, Corrupted, "unexpected end of index file");
	}
}


void
Codec::updateHeader(mstl::string const& rootname)
{
	mstl::string indexFilename(rootname + ".sci");
	checkPermissions(indexFilename);

	// if (isReadOnly())
		// IO_RAISE(Index, Read_Only, "index file '%s' is read-only", indexFilename.c_str());

	mstl::fstream indexStream;
	indexStream.open(	indexFilename,
							mstl::ios_base::in | mstl::ios_base::out | mstl::ios_base::binary);
	writeIndexHeader(indexStream);
}


void
Codec::doEncoding(util::ByteStream& strm,
						GameData const& data,
						Signature const& signature,
						TagBits const& allowedTags,
						bool allowExtraTags)
{
	//M_ASSERT(gameInfoList().size() <= maxGameCount());
	//M_ASSERT(namebase(Namebase::Player).size() <= maxPlayerCount());
	//M_ASSERT(namebase(Namebase::Site).size() <= maxSiteCount());
	//M_ASSERT(namebase(Namebase::Event).size() <= maxEventCount());

	Encoder encoder(strm);
	encoder.doEncoding(signature, data, allowedTags, allowExtraTags);
}


db::Consumer*
Codec::getConsumer(format::Type srcFormat)
{
	return new Consumer(srcFormat, *this, Consumer::TagBits(true), true);
}


save::State
Codec::doDecoding(db::Consumer& consumer, TagSet& tags, GameInfo const& info)
{
	ByteStream strm;
	getGameRecord(info, *m_gameData, strm);
	Decoder decoder(strm, m_gameData->blockSize() - info.gameOffset());
	return decoder.doDecoding(consumer, tags);
}


save::State
Codec::doDecoding(db::Consumer& consumer, ByteStream& strm, TagSet& tags)
{
	Decoder decoder(strm);
	return decoder.doDecoding(consumer, tags);
}


void
Codec::doDecoding(GameData& data, GameInfo& info, mstl::string*)
{
	ByteStream strm;
	getGameRecord(info, *m_gameData, strm);
	Decoder decoder(strm, m_gameData->blockSize() - info.gameOffset());
	decoder.doDecoding(data);
}


void
Codec::doOpen(mstl::string const& encoding)
{
//	//M_REQUIRE(encoding == sys::utf8::Codec::utf8());

	if (m_gameData)
	{
		m_gameData->close();
		delete m_gameData;
	}

	if (m_gameStream.is_open())
		m_gameData = new BlockFile(&m_gameStream, Block_Size, BlockFile::ReadWriteLength, m_magicGameFile);
	else
		m_gameData = new BlockFile(Block_Size, BlockFile::ReadWriteLength, m_magicGameFile);
}


DatabaseCodec*
Codec::makeCodec(mstl::string const& name)
{
	mstl::fstream strm;

	strm.open(name, mstl::ios_base::in | mstl::ios_base::binary);

	if (strm)
	{
		strm.exceptions(mstl::ios_base::badbit | mstl::ios_base::eofbit | mstl::ios_base::failbit);

		char header[120];

		if (!strm.read(header, sizeof(header))){}
			// IO_RAISE(Index, Corrupted, "unexpected end of index file");

		strm.close();

		ByteStream bstrm(header, sizeof(header));

#ifndef CODEBLOCKS

		bstrm.skip(8); // skip magic
		unsigned fileVersion	= bstrm.uint16();

#endif
	}

	return new Codec;
}


void
Codec::doOpen(mstl::string const& rootname, mstl::string const& encoding, util::Progress& progress)
{
//	//M_REQUIRE(encoding == sys::utf8::Codec::utf8());
	//M_ASSERT(m_gameData == 0);

	mstl::string indexFilename(rootname + ".sci");
	mstl::string gameFilename(rootname + ".scg");
	mstl::string namebaseFilename(rootname + ".scn");

	checkPermissions(indexFilename);
	mstl::fstream indexStream;
	openFile(indexStream, indexFilename, MagicIndexFile);

	uint16_t fileVersion = readIndexHeader(indexStream);

	mstl::fstream namebaseStream;

	//try
	{
		checkPermissions(gameFilename);
		checkPermissions(namebaseFilename);

		m_gameStream.set_unbuffered();
		namebaseStream.set_unbuffered();

		openFile(m_gameStream, gameFilename, MagicGameFile);
		checkFileVersion(m_gameStream, MagicGameFile, fileVersion);
		openFile(namebaseStream, namebaseFilename, MagicNamebase);
		checkFileVersion(namebaseStream, MagicNamebase, fileVersion);

		readNamebases(namebaseStream, progress);
		decodeIndex(indexStream, progress);
		m_gameData = new BlockFile(&m_gameStream, Block_Size, BlockFile::ReadWriteLength, m_magicGameFile);
	}
	/*catch (...)
	{
		indexStream.close();
		namebaseStream.close();
		m_gameStream.close();
		throw;
	}*/

	namebaseStream.close();
	indexStream.close();
}


void
Codec::checkFileVersion(mstl::fstream& fstrm, mstl::string const& magic, uint16_t fileVersion)
{
	char buf[2];

	fstrm.seekg(magic.size(), mstl::ios_base::beg);
	fstrm.read(buf, 2);

	ByteStream bstrm(buf, 2);
	uint16_t version = bstrm.uint16();

	if (version != fileVersion)
	{
		// if (&fstrm == m_gameStream)
			// IO_RAISE(Game, Unexpected_Version, "unexpected version (%u)", unsigned(version));
		// else
			// IO_RAISE(Namebase, Unexpected_Version, "unexpected version (%u)", unsigned(version));
	}
}


void
Codec::doOpen(mstl::string const& rootname, mstl::string const& encoding)
{
//	//M_REQUIRE(encoding == sys::utf8::Codec::utf8());

	mstl::string indexFilename(rootname + ".sci");
	mstl::string gameFilename(rootname + ".scg");
	mstl::string namebaseFilename(rootname + ".scn");

	mstl::fstream indexStream;
	mstl::fstream namebaseStream;

	m_gameStream.set_unbuffered();

	openFile(m_gameStream, gameFilename, Truncate);
	openFile(indexStream, indexFilename, MagicIndexFile, Truncate);
	openFile(namebaseStream, namebaseFilename, MagicNamebase, Truncate);
	writeNamebases(namebaseStream);
	writeIndexHeader(indexStream);
	doOpen(encoding);
}


void
Codec::doClear(mstl::string const& rootname)
{
	mstl::string indexFilename(rootname + ".sci");
	mstl::string gameFilename(rootname + ".scg");
	mstl::string namebaseFilename(rootname + ".scn");

	mstl::fstream indexStream;
	mstl::fstream namebaseStream;

	openFile(m_gameStream, gameFilename, Truncate);
	openFile(indexStream, indexFilename, MagicIndexFile, Truncate);
	openFile(namebaseStream, namebaseFilename, MagicNamebase, Truncate);
	writeNamebases(namebaseStream);
	writeIndexHeader(indexStream);

	doOpen("utf-8");
	m_gameData->attach(&m_gameStream);
}


uint16_t
Codec::readIndexHeader(mstl::fstream& fstrm)
{
	char header[120];

	if (!fstrm.read(header, sizeof(header))){}
		// IO_RAISE(Index, Corrupted, "unexpected end of index file");

	ByteStream bstrm(header, sizeof(header));

	unsigned version	= bstrm.uint16();
	unsigned numGames	= bstrm.uint24();
	unsigned baseType	= bstrm.uint8();
	unsigned created  = bstrm.uint32();

	if (version != ::FileVersion)
	{
		// if (version < FileVersion)
			// IO_RAISE(Index, Unexpected_Version, "old format Scidb version (%d)", unsigned(version));
		// else
			// IO_RAISE(Index, Unknown_Version, "unknown Scidb version (%u)", unsigned(version));
	}

	mstl::string description;
	bstrm.get(description);
	setDescription(description);

	setType(type::ID(baseType));
	setCreated(created);

	GameInfoList& infoList = gameInfoList();

	infoList.resize(numGames);
	for (unsigned i = 0; i < numGames; ++i)
		infoList[i] = allocGameInfo();

	// go to first index
	if (!fstrm.seekg(sizeof(header) + 8, mstl::ios_base::beg)){}
		// IO_RAISE(Index, Corrupted, "seek failed");

	return version;
}


void
Codec::decodeIndex(mstl::fstream &fstrm, util::Progress& progress)
{
	GameInfoList& infoList = gameInfoList();

	unsigned frequency	= progress.frequency(infoList.size(), 20000);
	unsigned reportAfter	= frequency;

	ProgressWatcher watcher(progress, infoList.size());
	progress.message("read-index");

	for (unsigned i = 0; i < infoList.size(); ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		char buf[sizeof(IndexEntry)];

		if (__builtin_expect(!fstrm.read(buf, sizeof(IndexEntry)), 0)){}
			// IO_RAISE(Index, Corrupted, "unexpected end of index file");

		ByteStream bstrm(buf, sizeof(IndexEntry));
		decodeIndex(bstrm, *infoList[i]);
	}
}


void
Codec::decodeIndex(ByteStream& strm, GameInfo& item)
{
	IndexEntry* bits = reinterpret_cast<IndexEntry*>(strm.data());

	bits->swapBytes();

#ifdef USE_BITFIELDS

# define GET(type, field) \
	::get##type(namebase(Namebase::type), m_lookup[Namebase::type][bits->field])

	NamebasePlayer* whitePlayer = GET(Player, whitePlayer);
	NamebasePlayer* blackPlayer = GET(Player, blackPlayer);

	whitePlayer->ref(); blackPlayer->ref();

	{
		NamebaseEvent* event			= GET(Event, event);
		NamebaseEntry* annotator	= GET(Annotator, annotator);

		event->ref(); annotator->ref();

		item.m_player[color::White]	= whitePlayer;
		item.m_player[color::Black]	= blackPlayer;
		item.m_event						= event;
		item.m_annotator					= annotator;
	}

# undef GET

	whitePlayer->setElo(item.m_pd[color::White].elo = bits->whiteElo);
	blackPlayer->setElo(item.m_pd[color::Black].elo = bits->blackElo);

	whitePlayer->setRating(	rating::Type(item.m_pd[color::White].ratingType = bits->whiteRatingType),
									item.m_pd[color::White].rating = bits->whiteRating);
	blackPlayer->setRating(	rating::Type(item.m_pd[color::Black].ratingType = bits->blackRatingType),
									item.m_pd[color::Black].rating = bits->blackRating);

	item.setMaterial(bits->material);

	item.m_signature.m_homePawns.value	= bits->homePawns;
	item.m_signature.m_progress.value	= bits->pawnProgress;
	item.m_pd[0].langFlag					= bits->commentEngFlag;
	item.m_pd[1].langFlag					= bits->commentOthFlag;
	item.m_variationCount					= bits->variationCount;
	item.m_annotationCount					= bits->annotationCount;
	item.m_commentCount						= bits->commentCount;
	item.m_termination						= bits->termination;
	item.m_gameFlags							= bits->gameFlags;
	item.m_round								= bits->round;
	item.m_subround							= bits->subround;
	item.m_dateMonth							= bits->dateMonth;
	item.m_dateDay								= bits->dateDay;
	item.m_result								= bits->result;
	item.m_gameOffset							= bits->gameOffset;
	item.m_plyCount							= bits->plyCount;
	item.m_positionId							= bits->positionId;
	item.m_dateYear							= bits->dateYear;
	item.m_positionData						= bits->positionData;
	item.m_signature.m_promotions			= bits->promotion;
	item.m_signature.m_underPromotions	= bits->underPromotion;
	item.m_signature.m_castling			= bits->castling;
	item.m_signature.m_hpCount				= bits->hpCount;

#else

# define GET(type, accessor) \
	::get##type(namebase(Namebase::type), m_lookup[Namebase::type][accessor(bits)])

	NamebasePlayer* whitePlayer = GET(Player, GET_WHITE_PLAYER);
	NamebasePlayer* blackPlayer = GET(Player, GET_BLACK_PLAYER);

	whitePlayer->ref(); blackPlayer->ref();

	{
		NamebaseEvent* event			= GET(Event, GET_EVENT);
		NamebaseEntry* annotator	= GET(Annotator, GET_ANNOTATOR);

		event->ref(); annotator->ref();

		item.m_player[color::White]	= whitePlayer;
		item.m_player[color::Black]	= blackPlayer;
		item.m_event						= event;
		item.m_annotator					= annotator;
	}

# undef GET

	whitePlayer->setElo(item.m_pd[color::White].elo = GET_WHITE_ELO(bits));
	blackPlayer->setElo(item.m_pd[color::Black].elo = GET_BLACK_ELO(bits));

	whitePlayer->setRating(
		rating::Type(item.m_pd[color::White].ratingType = GET_WHITE_RATING_TYPE(bits)),
		item.m_pd[color::White].rating = GET_WHITE_RATING(bits));
	blackPlayer->setRating(
		rating::Type(item.m_pd[color::Black].ratingType = GET_BLACK_RATING_TYPE(bits)),
		item.m_pd[color::Black].rating = GET_BLACK_RATING(bits));

	item.setMaterial(GET_MATERIAL(bits));

	item.m_signature.m_homePawns.value	= GET_HOME_PAWNS(bits);
	item.m_signature.m_progress.value	= GET_PAWN_PROGRESS(bits);
	item.m_pd[0].langFlag					= GET_COMMENT_ENG_FLAG(bits);
	item.m_pd[1].langFlag					= GET_COMMENT_OTH_FLAG(bits);
	item.m_variationCount					= GET_VARIATION_COUNT(bits);
	item.m_annotationCount					= GET_ANNOTATION_COUNT(bits);
	item.m_commentCount						= GET_COMMENT_COUNT(bits);
	item.m_termination						= GET_TERMINATION(bits);
	item.m_gameFlags							= GET_GAME_FLAGS(bits);
	item.m_round								= GET_ROUND(bits);
	item.m_subround							= GET_SUBROUND(bits);
	item.m_dateYear							= GET_DATE_YEAR(bits);
	item.m_dateMonth							= GET_DATE_MONTH(bits);
	item.m_dateDay								= GET_DATE_DAY(bits);
	item.m_result								= GET_RESULT(bits);
	item.m_gameOffset							= GET_GAME_OFFSET(bits);
	item.m_plyCount							= GET_PLY_COUNT(bits);
	item.m_positionId							= GET_POSITION_ID(bits);
	item.m_positionData						= GET_POSITION_DATA(bits);
	item.m_signature.m_promotions			= GET_PROMOTION(bits);
	item.m_signature.m_underPromotions	= GET_UNDER_PROMOTION(bits);
	item.m_signature.m_castling			= GET_CASTLING(bits);
	item.m_signature.m_hpCount				= GET_HP_COUNT(bits);

#endif
}


void
Codec::writeIndexHeader(mstl::ostream& strm)
{
	unsigned char header[128];
	::memset(header, 0, sizeof(header));

	ByteStream bstrm(header, sizeof(header));

	bstrm.put(::MagicIndexFile, 8);
	bstrm << uint16_t(FileVersion);				// Scidb version
	bstrm << uint24_t(gameInfoList().size());	// number of games
	bstrm << uint8_t(type());						// base type
	bstrm << uint32_t(created());					// creation time

	bstrm.put(	description(),
					mstl::min(	description().size(),
									mstl::string::size_type(sizeof(header) - strm.tellp() - 1)));

	if (!strm.seekp(0, mstl::ios_base::beg)){}	// skip magic
		// IO_RAISE(Index, Corrupted, "seek failed");

	if (!strm.write(header, sizeof(header))){}
		// IO_RAISE(Index, Write_Failed, "unexpected end of index file");
}


void
Codec::writeIndex(mstl::ostream& strm, unsigned start, util::Progress& progress)
{
	writeIndexHeader(strm);

	GameInfoList& infoList = gameInfoList();

	unsigned frequency	= progress.frequency(infoList.size(), 100000);
	unsigned reportAfter	= frequency + start;

	ProgressWatcher watcher(progress, infoList.size());
	progress.message("write-index");

	for (unsigned i = start; i < infoList.size(); ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		char buf[sizeof(IndexEntry)];

		ByteStream bstrm(buf, sizeof(IndexEntry));
		encodeIndex(*infoList[i], bstrm);

		if (__builtin_expect(!strm.write(buf, sizeof(IndexEntry)), 0)){}
			// IO_RAISE(Index, Write_Failed, "error while writing index entry");
	}
}


void
Codec::writeGames(mstl::ostream& strm, util::Progress& progress)
{
	//M_ASSERT(m_gameData);
	m_gameData->save(strm, &progress);
}


void
Codec::writeIndex(mstl::ostream& strm, util::Progress& progress)
{
	writeIndex(strm, 0, progress);
}


void
Codec::updateIndex(mstl::ostream& strm)
{
	// update header
	{
		if (!strm.seekp(10)){}
			// IO_RAISE(Index, Corrupted, "unexpected end of index file");

		unsigned char buf[3];
		ByteStream bstrm(buf, sizeof(buf));
		bstrm << uint24_t(gameInfoList().size());	// number of games

		if (!strm.write(buf, sizeof(buf))){}
			// IO_RAISE(Index, Write_Failed, "error while writing index entry");
	}

	GameInfoList& infoList = gameInfoList();

	for (unsigned i = 0; i < infoList.size(); ++i)
	{
		if (infoList[i]->isDirty())
		{
			unsigned char buf[sizeof(IndexEntry)];

			ByteStream bstrm(buf, sizeof(IndexEntry));
			encodeIndex(*infoList[i], bstrm);

			if (!strm.seekp(i*sizeof(IndexEntry) + 128)){}
				// IO_RAISE(Index, Corrupted, "unexpected end of index file");
			if (!strm.write(buf, sizeof(IndexEntry))){}
				// IO_RAISE(Index, Write_Failed, "error while writing index entry");

			infoList[i]->setDirty(false);
		}
	}
}


void
Codec::encodeIndex(GameInfo const& item, ByteStream& strm)
{
	IndexEntry* bits = reinterpret_cast<IndexEntry*>(strm.buffer());

#ifdef USE_BITFIELDS

	bits->whitePlayer			= item.m_player[color::White]->id();
	bits->blackPlayer			= item.m_player[color::Black]->id();
	bits->event					= item.m_event->id();
	bits->annotator			= item.m_annotator->id();
	bits->gameOffset			= item.m_gameOffset;
	bits->homePawns			= item.m_signature.m_homePawns.value;
	bits->pawnProgress		= item.m_signature.m_progress.value;
	bits->material				= item.material().value;
	bits->round					= item.m_round;
	bits->subround				= item.m_subround;
	bits->gameFlags			= item.m_gameFlags;
	bits->promotion			= item.m_signature.hasPromotion();
	bits->underPromotion		= item.m_signature.hasUnderPromotion();
	bits->castling				= item.m_signature.m_castling;
	bits->hpCount				= item.m_signature.m_hpCount;
	bits->plyCount				= item.m_plyCount;
	bits->positionId			= item.m_positionId;
	bits->variationCount		= item.m_variationCount;
	bits->commentCount		= item.m_commentCount;
	bits->annotationCount	= item.m_annotationCount;
	bits->dateDay				= item.m_dateDay;
	bits->dateMonth			= item.m_dateMonth;
	bits->dateYear				= item.m_dateYear;
	bits->result				= item.m_result;
	bits->termination			= item.m_termination;
	bits->whiteElo				= item.m_pd[color::White].elo;
	bits->blackElo				= item.m_pd[color::Black].elo;
	bits->whiteRating			= item.m_pd[color::White].rating;
	bits->blackRating			= item.m_pd[color::Black].rating;
	bits->whiteRatingType	= item.m_pd[color::White].ratingType;
	bits->blackRatingType	= item.m_pd[color::Black].ratingType;
	bits->commentEngFlag		= item.m_pd[0].langFlag;
	bits->commentOthFlag		= item.m_pd[1].langFlag;
	bits->positionData		= item.m_positionData;

#else

	// first we have to zero the item!
	::memset(bits, 0, sizeof(IndexEntry));

	SET_WHITE_PLAYER		(bits, item.m_player[color::White]->id());
	SET_BLACK_PLAYER		(bits, item.m_player[color::Black]->id());
	SET_EVENT				(bits, item.m_event->id());
	SET_ANNOTATOR			(bits, item.m_annotator->id());
	SET_GAME_OFFSET		(bits, item.m_gameOffset);
	SET_HOME_PAWNS			(bits, item.m_signature.m_homePawns.value);
	SET_PAWN_PROGRESS		(bits, item.m_signature.m_progress.value);
	SET_MATERIAL			(bits, item.material().value);
	SET_ROUND				(bits, item.m_round);
	SET_SUBROUND			(bits, item.m_subround);
	SET_GAME_FLAGS			(bits, item.m_gameFlags);
	SET_PROMOTION			(bits, item.m_signature.hasPromotion());
	SET_UNDER_PROMOTION	(bits, item.m_signature.hasUnderPromotion());
	SET_CASTLING			(bits, item.m_signature.m_castling);
	SET_HP_COUNT			(bits, item.m_signature.m_hpCount);
	SET_PLY_COUNT			(bits, item.m_plyCount);
	SET_POSITION_ID		(bits, item.m_positionId);
	SET_VARIATION_COUNT	(bits, item.m_variationCount);
	SET_COMMENT_COUNT		(bits, item.m_commentCount);
	SET_ANNOTATION_COUNT	(bits, item.m_annotationCount);
	SET_DATE_DAY			(bits, item.m_dateDay);
	SET_DATE_MONTH			(bits, item.m_dateMonth);
	SET_DATE_YEAR			(bits, item.m_dateYear);
	SET_RESULT				(bits, item.m_result);
	SET_TERMINATION		(bits, item.m_termination);
	SET_WHITE_ELO			(bits, item.m_pd[color::White].elo);
	SET_BLACK_ELO			(bits, item.m_pd[color::Black].elo);
	SET_WHITE_RATING		(bits, item.m_pd[color::White].rating);
	SET_BLACK_RATING		(bits, item.m_pd[color::Black].rating);
	SET_WHITE_RATING_TYPE(bits, item.m_pd[color::White].ratingType);
	SET_BLACK_RATING_TYPE(bits, item.m_pd[color::Black].ratingType);
	SET_COMMENT_ENG_FLAG	(bits, item.m_pd[0].langFlag);
	SET_COMMENT_OTH_FLAG	(bits, item.m_pd[1].langFlag);
	SET_POSITION_DATA		(bits, item.m_positionData);

	//M_ASSERT(item.m_player[color::White]->id()		== (GET_WHITE_PLAYER(bits)));
	//M_ASSERT(item.m_player[color::Black]->id()		== (GET_BLACK_PLAYER(bits)));
	//M_ASSERT(item.m_event->id()							== (GET_EVENT(bits)));
	//M_ASSERT(item.m_annotator->id()						== (GET_ANNOTATOR(bits)));
	//M_ASSERT(item.m_gameOffset								== (GET_GAME_OFFSET(bits)));
	//M_ASSERT(item.m_signature.m_homePawns.value		== (GET_HOME_PAWNS(bits)));
	//M_ASSERT(item.m_signature.m_progress.value		== (GET_PAWN_PROGRESS(bits)));
	//M_ASSERT(item.material().value						== (GET_MATERIAL(bits)));
	//M_ASSERT(item.m_round									== (GET_ROUND(bits)));
	//M_ASSERT(item.m_subround								== (GET_SUBROUND(bits)));
	//M_ASSERT(item.m_gameFlags								== (GET_GAME_FLAGS(bits)));
	//M_ASSERT(item.m_signature.hasPromotion()			== (GET_PROMOTION(bits)));
	//M_ASSERT(item.m_signature.hasUnderPromotion()	== (GET_UNDER_PROMOTION(bits)));
	//M_ASSERT(item.m_signature.m_castling				== (GET_CASTLING(bits)));
	//M_ASSERT(item.m_signature.m_hpCount					== (GET_HP_COUNT(bits)));
	//M_ASSERT(item.m_plyCount								== (GET_PLY_COUNT(bits)));
	//M_ASSERT(item.m_positionId								== (GET_POSITION_ID(bits)));
	//M_ASSERT(item.m_variationCount						== (GET_VARIATION_COUNT(bits)));
	//M_ASSERT(item.m_commentCount							== (GET_COMMENT_COUNT(bits)));
	//M_ASSERT(item.m_annotationCount						== (GET_ANNOTATION_COUNT(bits)));
	//M_ASSERT(item.m_dateDay									== (GET_DATE_DAY(bits)));
	//M_ASSERT(item.m_dateMonth								== (GET_DATE_MONTH(bits)));
	//M_ASSERT(item.m_dateYear								== (GET_DATE_YEAR(bits)));
	//M_ASSERT(item.m_result									== (GET_RESULT(bits)));
	//M_ASSERT(item.m_termination							== (GET_TERMINATION(bits)));
	//M_ASSERT(item.m_pd[color::White].elo				== (GET_WHITE_ELO(bits)));
	//M_ASSERT(item.m_pd[color::Black].elo				== (GET_BLACK_ELO(bits)));
	//M_ASSERT(item.m_pd[color::White].rating			== (GET_WHITE_RATING(bits)));
	//M_ASSERT(item.m_pd[color::Black].rating			== (GET_BLACK_RATING(bits)));
	//M_ASSERT(item.m_pd[color::White].ratingType		== (GET_WHITE_RATING_TYPE(bits)));
	//M_ASSERT(item.m_pd[color::Black].ratingType		== (GET_BLACK_RATING_TYPE(bits)));
	//M_ASSERT(item.m_pd[0].langFlag						== (GET_COMMENT_ENG_FLAG(bits)));
	//M_ASSERT(item.m_pd[1].langFlag						== (GET_COMMENT_OTH_FLAG(bits)));
	//M_ASSERT(item.m_positionData							== (GET_POSITION_DATA(bits)));

#endif

	bits->swapBytes(item.m_positionId);
}


void
Codec::readNamebases(mstl::fstream& stream, util::Progress& progress)
{
#ifdef USE_LZO
	LzoByteStream bstrm(stream);
#else
	stream.set_bufsize(65536);
	ByteIStream bstrm(stream);
#endif

	unsigned total = bstrm.uint32();

	ProgressWatcher watcher(progress, total);
	progress.message("read-namebase");

	m_progressFrequency		= progress.frequency(total, 1000);
	m_progressReportAfter	= m_progressFrequency;
	m_progressCount			= 0;

	for (unsigned i = 0; i < U_NUMBER_OF(::NamebaseTags); ++i)
	{
		Namebase::Type type;

		char tag[8];
		bstrm.get(tag, 8);

		switch (tag[0])
		{
			case 's':	type = Namebase::Site; break;
			case 'p':	type = Namebase::Player; break;
			case 'e':	type = Namebase::Event; break;
			case 'a':	type = Namebase::Annotator; break;
			default:		return; //IO_RAISE(Namebase, Corrupted, "unexpected tag entry");
		}

		Namebase& base = namebase(type);

		unsigned size = bstrm.uint24();
		unsigned maxFreq = bstrm.uint24();
		unsigned maxUsage = bstrm.uint24();
		unsigned nextId = bstrm.uint24();

		m_lookup[i].resize(nextId);

		switch (i)
		{
			case Namebase::Event:	readEventbase(bstrm, namebase(type), size, progress); break;
			case Namebase::Site:		readSitebase(bstrm, namebase(type), size, progress); break;
			case Namebase::Player:	readPlayerbase(bstrm, namebase(type), size, progress); break;
			default:						readNamebase(bstrm, namebase(type), size, progress); break;
		}

		m_progressCount += size;
		progress.update(m_progressCount);
		m_progressReportAfter = m_progressFrequency - (m_progressCount % m_progressFrequency);
		base.setPrepared(maxFreq, nextId - 1, maxUsage);
	}
}


void
Codec::readNamebase(ByteStream& bstrm, Namebase& base, unsigned count, util::Progress& progress)
{
	if (count == 0)
		return;

	mstl::string name;

	base.reserve(count, 1 << 24);

	unsigned	index		= bstrm.uint24();
	unsigned	length	= bstrm.get();
	char*		prev		= base.alloc(length);
	Lookup&	lookup	= m_lookup[base.type()];

	bstrm.get(prev, length);
	name.hook(prev, length);
	base.append(name, index);
	lookup[index] = 0;

	for (unsigned i = 1; i < count; ++i)
	{
		if (m_progressReportAfter <= i)
		{
			progress.update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		unsigned index 	= bstrm.uint24();
		unsigned prefix	= bstrm.get();
		unsigned length	= bstrm.get();

		// if (prefix >= length)
			// IO_RAISE(Namebase, Corrupted, "namebase file is broken");

		char* curr = base.alloc(length);
		::memcpy(curr, prev, prefix);
		bstrm.get(curr + prefix, length - prefix);
		name.hook(curr, length);
		prev = curr;
		base.append(name, index);
		lookup[index] = i;
	}
}


void
Codec::readSitebase(ByteStream& bstrm, Namebase& base, unsigned count, util::Progress& progress)
{
	if (count == 0)
		return;

	mstl::string name;

	base.reserve(count, 1 << 24);

	unsigned	index		= bstrm.uint24();
	unsigned	length	= bstrm.get();
	char*		prev		= base.alloc(length);
	Lookup&	lookup	= m_lookup[Namebase::Site];

	bstrm.get(prev, length);
	name.hook(prev, length);

	base.appendSite(name, index, country::Code(bstrm.uint16()));
	lookup[index] = 0;

	for (unsigned i = 1; i < count; ++i)
	{
		if (m_progressReportAfter <= i)
		{
			progress.update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		unsigned	index		= bstrm.uint24();
		unsigned prefix	= bstrm.get();
		unsigned length	= bstrm.get();

		// if (prefix > length)
			// IO_RAISE(Namebase, Corrupted, "namebase file is broken");

		if (prefix < length)
		{
			char* curr = base.alloc(length);
			::memcpy(curr, prev, prefix);
			bstrm.get(curr + prefix, length - prefix);
			name.hook(curr, length);
			prev = curr;
		}

		base.appendSite(name, index, country::Code(bstrm.uint16()));
		lookup[index] = i;
	}
}


void
Codec::readEventbase(ByteStream& bstrm, Namebase& base, unsigned count, util::Progress& progress)
{
	if (count == 0)
		return;

	mstl::string name;

	base.reserve(count, 1 << 24);

	char*		prev		= 0;
	unsigned	index		= bstrm.uint24();
	unsigned	length	= bstrm.get();
	Lookup&	lookup	= m_lookup[Namebase::Event];

	prev = base.alloc(length);
	bstrm.get(prev, length);
	name.hook(prev, length);

	NamebaseSite* site = ::getSite(namebase(Namebase::Site), m_lookup[Namebase::Site][bstrm.uint24()]);

	site->ref();

	if (uint16_t flags = bstrm.uint16())
	{
		// flags (16 bit)
		// -------------------------------
		// 0000 0000 0000 1111  type
		// 0000 0000 0111 0000  event mode
		// 0000 0011 1000 0000  time mode
		// 0111 1100 0000 0000	date day
		// 1000 0000 0000 0000  extranouos data flag
		//
		// extraneous data (16 bit)
		// -------------------------------
		// 0000 0011 1111 1111  date year
		// 0011 1100 0000 0000  date month
		// 1100 0000 0000 0000  <unused>

		unsigned dateYear;
		unsigned dateMonth;
		unsigned dateDay;

		if (flags & 0x8000)
		{
			uint16_t extraneous = bstrm.uint16();

			dateYear = Date::decodeYearFrom10Bits(extraneous & 0x03ff);
			dateMonth = (extraneous >> 10) & 0x000f;
			dateDay = (flags >> 10) & 0x001f;
		}
		else
		{
			dateYear = dateMonth = dateDay = 0;
		}

		base.appendEvent(	name,
								0,
								dateYear,
								dateMonth,
								dateDay,
								event::Type(flags & 0x000f),
								time::Mode((flags >> 7) & 0x0007),
								event::Mode((flags >> 4) & 0x0007),
								site);
	}
	else
	{
		base.appendEvent(name, index, site);
	}

	lookup[index] = 0;

	for (unsigned i = 1; i < count; ++i)
	{
		if (m_progressReportAfter <= i)
		{
			progress.update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		unsigned	index		= bstrm.uint24();
		unsigned prefix	= bstrm.get();
		unsigned length	= bstrm.get();

		// if (prefix > length)
			// IO_RAISE(Namebase, Corrupted, "namebase file is broken");

		if (prefix < length)
		{
			char* curr = base.alloc(length);
			// //M_ASSERT(prev || prefix == 0);
			::memcpy(curr, prev, prefix);
			bstrm.get(curr + prefix, length - prefix);
			name.hook(curr, length);
			prev = curr;
		}

		NamebaseSite* site = ::getSite(namebase(Namebase::Site), m_lookup[Namebase::Site][bstrm.uint24()]);

		site->ref();

		if (uint16_t flags = bstrm.uint16())
		{
			unsigned dateYear;
			unsigned dateMonth;
			unsigned dateDay;

			if (flags & 0x8000)
			{
				uint16_t extraneous = bstrm.uint16();

				dateYear = Date::decodeYearFrom10Bits(extraneous & 0x03ff);
				dateMonth = (extraneous >> 10) & 0x000f;
				dateDay = (flags >> 10) & 0x001f;
			}
			else
			{
				dateYear = dateMonth = dateDay = 0;
			}

			base.appendEvent(	name,
									index,
									dateYear,
									dateMonth,
									dateDay,
									event::Type(flags & 0x000f),
									time::Mode((flags >> 7) & 0x0007),
									event::Mode((flags >> 4) & 0x0007),
									site);
		}
		else
		{
			base.appendEvent(name, index, site);
		}

		lookup[index] = i;
	}
}


void
Codec::readPlayerbase(ByteStream& bstrm, Namebase& base, unsigned count, util::Progress& progress)
{
	if (count == 0)
		return;

	mstl::string name;

	base.reserve(count, 1 << 24);

	unsigned	index		= bstrm.uint24();
	unsigned	length	= bstrm.get();
	char*		prev		= base.alloc(length);
	Lookup&	lookup	= m_lookup[Namebase::Player];

	bstrm.get(prev, length);
	name.hook(prev, length);

	if (Byte flags = bstrm.get())
	{
		// flags (8 bit)
		// -------------------------------
		// 0000 0111  species
		// 0011 1000  sex
		// 0100 0000  country/title data flag
		// 1000 0000  fide id flag
		//
		// country/title data (16 bit)
		// -------------------------------
		// 0000 0001 1111 1111  country
		// 0011 1110 0000 0000  title
		// 1100 0000 0000 0000  <unused>
		//
		// fide id (32 bit)
		// -------------------------------
		// 1111 1111 1111 1111  fide id

		country::Code	country;
		title::ID		title;

		if (flags & 0x40)
		{
			uint16_t extraneous = bstrm.uint16();

			country = country::Code(extraneous & 0x01ff);
			title = title::ID((extraneous >> 9) & 0x001f);
		}
		else
		{
			country = country::Unknown;
			title = title::None;
		}

		base.appendPlayer(name,
								index,
								country,
								title,
								species::ID(flags & 0x03),
								sex::ID((flags >> 3) & 0x03),
								(flags & 0x80) ? bstrm.uint32() : 0); // fide id
	}
	else
	{
		base.appendPlayer(name, index);
	}

	lookup[index] = 0;

	for (unsigned i = 1; i < count; ++i)
	{
		if (m_progressReportAfter <= i)
		{
			progress.update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		unsigned	index		= bstrm.uint24();
		unsigned prefix	= bstrm.get();
		unsigned length	= bstrm.get();

		// if (prefix > length)
			// IO_RAISE(Namebase, Corrupted, "namebase file is broken");

		if (prefix < length)
		{
			char* curr = base.alloc(length);
			//M_ASSERT(prev || prefix == 0);
			::memcpy(curr, prev, prefix);
			bstrm.get(curr + prefix, length - prefix);
			name.hook(curr, length);
			prev = curr;
		}

		if (Byte flags = bstrm.get())
		{
			country::Code	country;
			title::ID		title;

			if (flags & 0x40)
			{
				uint16_t extraneous = bstrm.uint16();

				country = country::Code(extraneous & 0x01ff);
				title = title::ID((extraneous >> 9) & 0x1f);
			}
			else
			{
				country = country::Unknown;
				title = title::None;
			}

			base.appendPlayer(name,
									index,
									country,
									title,
									species::ID(flags & 0x03),
									sex::ID((flags >> 3) & 0x03),
									(flags & 0x80) ? bstrm.uint32() : 0); // fide id
		}
		else
		{
			base.appendPlayer(name, index);
		}

		lookup[index] = i;
	}
}


void
Codec::writeNamebases(mstl::string const& filename)
{
	if (!namebases().isModified())
		return;

	mstl::string namebaseTempFilename(filename + ".temp.38583276");

	//try
	{
		mstl::fstream namebaseStream;
		openFile(namebaseStream, namebaseTempFilename, MagicNamebase, Truncate);
		writeNamebases(namebaseStream);
		namebaseStream.close();
		//sys::file::rename(namebaseTempFilename, filename, true);
	}
	/*catch (...)
	{
		//sys::file::deleteIt(namebaseTempFilename);
		throw;
	}*/
}


void
Codec::writeNamebases(mstl::ostream& stream, util::Progress* progress)
{
	if (!namebases().isModified())
		return;

	if (!stream.seekp(0, mstl::ios_base::beg)){}
		// IO_RAISE(Namebase, Corrupted, "seek failed");

#ifdef USE_LZO
	LzoByteStream bstrm(stream);
#else
	unsigned char	buf[32768];
	ByteOStream		bstrm(stream, buf, sizeof(buf));
#endif

	unsigned total = 0;

	for (unsigned i = 0; i < U_NUMBER_OF(::NamebaseTags); ++i){}
		// total += namebase(Namebase::Type(i)).used();

	ProgressWatcher watcher(progress, total);

	if (progress)
		progress->message("write-namebase");

	if (progress)
	{
		m_progressFrequency		= progress->frequency(total, 1000);
		m_progressReportAfter	= m_progressFrequency;
		m_progressCount			= 0;
	}
	else
	{
		m_progressReportAfter = unsigned(-1);
	}

	bstrm.put(::MagicNamebase, 8);
	bstrm << uint16_t(FileVersion);
	bstrm << uint32_t(total);

	for (unsigned i = 0; i < U_NUMBER_OF(::NamebaseTags); ++i)
	{
		Namebase& base = namebase(Namebase::Type(i));

		bstrm.put(::NamebaseTags[i], 8);
		bstrm << uint24_t(base.used());
		bstrm << uint24_t(base.maxFrequency());
		bstrm << uint24_t(base.maxUsage());
		bstrm << uint24_t(base.nextId());	// NOTE: may cause integer overflow (that means 1<<24 == 0)

		if (base.used() > 0)
		{
			switch (i)
			{
				case Namebase::Site:		writeSitebase(bstrm, base, progress); break;
				case Namebase::Event:	writeEventbase(bstrm, base, progress); break;
				case Namebase::Player:	writePlayerbase(bstrm, base, progress); break;
				default:						writeNamebase(bstrm, base, progress); break;
			}
		}

		if (progress)
		{
			m_progressCount += base.used();
			progress->update(m_progressCount);
			m_progressReportAfter = m_progressFrequency - (m_progressCount % m_progressFrequency);
		}
	}

	bstrm.flush();
	namebases().setModified(false);
}


void
Codec::writeNamebase(ByteStream& bstrm, Namebase& base, util::Progress* progress)
{
	//M_ASSERT(base.used() > 0);

	NamebaseEntry* prev = base.entry(0);

	// //M_ASSERT(prev->name().size() <= 255);

	bstrm << uint24_t(prev->id());
	bstrm.put(prev->name().size());
	bstrm.put(prev->name(), prev->name().size());

	for (unsigned i = 1; i < base.used(); ++i)
	{
		if (m_progressReportAfter <= i)
		{
			// //M_ASSERT(progress);
			progress->update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		NamebaseEntry* entry = base.entry(i);

		unsigned prefix = ::prefix(entry->name(), prev->name());
		unsigned length = entry->name().size();

		// //M_ASSERT(length <= 255);
		// //M_ASSERT(prefix < length);

		bstrm << uint24_t(entry->id());
		bstrm.put(prefix);
		bstrm.put(length);
		bstrm.put(entry->name().c_str() + prefix, length - prefix);

		prev = entry;
	}
}


void
Codec::writeSitebase(ByteStream& bstrm, Namebase& base, util::Progress* progress)
{
	// //M_ASSERT(base.used() > 0);

	NamebaseSite* prev = base.site(0);

	// //M_ASSERT(prev->name().size() <= 255);

	bstrm << uint24_t(prev->id());
	bstrm.put(prev->name().size());
	bstrm.put(prev->name(), prev->name().size());
	bstrm << uint16_t(prev->country());

	for (unsigned i = 1; i < base.used(); ++i)
	{
		if (m_progressReportAfter <= i)
		{
			// //M_ASSERT(progress);
			progress->update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		NamebaseSite* entry = base.site(i);

		unsigned prefix = ::prefix(entry->name(), prev->name());
		unsigned length = entry->name().size();

		// //M_ASSERT(length <= 255);
		// //M_ASSERT(prefix <= length);

		bstrm << uint24_t(entry->id());
		bstrm.put(prefix);
		bstrm.put(length);
		bstrm.put(entry->name().c_str() + prefix, length - prefix);
		bstrm << uint16_t(entry->country());

		prev = entry;
	}
}


void
Codec::writeEventbase(util::ByteStream& bstrm, Namebase& base, util::Progress* progress)
{
	//M_ASSERT(base.used() > 0);

	NamebaseEvent* prev = base.event(0);

	//M_ASSERT(prev->name().size() <= 255);

	bstrm << uint24_t(prev->id());
	bstrm.put(prev->name().size());
	bstrm.put(prev->name(), prev->name().size());

	bstrm << uint24_t(prev->site()->id());

	uint16_t flags =	 (prev->type() & 0x000f)
						 | ((prev->eventMode() & 0x0007) << 4)
						 | ((prev->timeMode() & 0x0007) << 7);

	if (prev->hasDate())
	{
		uint16_t year = Date::encodeYearTo10Bits(prev->dateYear());

		flags |= ((prev->dateDay() & 0x001f) << 10) | 0x8000;

		bstrm << flags;
		bstrm << uint16_t(uint16_t(year & 0x03ff) | (uint16_t(prev->dateMonth() & 0x000f) << 10));
	}
	else
	{
		bstrm << flags;
	}

	for (unsigned i = 1; i < base.used(); ++i)
	{
		if (m_progressReportAfter <= i)
		{
			//M_ASSERT(progress);
			progress->update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		NamebaseEvent* entry = base.event(i);

		unsigned prefix = ::prefix(entry->name(), prev->name());
		unsigned length = entry->name().size();

		// //M_ASSERT(length <= 255);
		// //M_ASSERT(prefix <= length);

		bstrm << uint24_t(entry->id());
		bstrm.put(prefix);
		bstrm.put(length);
		bstrm.put(entry->name().c_str() + prefix, length - prefix);

		bstrm << uint24_t(entry->site()->id());

		flags =	 (entry->type() & 0x000f)
				 | ((entry->eventMode() & 0x0007) << 4)
				 | ((entry->timeMode() & 0x0007) << 7);

		if (entry->hasDate())
		{
			uint16_t year = Date::encodeYearTo10Bits(entry->dateYear());

			flags |= ((entry->dateDay() & 0x001f) << 10) | 0x8000;

			bstrm << flags;
			bstrm << uint16_t(uint16_t(year & 0x03ff) | (uint16_t(entry->dateMonth() & 0x000f) << 10));
		}
		else
		{
			bstrm << flags;
		}

		prev = entry;
	}
}


void
Codec::writePlayerbase(util::ByteStream& bstrm, Namebase& base, util::Progress* progress)
{
	// //M_ASSERT(base.used() > 0);

	NamebasePlayer* prev = base.player(0);

	// //M_ASSERT(prev->name().size() <= 255);

	bstrm << uint24_t(prev->id());
	bstrm.put(prev->name().size());
	bstrm.put(prev->name(), prev->name().size());

	Byte		flags		= (prev->type() & 0x03) | ((prev->sex() & 0x03) << 3);
	uint32_t	fideID	= prev->fideID();

	if (fideID)
		flags |= 0x80;

	if (uint16_t extranouos =	(uint16_t(prev->title() & 0x1f) << 9)
									 | uint16_t(prev->federation() & 0x01ff))
	{
		bstrm.put(flags | 0x40);
		bstrm << extranouos;
	}
	else
	{
		bstrm.put(flags);
	}

	if (fideID)
		bstrm << fideID;

	for (unsigned i = 1; i < base.used(); ++i)
	{
		if (m_progressReportAfter <= i)
		{
			// //M_ASSERT(progress);
			progress->update(i + m_progressCount);
			m_progressReportAfter += m_progressFrequency;
		}

		NamebasePlayer* entry = base.player(i);

		unsigned prefix = ::prefix(entry->name(), prev->name());
		unsigned length = entry->name().size();

		// //M_ASSERT(length <= 255);
		// //M_ASSERT(prefix <= length);

		bstrm << uint24_t(entry->id());
		bstrm.put(prefix);
		bstrm.put(length);
		bstrm.put(entry->name().c_str() + prefix, length - prefix);

		flags = (entry->type() & 0x03) | ((entry->sex() & 0x03) << 3);

		if ((fideID = entry->fideID()))
			flags |= 0x80;

		if (uint16_t extranouos = (uint16_t(entry->title() & 0x1f) << 9)
										 | uint16_t(entry->federation() & 0x01ff))
		{
			bstrm.put(flags | 0x40);
			bstrm << extranouos;
		}
		else
		{
			bstrm.put(flags);
		}

		if (fideID)
			bstrm << fideID;

		prev = entry;
	}
}


void
Codec::useAsyncReader(bool flag)
{
	// //M_ASSERT(m_gameData);

	if (flag)
	{
		if (m_asyncReader == 0)
		{
			// //M_ASSERT(m_gameStream.is_open());
			m_asyncReader = new BlockFile(&m_gameStream, Block_Size, BlockFile::ReadWriteLength);
		}
	}
	else if (m_asyncReader)
	{
		delete m_asyncReader;
		m_asyncReader = 0;
	}
}


Move
Codec::findExactPositionAsync(GameInfo const& info, Board const& position, bool skipVariations)
{
	// //M_ASSERT(m_asyncReader);

	ByteStream src;
	getGameRecord(info, *m_asyncReader, src);
	Decoder decoder(src, m_asyncReader->blockSize() - info.gameOffset());
	return decoder.findExactPosition(position, skipVariations);
}


void
Codec::rename(mstl::string const& oldName, mstl::string const& newName)
{
	static char const Type[3] = { 'i', 'g', 'n' };

	mstl::string oldBase(::util::misc::file::rootname(oldName));
	mstl::string newBase(::util::misc::file::rootname(newName));

	for (unsigned i = 0; i < 3; ++i)
	{
		mstl::string oldFile(oldBase + ".sc" + Type[i]);
		mstl::string newFile(newBase + ".sc" + Type[i]);

		//::sys::file::rename(oldFile, newFile, true);
	}
}


void
Codec::removeAllFiles(mstl::string const& rootname)
{
	//M_ASSERT(!m_gameStream.is_open());
	remove(rootname);
}


void
Codec::remove(mstl::string const& fileName)
{
	mstl::string base(::util::misc::file::rootname(fileName));

	//::sys::file::deleteIt(base + ".scn");
	//::sys::file::deleteIt(base + ".scg");
	//::sys::file::deleteIt(base + ".sci");
}


bool
Codec::getAttributes(mstl::string const& filename,
							int& numGames,
							db::type::ID& type,
							uint32_t& creationTime,
							mstl::string* description)
{
	mstl::fstream strm(filename, mstl::ios_base::in | mstl::ios_base::binary);

	if (!strm)
		return false;

	char header[120];

	strm.seekp(8, mstl::ios_base::beg);

	if (!strm.read(header, description ? sizeof(header) : 24))
		return false;

	ByteStream bstrm(header, sizeof(header));

	bstrm.skip(2);
	numGames	= bstrm.uint24();
	type = type::ID(bstrm.uint8());
	creationTime  = bstrm.uint32();

	if (description)
		bstrm.get(*description);

	strm.close();

	return true;
}


void
Codec::getSuffixes(mstl::string const&, StringList& result)
{
	result.push_back("sci");
	result.push_back("scg");
	result.push_back("scn");
}

// vi:set ts=3 sw=3:
