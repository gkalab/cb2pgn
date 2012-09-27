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

#ifndef _db_common_included
#define _db_common_included

#include "u_base.h"

namespace mstl { class string; }

namespace db {

typedef unsigned char Byte;
typedef unsigned char Square;

namespace color
{
	// coincides with Scid 3.x
	enum ID { White, Black };

	ID opposite(ID color);

	bool isWhite(ID color);
	bool isBlack(ID color);

	char const* printColor(ID color);
	ID fromSide(char const* side);
}

namespace piece
{
	namespace value
	{
		enum
		{
			Queen		= 9,
			Rook		= 5,
			Bishop	= 3,
			Knight	= 3,
			Pawn		= 1,

			Minor		= 3,
		};
	}

	// coincides with Scid 3.x
	enum Type
	{
		None,
		King,		K = King,
		Queen,	Q = Queen,
		Rook,		R = Rook,
		Bishop,	B = Bishop,
		Knight,	N = Knight,
		Pawn,		P = Pawn,
	};

	// coincides with Scid 3.x
	enum ID
	{
		Empty			= 0,
		WhiteKing	= King,					WK = WhiteKing,
		WhiteQueen	= Queen,					WQ = WhiteQueen,
		WhiteRook	= Rook,					WR = WhiteRook,
		WhiteBishop	= Bishop,				WB = WhiteBishop,
		WhiteKnight	= Knight,				WN = WhiteKnight,
		WhitePawn	= Pawn,					WP = WhitePawn,
		BlackKing	= King	| (1 << 3),	BK = BlackKing,
		BlackQueen	= Queen	| (1 << 3),	BQ = BlackQueen,
		BlackRook	= Rook	| (1 << 3),	BR = BlackRook,
		BlackBishop	= Bishop	| (1 << 3),	BB = BlackBishop,
		BlackKnight	= Knight	| (1 << 3),	BN = BlackKnight,
		BlackPawn	= Pawn	| (1 << 3),	BP = BlackPawn,

		Last = BlackPawn,
	};

	bool isWhite(ID piece);
	bool isBlack(ID piece);
	bool canPromoteTo(Type type);

	Type type(ID piece);
	color::ID color(ID piece);

	ID piece(Type type, db::color::ID color);

	/// Return ASCII character for given piece to be used in FEN.
	char print(ID piece);
	/// Return the ASCII character for a given piece type.
	char print(Type type);

	/// Return the numerical ASCII value for given piece type.
	char printNumeric(Type type);

	Type fromLetter(char piece);
	ID pieceFromLetter(char piece);

	namespace utf8 { mstl::string const& asString(Type type); }
}

namespace sq
{
	// coincides with Scid 3.x
	enum ID
	{
		a1, b1, c1, d1, e1, f1, g1, h1,
		a2, b2, c2, d2, e2, f2, g2, h2,
		a3, b3, c3, d3, e3, f3, g3, h3,
		a4, b4, c4, d4, e4, f4, g4, h4,
		a5, b5, c5, d5, e5, f5, g5, h5,
		a6, b6, c6, d6, e6, f6, g6, h6,
		a7, b7, c7, d7, e7, f7, g7, h7,
		a8, b8, c8, d8, e8, f8, g8, h8,
		Null = 65,
	};

	enum Fyle { FyleA, FyleB, FyleC, FyleD, FyleE, FyleF, FyleG, FyleH };
	enum Rank { Rank1, Rank2, Rank3, Rank4, Rank5, Rank6, Rank7, Rank8 };

	ID make(Square fyle, Square rank);
	ID make(char const* s);

	Fyle fyle(Square square);
	Rank rank(Square square);

	ID rankToSquare(Rank rank);

	color::ID color(ID s);

	int fyleDistance(ID a, ID b);
	int rankDistance(ID a, ID b);

	int distance(ID a, ID b);

	ID flipFyle(ID s);
	ID flipRank(ID s);

	Rank flipRank(Rank rank);

	Rank homeRank(db::color::ID color);
	Rank pawnRank(db::color::ID color);

	bool isValid(char const* s);
	bool isAdjacent(ID a, ID b);

	char printFyle(Square square);
	char printFYLE(Square square);
	char printRank(Square square);
	char printRank(Rank rank);

	char const* printAlgebraic(Square square);
	char const* printNumeric(Square square);
	char const* printAlphabetic(Square square);
	char const* printDescriptive(Square square);
	char const* printDescriptive(Square square, color::ID color);
}

namespace opening
{
	enum { Max_Line_Length = 41 };
}

namespace material
{
	struct Count
	{
		union
		{
			struct
			{
				uint32_t pawn  :4;
				uint32_t knight:4;
				uint32_t bishop:4;
				uint32_t rook  :4;
				uint32_t queen :4;
				uint32_t king  :4;
			};

			struct
			{
				uint32_t value:24;
				uint32_t _rest: 8;
			};
		};
	};

	struct SigPart
	{
		union
		{
			struct
			{
				uint16_t pawn	:8;
				uint16_t knight:2;
				uint16_t bishop:2;
				uint16_t rook	:2;
				uint16_t queen	:2;
			};

			struct
			{
				uint16_t __skip_pawn_1_:8;
				uint16_t minor:4;
				uint16_t major:4;
			};

			struct
			{
				uint16_t __skip_pawn_2_:8;
				uint16_t piece:8;
			};

			struct
			{
				uint16_t butNotQueen:14;
				uint16_t __skip_queen_1_:2;
			};

			uint16_t value;
		};
	};

	struct Signature
	{
		union
		{
			struct { SigPart part[2]; };
			uint32_t value;
		};
	};

	unsigned count(Signature sig);

	unsigned count(SigPart sig);
	unsigned minor(SigPart sig);
	unsigned major(SigPart sig);

	mstl::string& print(SigPart sig, mstl::string& result);
	mstl::string& print(Signature signature, mstl::string& result);

	namespace si3
	{
		// compatible to matSigT (Scid 3.x)
		union Signature
		{
			Signature();
			Signature(uint32_t sig);

			struct
			{
				uint32_t bp:4;
				uint32_t bn:2;
				uint32_t bb:2;
				uint32_t br:2;
				uint32_t bq:2;

				uint32_t wp:4;
				uint32_t wn:2;
				uint32_t wb:2;
				uint32_t wr:2;
				uint32_t wq:2;

				uint32_t __unused_1:8;
			};

			struct
			{
				uint32_t value:24;
				uint32_t __unused_2:8;
			};

			uint32_t u32;
		};

		mstl::string& print(Signature signature, mstl::string& result);

		namespace utf8
		{
			mstl::string& print(Signature signature, mstl::string& result);
		}
	};
}

namespace hp
{
	union Pawns
	{
		struct { unsigned char bytes[8]; };
		uint64_t value;
	};
}

namespace pawns
{
	union Side
	{
		struct { uint8_t rank[2]; };
		uint16_t rankValue;

		bool test(uint8_t rank, uint8_t fyle);
		bool testRank2(uint8_t fyle);

		void add(uint8_t square);
		void remove(uint8_t square);
		void move(uint8_t from, uint8_t to);
	}
	__attribute__((packed));

	union Progress
	{
		struct { Side side[2]; };
		uint32_t value;
	}
	__attribute__((packed));

	mstl::string& print(Progress progrss, color::ID color, mstl::string& result);
}

namespace tb
{
	enum
	{
		Not_Found			= ~0u,				// position not found
		Illegal_Position	= Not_Found + 1,	// illegal position
		Broken				= Not_Found + 2,	// broken data
		Incomplete_Data	= Not_Found + 3,	// incomplete tablebases
		Any_Move				= Not_Found + 4,	// any (legal) move is the best
		No_Legal_Move		= Not_Found + 5,	// no legal move possible
		Is_Check_Mate		= Not_Found + 6,	// side to move is check mate
		Is_Stale_Mate		= Not_Found + 7,	// no legal move
	};

	bool isError(int score);
	bool isScore(int score);
}

namespace result
{
	// coincides with Scid 3.x (only first four entries)
	enum ID
	{
		Unknown,	///< Game still in progress, game abandoned, or result otherwise unknown
		White,	///< White won the game
		Black,	///< Black won the game
		Draw,		///< The game was a draw
		Lost,		///< The game was declared lost for both players
	};

	mstl::string const& toString(ID result);
	ID fromString(mstl::string const& s);
	ID opponent(ID result);
	ID fromColor(color::ID color);
	unsigned value(ID result);
}

namespace castling
{
	enum Handicap { DontAllowHandicap, AllowHandicap };

	enum Index
	{
		WhiteQS = 0,
		WhiteKS = 1,
		BlackQS = 2,
		BlackKS = 3,
	};

	Index kingSideIndex(color::ID color);
	Index queenSideIndex(color::ID color);

	// coincides with Scid 3.x
	enum Rights
	{
		NoRights			= 0,
		WhiteQueenside	= 1 << WhiteQS,
		WhiteKingside	= 1 << WhiteKS,
		BlackQueenside	= 1 << BlackQS,
		BlackKingside	= 1 << BlackKS,
		Kingside			= WhiteKingside | BlackKingside,
		Queenside		= WhiteQueenside | BlackQueenside,
		WhiteBothSides	= WhiteKingside | WhiteQueenside,
		BlackBothSides	= BlackKingside | BlackQueenside,
		AllRights		= WhiteBothSides | BlackBothSides,
	};

	Rights kingSide(color::ID color);
	Rights queenSide(color::ID color);
	Rights bothSides(color::ID color);

	unsigned transpose(unsigned rights);

	mstl::string& print(Rights rights, mstl::string& result);

	void initialize();		// only a hack for corrupted systems like Deb ian Wheezy

} // namespace castling

namespace move
{
	enum Constraint	{ DontAllowIllegalMove, AllowIllegalMove };
	enum Position		{ Ante, Post };

	enum Notation
	{
		Algebraic, ShortAlgebraic, LongAlgebraic, Descriptive,  Correspondence, Telegraphic
	};

} // namespace move

namespace tag
{
	enum ID
	{
//#define ORD(x) = x	// use this if the tags should be ordered alphabetically
#define ORD(x)
		// mandatory tags (seven tag roster)
		Event				=  0,	///< Name of the tournament or match event
		Site				=  1,	///< Location of the event
		Date				=  2,	///< Starting date of the game
		Round				=  3,	///< Playing round ordinal of the game
		White				=  4,	///< Player of the White pieces
		Black				=  5,	///< Player of the Black pieces
		Result			=  6,	///< Result of the game

		// event related information
		EventDate		ORD(29),	///< Starting date of the event
		EventCountry	ORD(28),	///< Country of the event (.e.g. "GER")
		EventType		ORD(31),	///< Type of the event (.e.g. "tourn")
		EventRounds		ORD(30),	///< Number of rounds of the event
		EventCategory	ORD(27),	///< Category of the event

		// opening information
		Eco				ORD(26),
		Opening			ORD(34),
		Variation		ORD(46),
		SubVariation	ORD(41),

		// chess variants
		Variant			ORD(45),

		// alternative starting positions
		SetUp				ORD(38),	///< Denotes the "set-up" status of the game
		Fen				ORD(32),	///< Position at the start of the game
		Idn				ORD(36),	///< Unique position IDentification Number (chess 960 position number)

		// game conclusion
		Termination		ORD(42),	///< Describes the reason for the conclusion of the game

		// time information
		TimeControl		ORD(43),	///< Describes the time control.
		TimeMode			ORD(44),	///< Describes the time mode (normal, rapid, blitz, corr)

		// player related information
		WhiteCountry	ORD(48),	BlackCountry	ORD( 9),
		WhiteTitle		ORD(61),	BlackTitle		ORD(22),	///< Titles of the players
		WhiteNA			ORD(55),	BlackNA			ORD(16),	///< E-mail or network addresses
		WhiteType		ORD(62),	BlackType		ORD(23),	///< Player types ("human" or "program")
		WhiteSex			ORD(58),	BlackSex			ORD(19),	///< Sex of the players ("m" or "f")
		WhiteFideId		ORD(52),	BlackFideId		ORD(13),	///< Fide ID of the players
		WhiteClock		ORD(47),	BlackClock		ORD( 8),	///< Time at end of game

		// rating types; should be ordered due to rating type order
		WhiteElo			ORD(51),	///< FIDE Rating
		WhiteRating		ORD(57),	///< CCRL Computer Rating
		WhiteRapid		ORD(56),	///< English Chess Federation Rapid Rating
		WhiteICCF		ORD(53),	///< International Correspondence Chess Federation
		WhiteUSCF		ORD(63),	///< United States Chess Federation
		WhiteDWZ			ORD(49),	///< Deutsche Wertungszahl
		WhiteECF			ORD(50),	///< English Chess Federation
		WhiteIPS			ORD(54),	///< Individual Player Strength (Chess 960 Rating)

		BlackElo			ORD(12),	///< FIDE Rating
		BlackRating		ORD(18),	///< CCRL Computer Rating
		BlackRapid		ORD(17),	///< English Chess Federation Rapid Rating
		BlackICCF		ORD(14),	///< International Correspondence Chess Federation
		BlackUSCF		ORD(24),	///< United States Chess Federation
		BlackDWZ			ORD(10),	///< Deutsche Wertungszahl
		BlackECF			ORD(11),	///< English Chess Federation
		BlackIPS			ORD(15),	///< Individual Player Strength (Chess 960 Rating)

		// team related information
		WhiteTeam			ORD(59),	BlackTeam			ORD(20),
		WhiteTeamCountry	ORD(60),	BlackTeamCountry	ORD(21),

		// miscellaneous
		Annotator		ORD( 7),	///< Identifies the annotator or annotators of the game
		Mode				ORD(33),	///< Playing mode of the game (e.g. "OTB" over the board)
		Source			ORD(39),	///< The provider of the game annotation (e.g. "ChessBase")
		SourceDate		ORD(40),	///< The date when the game is provided
		PlyCount			ORD(35),	///< The number of ply (moves) in the game
		Remark			ORD(37),	///< Any comment to this game
		Board				ORD(25),	///< The board number

		// # of tags
		ExtraTag			ORD(64),
#undef ORD
	};

	bool isMandatory(ID tag);
	bool isRatingTag(ID tag);
	bool isWhiteRatingTag(ID tag);
	bool isBlackRatingTag(ID tag);

	mstl::string const& toName(ID tag);
	ID fromName(mstl::string const& tag);
	ID fromName(char const* name, unsigned length);

	void initialize();		// only a hack for corrupted systems like Deb ian Wheezy
	bool initializeIsOk();	// only a hack for g++-4.7

} // namespace tag

namespace species
{
	enum ID
	{
		Unspecified,
		Human,
		Program,
	};

	char toChar(ID type);
	mstl::string const& toString(ID type);
	ID fromString(char const* s);

} // namespace species

namespace title
{
	enum ID
	{
		None,	///< No title
		GM,	///< Grandmaster (FIDE)
		IM,	///< International Master (FIDE)
		FM,	///< Fide Master (FIDE)
		CM,	///< Candidate Master (FIDE)
		WGM,	///< Woman Grandmaster (FIDE)
		WIM,	///< Woman International Master (FIDE)
		WFM,	///< Woman Fide Master (FIDE)
		WCM,	///< Woman Candidate Master (FIDE)
		HGM,	///< Honorary Grandmaster (FIDE)
		NM,	///< National Master (USCF)
		SM,	///< Senior Master (USCF)
		LM,	///< Life Master (USCF)
		CGM,	///< Correspondence Grandmaster (ICCF)
		CIM,	///< Correspondence International Master (ICCF)
		CLGM,	///< Correspondence Lady Grandmaster (ICCF)
		CILM,	///< Correspondence Lady International Master (ICCF)
		CSIM,	///< Correspondence Senior International Master (ICCF)

		Last,
	};

	enum
	{
		Mask_GM		= 1 << (GM   - 1),
		Mask_IM		= 1 << (IM   - 1),
		Mask_FM		= 1 << (FM   - 1),
		Mask_CM		= 1 << (CM   - 1),
		Mask_WGM		= 1 << (WGM  - 1),
		Mask_WIM		= 1 << (WIM  - 1),
		Mask_WFM		= 1 << (WFM  - 1),
		Mask_WCM		= 1 << (WCM  - 1),
		Mask_HGM		= 1 << (HGM  - 1),
		Mask_NM		= 1 << (NM   - 1),
		Mask_SM		= 1 << (SM   - 1),
		Mask_LM		= 1 << (LM   - 1),
		Mask_CGM		= 1 << (CGM  - 1),
		Mask_CIM		= 1 << (CIM  - 1),
		Mask_CLGM	= 1 << (CLGM - 1),
		Mask_CILM	= 1 << (CILM - 1),
		Mask_CSIM	= 1 << (CSIM - 1),
	};

	ID toID(unsigned title);
	unsigned fromID(ID title);

	mstl::string const& toString(ID title);
	ID fromString(char const* title);

	title::ID best(unsigned titles);
	bool contains(unsigned titles, title::ID title);
	bool containsFemaleTtile(unsigned titles);
}

namespace sex
{
	enum ID
	{
		Unspecified,
		Male,
		Female,
	};

	char toChar(ID sex);
	mstl::string const& toString(ID sex);
	ID fromChar(char sex);
	ID fromString(char const* sex);
}

namespace rating
{
	enum { Max_Value = 4000 };

	// first seven coincides with Scid
	enum Type
	{
		Elo,			///< FIDE rating
		Rating,		///< CCRL Rating (Computer Rating) [Scid in not defining this rating type]
		Rapid,		///< For compatibility with Scid (possibly ECF rapid rating is meant)
		ICCF,			///< International Correspondence Chess Federation rating
		USCF,			///< United States Chess Federation rating
		DWZ,			///< Deutsche Wertungszahl (German rating)
		ECF,			///< English Chess Federation rating (formerly BCF)
		IPS,			///< Individual Player Strength (WNCA rating) [not known in Scid]
		Any,			///< Any rating type

		Last = Any,	///< last index
	};

	tag::ID toWhiteTag(rating::Type type);
	tag::ID toBlackTag(rating::Type type);

	unsigned convertEloToUscf(unsigned elo);
	unsigned convertEloToEcf(unsigned elo);
	unsigned convertUscfToElo(unsigned uscf);
	unsigned convertEcfToElo(unsigned ecf);
	unsigned convert(unsigned rating, Type from, Type to);

	mstl::string const& toString(Type type);
	Type fromString(char const* s);
}

namespace termination
{
	enum Reason
	{
		Unknown,				///< Termination reason not known
		Normal,				///< Game terminated in a normal fashion
		Unplayed,			///< Game not played but one party gets the point
		Abandoned,			///< Abandoned game
		Adjudication,		///< Result due to third party adjudication process
		Death,				///< Game concluded due to death of a player
		Emergency,			///< Game concluded due to unforeseen circumstances
		RulesInfraction,	///< Administrative forfeit due to losing player's failure
		TimeForfeit,		///< Loss due to losing player's failure to meet time control requirements
		Unterminated,		///< Game not terminated
	};

	mstl::string const& toString(Reason reason);
	Reason fromString(mstl::string const& s);
}

namespace time
{
	enum Mode
	{
		Unknown,
		Normal,
		Rapid,
		Blitz,
		Bullet,
		Corr,
	};

	mstl::string const& toString(Mode time);
	Mode fromString(mstl::string const& s);
}

namespace event
{
	// coincides with internal values in CBH
	enum Type
	{
		Unknown,
		Game,				///< Game
		Match,			///< Match
		Tournament,		///< Tournament (all play all)
		Swiss,			///< Swiss-System Tournament
		Team,				///< Team Tournament
		Knockout,		///< Knockout Tournament
		Simultan,		///< Simultaneous Tournament
		Schev,			///< Scheveningen-System Tournament
	};

	enum Mode
	{
		Undetermined,
		OverTheBoard,			///< Over The Board
		PaperMail,				///< Paper Mail (Correspondence)
		Email,					///< E-Mail (includes Xfcc)
		Internet,				///< Internet Chess Server
		Telecommunication,	///< (general) Telecommunication
		Analysis,				///< Analysis
		Composition,			///< Composition
	};

	Type typeFromString(mstl::string const& s);
	Mode modeFromString(mstl::string const& s);

	mstl::string const& toString(Type type);
	mstl::string const& toString(Mode mode);
}

namespace chess960
{
	enum
	{
		StandardIdn		= 518,
		TransposedIdn	= 534,
	};

	unsigned twin(unsigned idn);
	unsigned lookup(mstl::string const& position);

	mstl::string fen(unsigned idn);
	mstl::string const& position(unsigned idn);
	mstl::string const& identifier();

	namespace utf8 { mstl::string& position(unsigned idn, mstl::string& result); }
}


namespace shuffle
{
	unsigned twin(unsigned idn);
	unsigned lookup(mstl::string const& position);

	mstl::string fen(unsigned idn);
	mstl::string position(unsigned idn);
	mstl::string const& identifier();

	namespace utf8 { mstl::string& position(unsigned idn, mstl::string& result); }
}


namespace variant
{
	enum Type { Unknown, Standard, Chess960, Shuffle, Other };

	Type fromIdn(unsigned idn);
}

namespace mark
{
	enum Command	{ None, Draw, Diagram };
	enum Type	{ Text, Full, Square, Arrow, Circle, Disk };
	enum Color	{ Red, Orange, Yellow, Green, Blue, DarkBlue, Purple, White, Black };

	char const* commandName(Command cmd);
	char const* colorName(Color color);
	char const* typeName(Type type);

	Type typeFromString(char const* type);
	Color colorFromString(char const* color);

	bool isTextChar(char c);
}

namespace tree
{
	enum Mode { Exact, Fast, Rapid, };
}

namespace format
{
	enum Type
	{
		Scidb			= 1 << 0,
		Scid3			= 1 << 1,
		Scid4			= 1 << 2,
		ChessBase	= 1 << 3,
		Pgn			= 1 << 4,
		LaTeX			= 1 << 5,

		Invalid		= LaTeX, // makes sense if used for input format
	};

	bool isScidFormat(Type type);
}

namespace encoding
{
	enum CharSet { Latin1, Utf8 };
}

namespace nag
{
	enum ID
	{
		/*   0 */ Null,

		// Standard NAG Values
		/*   1 */ GoodMove,
		/*   2 */ PoorMove,
		/*   3 */ VeryGoodMove,
		/*   4 */ VeryPoorMove,
		/*   5 */ SpeculativeMove,
		/*   6 */ QuestionableMove,
		/*   7 */ ForcedMove,
		/*   8 */ SingularMove,
		/*   9 */ WorstMove,
		/*  10 */ DrawishPosition,
		/*  11 */ EqualChancesQuietPosition,
		/*  12 */ EqualChancesActivePosition,
		/*  13 */ UnclearPosition,
		/*  14 */ WhiteHasASlightAdvantage,
		/*  15 */ BlackHasASlightAdvantage,
		/*  16 */ WhiteHasAModerateAdvantage,
		/*  17 */ BlackHasAModerateAdvantage,
		/*  18 */ WhiteHasADecisiveAdvantage,
		/*  19 */ BlackHasADecisiveAdvantage,
		/*  20 */ WhiteHasACrushingAdvantage,
		/*  21 */ BlackHasACrushingAdvantage,
		/*  22 */ WhiteIsInZugzwang,
		/*  23 */ BlackIsInZugzwang,
		/*  24 */ WhiteHasASlightSpaceAdvantage,
		/*  25 */ BlackHasASlightSpaceAdvantage,
		/*  26 */ WhiteHasAModerateSpaceAdvantage,
		/*  27 */ BlackHasAModerateSpaceAdvantage,
		/*  28 */ WhiteHasADecisiveSpaceAdvantage,
		/*  29 */ BlackHasADecisiveSpaceAdvantage,
		/*  30 */ WhiteHasASlightTimeAdvantage,
		/*  31 */ BlackHasASlightTimeAdvantage,
		/*  32 */ WhiteHasAModerateTimeAdvantage,
		/*  33 */ BlackHasAModerateTimeAdvantage,
		/*  34 */ WhiteHasADecisiveTimeAdvantage,
		/*  35 */ BlackHasADecisiveTimeAdvantage,
		/*  36 */ WhiteHasTheInitiative,
		/*  37 */ BlackHasTheInitiative,
		/*  38 */ WhiteHasALastingInitiative,
		/*  39 */ BlackHasALastingInitiative,
		/*  40 */ WhiteHasTheAttack,
		/*  41 */ BlackHasTheAttack,
		/*  42 */ WhiteHasInsufficientCompensationForMaterialDeficit,
		/*  43 */ BlackHasInsufficientCompensationForMaterialDeficit,
		/*  44 */ WhiteHasSufficientCompensationForMaterialDeficit,
		/*  45 */ BlackHasSufficientCompensationForMaterialDeficit,
		/*  46 */ WhiteHasMoreThanAdequateCompensationForMaterialDeficit,
		/*  47 */ BlackHasMoreThanAdequateCompensationForMaterialDeficit,
		/*  48 */ WhiteHasASlightCenterControlAdvantage,
		/*  49 */ BlackHasASlightCenterControlAdvantage,
		/*  50 */ WhiteHasAModerateCenterControlAdvantage,
		/*  51 */ BlackHasAModerateCenterControlAdvantage,
		/*  52 */ WhiteHasADecisiveCenterControlAdvantage,
		/*  53 */ BlackHasADecisiveCenterControlAdvantage,
		/*  54 */ WhiteHasASlightKingsideControlAdvantage,
		/*  55 */ BlackHasASlightKingsideControlAdvantage,
		/*  56 */ WhiteHasAModerateKingsideControlAdvantage,
		/*  57 */ BlackHasAModerateKingsideControlAdvantage,
		/*  58 */ WhiteHasADecisiveKingsideControlAdvantage,
		/*  59 */ BlackHasADecisiveKingsideControlAdvantage,
		/*  60 */ WhiteHasASlightQueensideControlAdvantage,
		/*  61 */ BlackHasASlightQueensideControlAdvantage,
		/*  62 */ WhiteHasAModerateQueensideControlAdvantage,
		/*  63 */ BlackHasAModerateQueensideControlAdvantage,
		/*  64 */ WhiteHasADecisiveQueensideControlAdvantage,
		/*  65 */ BlackHasADecisiveQueensideControlAdvantage,
		/*  66 */ WhiteHasAVulnerableFirstRank,
		/*  67 */ BlackHasAVulnerableFirstRank,
		/*  68 */ WhiteHasAWellProtectedFirstRank,
		/*  69 */ BlackHasAWellProtectedFirstRank,
		/*  70 */ WhiteHasAPoorlyProtectedKing,
		/*  71 */ BlackHasAPoorlyProtectedKing,
		/*  72 */ WhiteHasAWellProtectedKing,
		/*  73 */ BlackHasAWellProtectedKing,
		/*  74 */ WhiteHasAPoorlyPlacedKing,
		/*  75 */ BlackHasAPoorlyPlacedKing,
		/*  76 */ WhiteHasAWellPlacedKing,
		/*  77 */ BlackHasAWellPlacedKing,
		/*  78 */ WhiteHasAVeryWeakPawnStructure,
		/*  79 */ BlackHasAVeryWeakPawnStructure,
		/*  80 */ WhiteHasAModeratelyWeakPawnStructure,
		/*  81 */ BlackHasAModeratelyWeakPawnStructure,
		/*  82 */ WhiteHasAModeratelyStrongPawnStructure,
		/*  83 */ BlackHasAModeratelyStrongPawnStructure,
		/*  84 */ WhiteHasAVeryStrongPawnStructure,
		/*  85 */ BlackHasAVeryStrongPawnStructure,
		/*  86 */ WhiteHasPoorKnightPlacement,
		/*  87 */ BlackHasPoorKnightPlacement,
		/*  88 */ WhiteHasGoodKnightPlacement,
		/*  89 */ BlackHasGoodKnightPlacement,
		/*  90 */ WhiteHasPoorBishopPlacement,
		/*  91 */ BlackHasPoorBishopPlacement,
		/*  92 */ WhiteHasGoodBishopPlacement,
		/*  93 */ BlackHasGoodBishopPlacement,
		/*  94 */ WhiteHasPoorRookPlacement,
		/*  95 */ BlackHasPoorRookPlacement,
		/*  96 */ WhiteHasGoodRookPlacement,
		/*  97 */ BlackHasGoodRookPlacement,
		/*  98 */ WhiteHasPoorQueenPlacement,
		/*  99 */ BlackHasPoorQueenPlacement,
		/* 100 */ WhiteHasGoodQueenPlacement,
		/* 101 */ BlackHasGoodQueenPlacement,
		/* 102 */ WhiteHasPoorPieceCoordination,
		/* 103 */ BlackHasPoorPieceCoordination,
		/* 104 */ WhiteHasGoodPieceCoordination,
		/* 105 */ BlackHasGoodPieceCoordination,
		/* 106 */ WhiteHasPlayedTheOpeningVeryPoorly,
		/* 107 */ BlackHasPlayedTheOpeningVeryPoorly,
		/* 108 */ WhiteHasPlayedTheOpeningPoorly,
		/* 109 */ BlackHasPlayedTheOpeningPoorly,
		/* 110 */ WhiteHasPlayedTheOpeningWell,
		/* 111 */ BlackHasPlayedTheOpeningWell,
		/* 112 */ WhiteHasPlayedTheOpeningVeryWell,
		/* 113 */ BlackHasPlayedTheOpeningVeryWell,
		/* 114 */ WhiteHasPlayedTheMiddlegameVeryPoorly,
		/* 115 */ BlackHasPlayedTheMiddlegameVeryPoorly,
		/* 116 */ WhiteHasPlayedTheMiddlegamePoorly,
		/* 117 */ BlackHasPlayedTheMiddlegamePoorly,
		/* 118 */ WhiteHasPlayedTheMiddlegameWell,
		/* 119 */ BlackHasPlayedTheMiddlegameWell,
		/* 120 */ WhiteHasPlayedTheMiddlegameVeryWell,
		/* 121 */ BlackHasPlayedTheMiddlegameVeryWell,
		/* 122 */ WhiteHasPlayedTheEndingVeryPoorly,
		/* 123 */ BlackHasPlayedTheEndingVeryPoorly,
		/* 124 */ WhiteHasPlayedTheEndingPoorly,
		/* 125 */ BlackHasPlayedTheEndingPoorly,
		/* 126 */ WhiteHasPlayedTheEndingWell,
		/* 127 */ BlackHasPlayedTheEndingWell,
		/* 128 */ WhiteHasPlayedTheEndingVeryWell,
		/* 129 */ BlackHasPlayedTheEndingVeryWell,
		/* 130 */ WhiteHasSlightCounterplay,
		/* 131 */ BlackHasSlightCounterplay,
		/* 132 */ WhiteHasModerateCounterplay,
		/* 133 */ BlackHasModerateCounterplay,
		/* 134 */ WhiteHasDecisiveCounterplay,
		/* 135 */ BlackHasDecisiveCounterplay,
		/* 136 */ WhiteHasModerateTimeControlPressure,
		/* 137 */ BlackHasModerateTimeControlPressure,
		/* 138 */ WhiteHasSevereTimeControlPressure,
		/* 139 */ BlackHasSevereTimeControlPressure,

		// More suggested NAG Values (for Informator Symbols, etc.)
		// Coincides with Scid 3.x; 140 - 146 coincides with ChessPad
		/* 140 */ WithTheIdea, Pgn_Last = WithTheIdea,
		/* 141 */ AimedAgainst,
		/* 142 */ BetterMove,
		/* 143 */ WorseMove,
		/* 144 */ EquivalentMove,
		/* 145 */ EditorsRemark,
		/* 146 */ Novelty,
		/* 147 */ WeakPoint,
		/* 148 */ Endgame,
		/* 149 */ Line,
		/* 150 */ Diagonal,
		/* 151 */ WhiteHasAPairOfBishops,
		/* 152 */ BlackHasAPairOfBishops,
		/* 153 */ BishopsOfOppositeColor,
		/* 154 */ BishopsOfSameColor,

		// Scidb specific:
		/* 155 */ Diagram, Scidb_Specific = Diagram,
		/* 156 */ DiagramFromBlack,
		/* 157 */ SeparatedPawns,
		/* 158 */ DoublePawns,
		/* 159 */ UnitedPawns,
		/* 160 */ PassedPawn,
		/* 161 */ HangingPawns,
		/* 162 */ BackwardPawns,
		/* 163 */ MorePawns,
		/* 164 */ MoreRoom,
		/* 165 */ With,
		/* 166 */ Without,
		/* 167 */ Center,
		/* 168 */ File,
		/* 169 */ Rank,
		/* 170 */ See,
		/* 171 */ Various,
		/* 172 */ Etc,

		// Extensions for ChessBase support (used in commentaries):
		/* 173 */ Space, ChessBase_Specific = Space,
		/* 174 */ Zeitnot,
		/* 175 */ Development,
		/* 176 */ Zugzwang,
		/* 177 */ TimeLimit,
		/* 178 */ Attack,
		/* 179 */ Initiative,
		/* 180 */ Counterplay,
		/* 181 */ WithCompensationForMaterial,
		/* 182 */ PairOfBishops,
		/* 183 */ Kingside,
		/* 184 */ Queenside,

		// # of NAG values
		Scidb_Last,

		// Scid 3.x specific:
		Scid3_Etc						= 190, Scid3_Specific = Scid3_Etc,
		Scid3_DoublePawns				= 191,
		Scid3_SeparatedPawns			= 192,
		Scid3_UnitedPawns				= 193,
		Scid3_HangingPawns			= 194,
		Scid3_BackwardPawns			= 195,
		Scid3_Diagram					= 201,

		Scid3_Last,

		// ChessPad specific:
		ChessPad_Diagram				= 220, ChessPad_Specific = ChessPad_Diagram,
		ChessPad_DiagramFromBlack	= 221,
		ChessPad_SpaceAdvantage		= 238,
		ChessPad_File					= 239,
		ChessPad_Diagonal				= 240,
		ChessPad_Center				= 241,
		ChessPad_Kingside				= 242,
		ChessPad_Queenside			= 243,
		ChessPad_WeakPoint			= 244,
		ChessPad_Ending				= 245,
		ChessPad_BishopPair			= 246,
		ChessPad_OppositeBishops	= 247,
		ChessPad_SameBishops			= 248,
		ChessPad_ConnectedPawns		= 249,
		ChessPad_IsolatedPawns		= 250,
		ChessPad_DoubledPawns		= 251,
		ChessPad_PassedPawn			= 252,
		ChessPad_PawnMajority		= 253,
		ChessPad_With					= 254,
		ChessPad_Without				= 255,

		ChessPad_Last,

		// Jose specific:
		Jose_Diagram					= 250,
	};

	bool isPrefix(ID nag);
	bool isInfix(ID nag);
	bool isSuffix(ID nag);

	ID fromScid3(ID nag);
	ID toScid3(ID nag);

	ID fromChessPad(ID nag);

	char const* toSymbol(ID nag);
	ID fromSymbol(mstl::string const& symbol);
	ID fromSymbol(char const* symbol);
	ID fromSymbol(char const* symbol, unsigned len);
}

namespace save
{
	enum State
	{
		Ok,
		UnsupportedVariant,
		DecodingFailed,
		TooManyGames,
		FileSizeExeeded,
		GameTooLong,
		TooManyPlayerNames,
		TooManyEventNames,
		TooManySiteNames,
		TooManyRoundNames,
		TooManyAnnotatorNames,
	};

	bool isOk(State state);
}


namespace load
{
	enum State
	{
		Ok,
		None,
		Failed,
		Corrupted,
	};
};

namespace display
{
	enum
	{
		CompactStyle			= 1 << 0,
		ColumnStyle				= 1 << 1,
		ParagraphSpacing		= 1 << 2,
		ShowDiagrams			= 1 << 3,
		ShowMoveInfo			= 1 << 4,
		ShowVariationNumbers	= 1 << 5,
	};
};

namespace type
{
	enum ID
	{
		Unspecific,
		Temporary,					///< Temporary database (e.g. PGN Database)
		Work,							///< Working database
		Clipbase,					///< Clipbase
		My_Games,					///< My games
		Informant,					///< Informant games
		Large_Database,			///< Large database
		Correspondence_Chess,	///< Correspondence Chess
		Email_Chess,				///< E-Mail games
		Internet_Chess,			///< Internet Chess
		Computer_Chess,			///< Computer Chess
		Chess_960,					///< Chess 960 (Fischer random chess)
		Player_Collection,		///< Player collection
		Tournament,					///< Tournament: All-play-all
		Tournament_Swiss,			///< Tournament: Swiss
		GM_Games,					///< Grandmaster games
		IM_Games,					///< International master games
		Blitz_Games,				///< Blitz (fast) games
		Tactics,						///< Tactical games
		Endgames,					///< Endgames
		Analysis,					///< Analysis games
		Training,					///< Training games
		Match,						///< Games from matches
		Studies,						///< Studies games
		Jewels,						///< Jewels (exceptional games)
		Problems,					///< Chess problems
		Patzer,						///< Patzer games
		Gambit,						///< Gambit games
		Important,					///< Important games
		Openings_White,			///< Openings for White
		Openings_Black,			///< Openings for Black
		Openings,					///< Openings for either color
	};
}

namespace attribute
{
	namespace game
	{
		enum ID
		{
			Number,
			WhitePlayer,
			WhiteFideID,
			WhiteRating1,
			WhiteRating2,
			WhiteRatingType,
			WhiteCountry,
			WhiteTitle,
			WhiteType,
			WhiteSex,
			BlackPlayer,
			BlackFideID,
			BlackRating1,
			BlackRating2,
			BlackRatingType,
			BlackCountry,
			BlackTitle,
			BlackType,
			BlackSex,
			Event,
			EventType,
			EventDate,
			Result,
			EventCountry,
			Site,
			Date,
			Round,
			Annotator,
			Idn,
			Position,
			Length,
			Eco,
			Flags,
			Material,
			Deleted,
			Acv,
			CommentEngFlag,
			CommentOthFlag,
			Changed,
			Promotion,
			UnderPromotion,
			StandardPosition,
			Chess960Position,
			Termination,
			Mode,
			TimeMode,
			Overview,

			// additional attributes
			Opening,
			Variation,
			SubVariation,
			InternalEco,

			// extraneous attributes
			WhiteElo,
			BlackElo,
			WhiteRating,
			BlackRating,
			AverageElo,
			AverageRating,

			// last column
			LastColumn = WhiteElo,
		};
	}

	namespace player
	{
		enum ID
		{
			Name,
			FideID,
			Type,
			Sex,
			Rating1,
			Rating2,
			RatingType,
			Country,
			Title,
			PlayerInfo,
			Frequency,

			// additional attributes
			DateOfBirth,
			DateOfDeath,
			DsbID,
			EcfID,
			IccfID,
			ViafID,
			PndID,
			ChessgComLink,
			WikiLink,
			Aliases,

			// extraneous attributes
			EloLatest,
			EloHighest,
			RatingLatest,
			RatingHighest,

			// last column
			LastColumn = DateOfBirth,
			LastInfo = EloLatest,
		};
	}

	namespace event
	{
		enum ID
		{
			Title,
			Type,
			Date,
			Mode,
			TimeMode,
			Country,
			Site,
			Frequency,

			// last column
			LastColumn,
		};
	};

	namespace site
	{
		enum ID
		{
			Site,
			Country,
			Frequency,

			// last column
			LastColumn,
		};
	};

	namespace annotator
	{
		enum ID
		{
			Name,
			Frequency,

			// last column
			LastColumn,
		};
	}

	namespace tree
	{
		enum ID
		{
			Move,
			Eco,
			Frequency,
			Score,
			Draws,
			AverageRating,
			Performance,
			BestRating,
			AverageYear,
			LastYear,
			BestPlayer,
			MostFrequentPlayer,

			// last column
			LastColumn,
		};
	}
}

namespace order
{
	enum ID
	{
		Ascending,
		Descending,
	};
}

namespace country
{
	enum Code
	{
		Unknown												=   0, // UNK

		Aaland_Islands										=   1, // ALA
		Afghanistan											=   2, // AFG
		Albania												=   3, // ALB
		Algeria												=   4, // ALG
		American_Samoa										=   5, // ASA
		Andorra												=   6, // AND
		Angola												=   7, // ANG
		Anguilla												=   8, // AIA
		Antarctica											=   9, // ATA
		Antigua												=  10, // ANT	(often confused with Netherlands_Antilles)
		Argentina											=  11, // ARG
		Armenia												=  12, // ARM
		Aruba													=  13, // ARU
		Australasia											=  14, // ANZ	(for historical reasons)
		Australia											=  15, // AUS
		Austria												=  16, // AUT
		Azerbaijan											=  17, // AZE
		Netherlands_Antilles								=  18, // AHO

		Bahamas												=  19, // BAH
		Bahrain												=  20, // BRN
		Bangladesh											=  21, // BAN
		Barbados												=  22, // BAR
		Basque												=  23, // BAS
		Belarus												=  24, // BLR
		Belgium												=  25, // BEL
		Belize												=  26, // BIZ
		Benin													=  27, // BEN
		Bermuda												=  28, // BER
		Bhutan												=  29, // BHU
		Bolivia												=  30, // BOL
		Bosnia_and_Herzegovina							=  31, // BIH
		Botswana												=  32, // BOT
		Bouvet_Islands										=  33, // BVT
		Brazil												=  34, // BRA
		British_Indian_Ocean_Territory				=  35, // IOT
		British_Virgin_Islands							=  36, // IVB
		Brunei												=  37, // BRU
		Bulgaria												=  38, // BUL
		Burkina_Faso										=  39, // BUR
		Burundi												=  40, // BDI

		Cambodia												=  41, // CAM	(somtimes confused with Cameroon)
		Cameroon												=  42, // CMR
		Canada												=  43, // CAN
		Cape_Verde											=  44, // CPV
		Catalonia											=  45, // CAT
		Cayman_Islands										=  46, // CAY
		Central_African_Republic						=  47, // CAF
		Chad													=  48, // CHA
//		Channel_Islands									=  XX, // CIB
		Chile													=  49, // CHI
		China													=  50, // CHN
		Chinese_Taipei										=  51, // TPE
		Christmas_Island									=  52, // CXR
		Cocos_Islands										=  53, // CCK
		Colombia												=  54, // COL
		Comoros												=  55, // COM
		Congo													=  56, // CGO
		Cook_Islands										=  57, // COK
		Costa_Rica											=  58, // CRC
		Croatia												=  59, // CRO
		Cuba													=  60, // CUB
		Cyprus												=  61, // CYP
		Czech_Republic										=  62, // CSR	(somtimes confused with Czechoslovakia)
		Czechoslovakia										=  63, // TCH	(for historical reasons)

		DR_Congo												=  64, // COD
		Denmark												=  65, // DEN
		Djibouti												=  66, // DJI
		Dominica												=  67, // DMA
		Dominican_Republic								=  68, // DOM

		Ecuador												=  69, // ECU
		East_Germany										=  70, // GDR	(for historical reasons)
		Egypt													=  71, // EGY
		El_Salvador											=  72, // ESA
		England												=  73, // ENG
		Equatorial_Guinea									=  74, // GEQ
		Eritrea												=  75, // ERI
		Estonia												=  76, // EST
		Ethiopia												=  77, // ETH

		Falkland_Islands									=  78, // FLK
		Faroe_Islands										=  79, // FAI
		Fiji													=  80, // FIJ
		Finland												=  81, // FIN
		France												=  82, // FRA
		French_Guiana										=  83, // GUF
		French_Polynesia									=  84, // PYF
		French_Southern_Territories					=  85, // ATF

		Gabon													=  86, // GAB
		Gambia												=  87, // GAM
		Georgia												=  88, // GEO
		Germany												=  89, // GER
		Ghana													=  90, // GHA
		Gibraltar											=  91, // GBZ
		Great_Britain										=  92, // GBR
		Greece												=  93, // GRE
		Greenland											=  94, // GRL
		Grenada												=  95, // GRN
		Guadeloupe											=  96, // GLP
		Guam													=  97, // GUM
		Guatemala											=  98, // GUA
		Guernsey												=  99, // GGY
		Guinea												= 100, // GUI
		Guinea_Bissau										= 101, // GBS
		Guyana												= 102, // GUY

		Haiti													= 103, // HAI
		Heard_Island_and_McDonald_Islands			= 104, // HMD
		Honduras												= 105, // HON
		Hong_Kong											= 106, // HKG
		Hungary												= 107, // HUN

		Iceland												= 108, // ISL
		India													= 109, // IND
		Indonesia											= 110, // INA
		Iran													= 111, // IRI
		Iraq													= 112, // IRQ
		Ireland												= 113, // IRL
		Isle_of_Man											= 114, // IMN	(Scid is using the ChessBase code IOM)
		Israel												= 115, // ISR
		Italy													= 116, // ITA
		Ivory_Coast											= 117, // CIV

		Jamaica												= 118, // JAM
		Jan_Mayen_and_Svalbard							= 119, // SJM
		Japan													= 120, // JPN
		Jersey												= 121, // JEY	(Scid is using the ChessBase code JCI)
		Jordan												= 122, // JOR

		Kazakhstan											= 123, // KAZ
		Kenya													= 124, // KEN
		Kiribati												= 125, // KIR	(often confused with Kyrgyzstan)
		Kosovo												= 126, // KOS
		Kuwait												= 127, // KUW
		Kyrgyzstan											= 128, // KGZ

		Laos													= 129, // LAO
		Latvia												= 130, // LAT
		Lebanon												= 131, // LIB	(often confused with Libya)
		Lesotho												= 132, // LES
		Liberia												= 133, // LBR
		Libya													= 134, // LBA
		Liechtenstein										= 135, // LIE
		Lithuania											= 136, // LTU
		Luxembourg											= 137, // LUX

		Macao													= 138, // MAC
		Macedonia											= 139, // MKD
		Madagascar											= 140, // MAD
		Malawi												= 141, // MAW
		Malaysia												= 142, // MAS
		Maldives												= 143, // MDV
		Mali													= 144, // MLI
		Malta													= 145, // MLT
		Marshall_Islands									= 146, // MHL
		Martinique											= 147, // MTQ
		Mauritania											= 148, // MTN
		Mauritius											= 149, // MRI
		Mayotte												= 150, // MYT
		Mexico												= 151, // MEX
		Micronesia											= 152, // FSM
		Moldova												= 153, // MDA
		Monaco												= 154, // MNC
		Mongolia												= 155, // MGL
		Montenegro											= 156, // MNE
		Montserrat											= 157, // MSR
		Morocco												= 158, // MAR
		Mozambique											= 159, // MOZ
		Myanmar												= 160, // MYA

		Namibia												= 161, // NAM
		Nauru													= 162, // NRU
		Nepal													= 163, // NEP
		Netherlands											= 164, // NED
		New_Caledonia										= 165, // NCL
		New_Zealand											= 166, // NZL
		Nicaragua											= 167, // NCA
		Niger													= 168, // NIG	(often confused with Nigeria)
		Nigeria												= 169, // NGR
		Niue													= 170, // NIU
		Norfolk_Island										= 171, // NFK
		North_Korea											= 172, // PRK
		Northern_Ireland									= 173, // NIR
		Northern_Mariana_Islands						= 174, // MNP
		Norway												= 175, // NOR

		Oman													= 176, // OMN

		Pakistan												= 177, // PAK
		Palau													= 178, // PLW
		Palestine											= 179, // PLE
		Panama												= 180, // PAN
		Papua_New_Guinea									= 181, // PNG
		Paraguay												= 182, // PAR
		Peru													= 183, // PER
		Philippines											= 184, // PHI
		Pitcairn_Islands									= 185, // PCN
		Poland												= 186, // POL
		Portugal												= 187, // POR
		Puerto_Rico											= 188, // PUR

		Qatar													= 189, // QAT

		Reunion												= 190, // REU
		Romania												= 191, // ROU
		Russia												= 192, // RUS
		Rwanda												= 193, // RWA

		Saint_Helena										= 194, // SHN
		Saint_Kitts_and_Nevis							= 195, // SKN
		Saint_Lucia											= 196, // LCA
		Saint_Vincent_and_the_Grenadines				= 197, // VIN
		Saint_Pierre_and_Miquelon						= 198, // SPM
		Samoa													= 199, // SAM
		San_Marino											= 200, // SMR
		Sao_Tome_and_Principe							= 201, // STP
		Saudi_Arabia										= 202, // KSA
		Scotland												= 203, // SCO
		Senegal												= 204, // SEN
		Serbia												= 205, // SRB
		Serbia_and_Montenegro							= 206, // SCG	(for historical reasons)
		Seychelles											= 207, // SEY
		Sierra_Leone										= 208, // SLE
		Singapore											= 209, // SIN
		Slovakia												= 210, // SVK
		Slovenia												= 211, // SLO
		Solomon_Islands									= 212, // SOL
		Somalia												= 213, // SOM
		South_Africa										= 214, // RSA
		South_Georgia_and_South_Sandwich_Islands	= 215, // SGS
		South_Korea											= 216, // KOR
		Soviet_Union										= 217, // URS	(for historical reasons)
		Spain													= 218, // ESP
		Sri_Lanka											= 219, // SRI
		Sudan													= 220, // SUD
		Suriname												= 221, // SUR
		Swaziland											= 222, // SWZ	(often confused with Switzerland)
		Sweden												= 223, // SWE
		Switzerland											= 224, // SUI
		Syria													= 225, // SYR

		Tajikistan											= 226, // TJK
		Tanzania												= 227, // TAN
		Thailand												= 228, // THA
		Tibet													= 229, // TIB
		Timor_Leste											= 230, // TLS
		Togo													= 231, // TOG
		Tokelau												= 232, // TKL
		Tonga													= 233, // TGA
		Trinidad_and_Tobago								= 234, // TRI
		Tunisia												= 235, // TUN
		Turkey												= 236, // TUR
		Turkmenistan										= 237, // TKM
		Turks_and_Caicos_Islands						= 238, // TCA
		Tuvalu												= 239, // TUV

		US_Virgin_Islands									= 240, // VUS
		Uganda												= 241, // UGA
		Ukraine												= 242, // UKR
		United_Arab_Emirates								= 243, // UAE
		United_States_Minor_Outlying_Islands		= 244, // UMI
		United_States_of_America						= 245, // USA
		Uruguay												= 246, // URU
		Uzbekistan											= 247, // UZB

		Vanuatu												= 248, // VAN
		Vatican												= 249, // VAT
		Venezuela											= 250, // VEN
		Vietnam												= 251, // VIE

		Wales													= 252, // WLS
		Wallis_and_Futuna									= 253, // WLF
		West_Germany										= 254, // FRG	(for historical reasons)
		Western_Sahara										= 255, // ESH

		Yemen													= 256, // YEM
		Yugoslavia											= 257, // YUG	(for historical reasons)

		Zambia												= 258, // ZAM
		Zanzibar												= 259, // EAZ
		Zimbabwe												= 260, // ZIM

		Aboard_Aircraft									= 261, // AIR
		Aboard_Spacecraft									= 262, // SPC
		At_Sea												= 263, // SEA
		// NOTE: Scid is using INT, but the PGN standard says NET
		The_Internet										= 264, // NET	(sometimes confused with Netherlands)

		Mixed_Team											= 265, // ZZX

		LAST													= Mixed_Team,
	};

	bool isGermanSpeakingCountry(Code code);

	char const* toString(Code code);
	char const* toChessBaseCode(Code code);
	Code fromString(mstl::string const& country);
	Code fromString(char const* country);
	unsigned toRegion(Code code);
	unsigned count();

	bool match(Code lhs, Code rhs);
	int compare(Code lhs, Code rhs);
}

} // namespace db

#include "db_common.ipp"

#endif // _db_common_included

// vi:set ts=3 sw=3:
