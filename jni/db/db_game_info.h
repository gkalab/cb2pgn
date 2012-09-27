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

#ifndef _db_game_info_included
#define _db_game_info_included

#include "db_signature.h"
#include "db_date.h"
#include "db_eco.h"

#include "u_crc.h"

#include "m_string.h"

namespace sys  { namespace utf8 { class Codec; } }

namespace db {

namespace sci  { namespace v91 { class Codec; } }
namespace sci  { class Codec; }
namespace si3  { class Codec; }
namespace cbh  { class Codec; }
namespace bits { template <int> struct Accessor; }

class DatabaseContent;
class Namebases;
class NamebaseEntry;
class NamebaseEvent;
class NamebasePlayer;
class Player;
class TagSet;
class Provider;
class Line;

class GameInfo
{
public:

	static unsigned const MaxPlyCount = 0x0fff - 1;

	enum
	{
		// --- coincides with Scid 3.x, except (eliminated) first three flags ---
		Flag_Deleted					= 1 <<  0,	///< Game marked for deletion

		Flag_White_Opening			= 1 <<  1,	///< White openings flag
		Flag_Black_Opening			= 1 <<  2,	///< Black openings flag
		Flag_Middle_Game				= 1 <<  3,	///< Middlegames flag
		Flag_End_Game					= 1 <<  4,	///< Endgames flag
		Flag_Novelty					= 1 <<  5,	///< Novelty flag
		Flag_Pawn_Structure			= 1 <<  6,	///< Pawn structure flag
		Flag_Tactics					= 1 <<  7,	///< Tactics flag
		Flag_King_Side					= 1 <<  8,	///< Kingside play flag
		Flag_Queen_Side				= 1 <<  9,	///< Queenside play flag
		Flag_Brilliancy				= 1 << 10,	///< Brilliancy or good play
		Flag_Blunder					= 1 << 11,	///< Blunder or bad play
		Flag_User						= 1 << 12,	///< User-defined flag

		// --- additional flags, for ChessBase compatibility --------------------
		Flag_Best_Game					= 1 << 13,	///< Best game flag
		Flag_Decided_Tournament		= 1 << 14,	///< Decided tournament flag
		Flag_Model_Game				= 1 << 15,	///< Model game flag
		Flag_Strategy					= 1 << 16,	///< Strategy flag
		Flag_With_Attack				= 1 << 17,	///< With attack flag
		Flag_Sacrifice					= 1 << 18,	///< Sacrifice flag
		Flag_Defense					= 1 << 19,	///< Defense flag
		Flag_Material					= 1 << 20,	///< Material flag
		Flag_Piece_Play				= 1 << 21,	///< Piece Play flag
//		Flag_Tactical_Blunder		= 1 << 22,	///< Tactical blunder flag
//		Flag_Strategical_Blunder	= 1 << 23,	///< Strategical blunder flag

		// --- Scidb specific flags ---------------------------------------------
		Flag_Illegal_Castling		= 1 << 22,	///< Illegal castling flag
		Flag_Illegal_Move				= 1 << 23,	///< Illegal move flag

		// --- temporary flags --------------------------------------------------
		Flag_Dirty						= 1 << 24,	///< Dirty flag
		Flag_Changed					= 1 << 25,	///< Changed flag

		// --- flags for Scid 4.x -----------------------------------------------
		Flag_User1						= Flag_Best_Game,
		Flag_User2						= Flag_Decided_Tournament,
		Flag_User3						= Flag_Model_Game,
		Flag_User4						= Flag_Strategy,
		Flag_User5						= Flag_With_Attack,
		Flag_User6						= Flag_Sacrifice,
	};

	GameInfo();

	bool isEmpty() const;
	bool isDeleted() const;
	bool isDirty() const;
	bool isChanged() const;
	bool hasPromotion() const;								// ChessBase: n/a
	bool hasUnderPromotion() const;						// ChessBase: n/a
	bool containsIllegalMoves() const;
	bool containsIllegalCastlings() const;
	bool isEngine(color::ID color) const;
	bool hasGameRecordLength() const;

	Date date() const;
	unsigned dateYear() const;
	Eco eco() const;
	Eco ecoKey() const;										// ChessBase: n/a
	Eco userEco() const;
	template <int N> uint16_t ply() const;
	bool hasShuffleChessPosition() const;
	bool hasChess960Position() const;
	bool hasStandardPosition() const;
	result::ID result() const;
	uint16_t plyCount() const;								// ChessBase: move count times 2
	uint16_t idn() const;
	mstl::string position() const;
	uint32_t flags() const;
	uint16_t elo(color::ID color) const;
	uint16_t rating(color::ID color) const;			// ChessBase: n/a
	rating::Type ratingType(color::ID color) const;	// ChessBase: n/a
	species::ID playerType(color::ID color) const;
	sex::ID sex(color::ID color) const;
	mstl::string const& event() const;
	country::Code eventCountry() const;
	event::Type eventType() const;
	uint32_t gameOffset() const;
	uint32_t gameRecordLength() const;
	mstl::string const& playerName(color::ID color) const;
	uint32_t fideID(color::ID color) const;
	country::Code federation(color::ID color) const;
	title::ID title(color::ID color) const;
	unsigned round() const;
	unsigned subround() const;
	mstl::string roundAsString() const;
	mstl::string const& site() const;
	mstl::string const& annotator() const;				// Scid: n/a
	bool containsEnglishLanguage() const;
	bool containsOtherLanguage() const;
	uint8_t countComments() const;						// ChessBase: very roughly
	uint8_t countAnnotations() const;					// ChessBase: very roughly
	uint8_t countVariations() const;						// ChessBase: very roughly
	Player const* player(color::ID color) const;
	NamebasePlayer const* playerEntry(color::ID color) const;
	NamebaseEvent const* eventEntry() const;
	material::si3::Signature material() const;
	Signature signature() const;							// ChessBase: n/a; Scid: roughly

	// Scid 3.x: possibly n/a until game is loaded
	Date eventDate() const;

	termination::Reason terminationReason() const;	// Scid/ChessBase: n/a
	event::Mode eventMode() const;						// Scid: n/a
	time::Mode timeMode() const;							// Scid: n/a

	country::Code findFederation(color::ID color) const;
	title::ID findTitle(color::ID color) const;
	species::ID findPlayerType(color::ID color) const;
	sex::ID findSex(color::ID color) const;
	uint16_t findElo(color::ID color) const;
	uint16_t findRating(color::ID color, rating::Type type) const;
	rating::Type findRatingType(color::ID color) const;
	country::Code findEventCountry() const;
	int32_t findFideID(color::ID color) const;

	bool isGameRating(color::ID color, rating::Type type) const;
	uint16_t playerElo(color::ID color) const;
	uint16_t playerRating(color::ID color, rating::Type type) const;
	unsigned moveCount() const;

//	uint16_t playerHighestElo(color::ID color) const;
//	uint16_t playerHighestRating(color::ID color, rating::Type type) const;
//	uint16_t playerLatestElo(color::ID color) const;
//	uint16_t playerLatestRating(color::ID color, rating::Type type) const;
//	rating::Type playerRatingType(color::ID color) const;
//	bool isPlayerRating(color::ID color, rating::Type type) const;

	util::crc::checksum_t computeChecksum(util::crc::checksum_t crc = 0) const;

	void setupTags(TagSet& tags) const;
	void setup(	uint32_t gameOffset,
					uint32_t gameRecordLength);
	void setup(	uint32_t gameOffset,
					uint32_t gameRecordLength,
					NamebasePlayer* whitePlayer,
					NamebasePlayer* blackPlayer,
					NamebaseEvent* event,
					NamebaseEntry* annotator,
					TagSet const& tags,
					Provider const& provider,
					Namebases& namebases);
	void setup(	uint32_t gameOffset,
					uint32_t gameRecordLength,
					NamebasePlayer* whitePlayer,
					NamebasePlayer* blackPlayer,
					NamebaseEvent* event,
					NamebaseEntry* annotator,
					Namebases& namebases);
	void update(NamebasePlayer* whitePlayer,
					NamebasePlayer* blackPlayer,
					NamebaseEvent* event,
					NamebaseEntry* annotator,
					TagSet const& tags,
					Namebases& namebases);
	void update(Provider const& provider);
	template <int N> void setPly(uint16_t move);
	void setLanguageCount(unsigned count);
	void setRecord(uint32_t offset, uint32_t length);
	void setDeleted(bool flag = true);
	void setChanged(bool flag = true);
	void setFlags(unsigned flags);
	void setDirty(bool flag);
	void setIllegalCastling(bool flag);
	void setIllegalMove(bool flag);
	void reset(Namebases& namebases);
	void resetCharacteristics(Namebases& namebases);
	void restore(GameInfo& oldInfo, Namebases& namebases);
	void reallocate(Namebases& namebases);

	void debug() const;

	static void setupTags(TagSet& tags, Provider const& provider);
	static mstl::string& flagsToString(uint32_t flags, mstl::string& result);
	static unsigned stringToFlags(char const* str);
	static char mapFlag(uint32_t flag);

private:

	friend class bits::Accessor<0>;
	friend class bits::Accessor<1>;
	friend class bits::Accessor<2>;
	friend class bits::Accessor<3>;

	struct PlayerData
	{
		union __attribute__((packed))
		{
			struct
			{
				uint32_t elo			:12;
				uint32_t rating		:12;
				uint32_t ratingType	:3;
				uint32_t langFlag		:1;
				uint32_t matQ			:1;
				uint32_t matR			:1;
				uint32_t matB			:1;
				uint32_t matN			:1;
			};

			struct
			{
				uint32_t ratingValue	:27;
				uint32_t __dontUse	:5;
			};

			uint32_t value;
		};
	};

	friend class sci::v91::Codec;
	friend class sci::Codec;
	friend class si3::Codec;
	friend class cbh::Codec;

	struct Initializer {};

	GameInfo(Initializer const&);

	void setRating(color::ID color, rating::Type ratingType, uint16_t value);
	void setMaterial(material::si3::Signature sig);
	void setGameRecordLength(unsigned length);
	void setLangCount(unsigned count);
	void setupOpening(unsigned idn, Line const& line);
	void setupRating(TagSet const& tags, color::ID color, rating::Type rtType, tag::ID tag);

	static void setupIdn(TagSet& tags, uint16_t idn);

	NamebaseEvent*		m_event;
	NamebasePlayer*	m_player[2];

	// .sci, .cbh:	contains pointer to namebase entry (address is always even)
	// .si3, .si4: contains game record length
	union
	{
		NamebaseEntry*	m_annotator;	// Scid: n/a
		unsigned long	m_recordLength;
	};

	PlayerData	m_pd[2];
	Signature	m_signature;

	uint16_t m_variationCount	: 4;
	uint16_t m_annotationCount	: 4;
	uint16_t m_commentCount		: 4;
	uint16_t m_termination		: 4;

	uint32_t m_gameOffset;

	uint64_t m_gameFlags			:26;
	uint64_t m_plyCount			:12;
	uint64_t m_positionId		:12;
	uint64_t m_dateYear			:10;
	uint64_t m_dateMonth			: 4;

	union __attribute__((packed))
	{
		struct
		{
			uint32_t m_ecoKey		:20;
			uint32_t m_eco			: 9;
			uint32_t m_rest		: 3;
		};

		uint16_t m_ply[2];
		uint32_t m_positionData;
	};

	uint32_t m_round				: 8;
	uint32_t m_subround			: 8;
	uint32_t m_dateDay			: 5;
	uint32_t m_result				: 3;
	uint32_t __unused__			: 8;

	static const GameInfo m_initializer;
}
//#if defined(__i386__) || defined(__x86_64__)// Intel/AMD has hardware support for unaligned word access
//__attribute__((packed))
//#endif
;

// NOTE: 64 bytes on all 32 bit platforms (63 bytes if packed on Intel based platforms)
// NOTE: IndexEntry (Scid) has 48 bytes

} // namespace db

namespace mstl {

template <typename T> struct is_movable;

template <> struct is_movable<db::GameInfo>
{
	enum { value = is_pod<db::Date>::value & is_pod<db::Eco>::value };
};

} // namespace mstl

#include "db_game_info.ipp"

#endif // _db_game_info_included

// vi:set ts=3 sw=3:
