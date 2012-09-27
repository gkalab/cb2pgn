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
// Copyright: (C) 2008-2012 Gregor Cramer
// ======================================================================

// ======================================================================
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// ======================================================================

#include "db_common.h"

#include "m_string.h"
#include "m_algorithm.h"
#include "m_bitfield.h"
#include "m_bit_functions.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

using namespace db;


static bool
isCaseEqual(mstl::string const& lhs, mstl::string const& rhs)
{
	return lhs.size() == rhs.size() && ::strncasecmp(lhs, rhs, lhs.size()) == 0;
}


namespace db {
namespace tag {

struct Pair { mstl::string name; ID id; };

#if __GNUC_PREREQ(4,7)
__attribute__((init_priority(65534)))
#endif
static Pair const NameMap[] =
{
	{ "Annotator",				Annotator },
	{ "Black",					Black, },
	{ "BlackBCF",				BlackECF },
	{ "BlackClock",			BlackClock },
	{ "BlackCountry",			BlackCountry },
	{ "BlackDWZ",				BlackDWZ },
	{ "BlackECF",				BlackECF },
	{ "BlackElo",				BlackElo} ,
	{ "BlackFideId",			BlackFideId },
	{ "BlackICCF",				BlackICCF },
	{ "BlackIPS",				BlackIPS },
	{ "BlackNA",				BlackNA },
	{ "BlackRapid",			BlackRapid },
	{ "BlackRating",			BlackRating },
	{ "BlackSex",				BlackSex },
	{ "BlackTeam",				BlackTeam },
	{ "BlackTeamCountry",	BlackTeamCountry },
	{ "BlackTitle",			BlackTitle },
	{ "BlackType",				BlackType },
	{ "BlackUSCF",				BlackUSCF },
	{ "Board",					Board },
	{ "Date",					Date },
	{ "ECO",						Eco },
	{ "Event",					Event },
	{ "EventCategory",		EventCategory },
	{ "EventCountry",			EventCountry },
	{ "EventDate",				EventDate },
	{ "EventRounds",			EventRounds },
	{ "EventType",				EventType },
	{ "FEN",						Fen },
	{ "Mode",					Mode },
	{ "Opening",				Opening },
	{ "PlyCount",				PlyCount },
	{ "Position",				Idn },
	{ "Remark",					Remark },
	{ "Result",					Result },
	{ "Round",					Round },
	{ "SetUp",					SetUp, },
	{ "Site",					Site },
	{ "Source",					Source },
	{ "SourceDate",			SourceDate },
	{ "SubVariation",			SubVariation },
	{ "Termination",			Termination },
	{ "TimeControl",			TimeControl },
	{ "TimeMode",				TimeMode },
	{ "Variant",				Variant },
	{ "Variation",				Variation },
	{ "White",					White },
	{ "WhiteBCF",				WhiteECF },
	{ "WhiteClock",			WhiteClock },
	{ "WhiteCountry",			WhiteCountry },
	{ "WhiteDWZ",				WhiteDWZ },
	{ "WhiteECF",				WhiteECF },
	{ "WhiteElo",				WhiteElo },
	{ "WhiteFideId",			WhiteFideId },
	{ "WhiteICCF",				WhiteICCF },
	{ "WhiteIPS",				WhiteIPS },
	{ "WhiteNA",				WhiteNA },
	{ "WhiteRapid",			WhiteRapid },
	{ "WhiteRating",			WhiteRating },
	{ "WhiteSex",				WhiteSex },
	{ "WhiteTeam",				WhiteTeam },
	{ "WhiteTeamCountry",	WhiteTeamCountry },
	{ "WhiteTitle",			WhiteTitle },
	{ "WhiteType",				WhiteType },
	{ "WhiteUSCF",				WhiteUSCF },
};

static int
compareTags(void const* lhs, const void* rhs)
{
	mstl::string const* s = static_cast<mstl::string const*>(lhs);
	mstl::string const& t = static_cast<Pair const*>(rhs)->name;

	int rc = ::strncasecmp(s->c_str(), t, s->size());
	return rc ? rc : (t.size() == s->size() ? 0 : -1);
}

} // namespace tag

namespace termination {

static mstl::string const Lookup[] =
{
	"",
	"Normal",
	"Unplayed",
	"Abandoned",
	"Adjudication",
	"Death",
	"Emergency",
	"RulesInfraction",
	"TimeForfeit",
	"Unterminated",
};

} // namespace termination

namespace title {

static mstl::string const Lookup[] =
{
	"",
	"GM", "IM", "FM", "CM",
	"WGM", "WIM", "WFM", "WCM",
	"HGM",
	"NM", "SM", "LM",
	"CGM", "CIM", "LGM", "ILM", "CSIM",
};

} // namespace title

namespace species {

static mstl::string const StrHuman("human");
static mstl::string const StrProgram("program");

} // namespace species

namespace piece {
namespace utf8 {

static mstl::string const Table[7] =
{
	"",
	"\xe2\x99\x94",
	"\xe2\x99\x95",
	"\xe2\x99\x96",
	"\xe2\x99\x97",
	"\xe2\x99\x98",
	"\xe2\x99\x99",
};

} // namespace utf8
} // namespace piece

namespace rating {

static mstl::string Map[Last + 1] =
{
	"Elo",
	"Rating",
	"Rapid",
	"ICCF",
	"USCF",
	"DWZ",
	"ECF",
	"IPS",
	"",
};

} // namespace rating

namespace event {

static mstl::string const ModeLookup[] =
{
	"", "OTB", "PM", "EM", "ICS", "TC", "Analysis", "Composition",
};

static mstl::string const TypeLookup[] =
{
	"", "game", "match", "tourn", "swiss", "team", "k.o.", "simul", "schev",
};

} // namespace event

namespace time {

static mstl::string const Lookup[] = { "", "normal", "rapid", "blitz", "bullet", "corr" };

}

namespace chess960 {

static mstl::string const PositionTable[961] =
{
	"",         "BQNBNRKR", "BQNNRBKR", "BQNNRKRB", "QBBNNRKR", "QNBBNRKR", "QNBNRBKR", "QNBNRKRB",
	"QBNNBRKR", "QNNBBRKR", "QNNRBBKR", "QNNRBKRB", "QBNNRKBR", "QNNBRKBR", "QNNRKBBR", "QNNRKRBB",
	"BBNQNRKR", "BNQBNRKR", "BNQNRBKR", "BNQNRKRB", "NBBQNRKR", "NQBBNRKR", "NQBNRBKR", "NQBNRKRB",
	"NBQNBRKR", "NQNBBRKR", "NQNRBBKR", "NQNRBKRB", "NBQNRKBR", "NQNBRKBR", "NQNRKBBR", "NQNRKRBB",
	"BBNNQRKR", "BNNBQRKR", "BNNQRBKR", "BNNQRKRB", "NBBNQRKR", "NNBBQRKR", "NNBQRBKR", "NNBQRKRB",
	"NBNQBRKR", "NNQBBRKR", "NNQRBBKR", "NNQRBKRB", "NBNQRKBR", "NNQBRKBR", "NNQRKBBR", "NNQRKRBB",
	"BBNNRQKR", "BNNBRQKR", "BNNRQBKR", "BNNRQKRB", "NBBNRQKR", "NNBBRQKR", "NNBRQBKR", "NNBRQKRB",
	"NBNRBQKR", "NNRBBQKR", "NNRQBBKR", "NNRQBKRB", "NBNRQKBR", "NNRBQKBR", "NNRQKBBR", "NNRQKRBB",
	"BBNNRKQR", "BNNBRKQR", "BNNRKBQR", "BNNRKQRB", "NBBNRKQR", "NNBBRKQR", "NNBRKBQR", "NNBRKQRB",
	"NBNRBKQR", "NNRBBKQR", "NNRKBBQR", "NNRKBQRB", "NBNRKQBR", "NNRBKQBR", "NNRKQBBR", "NNRKQRBB",
	"BBNNRKRQ", "BNNBRKRQ", "BNNRKBRQ", "BNNRKRQB", "NBBNRKRQ", "NNBBRKRQ", "NNBRKBRQ", "NNBRKRQB",
	"NBNRBKRQ", "NNRBBKRQ", "NNRKBBRQ", "NNRKBRQB", "NBNRKRBQ", "NNRBKRBQ", "NNRKRBBQ", "NNRKRQBB",
	"BBQNRNKR", "BQNBRNKR", "BQNRNBKR", "BQNRNKRB", "QBBNRNKR", "QNBBRNKR", "QNBRNBKR", "QNBRNKRB",
	"QBNRBNKR", "QNRBBNKR", "QNRNBBKR", "QNRNBKRB", "QBNRNKBR", "QNRBNKBR", "QNRNKBBR", "QNRNKRBB",
	"BBNQRNKR", "BNQBRNKR", "BNQRNBKR", "BNQRNKRB", "NBBQRNKR", "NQBBRNKR", "NQBRNBKR", "NQBRNKRB",
	"NBQRBNKR", "NQRBBNKR", "NQRNBBKR", "NQRNBKRB", "NBQRNKBR", "NQRBNKBR", "NQRNKBBR", "NQRNKRBB",
	"BBNRQNKR", "BNRBQNKR", "BNRQNBKR", "BNRQNKRB", "NBBRQNKR", "NRBBQNKR", "NRBQNBKR", "NRBQNKRB",
	"NBRQBNKR", "NRQBBNKR", "NRQNBBKR", "NRQNBKRB", "NBRQNKBR", "NRQBNKBR", "NRQNKBBR", "NRQNKRBB",
	"BBNRNQKR", "BNRBNQKR", "BNRNQBKR", "BNRNQKRB", "NBBRNQKR", "NRBBNQKR", "NRBNQBKR", "NRBNQKRB",
	"NBRNBQKR", "NRNBBQKR", "NRNQBBKR", "NRNQBKRB", "NBRNQKBR", "NRNBQKBR", "NRNQKBBR", "NRNQKRBB",
	"BBNRNKQR", "BNRBNKQR", "BNRNKBQR", "BNRNKQRB", "NBBRNKQR", "NRBBNKQR", "NRBNKBQR", "NRBNKQRB",
	"NBRNBKQR", "NRNBBKQR", "NRNKBBQR", "NRNKBQRB", "NBRNKQBR", "NRNBKQBR", "NRNKQBBR", "NRNKQRBB",
	"BBNRNKRQ", "BNRBNKRQ", "BNRNKBRQ", "BNRNKRQB", "NBBRNKRQ", "NRBBNKRQ", "NRBNKBRQ", "NRBNKRQB",
	"NBRNBKRQ", "NRNBBKRQ", "NRNKBBRQ", "NRNKBRQB", "NBRNKRBQ", "NRNBKRBQ", "NRNKRBBQ", "NRNKRQBB",
	"BBQNRKNR", "BQNBRKNR", "BQNRKBNR", "BQNRKNRB", "QBBNRKNR", "QNBBRKNR", "QNBRKBNR", "QNBRKNRB",
	"QBNRBKNR", "QNRBBKNR", "QNRKBBNR", "QNRKBNRB", "QBNRKNBR", "QNRBKNBR", "QNRKNBBR", "QNRKNRBB",
	"BBNQRKNR", "BNQBRKNR", "BNQRKBNR", "BNQRKNRB", "NBBQRKNR", "NQBBRKNR", "NQBRKBNR", "NQBRKNRB",
	"NBQRBKNR", "NQRBBKNR", "NQRKBBNR", "NQRKBNRB", "NBQRKNBR", "NQRBKNBR", "NQRKNBBR", "NQRKNRBB",
	"BBNRQKNR", "BNRBQKNR", "BNRQKBNR", "BNRQKNRB", "NBBRQKNR", "NRBBQKNR", "NRBQKBNR", "NRBQKNRB",
	"NBRQBKNR", "NRQBBKNR", "NRQKBBNR", "NRQKBNRB", "NBRQKNBR", "NRQBKNBR", "NRQKNBBR", "NRQKNRBB",
	"BBNRKQNR", "BNRBKQNR", "BNRKQBNR", "BNRKQNRB", "NBBRKQNR", "NRBBKQNR", "NRBKQBNR", "NRBKQNRB",
	"NBRKBQNR", "NRKBBQNR", "NRKQBBNR", "NRKQBNRB", "NBRKQNBR", "NRKBQNBR", "NRKQNBBR", "NRKQNRBB",
	"BBNRKNQR", "BNRBKNQR", "BNRKNBQR", "BNRKNQRB", "NBBRKNQR", "NRBBKNQR", "NRBKNBQR", "NRBKNQRB",
	"NBRKBNQR", "NRKBBNQR", "NRKNBBQR", "NRKNBQRB", "NBRKNQBR", "NRKBNQBR", "NRKNQBBR", "NRKNQRBB",
	"BBNRKNRQ", "BNRBKNRQ", "BNRKNBRQ", "BNRKNRQB", "NBBRKNRQ", "NRBBKNRQ", "NRBKNBRQ", "NRBKNRQB",
	"NBRKBNRQ", "NRKBBNRQ", "NRKNBBRQ", "NRKNBRQB", "NBRKNRBQ", "NRKBNRBQ", "NRKNRBBQ", "NRKNRQBB",
	"BBQNRKRN", "BQNBRKRN", "BQNRKBRN", "BQNRKRNB", "QBBNRKRN", "QNBBRKRN", "QNBRKBRN", "QNBRKRNB",
	"QBNRBKRN", "QNRBBKRN", "QNRKBBRN", "QNRKBRNB", "QBNRKRBN", "QNRBKRBN", "QNRKRBBN", "QNRKRNBB",
	"BBNQRKRN", "BNQBRKRN", "BNQRKBRN", "BNQRKRNB", "NBBQRKRN", "NQBBRKRN", "NQBRKBRN", "NQBRKRNB",
	"NBQRBKRN", "NQRBBKRN", "NQRKBBRN", "NQRKBRNB", "NBQRKRBN", "NQRBKRBN", "NQRKRBBN", "NQRKRNBB",
	"BBNRQKRN", "BNRBQKRN", "BNRQKBRN", "BNRQKRNB", "NBBRQKRN", "NRBBQKRN", "NRBQKBRN", "NRBQKRNB",
	"NBRQBKRN", "NRQBBKRN", "NRQKBBRN", "NRQKBRNB", "NBRQKRBN", "NRQBKRBN", "NRQKRBBN", "NRQKRNBB",
	"BBNRKQRN", "BNRBKQRN", "BNRKQBRN", "BNRKQRNB", "NBBRKQRN", "NRBBKQRN", "NRBKQBRN", "NRBKQRNB",
	"NBRKBQRN", "NRKBBQRN", "NRKQBBRN", "NRKQBRNB", "NBRKQRBN", "NRKBQRBN", "NRKQRBBN", "NRKQRNBB",
	"BBNRKRQN", "BNRBKRQN", "BNRKRBQN", "BNRKRQNB", "NBBRKRQN", "NRBBKRQN", "NRBKRBQN", "NRBKRQNB",
	"NBRKBRQN", "NRKBBRQN", "NRKRBBQN", "NRKRBQNB", "NBRKRQBN", "NRKBRQBN", "NRKRQBBN", "NRKRQNBB",
	"BBNRKRNQ", "BNRBKRNQ", "BNRKRBNQ", "BNRKRNQB", "NBBRKRNQ", "NRBBKRNQ", "NRBKRBNQ", "NRBKRNQB",
	"NBRKBRNQ", "NRKBBRNQ", "NRKRBBNQ", "NRKRBNQB", "NBRKRNBQ", "NRKBRNBQ", "NRKRNBBQ", "NRKRNQBB",
	"BBQRNNKR", "BQRBNNKR", "BQRNNBKR", "BQRNNKRB", "QBBRNNKR", "QRBBNNKR", "QRBNNBKR", "QRBNNKRB",
	"QBRNBNKR", "QRNBBNKR", "QRNNBBKR", "QRNNBKRB", "QBRNNKBR", "QRNBNKBR", "QRNNKBBR", "QRNNKRBB",
	"BBRQNNKR", "BRQBNNKR", "BRQNNBKR", "BRQNNKRB", "RBBQNNKR", "RQBBNNKR", "RQBNNBKR", "RQBNNKRB",
	"RBQNBNKR", "RQNBBNKR", "RQNNBBKR", "RQNNBKRB", "RBQNNKBR", "RQNBNKBR", "RQNNKBBR", "RQNNKRBB",
	"BBRNQNKR", "BRNBQNKR", "BRNQNBKR", "BRNQNKRB", "RBBNQNKR", "RNBBQNKR", "RNBQNBKR", "RNBQNKRB",
	"RBNQBNKR", "RNQBBNKR", "RNQNBBKR", "RNQNBKRB", "RBNQNKBR", "RNQBNKBR", "RNQNKBBR", "RNQNKRBB",
	"BBRNNQKR", "BRNBNQKR", "BRNNQBKR", "BRNNQKRB", "RBBNNQKR", "RNBBNQKR", "RNBNQBKR", "RNBNQKRB",
	"RBNNBQKR", "RNNBBQKR", "RNNQBBKR", "RNNQBKRB", "RBNNQKBR", "RNNBQKBR", "RNNQKBBR", "RNNQKRBB",
	"BBRNNKQR", "BRNBNKQR", "BRNNKBQR", "BRNNKQRB", "RBBNNKQR", "RNBBNKQR", "RNBNKBQR", "RNBNKQRB",
	"RBNNBKQR", "RNNBBKQR", "RNNKBBQR", "RNNKBQRB", "RBNNKQBR", "RNNBKQBR", "RNNKQBBR", "RNNKQRBB",
	"BBRNNKRQ", "BRNBNKRQ", "BRNNKBRQ", "BRNNKRQB", "RBBNNKRQ", "RNBBNKRQ", "RNBNKBRQ", "RNBNKRQB",
	"RBNNBKRQ", "RNNBBKRQ", "RNNKBBRQ", "RNNKBRQB", "RBNNKRBQ", "RNNBKRBQ", "RNNKRBBQ", "RNNKRQBB",
	"BBQRNKNR", "BQRBNKNR", "BQRNKBNR", "BQRNKNRB", "QBBRNKNR", "QRBBNKNR", "QRBNKBNR", "QRBNKNRB",
	"QBRNBKNR", "QRNBBKNR", "QRNKBBNR", "QRNKBNRB", "QBRNKNBR", "QRNBKNBR", "QRNKNBBR", "QRNKNRBB",
	"BBRQNKNR", "BRQBNKNR", "BRQNKBNR", "BRQNKNRB", "RBBQNKNR", "RQBBNKNR", "RQBNKBNR", "RQBNKNRB",
	"RBQNBKNR", "RQNBBKNR", "RQNKBBNR", "RQNKBNRB", "RBQNKNBR", "RQNBKNBR", "RQNKNBBR", "RQNKNRBB",
	"BBRNQKNR", "BRNBQKNR", "BRNQKBNR", "BRNQKNRB", "RBBNQKNR", "RNBBQKNR", "RNBQKBNR", "RNBQKNRB",
	"RBNQBKNR", "RNQBBKNR", "RNQKBBNR", "RNQKBNRB", "RBNQKNBR", "RNQBKNBR", "RNQKNBBR", "RNQKNRBB",
	"BBRNKQNR", "BRNBKQNR", "BRNKQBNR", "BRNKQNRB", "RBBNKQNR", "RNBBKQNR", "RNBKQBNR", "RNBKQNRB",
	"RBNKBQNR", "RNKBBQNR", "RNKQBBNR", "RNKQBNRB", "RBNKQNBR", "RNKBQNBR", "RNKQNBBR", "RNKQNRBB",
	"BBRNKNQR", "BRNBKNQR", "BRNKNBQR", "BRNKNQRB", "RBBNKNQR", "RNBBKNQR", "RNBKNBQR", "RNBKNQRB",
	"RBNKBNQR", "RNKBBNQR", "RNKNBBQR", "RNKNBQRB", "RBNKNQBR", "RNKBNQBR", "RNKNQBBR", "RNKNQRBB",
	"BBRNKNRQ", "BRNBKNRQ", "BRNKNBRQ", "BRNKNRQB", "RBBNKNRQ", "RNBBKNRQ", "RNBKNBRQ", "RNBKNRQB",
	"RBNKBNRQ", "RNKBBNRQ", "RNKNBBRQ", "RNKNBRQB", "RBNKNRBQ", "RNKBNRBQ", "RNKNRBBQ", "RNKNRQBB",
	"BBQRNKRN", "BQRBNKRN", "BQRNKBRN", "BQRNKRNB", "QBBRNKRN", "QRBBNKRN", "QRBNKBRN", "QRBNKRNB",
	"QBRNBKRN", "QRNBBKRN", "QRNKBBRN", "QRNKBRNB", "QBRNKRBN", "QRNBKRBN", "QRNKRBBN", "QRNKRNBB",
	"BBRQNKRN", "BRQBNKRN", "BRQNKBRN", "BRQNKRNB", "RBBQNKRN", "RQBBNKRN", "RQBNKBRN", "RQBNKRNB",
	"RBQNBKRN", "RQNBBKRN", "RQNKBBRN", "RQNKBRNB", "RBQNKRBN", "RQNBKRBN", "RQNKRBBN", "RQNKRNBB",
	"BBRNQKRN", "BRNBQKRN", "BRNQKBRN", "BRNQKRNB", "RBBNQKRN", "RNBBQKRN", "RNBQKBRN", "RNBQKRNB",
	"RBNQBKRN", "RNQBBKRN", "RNQKBBRN", "RNQKBRNB", "RBNQKRBN", "RNQBKRBN", "RNQKRBBN", "RNQKRNBB",
	"BBRNKQRN", "BRNBKQRN", "BRNKQBRN", "BRNKQRNB", "RBBNKQRN", "RNBBKQRN", "RNBKQBRN", "RNBKQRNB",
	"RBNKBQRN", "RNKBBQRN", "RNKQBBRN", "RNKQBRNB", "RBNKQRBN", "RNKBQRBN", "RNKQRBBN", "RNKQRNBB",
	"BBRNKRQN", "BRNBKRQN", "BRNKRBQN", "BRNKRQNB", "RBBNKRQN", "RNBBKRQN", "RNBKRBQN", "RNBKRQNB",
	"RBNKBRQN", "RNKBBRQN", "RNKRBBQN", "RNKRBQNB", "RBNKRQBN", "RNKBRQBN", "RNKRQBBN", "RNKRQNBB",
	"BBRNKRNQ", "BRNBKRNQ", "BRNKRBNQ", "BRNKRNQB", "RBBNKRNQ", "RNBBKRNQ", "RNBKRBNQ", "RNBKRNQB",
	"RBNKBRNQ", "RNKBBRNQ", "RNKRBBNQ", "RNKRBNQB", "RBNKRNBQ", "RNKBRNBQ", "RNKRNBBQ", "RNKRNQBB",
	"BBQRKNNR", "BQRBKNNR", "BQRKNBNR", "BQRKNNRB", "QBBRKNNR", "QRBBKNNR", "QRBKNBNR", "QRBKNNRB",
	"QBRKBNNR", "QRKBBNNR", "QRKNBBNR", "QRKNBNRB", "QBRKNNBR", "QRKBNNBR", "QRKNNBBR", "QRKNNRBB",
	"BBRQKNNR", "BRQBKNNR", "BRQKNBNR", "BRQKNNRB", "RBBQKNNR", "RQBBKNNR", "RQBKNBNR", "RQBKNNRB",
	"RBQKBNNR", "RQKBBNNR", "RQKNBBNR", "RQKNBNRB", "RBQKNNBR", "RQKBNNBR", "RQKNNBBR", "RQKNNRBB",
	"BBRKQNNR", "BRKBQNNR", "BRKQNBNR", "BRKQNNRB", "RBBKQNNR", "RKBBQNNR", "RKBQNBNR", "RKBQNNRB",
	"RBKQBNNR", "RKQBBNNR", "RKQNBBNR", "RKQNBNRB", "RBKQNNBR", "RKQBNNBR", "RKQNNBBR", "RKQNNRBB",
	"BBRKNQNR", "BRKBNQNR", "BRKNQBNR", "BRKNQNRB", "RBBKNQNR", "RKBBNQNR", "RKBNQBNR", "RKBNQNRB",
	"RBKNBQNR", "RKNBBQNR", "RKNQBBNR", "RKNQBNRB", "RBKNQNBR", "RKNBQNBR", "RKNQNBBR", "RKNQNRBB",
	"BBRKNNQR", "BRKBNNQR", "BRKNNBQR", "BRKNNQRB", "RBBKNNQR", "RKBBNNQR", "RKBNNBQR", "RKBNNQRB",
	"RBKNBNQR", "RKNBBNQR", "RKNNBBQR", "RKNNBQRB", "RBKNNQBR", "RKNBNQBR", "RKNNQBBR", "RKNNQRBB",
	"BBRKNNRQ", "BRKBNNRQ", "BRKNNBRQ", "BRKNNRQB", "RBBKNNRQ", "RKBBNNRQ", "RKBNNBRQ", "RKBNNRQB",
	"RBKNBNRQ", "RKNBBNRQ", "RKNNBBRQ", "RKNNBRQB", "RBKNNRBQ", "RKNBNRBQ", "RKNNRBBQ", "RKNNRQBB",
	"BBQRKNRN", "BQRBKNRN", "BQRKNBRN", "BQRKNRNB", "QBBRKNRN", "QRBBKNRN", "QRBKNBRN", "QRBKNRNB",
	"QBRKBNRN", "QRKBBNRN", "QRKNBBRN", "QRKNBRNB", "QBRKNRBN", "QRKBNRBN", "QRKNRBBN", "QRKNRNBB",
	"BBRQKNRN", "BRQBKNRN", "BRQKNBRN", "BRQKNRNB", "RBBQKNRN", "RQBBKNRN", "RQBKNBRN", "RQBKNRNB",
	"RBQKBNRN", "RQKBBNRN", "RQKNBBRN", "RQKNBRNB", "RBQKNRBN", "RQKBNRBN", "RQKNRBBN", "RQKNRNBB",
	"BBRKQNRN", "BRKBQNRN", "BRKQNBRN", "BRKQNRNB", "RBBKQNRN", "RKBBQNRN", "RKBQNBRN", "RKBQNRNB",
	"RBKQBNRN", "RKQBBNRN", "RKQNBBRN", "RKQNBRNB", "RBKQNRBN", "RKQBNRBN", "RKQNRBBN", "RKQNRNBB",
	"BBRKNQRN", "BRKBNQRN", "BRKNQBRN", "BRKNQRNB", "RBBKNQRN", "RKBBNQRN", "RKBNQBRN", "RKBNQRNB",
	"RBKNBQRN", "RKNBBQRN", "RKNQBBRN", "RKNQBRNB", "RBKNQRBN", "RKNBQRBN", "RKNQRBBN", "RKNQRNBB",
	"BBRKNRQN", "BRKBNRQN", "BRKNRBQN", "BRKNRQNB", "RBBKNRQN", "RKBBNRQN", "RKBNRBQN", "RKBNRQNB",
	"RBKNBRQN", "RKNBBRQN", "RKNRBBQN", "RKNRBQNB", "RBKNRQBN", "RKNBRQBN", "RKNRQBBN", "RKNRQNBB",
	"BBRKNRNQ", "BRKBNRNQ", "BRKNRBNQ", "BRKNRNQB", "RBBKNRNQ", "RKBBNRNQ", "RKBNRBNQ", "RKBNRNQB",
	"RBKNBRNQ", "RKNBBRNQ", "RKNRBBNQ", "RKNRBNQB", "RBKNRNBQ", "RKNBRNBQ", "RKNRNBBQ", "RKNRNQBB",
	"BBQRKRNN", "BQRBKRNN", "BQRKRBNN", "BQRKRNNB", "QBBRKRNN", "QRBBKRNN", "QRBKRBNN", "QRBKRNNB",
	"QBRKBRNN", "QRKBBRNN", "QRKRBBNN", "QRKRBNNB", "QBRKRNBN", "QRKBRNBN", "QRKRNBBN", "QRKRNNBB",
	"BBRQKRNN", "BRQBKRNN", "BRQKRBNN", "BRQKRNNB", "RBBQKRNN", "RQBBKRNN", "RQBKRBNN", "RQBKRNNB",
	"RBQKBRNN", "RQKBBRNN", "RQKRBBNN", "RQKRBNNB", "RBQKRNBN", "RQKBRNBN", "RQKRNBBN", "RQKRNNBB",
	"BBRKQRNN", "BRKBQRNN", "BRKQRBNN", "BRKQRNNB", "RBBKQRNN", "RKBBQRNN", "RKBQRBNN", "RKBQRNNB",
	"RBKQBRNN", "RKQBBRNN", "RKQRBBNN", "RKQRBNNB", "RBKQRNBN", "RKQBRNBN", "RKQRNBBN", "RKQRNNBB",
	"BBRKRQNN", "BRKBRQNN", "BRKRQBNN", "BRKRQNNB", "RBBKRQNN", "RKBBRQNN", "RKBRQBNN", "RKBRQNNB",
	"RBKRBQNN", "RKRBBQNN", "RKRQBBNN", "RKRQBNNB", "RBKRQNBN", "RKRBQNBN", "RKRQNBBN", "RKRQNNBB",
	"BBRKRNQN", "BRKBRNQN", "BRKRNBQN", "BRKRNQNB", "RBBKRNQN", "RKBBRNQN", "RKBRNBQN", "RKBRNQNB",
	"RBKRBNQN", "RKRBBNQN", "RKRNBBQN", "RKRNBQNB", "RBKRNQBN", "RKRBNQBN", "RKRNQBBN", "RKRNQNBB",
	"BBRKRNNQ", "BRKBRNNQ", "BRKRNBNQ", "BRKRNNQB", "RBBKRNNQ", "RKBBRNNQ", "RKBRNBNQ", "RKBRNNQB",
	"RBKRBNNQ", "RKRBBNNQ", "RKRNBBNQ", "RKRNBNQB", "RBKRNNBQ", "RKRBNNBQ", "RKRNNBBQ", "RKRNNQBB",
	"BBQNNRKR",
};

static uint16_t Twins[961] =
{
	  0, 960, 956, 952, 948, 959, 955, 951, 947, 958, 954, 950, 946, 957, 953, 949, 945, 944, 940, 936,
	932, 943, 939, 935, 931, 942, 938, 934, 930, 941, 937, 933, 929, 928, 924, 920, 916, 927, 923, 919,
	915, 926, 922, 918, 914, 925, 921, 917, 913, 912, 908, 904, 900, 911, 907, 903, 899, 910, 906, 902,
	898, 909, 905, 901, 897, 896, 892, 888, 884, 895, 891, 887, 883, 894, 890, 886, 882, 893, 889, 885,
	881, 880, 876, 872, 868, 879, 875, 871, 867, 878, 874, 870, 866, 877, 873, 869, 865, 864, 860, 856,
	852, 863, 859, 855, 851, 862, 858, 854, 850, 861, 857, 853, 849, 848, 844, 840, 836, 847, 843, 839,
	835, 846, 842, 838, 834, 845, 841, 837, 833, 832, 828, 824, 820, 831, 827, 823, 819, 830, 826, 822,
	818, 829, 825, 821, 817, 816, 812, 808, 804, 815, 811, 807, 803, 814, 810, 806, 802, 813, 809, 805,
	801, 800, 796, 792, 788, 799, 795, 791, 787, 798, 794, 790, 786, 797, 793, 789, 785, 784, 780, 776,
	772, 783, 779, 775, 771, 782, 778, 774, 770, 781, 777, 773, 769, 672, 668, 664, 660, 671, 667, 663,
	659, 670, 666, 662, 658, 669, 665, 661, 657, 656, 652, 648, 644, 655, 651, 647, 643, 654, 650, 646,
	642, 653, 649, 645, 641, 640, 636, 632, 628, 639, 635, 631, 627, 638, 634, 630, 626, 637, 633, 629,
	625, 624, 620, 616, 612, 623, 619, 615, 611, 622, 618, 614, 610, 621, 617, 613, 609, 608, 604, 600,
	596, 607, 603, 599, 595, 606, 602, 598, 594, 605, 601, 597, 593, 592, 588, 584, 580, 591, 587, 583,
	579, 590, 586, 582, 578, 589, 585, 581, 577, 384, 380, 376, 372, 383, 379, 375, 371, 382, 378, 374,
	370, 381, 377, 373, 369, 368, 364, 360, 356, 367, 363, 359, 355, 366, 362, 358, 354, 365, 361, 357,
	353, 352, 348, 344, 340, 351, 347, 343, 339, 350, 346, 342, 338, 349, 345, 341, 337, 336, 332, 328,
	324, 335, 331, 327, 323, 334, 330, 326, 322, 333, 329, 325, 321, 320, 316, 312, 308, 319, 315, 311,
	307, 318, 314, 310, 306, 317, 313, 309, 305, 304, 300, 296, 292, 303, 299, 295, 291, 302, 298, 294,
	290, 301, 297, 293, 289, 768, 764, 760, 756, 767, 763, 759, 755, 766, 762, 758, 754, 765, 761, 757,
	753, 752, 748, 744, 740, 751, 747, 743, 739, 750, 746, 742, 738, 749, 745, 741, 737, 736, 732, 728,
	724, 735, 731, 727, 723, 734, 730, 726, 722, 733, 729, 725, 721, 720, 716, 712, 708, 719, 715, 711,
	707, 718, 714, 710, 706, 717, 713, 709, 705, 704, 700, 696, 692, 703, 699, 695, 691, 702, 698, 694,
	690, 701, 697, 693, 689, 688, 684, 680, 676, 687, 683, 679, 675, 686, 682, 678, 674, 685, 681, 677,
	673, 576, 572, 568, 564, 575, 571, 567, 563, 574, 570, 566, 562, 573, 569, 565, 561, 560, 556, 552,
	548, 559, 555, 551, 547, 558, 554, 550, 546, 557, 553, 549, 545, 544, 540, 536, 532, 543, 539, 535,
	531, 542, 538, 534, 530, 541, 537, 533, 529, 528, 524, 520, 516, 527, 523, 519, 515, 526, 522, 518,
	514, 525, 521, 517, 513, 512, 508, 504, 500, 511, 507, 503, 499, 510, 506, 502, 498, 509, 505, 501,
	497, 496, 492, 488, 484, 495, 491, 487, 483, 494, 490, 486, 482, 493, 489, 485, 481, 288, 284, 280,
	276, 287, 283, 279, 275, 286, 282, 278, 274, 285, 281, 277, 273, 272, 268, 264, 260, 271, 267, 263,
	259, 270, 266, 262, 258, 269, 265, 261, 257, 256, 252, 248, 244, 255, 251, 247, 243, 254, 250, 246,
	242, 253, 249, 245, 241, 240, 236, 232, 228, 239, 235, 231, 227, 238, 234, 230, 226, 237, 233, 229,
	225, 224, 220, 216, 212, 223, 219, 215, 211, 222, 218, 214, 210, 221, 217, 213, 209, 208, 204, 200,
	196, 207, 203, 199, 195, 206, 202, 198, 194, 205, 201, 197, 193, 480, 476, 472, 468, 479, 475, 471,
	467, 478, 474, 470, 466, 477, 473, 469, 465, 464, 460, 456, 452, 463, 459, 455, 451, 462, 458, 454,
	450, 461, 457, 453, 449, 448, 444, 440, 436, 447, 443, 439, 435, 446, 442, 438, 434, 445, 441, 437,
	433, 432, 428, 424, 420, 431, 427, 423, 419, 430, 426, 422, 418, 429, 425, 421, 417, 416, 412, 408,
	404, 415, 411, 407, 403, 414, 410, 406, 402, 413, 409, 405, 401, 400, 396, 392, 388, 399, 395, 391,
	387, 398, 394, 390, 386, 397, 393, 389, 385, 192, 188, 184, 180, 191, 187, 183, 179, 190, 186, 182,
	178, 189, 185, 181, 177, 176, 172, 168, 164, 175, 171, 167, 163, 174, 170, 166, 162, 173, 169, 165,
	161, 160, 156, 152, 148, 159, 155, 151, 147, 158, 154, 150, 146, 157, 153, 149, 145, 144, 140, 136,
	132, 143, 139, 135, 131, 142, 138, 134, 130, 141, 137, 133, 129, 128, 124, 120, 116, 127, 123, 119,
	115, 126, 122, 118, 114, 125, 121, 117, 113, 112, 108, 104, 100, 111, 107, 103,  99, 110, 106, 101,
	 98, 109, 105, 101,  97,  96,  92,  88,  84,  95,  91,  87,  83,  94,  90,  86,  82,  93,  89,  85,
	 81,  80,  76,  72,  68,  79,  75,  71,  67,  78,  74,  70,  66,  77,  73,  69,  65,  64,  60,  56,
	 52,  63,  59,  55,  51,  62,  58,  54,  50,  61,  57,  53,  49,  48,  44,  40,  36,  47,  43,  39,
	 35,  46,  42,  38,  34,  45,  41,  37,  33,  32,  28,  24,  20,  31,  27,  23,  19,  30,  26,  22,
	 18,  29,  25,  21,  17,  16,  12,   8,   4,  15,  11,   7,   3,  14,  10,   6,   2,  13,   9,   5,
	  1,
};

} // namespace chess960

namespace nag {

struct CommentToken
{
	mstl::string	token;
	int				value;
};

static CommentToken const Map[] =
{
	{ "!",		GoodMove																	}, //   1
	{ "!!",		VeryGoodMove															}, //   3
	{ "!?",		SpeculativeMove														}, //   5
	{ "&&",		EqualChancesActivePosition											},	//  12
	{ "&/=",		WithCompensationForMaterial										}, // 181
	{ "&~",		EqualChancesActivePosition											},	//  12
	{ "()",		Space																		},	// 173
	{ "(+)",		Zeitnot																	},	// 174
	{ "(.)",		Zugzwang																	},	// 176
	{ "++--",	WhiteHasACrushingAdvantage											},	//  20
	{ "+-",		WhiteHasADecisiveAdvantage											},	//  18
	{ "+--",		WhiteHasACrushingAdvantage											},	//  20
	{ "+/-",		WhiteHasAModerateAdvantage											},	//  16
	{ "+/=",		WhiteHasASlightAdvantage											},	//  14
	{ "+/~",		WhiteHasMoreThanAdequateCompensationForMaterialDeficit	},	//  46
	{ "+=",		WhiteHasASlightAdvantage											},	//  14
	{ "-+",		BlackHasADecisiveAdvantage											},	//  19
	{ "--+",		BlackHasACrushingAdvantage											},	//  21
	{ "--++",	BlackHasACrushingAdvantage											},	//  21
	{ "-/+",		BlackHasAModerateAdvantage											},	//  17
	{ "-/~",		BlackHasMoreThanAdequateCompensationForMaterialDeficit	},	//  47
	{ "->",		Attack																	},	// 178
	{ "->/<-",	Counterplay																},	// 180
	{ "//",		Diagonal																	},	// 150
	{ "/\\",		WithTheIdea																},	// 140
	{ "/^",		Diagonal																	},	// 150
	{ "<->",		Line																		},	// 149
	{ "<<",		Queenside																},	// 184
	{ "<=",		WorseMove																},	// 143
	{ "<=/=>",	Counterplay																},	// 180
	{ "<=>",		Counterplay																},	// 180
	{ "=",		EqualChancesQuietPosition											},	//  11
	{ "=&",		EqualChancesActivePosition											},	//  12
	{ "=+",		BlackHasASlightAdvantage											},	//  15
	{ "=/&",		WithCompensationForMaterial										}, // 181
	{ "=/+",		BlackHasASlightAdvantage											},	//  15
	{ "=/~",		WithCompensationForMaterial										}, // 181
	{ "==",		DrawishPosition														}, //  10
	{ "=>",		Attack																	},	// 178
	{ "=>/<=",	Counterplay																},	// 180
	{ "=~",		EqualChancesActivePosition											},	//  12
	{ "><",		WeakPoint																},	// 147
	{ ">=",		BetterMove																},	// 142
	{ ">>",		Kingside																	},	// 183
	{ "?",		PoorMove																	}, //   2
	{ "?!",		QuestionableMove														}, //   6
	{ "??",		VeryPoorMove															}, //   4
	{ "@",		Development																},	// 175
	{ "[+]",		Center																	},	// 167
	{ "[]",		SingularMove															},	//   8
	{ "\\/",		AimedAgainst															},	// 141
	{ "^",		Initiative																},	// 179
	{ "^=",		BishopsOfSameColor													},	// 154
	{ "^^",		PairOfBishops															},	// 182
	{ "^_",		BishopsOfOppositeColor												},	// 153
	{ "_|",		Without																	},	// 166
	{ "_|_",		Endgame																	},	// 148
	{ "o..o",	UnitedPawns																},	// 159
	{ "o.o",		UnitedPawns																},	// 159
	{ "o/o",		SeparatedPawns															},	// 157
	{ "o^",		PassedPawn																},	// 160
	{ "oo",		DoublePawns																},	// 158
	{ "|^",		Initiative																},	// 179
	{ "|_",		With																		},	// 165
	{ "||",		File																		},	// 168
	{ "~",		UnclearPosition														},	//  13
	{ "~&",		EqualChancesActivePosition											},	//  12
	{ "~/=",		WithCompensationForMaterial										}, // 181
	{ "~~",		UnclearPosition														},	//  13
};

inline static bool
operator<(CommentToken const& commentToken, mstl::string const& token)
{
	return ::strncmp(commentToken.token, token, token.size()) < 0;
}

} // namespace nag

namespace country {

struct Pair { mstl::string name; Code code; };

static Pair const NameMap[] =
{
	// The PGN standard is using the IOC country codes. This is really stupid. The IOC
	// country codes has changed many times. ChessBase is using his own set of country
	// codes. This is more stupid than using the IOC country codes. As a result the
	// lookup of the country code cannot garantuee the proper country name.

	// IOC = IOC country codes	http://en.wikipedia.org/wiki/List_of_IOC_country_codes
	// PGN = PGN specification	http://www.chessclub.com/help/PGN-spec
	// ISO = ISO-3166-3			http://www.worldlingo.com/ma/enwiki/de/ISO_3166-1_alpha-3

	//  Plate Codes: http://en.wikipedia.org/wiki/List_of_international_vehicle_registration_codes

	{ "ABW", Aruba													},	// ISO
	{ "ADE", East_Germany										},	// 				IOC 1968
	{ "AFC", Central_African_Republic						},	// 				IOC 1968
	{ "AFG", Afghanistan											},	// ISO	PGN	IOC
	{ "AGG", Aruba													},	// --- ChessBase ---
	{ "AGL", Algeria												},	// 				IOC 1968 S
	{ "AGO", Angola												},	// ISO
	{ "AGR", Algeria												},	// 				IOC 1964
	{ "AHO", Netherlands_Antilles								},	//					IOC
	{ "AIA",	Anguilla												}, // ISO
	{ "AIR", Aboard_Aircraft									},	// 		PGN
	{ "ALA", Aaland_Islands										},	// ISO
	{ "ALB", Albania												},	// ISO	PGN	IOC
	{ "ALE", West_Germany										},	// 				IOC 1968 S
	{ "ALG", Algeria												},	// 		PGN	IOC
	{ "ALL", West_Germany										},	// 				IOC 1968 W
	{ "AND", Andorra												},	// ISO	PGN	IOC
	{ "ANG", Angola												},	//			PGN	IOC
	{ "ANT", Antigua												},	//			PGN	IOC
//	{ "ANT", Netherlands_Antilles								},	// ISO
	{ "ANZ", Australasia											},	// 				IOC
	{ "ARE", United_Arab_Emirates								},	// ISO
	{ "ARG", Argentina											},	// ISO	PGN	IOC
	{ "ARK", Antarctica											},	// --- plate code ---
	{ "ARM", Armenia												},	// ISO	PGN	IOC
	{ "ARS", Saudi_Arabia										},	// 				IOC 1968-1976
	{ "ARU", Aruba													},	// 				IOC
	{ "ASA", American_Samoa										},	//					IOC
	{ "ASM", American_Samoa										},	//	ISO
	{ "ASU", Anguilla												},	// --- ChessBase ---
	{ "ATA", Antarctica											},	//	ISO	PGN
	{ "ATF", French_Southern_Territories					},	//	ISO
	{ "ATG", Antigua												},	//	ISO
	{ "ATO", Netherlands_Antilles								},	// 				IOC 1960
	{ "AUA", Aruba													},	// --- plate code ---
	{ "AUS", Australia											},	// ISO	PGN	IOC
	{ "AUT", Austria,												},	// ISO			IOC
	{ "AXA", Anguilla												},	// --- plate code ---
	{ "AZB", Azerbaijan											},	// 		PGN
	{ "AZE", Azerbaijan											},	//	ISO			IOC
	{ "BAD", Barbados												},	// 				IOC 1964
	{ "BAH", Bahamas												},	// 				IOC
	{ "BAN", Bangladesh											},	// 		PGN	IOC
	{ "BAR", Barbados												},	// 		PGN	IOC
	{ "BAS", Basque												},	// --- ChessBase ---
	{ "BDI", Burundi												},	// ISO			IOC
	{ "BDS", Barbados												},	// --- plate code ---
	{ "BEL", Belgium												},	// ISO	PGN	IOC
	{ "BEN", Benin,												},	// ISO			IOC
	{ "BER", Bermuda												},	// 		PGN	IOC
	{ "BFA", Burkina_Faso										},	// ISO
	{ "BGR", Bulgaria												},	//	ISO
	{ "BHG", Bosnia_and_Herzegovina							},	// --- common abbrev. ---
	{ "BHM", Bahamas												},	//			PGN
	{ "BHN", Bahrain												},	// --- ChessBase ---
	{ "BHR", Bahrain												},	// ISO
	{ "BHS", Bahamas												},	// ISO
	{ "BHT", Bhutan,												},	// --- plate code ---
	{ "BHU", Bhutan,												},	// 				IOC
	{ "BIH", Bosnia_and_Herzegovina							},	// ISO	PGN	IOC
	{ "BIR", Myanmar												},	// 				IOC 1948-1988
	{ "BIZ", Belize												},	// 				IOC
	{ "BLA", Belarus												},	// 		PGN
	{ "BLG", Bulgaria												},	// 		PGN
	{ "BLR", Belarus												},	//	ISO			IOC
	{ "BLZ", Belize												},	// ISO	PGN
	{ "BMU", Bermuda												},	// ISO
	{ "BNN", Benin													},	// --- ChessBase ---
	{ "BOL", Bolivia												},	// ISO	PGN	IOC
	{ "BOT", Botswana												},	//					IOC
	{ "BRA", Brazil												},	//	ISO			IOC
	{ "BRB", Barbados												},	// ISO	PGN
	{ "BRD", West_Germany										},	// --- common abbrev. ---
	{ "BRI", Burundi												},	// --- ChessBase ---
	{ "BRN", Bahrain												},	// 				IOC
	{ "BRS", Brazil												},	// 		PGN
	{ "BRU", Brunei												},	// 		PGN	IOC
	{ "BSH", Bosnia_and_Herzegovina							},	// 				IOC 1992 S
	{ "BSW", Botswana												},	// 		PGN
	{ "BTN", Bhutan												},	// ISO
	{ "BUL", Bulgaria												},	//					IOC
	{ "BUR", Burkina_Faso										},	//					IOC
	{ "BVI", British_Virgin_Islands							},	// --- common abbrev. ---
	{ "BVT", Bouvet_Islands										},	// ISO
	{ "BWA", Botswana												},	// ISO
	{ "CAB", Cambodia												},	// 				IOC 1964
	{ "CAF", Central_African_Republic						},	// ISO			IOC
	{ "CAM", Cambodia												},	// 				IOC
//	{ "CAM", Cameroon												},	// --- ChessBase ---
	{ "CAN", Canada												},	// ISO	PGN	IOC
	{ "CAR", Central_African_Republic						},	// --- ChessBase ---
	{ "CAT", Catalonia											},	// --- ChessBase ---
	{ "CAY", Cayman_Islands										},	// 				IOC
	{ "CCK", Cocos_Islands										},	//	ISO
	{ "CDN", Canada												},	// --- plate code ---
	{ "CEY", Sri_Lanka											},	// 				IOC 1948-1972
	{ "CGO", Congo													},	// 				IOC
	{ "CHA", Chad,													},	// 				IOC
	{ "CHD", Chad,													},	// 				IOC 1964
	{ "CHE", Czech_Republic										},	// 				IOC 1968 S
//	{ "CHE", Switzerland											},	// ISO
	{ "CHI", Chile,												},	// 		PGN	IOC
	{ "CHL", Chile,												},	// ISO
	{ "CHN", China,												},	// ISO			IOC
	{ "CIA", Christmas_Island									},	// --- ChessBase ---
//	{ "CIB", Channel_Islands									},	// --- ChessBase ---
	{ "CIL", Chile,												},	// 				IOC 1956 W, 1960 S
	{ "CIN", Cook_Islands										},	// --- ChessBase ---
	{ "CIV", Ivory_Coast,										},	// ISO			IOC
	{ "CMB", Cambodia												},	// --- ChessBase ---
	{ "CML", Ivory_Coast,										},	// 				IOC 1968
	{ "CMR", Cameroon,											},	// ISO			IOC
	{ "COA", Cocos_Islands										},	// --- ChessBase ---
	{ "COD", DR_Congo,											},	// ISO			IOC
	{ "COG", Congo													},	// ISO
	{ "COK", Cook_Islands,										},	// ISO			IOC
	{ "COL", Colombia												},	// ISO	PGN	IOC
	{ "COM", Comoros												},	// ISO			IOC
	{ "CON", Congo													},	// --- ChessBase ---
	{ "COR", South_Korea											},	// 				IOC 1956 W, 1960 S, 1968 S,1972 S
	{ "COS", Costa_Rica											},	// 				IOC 1964
	{ "CPV", Cape_Verde											},	// ISO			IOC
	{ "CRA", Costa_Rica											},	// 		PGN
	{ "CRC", Costa_Rica											},	// 				IOC
	{ "CRI", Costa_Rica											},	//	ISO
	{ "CRO", Croatia												},	// 		PGN	IOC
	{ "CRS", Czech_Republic										}, // --- common scrambled letters ---
	{ "CSK", Czech_Republic										},	// --- common abbrev. ---
	{ "CSL", Czech_Republic										},	// 				IOC 1956 W
	{ "CSR", Czechoslovakia										},	// 		PGN
//	{ "CSR", Czechoslovakia										},	// --- ChessBase ---
	{ "CSV", Czech_Republic										},	// 				IOC 1960 S
	{ "CTC", Costa_Rica,											},	// 				IOC 1984 W
	{ "CUB", Cuba													},	//	ISO	PGN	IOC
	{ "CXR",	Christmas_Island									},	//	ISO
	{ "CYM", Cayman_Islands										},	//	ISO
	{ "CYP", Cyprus												},	// ISO	PGN	IOC
	{ "CZE", Czech_Republic										},	// ISO			IOC
	{ "CZS", Czech_Republic										},	// 				IOC 1964 S
	{ "DAH", Benin													},	// 				IOC 1968-1976
	{ "DAN", Denmark												},	// 				IOC 1960 W, 1968 W
	{ "DAY", Benin													},	// 				IOC 1964
	{ "DDR", East_Germany										},	//
	{ "DEN", Denmark												},	//			PGN	IOC
	{ "DEU", Germany												},	// ISO
	{ "DIN", Denmark												},	// 				IOC 1968 S
	{ "DJI", Djibouti												},	//	ISO			IOC
	{ "DMA", Dominica												},	//	ISO			IOC
	{ "DNK", Denmark												},	// ISO
	{ "DOM", Dominican_Republic								},	// ISO	PGN	IOC
	{ "DZA", Algeria												},	// ISO
	{ "EAK", Kenya													},	// --- plate code ---
	{ "EAT", Tanzania												},	// --- plate code ---
	{ "EAU", Uganda												},	// --- plate code ---
	{ "EAZ", Zanzibar												},	// ISO
	{ "ECU", Ecuador												},	// ISO	PGN	IOC
	{ "EGY", Egypt													},	// ISO	PGN	IOC
	{ "ELG", Equatorial_Guinea									},	// --- ChessBase ---
	{ "ENG", England												},	// ISO	PGN
	{ "ERI", Eritrea												},	// ISO			IOC
	{ "ESA", El_Salvador											},	//					IOC
	{ "ESH", Western_Sahara										},	// ISO
	{ "ESP", Spain													},	// ISO	PGN	IOC
	{ "EST", Estonia												},	// ISO	PGN	IOC
	{ "ETH", Ethiopia												},	// ISO			IOC
	{ "ETI", Estonia												},	// 				IOC 1960, 1968
	{ "EUA", Germany												},	// 				IOC 1956-1964
//	{ "EUA", United_States_of_America						},	// 				IOC 1968 S
	{ "EUN", Mixed_Team											},	// 				IOC 1992
	{ "FAI", Faroe_Islands										},	// 		PGN
	{ "FGA", Guadeloupe											},	// --- ChessBase ---
	{ "FGB", Falkland_Islands									},	// --- ChessBase ---
	{ "FIG", Fiji													},	// 				IOC 1960
	{ "FIJ", Fiji													},	// ISO	PGN	IOC
	{ "FIL", Philippines											},	// 				IOC 1960, 1968
	{ "FIN", Finland												},	//	ISO	PGN	IOC
	{ "FJI", Fiji													},	// ISO
	{ "FLK", Falkland_Islands									},	// ISO
	{ "FRA", France												},	// ISO	PGN	IOC
	{ "FRB", Germany												},	// 				IOC 1980-1988
	{ "FRG", West_Germany										},	// 				IOC
//	{ "FRG", French_Guiana										},	// --- ChessBase ---
	{ "FRM", Macedonia											},	// --- common abbrev. ---
	{ "FRO", Faroe_Islands										},	// ISO
	{ "FRP", French_Polynesia									},	// --- ChessBase ---
	{ "FSM", Micronesia											},	// ISO			IOC
	{ "GAB", Gabon													},	// ISO			IOC
	{ "GAM", Gambia												},	// 		PGN	IOC
	{ "GBA", Guernsey												},	// --- plate code ---
	{ "GBG", Guernsey												},	// --- plate code ---
	{ "GBI", Great_Britain										},	// 				IOC 1964
	{ "GBJ", Jersey												},	// --- plate code ---
	{ "GBM", Isle_of_Man											},	// --- plate code ---
	{ "GBR", Great_Britain										},	//	ISO			IOC
	{ "GBS", Guinea_Bissau										},	//					IOC
	{ "GBZ", Gibraltar											},	// ISO
	{ "GCA", Guatemala											},	// --- plate code ---
	{ "GCI", Guernsey												},	// --- ChessBase ---
	{ "GDR", East_Germany										},	//					IOC
	{ "GE2", Germany												},	// --- ChessBase ---
	{ "GEO", Georgia												},	// ISO	PGN	IOC
	{ "GEQ", Equatorial_Guinea									},	//					IOC
	{ "GER", Germany												},	//			PGN	IOC
	{ "GGB", Gibraltar											},	// --- ChessBase ---
	{ "GGY", Guernsey,											}, // ISO
	{ "GHA", Ghana													},	//	ISO	PGN	IOC
	{ "GIA", Japan													},	// 				IOC 1956 W, 1960 S
	{ "GIN", Guinea												},	// ISO
	{ "GLP", Guadeloupe											},	// ISO
	{ "GMA", Guam													},	// --- ChessBase ---
	{ "GMB", Gambia												},	// ISO
	{ "GNB", Guinea_Bissau										},	// ISO
	{ "GNQ", Equatorial_Guinea									},	// ISO
	{ "GRA", Grenada												},	// --- ChessBase ---
	{ "GRB", Great_Britain										},	// 				IOC 1956 W-1960
	{ "GRC", Greece												},	// ISO	PGN
	{ "GRD", Grenada												},	// ISO
	{ "GRE", Greece												},	// 				IOC
	{ "GRL", Greenland											},	// --- ChessBase ---
	{ "GRN", Grenada												},	//					IOC
	{ "GRO", Greenland											},	// ISO
	{ "GTM", Guatemala											},	// ISO
	{ "GUA", Guatemala											},	//			PGN	IOC
	{ "GUB", Guinea_Bissau										},	// --- ChessBase ---
	{ "GUF", French_Guiana										},	// ISO
	{ "GUI", Guinea												},	//					IOC
	{ "GUM", Guam													},	// ISO			IOC
	{ "GUT", Guatemala											},	// 				IOC 1964
	{ "GUY", Guyana												},	// ISO	PGN	IOC
	{ "HAI", Haiti													},	// 		PGN	IOC
	{ "HBR", Belize,												},	// 				IOC 1968-1972
	{ "HGB", Saint_Helena										},	// --- ChessBase ---
	{ "HGK", Hong_Kong											},	// --- common abbrev. ---
	{ "HIM", Heard_Island_and_McDonald_Islands			},	// --- common abbrev. ---
	{ "HKG", Hong_Kong											},	// ISO	PGN	IOC
	{ "HMD", Heard_Island_and_McDonald_Islands			},	// ISO
	{ "HND", Honduras												},	// ISO
	{ "HOK", Hong_Kong											},	// 				IOC 1960-1968
	{ "HOL", Netherlands											},	// 				IOC 1968-1988
	{ "HON", Honduras												},	//			PGN	IOC
	{ "HRV", Croatia												},	// ISO
	{ "HTI", Haiti													},	// ISO
	{ "HUN", Hungary												},	// ISO	PGN	IOC
	{ "ICE", Iceland												},	// 				IOC 1960 W, 1964 S
	{ "ICL", Iceland												},	// --- common abbrev. ---
	{ "IDN", Indonesia											},	// ISO
	{ "IMN", Isle_of_Man											},	// ISO
	{ "INA", Indonesia											},	//					IOC
	{ "IND", India													},	//	ISO	PGN	IOC
	{ "INS", Indonesia											},	// 				IOC 1960
	{ "INT", The_Internet										},	//
	{ "IOM", Isle_of_Man											},	// --- ChessBase ---
	{ "IOT", British_Indian_Ocean_Territory				},	// ISO
	{ "IRA", Iran													},	// 				IOC 1968 W
	{ "IRE", Ireland												},	// --- common abbrev. ---
	{ "IRI", Iran													},	//					IOC
	{ "IRK", Iraq													},	// 				IOC 1960, 1968
	{ "IRL", Ireland												},	// ISO	PGN	IOC
	{ "IRN", Iran													},	// ISO	PGN	IOC 1956-1968
	{ "IRQ", Iraq													},	// ISO	PGN	IOC
	{ "ISD", Iceland												},	// 		PGN
	{ "ISL", Iceland												},	// ISO			IOC
	{ "ISR", Israel												},	// ISO	PGN	IOC
	{ "ISS", Aboard_Spacecraft									},	// --- ChessBase ---
	{ "ISV", US_Virgin_Islands									},	//					IOC
	{ "ITA", Italy													},	//	ISO	PGN	IOC
	{ "IVB", British_Virgin_Islands							},	//					IOC
	{ "IVC", Ivory_Coast,										},	// 				IOC 1964
	{ "IVO", Ivory_Coast											},	// 		PGN
	{ "JAM", Jamaica												},	//	ISO	PGN	IOC
	{ "JAP", Japan													},	// 		PGN	IOC 1960 W
	{ "JCI", Jersey												},	// --- ChessBase ---
	{ "JEY", Jersey												},	// ISO
	{ "JMY", Jan_Mayen_and_Svalbard							},	// --- ChessBase ---
	{ "JOR", Jordan												},	//					IOC
	{ "JPN", Japan													},	//	ISO			IOC
	{ "JRD", Jordan												},	// 		PGN
	{ "JUG", Yugoslavia											},	// 		PGN	IOC 1956-1960, 1968 W
	{ "KAN", Saint_Kitts_and_Nevis							},	// --- plate code ---
	{ "KAP", Cape_Verde											},	// --- ChessBase ---
	{ "KAZ", Kazakhstan											},	// ISO	PGN	IOC
	{ "KBA", Kiribati												},	// --- ChessBase ---
	{ "KEN", Kenya													},	// ISO	PGN	IOC
	{ "KGZ", Kyrgyzstan											},	// ISO			IOC
	{ "KHM", Cambodia												},	// ISO			IOC 1972-1976
	{ "KIR", Kiribati												},	// ISO			IOC
//	{ "KIR", Kyrgyzstan											},	// 		PGN
	{ "KNA", Saint_Kitts_and_Nevis							},	// ISO
	{ "KOR", South_Korea											},	// ISO			IOC
	{ "KOS", Kosovo												},	// --- ChessBase ---
	{ "KSA", Saudi_Arabia										},	//					IOC
	{ "KUW", Kuwait												},	//			PGN	IOC
	{ "KWT", Kuwait												},	// ISO
	{ "LAO", Laos													},	//	ISO			IOC
	{ "LAR", Libya													},	// --- plate code ---
	{ "LAT", Latvia												},	//			PGN	IOC
	{ "LBA", Libya													},	//					IOC
	{ "LBN", Lebanon												},	// ISO
	{ "LBR", Liberia												},	// ISO			IOC
	{ "LBY", Libya													},	// ISO			IOC 1968 W
	{ "LCA", Saint_Lucia											},	// ISO			IOC
	{ "LEB", Lebanon												},	// 		PGN	IOC 1960 W, 1964 S
	{ "LES", Lesotho												},	//					IOC
	{ "LIB", Lebanon												},	//					IOC
//	{ "LIB", Libya													},	// 		PGN
	{ "LIC", Liechtenstein										},	// 		PGN
	{ "LIE", Liechtenstein										},	// ISO			IOC
	{ "LIH", Liechtenstein										},	// --- common abbrev. ---
	{ "LIT", Lithuania											},	// 				IOC 1992 W
	{ "LKA", Sri_Lanka											},	// ISO
	{ "LSO", Lesotho												},	// ISO
	{ "LTU", Lithuania											},	// ISO	PGN	IOC
	{ "LUX", Luxembourg											},	// ISO	PGN	IOC
	{ "LVA", Latvia												},	// ISO
	{ "LYA", Libya													},	// 				IOC 1964
	{ "MAC",	Macao													},	// ISO
	{ "MAD", Madagascar											},	//					IOC
	{ "MAL", Malaysia												},	// 		PGN	IOC 1964-1988
	{ "MAR", Morocco												},	//	ISO			IOC
	{ "MAS", Malaysia												},	//					IOC
	{ "MAT", Malta,												},	//					IOC
	{ "MAU", Mauritania											},	// 		PGN
	{ "MAW", Malawi												},	//					IOC
	{ "MCO", Monaco												},	// ISO
	{ "MDA", Moldova												},	// ISO			IOC
	{ "MDG", Madagascar											},	// ISO
	{ "MDV", Maldives												},	// ISO			IOC
	{ "MEX", Mexico												},	// ISO	PGN	IOC
	{ "MFR", Martinique											},	// --- ChessBase ---
	{ "MGL", Mongolia												},	// 				IOC
	{ "MHL", Marshall_Islands									},	// ISO			IOC
	{ "MIC", Micronesia											},	// --- ChessBase ---
	{ "MKD", Macedonia											},	//					IOC
	{ "MLD", Moldova												},	// 				IOC 1994
	{ "MLI", Mali													},	// ISO	PGN	IOC
	{ "MLT", Malta													},	// ISO	PGN	IOC
	{ "MMR", Myanmar												},	// ISO
	{ "MNC", Monaco												},	// 		PGN
	{ "MNE", Montenegro											},	//	ISO			IOC
	{ "MNG", Mongolia												},	// ISO
	{ "MNP", Northern_Mariana_Islands						},	// ISO
	{ "MNT", Montenegro											},	// --- ChessBase ---
	{ "MOL", Moldova												},	// 		PGN
	{ "MON", Monaco												},	//					IOC
//	{ "MON", Mongolia												},	// 				IOC 1968 W
	{ "MOZ", Mozambique											},	// ISO	PGN	IOC
	{ "MRC", Morocco												},	// 		PGN	IOC 1964
	{ "MRI", Mauritius											},	//					IOC
	{ "MRT", Mauritius											},	// 		PGN
//	{ "MRT", Mauritania											},	// ISO
	{ "MSG", Montserrat											},	// --- ChessBase ---
	{ "MSH", Marshall_Islands									},	// --- ChessBase ---
	{ "MSR",	Montserrat											},	// ISO
	{ "MTN", Mauritania											},	//					IOC
	{ "MTQ", Martinique											},	//	ISO
	{ "MUS", Mauritius											},	// ISO
	{ "MWI", Malawi												},	// ISO
	{ "MYA", Myanmar												},	//					IOC
	{ "MYF", Mayotte												},	// --- ChessBase ---
	{ "MYN", Myanmar												},	// 		PGN
	{ "MYS", Malaysia												},	// ISO
	{ "MYT", Mayotte												},	// ISO
	{ "NAM", Namibia												},	// ISO			IOC
	{ "NAN", Netherlands_Antilles								},	// 				IOC 1964
	{ "NAU", Nauru													},	// --- ChessBase ---
	{ "NCA", Nicaragua											},	//					IOC
	{ "NCF", New_Caledonia										},	// --- ChessBase ---
	{ "NCG", Nicaragua											},	// 		PGN	IOC 1964
	{ "NCL",	New_Caledonia										},	// ISO
	{ "NED", Netherlands											},	//					IOC
	{ "NEP", Nepal													},	//					IOC
	{ "NER", Niger													},	// ISO
	{ "NET", The_Internet										},	// 		PGN
//	{ "NET", Netherlands											},	// 				IOC 1960 W
//	{ "NET", American_Samoa										},	// --- ChessBase ---
	{ "NFK", Norfolk_Island										},	// ISO
	{ "NGA", Nigeria												},	// ISO			IOC 1964
	{ "NGR", Nigeria												},	//					IOC
	{ "NGU", Papua_New_Guinea									},	// 				IOC 1984-1988
	{ "NGY", Papua_New_Guinea									},	// 				IOC 1976-1980
	{ "NIC", Nicaragua											},	// ISO			IOC 1968
	{ "NIG", Niger													},	//					IOC
//	{ "NIG", Nigeria												},	// 		PGN
	{ "NIR", Northern_Ireland									},	//	ISO
	{ "NIU", Niue													},	// ISO
	{ "NKO", North_Korea											},	// --- ChessBase ---
	{ "NLA", Netherlands_Antilles								},	// 		PGN
	{ "NLD", Netherlands											},	// ISO	PGN	IOC 1964 S
	{ "NMI", Northern_Mariana_Islands						},	// --- ChessBase ---
	{ "NNA", Norfolk_Island										},	// --- ChessBase ---
	{ "NNN", Niue													},	// --- ChessBase ---
	{ "NOR", Norway												},	// ISO	PGN	IOC
	{ "NPL", Nepal													},	// ISO
	{ "NRH", Zambia												},	// 				IOC 1964
	{ "NRU", Nauru													},	// ISO			IOC
	{ "NZD", New_Zealand											},	// 		PGN
	{ "NZE", New_Zealand											},	// 				IOC 1960, 1968 W
	{ "NZL", New_Zealand											},	// ISO			IOC
	{ "OLA", Netherlands											},	// 				IOC 1956 W
	{ "OMN", Oman													},	// ISO
	{ "OST", Austria												},	// 		PGN
	{ "OTM", Timor_Leste											},	// --- ChessBase ---
	{ "PAK", Pakistan												},	// ISO	PGN	IOC
	{ "PAL", Palestine											},	// 		PGN
//	{ "PAL", Palau													},	// --- ChessBase ---
	{ "PAN", Panama												},	// ISO	PGN	IOC
	{ "PAP", Papua_New_Guinea									},	// --- common abbrev. ---
	{ "PAR", Paraguay												},	//			PGN	IOC
	{ "PBA", Netherlands											},	// 				IOC 1960 S
	{ "PCN", Pitcairn_Islands									},	// ISO
	{ "PER", Peru													},	// ISO	PGN	IOC
	{ "PGB", Saint_Pierre_and_Miquelon						},	// --- ChessBase ---
	{ "PHI", Philippines											},	//			PGN	IOC
	{ "PHL", Philippines											},	// ISO
	{ "PIG", Pitcairn_Islands									},	// --- ChessBase ---
	{ "PLE", Palestine											},	//					IOC
	{ "PLW", Palau													},	//	ISO			IOC
	{ "PNG", Papua_New_Guinea									},	//	ISO	PGN	IOC
	{ "POL", Poland												},	//	ISO	PGN	IOC
	{ "POR", Portugal												},	//			PGN	IOC
	{ "PRC", China													},	// 		PGN
	{ "PRI", Puerto_Rico											},	// ISO			IOC 1960
	{ "PRK", North_Korea											},	// ISO			IOC
	{ "PRO", Puerto_Rico											},	// 		PGN	IOC 1968
	{ "PRT", Portugal												},	// ISO
	{ "PRY", Paraguay												},	// ISO
	{ "PSE", Palestine											},	// ISO
	{ "PUR", Puerto_Rico											},	//					IOC
	{ "PYF", French_Polynesia									},	// ISO
	{ "QAT", Qatar													},	// ISO			IOC
	{ "QTR", Qatar													},	// 		PGN
//	{ "RAU", Egypt													},	// 				IOC 1960, 1968
//	{ "RAU", Syria													},	// 				IOC 1960
	{ "RCA", Central_African_Republic						},	// --- plate code ---
	{ "RCB", DR_Congo												},	// --- plate code ---
	{ "RCH", Chile													},	// --- plate code ---
	{ "RCF", Chinese_Taipei										},	// 				IOC 1960
	{ "REU", Reunion												},	// ISO
	{ "RGB", Guinea_Bissau										},	// --- plate code ---
	{ "RHO", Zimbabwe												},	// 				IOC 1960-1972
	{ "RIM", Mauritania											},	// --- plate code ---
	{ "RIN", Indonesia											},	// 		PGN
	{ "RKS", Kosovo												},	// --- plate code ---
	{ "RMI", Marshall_Islands									}, // --- common abbrev. ---
	{ "RMM", Mali													},	// --- plate code ---
	{ "RNR", Zambia												},	// --- plate code ---
	{ "ROC", Chinese_Taipei										},	// 				IOC 1972-1976
	{ "ROK", South_Korea											},	// --- common abbrev. ---
	{ "ROM", Romania												},	// 		PGN	IOC 1956-1960, 1972-2006
	{ "ROU", Romania												},	// ISO			IOC
	{ "RSA", South_Africa										},	//					IOC
	{ "RSM", San_Marino											},	// --- plate code ---
	{ "RSR", Zimbabwe												},	// ISO
	{ "RUF", Reunion												},	// --- ChessBase ---
	{ "RUM", Romania												},	// 				IOC 1964-1972
	{ "RUS", Russia												},	// ISO	PGN	IOC
	{ "RWA", Rwanda												},	// ISO			IOC
	{ "SAA", Germany												},	// --- ChessBase ---
	{ "SAF", South_Africa										},	// 		PGN	IOC 1960-1972
	{ "SAL", El_Salvador											},	// 		PGN	IOC 1964-1976
//	{ "SAL", Solomon_Islands									},	// --- ChessBase ---
	{ "SAM", Samoa													},	//					IOC
	{ "SAO", Sao_Tome_and_Principe							},	// --- ChessBase ---
	{ "SAR", Saudi_Arabia										},	// --- common abbrev. ---
	{ "SAU", Saudi_Arabia										},	// ISO			IOC 1980-1984
	{ "SCG", Serbia_and_Montenegro							},	// ISO			IOC
	{ "SCO", Scotland												},	// 		PGN
	{ "SDN", Sudan													},	// ISO
	{ "SEA", At_Sea												},	// 		PGN
	{ "SEN", Senegal												},	// ISO	PGN	IOC
	{ "SER", Serbia												},	// --- ChessBase ---
	{ "SEY", Seychelles											},	//			PGN	IOC
	{ "SGL", Senegal												},	// 				IOC 1964
	{ "SGP", Singapore											},	// ISO
	{ "SGS", South_Georgia_and_South_Sandwich_Islands	},	// ISO
	{ "SHN", Saint_Helena										},	// ISO
	{ "SIE", Sierra_Leone										},	// --- ChessBase ---
	{ "SIN", Singapore											},	// 				IOC
	{ "SIP", Singapore											},	// 		PGN
	{ "SIR", Syria													},	// 				IOC 1968
	{ "SJM", Jan_Mayen_and_Svalbard							},	// ISO
	{ "SKI", Saint_Kitts_and_Nevis							},	// --- ChessBase ---
	{ "SKN", Saint_Kitts_and_Nevis							},	//					IOC
	{ "SLA", Sierra_Leone										},	// 				IOC 1968
	{ "SLB", Solomon_Islands									},	// ISO
	{ "SLE", Sierra_Leone										},	// ISO			IOC
	{ "SLO", Slovenia												},	//					IOC
	{ "SLU", Saint_Lucia											},	// --- ChessBase ---
	{ "SLV", Slovenia												},	// 		PGN
//	{ "SLV", El_Salvador											},	// ISO
	{ "SMA", San_Marino											},	// 		PGN	IOC 1960-1964
	{ "SME", Suriname												},	// --- plate code ---
	{ "SMR", San_Marino											},	// ISO			IOC
	{ "SOL", Solomon_Islands									},	//					IOC
	{ "SOM", Somalia												},	//	ISO			IOC
	{ "SOV", Soviet_Union										},	// 				IOC 1968 W
	{ "SPA", Spain													},	// 				IOC 1956-1964, 1968 W
	{ "SPC", Aboard_Spacecraft									},	// 		PGN
	{ "SPM", Saint_Pierre_and_Miquelon						},	// ISO
	{ "SRB", Serbia,												},	// ISO			IOC
	{ "SRI", Sri_Lanka											},	//			PGN	IOC
	{ "SRL", Sierra_Leone										},	// --- common abbrev. ---
	{ "SRV", Vietnam												},	// --- common abbrev. ---
	{ "STP", Sao_Tome_and_Principe							},	// ISO			IOC
	{ "SUA", United_States_of_America						},	// 				IOC 1960 S
	{ "SUD", Sudan													},	//			PGN	IOC
	{ "SUE", Sweden												},	// 				IOC 1968 S
	{ "SUI", Switzerland											},	//					IOC
	{ "SUR", Suriname												},	// ISO	PGN	IOC
	{ "SVE", Sweden												},	// 		PGN	IOC 1956 W, 1960 S
	{ "SVI", Switzerland											},	// 				IOC 1956 W, 1960 S
//	{ "SVI", Saint_Vincent_and_the_Grenadines				},	// --- ChessBase ---
	{ "SVK", Slovakia												},	// ISO			IOC
	{ "SVN", Slovenia												},	// ISO
//	{ "SVN", Jan_Mayen_and_Svalbard							},	// --- ChessBase ---
	{ "SVR", Syria													},	// --- plate code ---
	{ "SWA", Swaziland											},	// --- ChessBase ---
	{ "SWE",	Sweden												},	// ISO			IOC
	{ "SWI", Switzerland											},	// 				IOC 1960 W, 1964 S
	{ "SWZ", Swaziland											},	// ISO			IOC
//	{ "SWZ", Switzerland											},	// 		PGN
	{ "SYC", Seychelles											},	// ISO
	{ "SYR", Syria													},	// ISO	PGN	IOC
	{ "TAI", Thailand												},	// 		PGN	IOC 1960, 1968
	{ "TAN", Tanzania												},	//					IOC
	{ "TCA",	Turks_and_Caicos_Islands						},	// ISO
	{ "TCD", Chad,													},	// ISO
	{ "TCH", Czechoslovakia										},	// 				IOC
	{ "TCI", Turks_and_Caicos_Islands						},	// --- ChessBase ---
	{ "TGA", Tonga													},	//					IOC
	{ "TGO", Togo													},	// ISO
	{ "THA", Thailand												},	// ISO			IOC
	{ "TIB", Tibet,												},	// ISO
	{ "TJK", Tajikistan											},	// ISO			IOC
	{ "TKI", Tokelau												},	// --- ChessBase ---
	{ "TKL", Tokelau												},	// ISO
	{ "TKM", Turkmenistan										},	// ISO			IOC
	{ "TLS", Timor_Leste											},	// ISO			IOC
	{ "TMT", Turkmenistan										},	// 		PGN
	{ "TOG", Togo													},	//					IOC
	{ "TON", Tonga													},	// ISO			IOC 1984
	{ "TPE", Chinese_Taipei										},	//					IOC
	{ "TRI", Trinidad_and_Tobago								},	//					IOC
	{ "TRK", Turkey												},	// 		PGN
	{ "TRT", Trinidad_and_Tobago								},	// 				IOC 1964-1968
	{ "TTO", Trinidad_and_Tobago								},	// ISO	PGN
	{ "TUN", Tunisia												},	// ISO	PGN	IOC
	{ "TUR", Turkey												},	// ISO			IOC
	{ "TUV", Tuvalu												},	// ISO			IOC
	{ "TWN", Chinese_Taipei										},	// ISO			IOC 1964-1968
	{ "TZA", Tanzania												},	// ISO
	{ "UAE", United_Arab_Emirates								},	// 		PGN	IOC
	{ "UAR", Egypt													},	//					IOC 1964
	{ "UGA", Uganda												},	//	ISO	PGN	IOC
	{ "UKR", Ukraine												},	//	ISO	PGN	IOC
	{ "UMI", United_States_Minor_Outlying_Islands		},	// ISO
	{ "UNG", Hungary												},	// 				IOC 1956 W, 1960 S
	{ "UNK", Unknown												},	// 		PGN
	{ "URG", Uruguay												},	// 				IOC 1968
	{ "URS", Soviet_Union										},	//					IOC
	{ "URU", Uruguay												},	//			PGN	IOC
	{ "URY", Uruguay												},	// ISO
	{ "USA", United_States_of_America						},	// ISO	PGN	IOC
	{ "UZB", Uzbekistan											},	// ISO	PGN	IOC
	{ "VAE", United_Arab_Emirates								},	// --- common abbrev. ---
	{ "VAN", Vanuatu												},	// 				IOC
	{ "VAT",	Vatican												},	// ISO
	{ "VCT", Saint_Vincent_and_the_Grenadines				},	// ISO
	{ "VEN", Venezuela											},	// ISO	PGN	IOC
	{ "VET", Vietnam												},	// 				IOC 1964
	{ "VGB", British_Virgin_Islands							},	// ISO	PGN
	{ "VIE", Vietnam												},	// 		PGN	IOC
	{ "VIN", Saint_Vincent_and_the_Grenadines				},	//					IOC
	{ "VIR", US_Virgin_Islands									},	// ISO
	{ "VNM", Vietnam												},	// ISO			IOC 1968-1976
	{ "VOL", Burkina_Faso										},	// 				IOC 1972-1984
	{ "VUS", US_Virgin_Islands									},	// 		PGN
	{ "VUT", Vanuatu												},	// ISO
	{ "WAG", Gambia												},	// --- plate code ---
	{ "WAL", Wales													},	// --- common abbrev. ---
	{ "WAN", Nigeria												},	// --- plate code ---
	{ "WFR", Wallis_and_Futuna									},	// --- ChessBase ---
	{ "WLF", Wallis_and_Futuna									},	// ISO
	{ "WLS", Wales													},	// 		PGN
	{ "WSA", Western_Sahara										},	// --- plate code ---
	{ "WSM", Samoa													},	// ISO
	{ "XXX", Unknown												},	// ISO
	{ "YAR", Yemen													},	// --- plate code ---
	{ "YEM", Yemen													},	// ISO	PGN	IOC
	{ "YMD", Yemen													},	// 				IOC 1988
	{ "YUG", Yugoslavia											},	// ISO	PGN	IOC
	{ "YUS",	Yugoslavia											},	// 				IOC 1964 S
	{ "ZAF", South_Africa										},	// ISO
	{ "ZAI", DR_Congo,											},	// 				IOC 1972-1996
	{ "ZAM", Zambia												},	//			PGN	IOC
	{ "ZAR", DR_Congo												},	// 				IOC 1974
//	{ "ZAR", Russia												},	// --- ChessBase ---
	{ "ZIM", Zimbabwe												},	//			PGN	IOC
	{ "ZMB", Zambia												},	// ISO
	{ "ZRE", DR_Congo												},	// 		PGN
	{ "ZWE", Zimbabwe												},	// ISO
	{ "ZZX", Mixed_Team											},	//					IOC
};

static int
compareNames(void const* lhs, const void* rhs)
{
	//M_ASSERT(strlen(static_cast<char const*>(lhs)) >= 3);
	//M_ASSERT(static_cast<mstl::string const*>(rhs)->size() == 3);

	return ::strncmp(static_cast<char const*>(lhs), static_cast<mstl::string const*>(rhs)->c_str(), 3);
}

} // namespace country

namespace castling
{
	unsigned Transpose[AllRights + 1];

	void
	initialize()
	{
		::memset(Transpose, 0, sizeof(Transpose));

		for (unsigned i = 1; i <= AllRights; ++i)
		{
			if (i & WhiteQueenside)	Transpose[i] |= WhiteKingside;
			if (i & BlackQueenside)	Transpose[i] |= BlackKingside;
			if (i & WhiteKingside)	Transpose[i] |= WhiteQueenside;
			if (i & BlackKingside)	Transpose[i] |= BlackQueenside;
		}
	}
} // namespace castling

namespace tag
{
	static mstl::string const* NameLookup[ExtraTag];

	mstl::bitfield<uint64_t> IsWhiteRating;
	mstl::bitfield<uint64_t> IsBlackRating;
	mstl::bitfield<uint64_t> IsRating;

	void
	initialize()
	{
		static_assert(U_NUMBER_OF(NameLookup) == ExtraTag, "NameLookup expired");
		static_assert(U_NUMBER_OF(NameMap) - 2 == ExtraTag, "NameMap expired");
		static_assert(ExtraTag <= 8*sizeof(uint64_t), "BitField size exceeded");

#ifndef NDEBUG
		::memset(NameLookup, 0, sizeof(NameLookup));
#endif

		for (unsigned i = 0; i < U_NUMBER_OF(NameMap); ++i)
			NameLookup[NameMap[i].id] = &NameMap[i].name;

#if !defined(NDEBUG) && !__GNUC_PREREQ(4,7)
		for (int i = 0; i < ExtraTag; ++i)
			assert(NameLookup[i]);
#endif

		IsWhiteRating.set(WhiteDWZ);
		IsWhiteRating.set(WhiteECF);
		IsWhiteRating.set(WhiteElo);
		IsWhiteRating.set(WhiteICCF);
		IsWhiteRating.set(WhiteIPS);
		IsWhiteRating.set(WhiteRapid);
		IsWhiteRating.set(WhiteRating);
		IsWhiteRating.set(WhiteUSCF);

		IsBlackRating.set(BlackDWZ);
		IsBlackRating.set(BlackECF);
		IsBlackRating.set(BlackElo);
		IsBlackRating.set(BlackICCF);
		IsBlackRating.set(BlackIPS);
		IsBlackRating.set(BlackRapid);
		IsBlackRating.set(BlackRating);
		IsBlackRating.set(BlackUSCF);

		IsRating |= IsWhiteRating;
		IsRating |= IsBlackRating;
	}

	bool initializeIsOk() { return NameLookup[0] != 0; }

} // namespace tag
} // namespace db


static void
#if __GNUC_PREREQ(4,7)
__attribute__((constructor(65535)))
#else
__attribute__((constructor))
#endif
initialize()
{
	castling::initialize();
	tag::initialize();
}


unsigned
castling::transpose(unsigned rights)
{
	//M_ASSERT(rights <= AllRights);
	return Transpose[rights];
}


mstl::string const&
piece::utf8::asString(Type type)
{
	return Table[type];
}


piece::Type
piece::fromLetter(char piece)
{
	static Type Lookup[128] =
	{
#define _ None
		_, _, _, _, _, _, _, _, //   0 -   7
		_, _, _, _, _, _, _, _, //   8 -  15
		_, _, _, _, _, _, _, _, //  16 -  23
		_, _, _, _, _, _, _, _, //  24 -  31
		_, _, _, _, _, _, _, _, //  32 -  39
		_, _, _, _, _, _, _, _, //  40 -  47
		_, _, _, _, _, _, _, _, //  48 -  55
		_, _, _, _, _, _, _, _, //  56 -  63
		_, _, B, _, _, _, _, _, //  64 -  71
		_, _, _, K, _, _, N, _, //  72 -  79
		P, Q, R, _, _, _, _, _, //  80 -  87
		_, _, _, _, _, _, _, _, //  88 -  95
		_, _, B, _, _, _, _, _, //  96 - 103
		_, _, _, K, _, _, N, _, // 104 - 111
		P, Q, R, _, _, _, _, _, // 112 - 119
		_, _, _, _, _, _, _, _, // 120 - 127
#undef _
	};

	return piece >= 0 ? Lookup[static_cast<unsigned char>(piece) & 0x7f] : None;
}


piece::ID
piece::pieceFromLetter(char piece)
{
	static ID Lookup[128] =
	{
#define __ Empty
		__, __, __, __, __, __, __, __, //   0 -   7
		__, __, __, __, __, __, __, __, //   8 -  15
		__, __, __, __, __, __, __, __, //  16 -  23
		__, __, __, __, __, __, __, __, //  24 -  31
		__, __, __, __, __, __, __, __, //  32 -  39
		__, __, __, __, __, __, __, __, //  40 -  47
		__, __, __, __, __, __, __, __, //  48 -  55
		__, __, __, __, __, __, __, __, //  56 -  63
		__, __, WB, __, __, __, __, __, //  64 -  71
		__, __, __, WK, __, __, WN, __, //  72 -  79
		WP, WQ, WR, __, __, __, __, __, //  80 -  87
		__, __, __, __, __, __, __, __, //  88 -  95
		__, __, BB, __, __, __, __, __, //  96 - 103
		__, __, __, BK, __, __, BN, __, // 104 - 111
		BP, BQ, BR, __, __, __, __, __, // 112 - 119
		__, __, __, __, __, __, __, __, // 120 - 127
#undef __
	};

	return piece >= 0 ? Lookup[static_cast<unsigned char>(piece) & 0x7f] : Empty;
}


static void
append(mstl::string& result, unsigned count, mstl::string const& s)
{
	while (count--)
		result += s;
}


title::ID
title::toID(unsigned title)
{
	////M_REQUIRE(title <= Mask_CSIM);
	return title::ID(title ? mstl::bf::lsb_index(title) + 1 : 0);
}


unsigned
title::fromID(ID title)
{
	return title ? 1u << (title - 1) : unsigned(title::None);
}


title::ID
title::fromString(char const* title)
{
	switch (title[0])
	{
		case 'F': return title[1] == 'M' ? title::FM : title::None;
		case 'G': return title[1] == 'M' ? title::GM : title::None;
		case 'N': return title[1] == 'M' ? title::NM : title::None;
		case 'S': return title[1] == 'M' || (title[1] == 'I' && title[2] == 'M') ? title::SM : title::None;

		case 'C':
			switch (title[1])
			{
				case 'G': return title[2] == 'M' ? title::CGM : title::None;
				case 'I': return title[2] == 'M' ? title::CIM : title::None;
				case 'M': return title::CM;
				case 'S': return title[2] == 'I' && title[3] == 'M' ? title::CSIM : title::None;
			}
			break;

		case 'H':
			return title[1] == 'G' && title[2] == 'M' ? title::HGM : title::None;

		case 'I':
			switch (title[1])
			{
				case 'G': return title[2] == 'M' ? title::GM : title::None;
				case 'L': return title[2] == 'M' ? title::CILM : title::None;
				case 'M': return title::IM;
			}
			break;

		case 'L':
			switch (title[1])
			{
				case 'G': return title[2] == 'M' ? title::CLGM : title::None;
				case 'M': return title::LM;
			}
			break;

		case 'W':
			switch (title[1])
			{
				case 'G': return title[2] == 'M' ? title::WGM : title::None;
				case 'I': return title[2] == 'M' ? title::WIM : title::None;
				case 'F': return title[2] == 'M' ? title::WFM : title::None;
				case 'C': return title[2] == 'M' ? title::WCM : title::None;
			}
			break;
	}

	return title::None;
}


mstl::string const&
title::toString(ID title)
{
	//M_REQUIRE(title < Last);
	//M_ASSERT(unsigned(title) < U_NUMBER_OF(Lookup));

	return Lookup[title];
}


title::ID
title::best(unsigned titles)
{
	return titles ? title::ID(mstl::bf::lsb_index(titles) + 1) : title::None;
}


char
species::toChar(ID type)
{
	switch (int(type))
	{
		case Human:		return *StrHuman;
		case Program:	return *StrProgram;
	}

	return '?';
}


mstl::string const&
species::toString(ID type)
{
	switch (int(type))
	{
		case Human:		return StrHuman;
		case Program:	return StrProgram;
	}

	return mstl::string::empty_string;
}


species::ID
species::fromString(char const* s)
{
	switch (*s)
	{
		case 'h':
		case 'H':
			if (::strcasecmp(s, StrHuman) == 0)
				return species::Human;
			break;

		case 'p':
		case 'P':
			if (::strcasecmp(s, StrProgram) == 0)
				return species::Program;
			break;
	}

	return species::Unspecified;
}


bool tag::isWhiteRatingTag(ID tag)	{ return IsWhiteRating.test(tag); }
bool tag::isBlackRatingTag(ID tag)	{ return IsBlackRating.test(tag); }
bool tag::isRatingTag(ID tag)			{ return IsRating.test(tag); }


mstl::string const&
tag::toName(ID tag)
{
	//M_REQUIRE(tag < ExtraTag);
	return *NameLookup[tag];
}


tag::ID
tag::fromName(mstl::string const& tag)
{
	void const* p = ::bsearch(	&tag,
										NameMap,
										U_NUMBER_OF(NameMap),
										sizeof(NameMap[0]),
										compareTags);

	if (p && ::isCaseEqual(tag, static_cast<Pair const*>(p)->name))
		return static_cast<Pair const*>(p)->id;

	return ExtraTag;
}


tag::ID
tag::fromName(char const* name, unsigned length)
{
	mstl::string tag;
	tag.hook(const_cast<char*>(name), length);
	return fromName(tag);
}


unsigned
material::minor(SigPart sig)
{
	return mstl::bf::count_bits(sig.minor);
}


unsigned
material::major(SigPart sig)
{
	return mstl::bf::count_bits(sig.major);
}


unsigned
material::count(SigPart sig)
{
	return mstl::bf::count_bits(sig.piece);
}


mstl::string&
material::print(SigPart sig, mstl::string& result)
{
	unsigned size = result.size();

	result.append(mstl::bf::count_bits(sig.queen),	'q');
	result.append(mstl::bf::count_bits(sig.rook),	'r');
	result.append(mstl::bf::count_bits(sig.bishop),	'b');
	result.append(mstl::bf::count_bits(sig.knight),	'n');

	unsigned n = mstl::bf::count_bits(sig.pawn);

	if (n > 0 || result.size() == size)
		result.format("%u", n);

	return result;
}


mstl::string&
material::print(Signature signature, mstl::string& result)
{
	print(signature.part[color::White], result);
	result += ':';
	print(signature.part[color::Black], result);
	return result;
}


mstl::string&
material::si3::print(Signature sig, mstl::string& result)
{
	unsigned size = result.size();
	unsigned n;

	result.append(sig.wq, 'q');
	result.append(sig.wr, 'r');
	result.append(sig.wb, 'b');
	result.append(sig.wn, 'n');

	n = mstl::bf::count_bits(sig.wp);

	if (n > 0 || result.size() == size)
		result.format("%u", n);

	result += ':';
	size = result.size();

	result.append(sig.bq, 'q');
	result.append(sig.br, 'r');
	result.append(sig.bb, 'b');
	result.append(sig.bn, 'n');

	n = mstl::bf::count_bits(sig.bp);

	if (n > 0 || result.size() == size)
		result.format("%u", n);

	return result;
}


mstl::string&
material::si3::utf8::print(Signature sig, mstl::string& result)
{
	unsigned size = result.size();

	::append(result, sig.wq, piece::utf8::asString(piece::Queen));
	::append(result, sig.wr, piece::utf8::asString(piece::Rook));
	::append(result, sig.wb, piece::utf8::asString(piece::Bishop));
	::append(result, sig.wn, piece::utf8::asString(piece::Knight));

	if (sig.wp || result.size() == size)
		result.format("%u", unsigned(sig.wp));

	result += ':';
	size = result.size();

	::append(result, sig.bq, piece::utf8::asString(piece::Queen));
	::append(result, sig.br, piece::utf8::asString(piece::Rook));
	::append(result, sig.bb, piece::utf8::asString(piece::Bishop));
	::append(result, sig.bn, piece::utf8::asString(piece::Knight));

	if (sig.bp || result.size() == size)
		result.format("%u", unsigned(sig.bp));

	return result;
}


char const*
sq::printAlgebraic(Square square)
{
	static char const* Squares[64] =
	{
		"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
		"a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
		"a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
		"a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
		"a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
		"a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
		"a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
		"a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
	};

	if (square >= U_NUMBER_OF(Squares))
		return "--";

	return Squares[square];
}


char const*
sq::printDescriptive(Square square)
{
	static char const* Squares[8] =
	{
		"QR", "QN", "QB", "Q", "K", "KB", "KN", "KR",
	};

	return Squares[square % 8];
}


char const*
sq::printDescriptive(Square square, color::ID color)
{
	static char const* Squares[64] =
	{
		"QR1", "QN1", "QB1", "Q1", "K1", "KB1", "KN1", "KR1",
		"QR2", "QN2", "QB2", "Q2", "K2", "KB2", "KN2", "KR2",
		"QR3", "QN3", "QB3", "Q3", "K3", "KB3", "KN3", "KR3",
		"QR4", "QN4", "QB4", "Q4", "K4", "KB4", "KN4", "KR4",
		"QR5", "QN5", "QB5", "Q5", "K5", "KB5", "KN5", "KR5",
		"QR6", "QN6", "QB6", "Q6", "K6", "KB6", "KN6", "KR6",
		"QR7", "QN7", "QB7", "Q7", "K7", "KB7", "KN7", "KR7",
		"QR8", "QN8", "QB8", "Q8", "K8", "KB8", "KN8", "KR8",
	};

	if (square >= U_NUMBER_OF(Squares))
		return "--";

   Rank r = rank(square);

   if (color::isBlack(color))
      r = flipRank(r);

	return Squares[make(fyle(square), r)];
}


char const*
sq::printNumeric(Square square)
{
	static char const* Squares[64] =
	{
		"11", "21", "31", "41", "51", "61", "71", "81",
		"12", "22", "32", "42", "52", "62", "72", "82",
		"13", "23", "33", "43", "53", "63", "73", "83",
		"14", "24", "34", "44", "54", "64", "74", "84",
		"15", "25", "35", "45", "55", "65", "75", "85",
		"16", "26", "36", "46", "56", "66", "76", "86",
		"17", "27", "37", "47", "57", "67", "77", "87",
		"18", "28", "38", "48", "58", "68", "78", "88",
	};

	if (square >= U_NUMBER_OF(Squares))
		return "--";

	return Squares[square];
}


char const*
sq::printAlphabetic(Square square)
{
	static char const* Squares[64] =
	{
		"BA", "CA", "DA", "FA", "GA", "HA", "KA", "LA",
		"BE", "CE", "DE", "FE", "GE", "HE", "KE", "LE",
		"BI", "CI", "DI", "FI", "GI", "HI", "KI", "LI",
		"BO", "CO", "DO", "FO", "GO", "HO", "KO", "LO",
		"MO", "NO", "PO", "RO", "SO", "TO", "WO", "ZO",
		"MI", "NI", "PI", "RI", "SI", "TI", "WI", "ZI",
		"ME", "NE", "PE", "RE", "SE", "TE", "WE", "ZE",
		"MA", "NA", "PA", "RA", "SA", "TA", "WA", "ZA",
	};

	if (square >= U_NUMBER_OF(Squares))
		return "--";

	return Squares[square];
}


char const*
mark::colorName(Color color)
{
	switch (color)
	{
		case Red:		return "red";
		case Orange:	return "orange";
		case Yellow:	return "yellow";
		case Green:		return "green";
		case Blue:		return "blue";
		case DarkBlue:	return "darkblue";
		case Purple:	return "purple";
		case White:		return "white";
		case Black:		return "black";
	}

	return "";	// satisfies the compiler;
}


char const*
mark::typeName(Type type)
{
	switch (type)
	{
		case Text:		break;
		case Full:		return "full";
		case Square:	return "square";
		case Arrow:		return "arrow";
		case Circle:	return "circle";
		case Disk:		return "disk";
	}

	return "";	// satisfies the compiler;
}


char const*
mark::commandName(Command cmd)
{
	switch (cmd)
	{
		case None:		return "";
		case Draw:		return "draw";
		case Diagram:	return "#";
	}

	return "";	// satisfies the compiler;
}


bool
mark::isTextChar(char c)
{
	return strchr("x+-=?!123456789", c);
}


mark::Type
mark::typeFromString(char const* type)
{
	//M_REQUIRE(type);

	switch (::tolower(*type))
	{
		case 'f': return Full;
		case 's': return Square;
		case 'a': return Arrow;
		case 'c': return Circle;
		case 'd': return Disk;
	}

	return Text;
}


mark::Color
mark::colorFromString(char const* color)
{
	//M_REQUIRE(color);

	switch (::tolower(*color))
	{
		case 'r': return Red;
		case 'o': return Orange;
		case 'g': return Green;
		case 'd': return DarkBlue;
		case 'p': return Purple;
		case 'w': return White;
		case 'y': return Yellow;

		case 'b':
			switch (::tolower(color[1]))
			{
				case 'a': return DarkBlue;

				case 'l':
					if (::tolower(color[2]) == 'u')
						return Blue;
					break;
			}
			break;
	}

	return Black;
}


tag::ID
rating::toWhiteTag(rating::Type type)
{
	static tag::ID Map[rating::Last] =
	{
		tag::WhiteElo,
		tag::WhiteRating,
		tag::WhiteRapid,
		tag::WhiteICCF,
		tag::WhiteUSCF,
		tag::WhiteDWZ,
		tag::WhiteECF,
		tag::WhiteIPS,
	};

	//M_REQUIRE(type < rating::Last);
	return Map[type];
}


tag::ID
rating::toBlackTag(rating::Type type)
{
	static tag::ID Map[rating::Last] =
	{
		tag::BlackElo,
		tag::BlackRating,
		tag::BlackRapid,
		tag::BlackICCF,
		tag::BlackUSCF,
		tag::BlackDWZ,
		tag::BlackECF,
		tag::BlackIPS,
	};

	//M_REQUIRE(type < rating::Last);
	return Map[type];
}


rating::Type
rating::fromString(char const* s)
{
	//M_REQUIRE(s);

	switch (::toupper(*s))
	{
		case 'U': return USCF;
		case 'D': return DWZ;
		case 'B': return ECF;	// BCF is the former name

		case 'E':
			switch (::toupper(s[1]))
			{
				case 'L': return Elo;
				case 'C': return ECF;
			}
			break;

		case 'R':
			if (s[1])
			{
				switch (::tolower(s[2]))
				{
					case 't': return Rating;
					case 'p': return Rapid;
				}
			}
			break;

		case 'I':
			switch (::toupper(s[1]))
			{
				case 'C': return ICCF;
				case 'P': return IPS;
			}
			break;
	}

	return rating::Any;
}


mstl::string const&
rating::toString(Type type)
{
	return Map[type];
}


unsigned
rating::convert(unsigned rating, Type from, Type to)
{
	switch (unsigned(from))
	{
		case Elo:
			switch (unsigned(to))
			{
				case Elo:	return rating;
				case USCF:	return convertEloToUscf(rating);
				case ECF:	return convertEloToEcf(rating);
			};
			break;

		case USCF:
			switch (unsigned(to))
			{
				case Elo:	return convertUscfToElo(rating);
				case USCF:	return rating;
				case ECF:	return convertEloToEcf(convertUscfToElo(rating));
			};
			break;

		case ECF:
			switch (unsigned(to))
			{
				case Elo:	return convertEcfToElo(rating);
				case USCF:	return convertEloToUscf(convertEcfToElo(rating));
				case ECF:	return rating;
			}
			break;
	}

	return 0;
}


mstl::string const&
result::toString(ID result)
{
	static mstl::string Lookup[] = { "*", "1-0", "0-1", "1/2-1/2", "0-0" };
	return Lookup[result];
}


result::ID
result::fromString(mstl::string const& s)
{
	if (s.size() >= 3)
	{
		switch (s[0])
		{
			case '*': return Unknown;

			case '1':
				switch (s[2])
				{
					case '0': return White;
					case '2': return Draw;
				}
				break;

			case '0':
				switch (s[2])
				{
					case '0': return Lost;
					case '1': return Black;
				}
				break;
		}
	}

	return Unknown;	// satisfies the compiler
}


termination::Reason
termination::fromString(mstl::string const& s)
{
	switch (::toupper(*s.c_str()))
	{
		case 'A':
			if (::strcasecmp(s, "Abandoned") == 0)
				return Abandoned;
			if (::strcasecmp(s, "Adjudication") == 0)
				return Adjudication;
			break;

		case 'B':
			if (::strcasecmp(s, "Black forfeits on time") == 0)
				return TimeForfeit;
			break;

		case 'D':
			if (::strcasecmp(s, "Death") == 0)
				return Death;
			break;

		case 'E':
			if (::strcasecmp(s, "Emergency") == 0)
				return Emergency;
			break;

		case 'N':
			if (::strcasecmp(s, "Normal") == 0)
				return Normal;
			break;

		case 'R':
			if (::strcasecmp(s, "RulesInfraction") == 0)
				return RulesInfraction;
			if (::strcasecmp(s, "Rules Infraction") == 0)
				return RulesInfraction;
			break;

		case 'T':
			if (::strcasecmp(s, "TimeForfeit") == 0)
				return TimeForfeit;
			if (::strcasecmp(s, "Time Forfeit") == 0)
				return TimeForfeit;
			break;

		case 'U':
			if (::strcasecmp(s, "Unplayed") == 0)
				return Unplayed;
			if (::strcasecmp(s, "Unterminated") == 0)
				return Unterminated;
			break;

		case 'W':
			if (::strcasecmp(s, "White forfeits on time") == 0)
				return TimeForfeit;
			break;
	}

	return Unknown;
}


mstl::string const&
termination::toString(Reason reason)
{
	//M_ASSERT(reason < int(U_NUMBER_OF(Lookup)));
	return Lookup[reason];
}


mstl::string const&
time::toString(Mode time)
{
	static_assert(U_NUMBER_OF(Lookup) == Corr + 1, "time::Lookup expired");
	//M_ASSERT(time < int(U_NUMBER_OF(Lookup)));

	return Lookup[time];
}


time::Mode
time::fromString(mstl::string const& s)
{
	static_assert(U_NUMBER_OF(Lookup) == Corr + 1, "time::Lookup expired");

	switch (::tolower(s.c_str()[0]))
	{
		case 'n':
			if (::isCaseEqual(s, Lookup[Normal]))
				return Normal;
			break;

		case 'r':
			if (::isCaseEqual(s, Lookup[Rapid]))
				return Rapid;
			break;

		case 'b':
			switch (::tolower(s.c_str()[1]))
			{
				case 'l':
					if (::isCaseEqual(s, Lookup[Blitz]))
						return Blitz;
					break;

				case 'u':
					if (::isCaseEqual(s, Lookup[Bullet]))
						return Bullet;
					break;
			}
			break;

		case 'c':
			if (::isCaseEqual(s, Lookup[Corr]))
				return Corr;
			break;
	}

	return Unknown;
}


mstl::string&
castling::print(Rights rights, mstl::string& result)
{
	if (rights & WhiteQueenside)	result += "Q";
	if (rights & WhiteKingside)	result += "K";
	if (rights & BlackQueenside)	result += "q";
	if (rights & BlackKingside)	result += "k";

	return result;
}


int
country::compare(Code lhs, Code rhs)
{
	if (lhs == rhs)
		return 0;
	if (lhs == country::Unknown)
		return +1;
	if (rhs == country::Unknown)
		return -1;

	return int(lhs) - int(rhs);
}


bool
country::match(Code lhs, Code rhs)
{
	if (lhs == rhs)
		return true;

	if (lhs == Unknown || lhs == Mixed_Team || rhs == Unknown || rhs == Mixed_Team)
		return true;

	switch (int(lhs))
	{
		// USA

		case United_States_of_America:
			switch (int(rhs))
			{
				case United_States_of_America:
				case American_Samoa:
				case Marshall_Islands:
				case Northern_Mariana_Islands:
				case US_Virgin_Islands:
				case United_States_Minor_Outlying_Islands:
					return true;
			}
			return false;

		case American_Samoa:
		case Marshall_Islands:
		case Northern_Mariana_Islands:
		case US_Virgin_Islands:
		case United_States_Minor_Outlying_Islands:
			return rhs == United_States_of_America;

		// Soviet Union

		case Soviet_Union:
			switch (int(rhs))
			{
				case Soviet_Union:
				case Armenia:
				case Azerbaijan:
				case Belarus:
				case Estonia:
				case Georgia:
				case Kazakhstan:
				case Kyrgyzstan:
				case Latvia:
				case Lithuania:
				case Russia:
				case Tajikistan:
				case Turkmenistan:
				case Ukraine:
				case Uzbekistan:
					return true;
			}
			break;

		case Armenia:
		case Azerbaijan:
		case Belarus:
		case Estonia:
		case Georgia:
		case Kazakhstan:
		case Kyrgyzstan:
		case Latvia:
		case Lithuania:
		case Russia:
		case Tajikistan:
		case Turkmenistan:
		case Ukraine:
		case Uzbekistan:
			return rhs == Soviet_Union;

		// Australasia

		case Australasia:
			switch (int(rhs))
			{
				case Australasia:
				case Australia:
				case New_Zealand:
				case Indonesia:
				case Papua_New_Guinea:
				case Heard_Island_and_McDonald_Islands:
					return true;
			}
			break;

		case New_Zealand:
		case Papua_New_Guinea:
			return rhs == Australasia;

		// Australia

		case Australia:
			switch (int(rhs))
			{
				case Australia:
				case Australasia:
				case Norfolk_Island:
				case Heard_Island_and_McDonald_Islands:
					return true;
			}
			break;

		case Norfolk_Island:
		case Heard_Island_and_McDonald_Islands:
			switch (int(rhs))
			{
				case Australia:
				case Australasia:
					return true;
			}
			break;

		// Indonesia

		case Indonesia:
		case Timor_Leste:
			switch (int(rhs))
			{
				case Indonesia:
				case Timor_Leste:
				case Australasia:
					return true;
			}
			break;

		// Spain

		case Spain:
			switch (int(rhs))
			{
				case Spain:
				case Basque:
				case Catalonia:
					return true;
			}
			return false;

		case Basque:
		case Catalonia:
			return rhs == Spain;

		// Yugoslavia

		case Yugoslavia:
			switch (int(rhs))
			{
				case Yugoslavia:
				case Bosnia_and_Herzegovina:
				case Croatia:
				case Slovenia:
				case Serbia:
				case Serbia_and_Montenegro:
				case Montenegro:
				case Kosovo:
					return true;
			}
			break;

		case Bosnia_and_Herzegovina:
		case Croatia:
		case Slovenia:
			return rhs == Yugoslavia;

		case Serbia:
		case Serbia_and_Montenegro:
			switch (int(rhs))
			{
				case Serbia:
				case Serbia_and_Montenegro:
				case Montenegro:
				case Kosovo:
				case Yugoslavia:
					return true;
			}
			break;

		case Montenegro:
			switch (int(rhs))
			{
				case Serbia:
				case Serbia_and_Montenegro:
				case Montenegro:
				case Yugoslavia:
					return true;
			}
			break;

		case Kosovo:
			switch (int(rhs))
			{
				case Serbia:
				case Serbia_and_Montenegro:
				case Kosovo:
				case Yugoslavia:
					return true;
			}
			break;

		// Czechoslovakia

		case Czechoslovakia:
			switch (int(rhs))
			{
				case Czechoslovakia:
				case Czech_Republic:
				case Slovakia:
					return true;
			}
			break;

		case Czech_Republic:
		case Slovakia:
			return rhs == Czechoslovakia;

		// Great Britain

		case Great_Britain:
			switch (int(rhs))
			{
				case Great_Britain:
				case England:
				case Scotland:
				case Wales:
				case Northern_Ireland:
				case Isle_of_Man:
				case Guernsey:
				case Jersey:
				case Falkland_Islands:
				case Gibraltar:
				case Montserrat:
				case Pitcairn_Islands:
				case Saint_Helena:
				case South_Georgia_and_South_Sandwich_Islands:
				case Turks_and_Caicos_Islands:
				case British_Indian_Ocean_Territory:
				case British_Virgin_Islands:
					return true;
			}
			break;

		case England:
		case Scotland:
		case Wales:
		case Northern_Ireland:
		case Isle_of_Man:
		case Guernsey:
		case Jersey:
		case Falkland_Islands:
		case Gibraltar:
		case Montserrat:
		case Pitcairn_Islands:
		case Saint_Helena:
		case South_Georgia_and_South_Sandwich_Islands:
		case Turks_and_Caicos_Islands:
		case British_Indian_Ocean_Territory:
		case British_Virgin_Islands:
			return rhs == Great_Britain;

		// China

		case China:
			switch (int(rhs))
			{
				case China:
				case Hong_Kong:
				case Macao:
				case Tibet:
					return true;
			}
			return false;

		case Hong_Kong:
		case Macao:
		case Tibet:
			return rhs == China;

		// Germany

		case Germany:
			switch (int(rhs))
			{
				case Germany:
				case East_Germany:
				case West_Germany:
					return true;
			}
			return false;

		case East_Germany:
		case West_Germany:
			return rhs == Germany;

		// Finland

		case Finland:
			return rhs == Aaland_Islands;

		case Aaland_Islands:
			return rhs == Finland;

		// Denmark

		case Denmark:
			switch (int(rhs))
			{
				case Denmark:
				case Greenland:
				case Faroe_Islands:
					return true;
			}
			break;

		case Greenland:
		case Faroe_Islands:
			return rhs == Denmark;

		// Norway

		case Norway:
			return rhs == Jan_Mayen_and_Svalbard;

		case Jan_Mayen_and_Svalbard:
			return rhs == Norway;

		// France

		case France:
			switch (int(rhs))
			{
				case France:
				case Martinique:
				case New_Caledonia:
				case Saint_Pierre_and_Miquelon:
				case Wallis_and_Futuna:
				case French_Southern_Territories:
					return true;
			}
			break;

		case Martinique:
		case New_Caledonia:
		case Saint_Pierre_and_Miquelon:
		case Wallis_and_Futuna:
		case French_Southern_Territories:
			return rhs == France;

		// Netherlands

		case Netherlands:
			return rhs == Netherlands_Antilles;

		case Netherlands_Antilles:
			return rhs == Netherlands;

		// Palestine

		case Palestine:
			switch (int(rhs))
			{
				case Palestine:
				case Jordan:
				case Lebanon:
				case Syria:
				case Israel:
					return true;
			}
			break;

		case Jordan:
		case Lebanon:
		case Syria:
		case Israel:
			return rhs == Palestine;

		// Morocco

		case Morocco:
			return rhs == Western_Sahara;

		case Western_Sahara:
			return rhs == Morocco;

		// Tanzania

		case Tanzania:
			return rhs == Zanzibar;

		case Zanzibar:
			return rhs == Tanzania;
	}

	return false;
}


char const*
country::toString(Code code)
{
	// The PGN standard is using the IOC country codes and his own country code set.
	// The former codes has changed many times, and not all countries do have an
	// assigned code.
	//
	// For our standard (the PGN standard for country codes is unsufficient) we use
	// the following scheme:
	//		1. use the current (2009) IOC country code (if exisiting)
	//		2. use the PGN country code (if exisiting)
	// 	3. use the country code from ratings_utf8_2010_03_27.ssp (if exisiting)
	//		4. use the ISO-3166-3 code (as far no clash will occurr)
	//		5. use the FIPS code (as far no clash will occurr)
	//		6. use a self-defined code

	switch (code)
	{
		case Unknown:												return "";

		case Aaland_Islands:										return "ALA";
		case Afghanistan:											return "AFG";
		case Albania:												return "ALB";
		case Algeria:												return "ALG";
		case American_Samoa:										return "ASA";
		case Andorra:												return "AND";
		case Angola:												return "ANG";
		case Anguilla:												return "AIA";
		case Antarctica:											return "ATA";
		case Antigua:												return "ANT";
		case Argentina:											return "ARG";
		case Armenia:												return "ARM";
		case Aruba:													return "ARU";
		case Australasia:											return "ANZ";
		case Australia:											return "AUS";
		case Austria:												return "AUT";
		case Azerbaijan:											return "AZE";
		case Bahamas:												return "BAH";
		case Bahrain:												return "BRN";
		case Bangladesh:											return "BAN";
		case Barbados:												return "BAR";
		case Basque:												return "BAS";
		case Belarus:												return "BLR";
		case Belgium:												return "BEL";
		case Belize:												return "BIZ";
		case Benin:													return "BEN";
		case Bermuda:												return "BER";
		case Bhutan:												return "BHU";
		case Bolivia:												return "BOL";
		case Bosnia_and_Herzegovina:							return "BIH";
		case Botswana:												return "BOT";
		case Bouvet_Islands:										return "BVT";
		case Brazil:												return "BRA";
		case British_Indian_Ocean_Territory:				return "IOT";
		case British_Virgin_Islands:							return "IVB";
		case Brunei:												return "BRU";
		case Bulgaria:												return "BUL";
		case Burkina_Faso:										return "BUR";
		case Burundi:												return "BDI";
		case Cambodia:												return "CAM";
		case Cameroon:												return "CMR";
		case Canada:												return "CAN";
		case Cape_Verde:											return "CPV";
		case Catalonia:											return "CAT";
		case Cayman_Islands:										return "CAY";
		case Central_African_Republic:						return "CAF";
		case Chad:													return "CHA";
		case Chile:													return "CHI";
		case China:													return "CHN";
		case Chinese_Taipei:										return "TPE";
		case Christmas_Island:									return "CXR";
		case Cocos_Islands:										return "CCK";
		case Colombia:												return "COL";
		case Comoros:												return "COM";
		case Congo:													return "CGO";
		case Cook_Islands:										return "COK";
		case Costa_Rica:											return "CRC";
		case Croatia:												return "CRO";
		case Cuba:													return "CUB";
		case Cyprus:												return "CYP";
		case Czech_Republic:										return "CZE";
		case Czechoslovakia:										return "TCH";
		case DR_Congo:												return "COD";
		case Denmark:												return "DEN";
		case Djibouti:												return "DJI";
		case Dominica:												return "DMA";
		case Dominican_Republic:								return "DOM";
		case Ecuador:												return "ECU";
		case East_Germany:										return "GDR";
		case Egypt:													return "EGY";
		case El_Salvador:											return "ESA";
		case England:												return "ENG";
		case Equatorial_Guinea:									return "GEQ";
		case Eritrea:												return "ERI";
		case Estonia:												return "EST";
		case Ethiopia:												return "ETH";
		case Falkland_Islands:									return "FLK";
		case Faroe_Islands:										return "FAI";
		case Fiji:													return "FIJ";
		case Finland:												return "FIN";
		case France:												return "FRA";
		case French_Guiana:										return "GUF";
		case French_Polynesia:									return "PYF";
		case French_Southern_Territories:					return "ATF";
		case Gabon:													return "GAB";
		case Gambia:												return "GAM";
		case Georgia:												return "GEO";
		case Germany:												return "GER";
		case Ghana:													return "GHA";
		case Gibraltar:											return "GBZ";
		case Great_Britain:										return "GBR";
		case Greece:												return "GRE";
		case Greenland:											return "GRL";
		case Grenada:												return "GRN";
		case Guadeloupe:											return "GLP";
		case Guam:													return "GUM";
		case Guatemala:											return "GUA";
		case Guernsey:												return "GGY";
		case Guinea:												return "GUI";
		case Guinea_Bissau:										return "GBS";
		case Guyana:												return "GUY";
		case Haiti:													return "HAI";
		case Heard_Island_and_McDonald_Islands:			return "HMD";
		case Honduras:												return "HON";
		case Hong_Kong:											return "HKG";
		case Hungary:												return "HUN";
		case Iceland:												return "ISL";
		case India:													return "IND";
		case Indonesia:											return "INA";
		case Iran:													return "IRI";
		case Iraq:													return "IRQ";
		case Ireland:												return "IRL";
		case Isle_of_Man:											return "IMN";
		case Israel:												return "ISR";
		case Italy:													return "ITA";
		case Ivory_Coast:											return "CIV";
		case Jamaica:												return "JAM";
		case Jan_Mayen_and_Svalbard:							return "SJM";
		case Japan:													return "JPN";
		case Jersey:												return "JEY";
		case Jordan:												return "JOR";
		case Kazakhstan:											return "KAZ";
		case Kenya:													return "KEN";
		case Kiribati:												return "KIR";
		case Kosovo:												return "KOS";
		case Kuwait:												return "KUW";
		case Kyrgyzstan:											return "KGZ";
		case Laos:													return "LAO";
		case Latvia:												return "LAT";
		case Lebanon:												return "LIB";
		case Lesotho:												return "LES";
		case Liberia:												return "LBR";
		case Libya:													return "LBA";
		case Liechtenstein:										return "LIE";
		case Lithuania:											return "LTU";
		case Luxembourg:											return "LUX";
		case Macao:													return "MAC";
		case Macedonia:											return "MKD";
		case Madagascar:											return "MAD";
		case Malawi:												return "MAW";
		case Malaysia:												return "MAS";
		case Maldives:												return "MDV";
		case Mali:													return "MLI";
		case Malta:													return "MLT";
		case Marshall_Islands:									return "MHL";
		case Martinique:											return "MTQ";
		case Mauritania:											return "MTN";
		case Mauritius:											return "MRI";
		case Mayotte:												return "MYT";
		case Mexico:												return "MEX";
		case Micronesia:											return "FSM";
		case Moldova:												return "MDA";
		case Monaco:												return "MNC";
		case Mongolia:												return "MGL";
		case Montenegro:											return "MNE";
		case Montserrat:											return "MSR";
		case Morocco:												return "MAR";
		case Mozambique:											return "MOZ";
		case Myanmar:												return "MYA";
		case Namibia:												return "NAM";
		case Nauru:													return "NRU";
		case Nepal:													return "NEP";
		case Netherlands:											return "NED";
		case Netherlands_Antilles:								return "AHO";
		case New_Caledonia:										return "NCL";
		case New_Zealand:											return "NZL";
		case Nicaragua:											return "NCA";
		case Niger:													return "NIG";
		case Nigeria:												return "NGR";
		case Niue:													return "NIU";
		case Norfolk_Island:										return "NFK";
		case North_Korea:											return "PRK";
		case Northern_Ireland:									return "NIR";
		case Northern_Mariana_Islands:						return "MNP";
		case Norway:												return "NOR";
		case Oman:													return "OMN";
		case Pakistan:												return "PAK";
		case Palau:													return "PLW";
		case Palestine:											return "PLE";
		case Panama:												return "PAN";
		case Papua_New_Guinea:									return "PNG";
		case Paraguay:												return "PAR";
		case Peru:													return "PER";
		case Philippines:											return "PHI";
		case Pitcairn_Islands:									return "PCN";
		case Poland:												return "POL";
		case Portugal:												return "POR";
		case Puerto_Rico:											return "PUR";
		case Qatar:													return "QAT";
		case Reunion:												return "REU";
		case Romania:												return "ROU";
		case Russia:												return "RUS";
		case Rwanda:												return "RWA";
		case Saint_Helena:										return "SHN";
		case Saint_Kitts_and_Nevis:							return "SKN";
		case Saint_Lucia:											return "LCA";
		case Saint_Vincent_and_the_Grenadines:				return "VIN";
		case Saint_Pierre_and_Miquelon:						return "SPM";
		case Samoa:													return "SAM";
		case San_Marino:											return "SMR";
		case Sao_Tome_and_Principe:							return "STP";
		case Saudi_Arabia:										return "KSA";
		case Scotland:												return "SCO";
		case Senegal:												return "SEN";
		case Serbia:												return "SRB";
		case Serbia_and_Montenegro:							return "SCG";
		case Seychelles:											return "SEY";
		case Sierra_Leone:										return "SLE";
		case Singapore:											return "SIN";
		case Slovakia:												return "SVK";
		case Slovenia:												return "SLO";
		case Solomon_Islands:									return "SOL";
		case Somalia:												return "SOM";
		case South_Africa:										return "RSA";
		case South_Georgia_and_South_Sandwich_Islands:	return "SGS";
		case South_Korea:											return "KOR";
		case Soviet_Union:										return "URS";
		case Spain:													return "ESP";
		case Sri_Lanka:											return "SRI";
		case Sudan:													return "SUD";
		case Suriname:												return "SUR";
		case Swaziland:											return "SWZ";
		case Sweden:												return "SWE";
		case Switzerland:											return "SUI";
		case Syria:													return "SYR";
		case Tajikistan:											return "TJK";
		case Tanzania:												return "TAN";
		case Thailand:												return "THA";
		case Tibet:													return "TIB";
		case Timor_Leste:											return "TLS";
		case Tokelau:												return "TKL";
		case Togo:													return "TOG";
		case Tonga:													return "TGA";
		case Trinidad_and_Tobago:								return "TRI";
		case Tunisia:												return "TUN";
		case Turkey:												return "TUR";
		case Turkmenistan:										return "TKM";
		case Turks_and_Caicos_Islands:						return "TCA";
		case Tuvalu:												return "TUV";
		case US_Virgin_Islands:									return "VUS";
		case Uganda:												return "UGA";
		case Ukraine:												return "UKR";
		case United_Arab_Emirates:								return "UAE";
		case United_States_Minor_Outlying_Islands:		return "UMI";
		case United_States_of_America:						return "USA";
		case Uruguay:												return "URU";
		case Uzbekistan:											return "UZB";
		case Vanuatu:												return "VAN";
		case Vatican:												return "VAT";
		case Venezuela:											return "VEN";
		case Vietnam:												return "VIE";
		case Wales:													return "WLS";
		case Wallis_and_Futuna:									return "WLF";
		case West_Germany:										return "FRG";
		case Western_Sahara:										return "ESH";
		case Yemen:													return "YEM";
		case Yugoslavia:											return "YUG";
		case Zambia:												return "ZAM";
		case Zanzibar:												return "EAZ";
		case Zimbabwe:												return "ZIM";

		case Aboard_Aircraft:									return "AIR";
		case Aboard_Spacecraft:									return "SPC";
		case At_Sea:												return "SEA";
		case The_Internet:										return "NET";
		case Mixed_Team:											return "ZZX";
	}

	return "";	// satisfies the compiler
}


char const*
country::toChessBaseCode(country::Code code)
{
	switch (code)
	{
		case Aaland_Islands:										return "FIN";
		case Afghanistan:											return "AFG";
		case Albania:												return "ALB";
		case Algeria:												return "ALG";
		case American_Samoa:										return "NET";
		case Andorra:												return "AND";
		case Angola:												return "ANG";
		case Anguilla:												return "ASU";
		case Antigua:												return "ANT";
		case Argentina:											return "ARG";
		case Armenia:												return "ARM";
		case Aruba:													return "AGG";
		case Australia:											return "AUS";
		case Austria:												return "AUT";
		case Azerbaijan:											return "AZE";
		case Bahamas:												return "BAH";
		case Bahrain:												return "BHN";
		case Bangladesh:											return "BAN";
		case Barbados:												return "BAR";
		case Basque:												return "BAS";
		case Belarus:												return "BLR";
		case Belgium:												return "BEL";
		case Belize:												return "BLZ";
		case Benin:													return "BNN";
		case Bermuda:												return "BER";
		case Bhutan:												return "BTN";
		case Bolivia:												return "BOL";
		case Bosnia_and_Herzegovina:							return "BIH";
		case Botswana:												return "BOT";
		case Brazil:												return "BRA";
		case British_Indian_Ocean_Territory:				return "GBR";
		case British_Virgin_Islands:							return "IVB";
		case Brunei:												return "BRU";
		case Bulgaria:												return "BUL";
		case Burkina_Faso:										return "BUR";
		case Burundi:												return "BRI";
		case Cambodia:												return "CMB";
		case Cameroon:												return "CAM";
		case Canada:												return "CAN";
		case Cape_Verde:											return "KAP";
		case Catalonia:											return "CAT";
		case Cayman_Islands:										return "CAY";
		case Central_African_Republic:						return "CAR";
		case Chad:													return "CHD";
		case Chile:													return "CHI";
		case China:													return "CHN";
		case Chinese_Taipei:										return "TWN";
		case Christmas_Island:									return "CIA";
		case Cocos_Islands:										return "COA";
		case Colombia:												return "COL";
		case Comoros:												return "COM";
		case Congo:													return "CON";
		case Cook_Islands:										return "CIN";
		case Costa_Rica:											return "CRI";
		case Croatia:												return "CRO";
		case Cuba:													return "CUB";
		case Cyprus:												return "CYP";
		case Czech_Republic:										return "CZE";
		case Czechoslovakia:										return "CSR";
		case DR_Congo:												return "ZRE";
		case Denmark:												return "DEN";
		case Djibouti:												return "DJI";
		case Dominican_Republic:								return "DOM";
		case East_Germany:										return "DDR";
		case Ecuador:												return "ECU";
		case Egypt:													return "EGY";
		case El_Salvador:											return "ESA";
		case England:												return "ENG";
		case Equatorial_Guinea:									return "ELG";
		case Eritrea:												return "ERI";
		case Estonia:												return "EST";
		case Ethiopia:												return "ETH";
		case Falkland_Islands:									return "FGB";
		case Faroe_Islands:										return "FAI";
		case Fiji:													return "FIJ";
		case Finland:												return "FIN";
		case France:												return "FRA";
		case French_Guiana:										return "FRG";
		case French_Polynesia:									return "FRP";
		case French_Southern_Territories:					return "FRA";
		case Gabon:													return "GAB";
		case Gambia:												return "GAM";
		case Georgia:												return "GEO";
		case Germany:												return "GER";
		case Ghana:													return "GHA";
		case Gibraltar:											return "GGB";
		case Great_Britain:										return "GBR";
		case Greece:												return "GRE";
		case Greenland:											return "GRL";
		case Grenada:												return "GRA";
		case Guadeloupe:											return "FGA";
		case Guam:													return "GMA";
		case Guatemala:											return "GUA";
		case Guernsey:												return "GCI";
		case Guinea:												return "GUI";
		case Guinea_Bissau:										return "GUB";
		case Guyana:												return "GUY";
		case Haiti:													return "HAI";
		case Heard_Island_and_McDonald_Islands:			return "AUS";
		case Honduras:												return "HON";
		case Hong_Kong:											return "HKG";
		case Hungary:												return "HUN";
		case Iceland:												return "ISL";
		case India:													return "IND";
		case Indonesia:											return "INA";
		case Iran:													return "IRI";
		case Iraq:													return "IRQ";
		case Ireland:												return "IRL";
		case Isle_of_Man:											return "IOM";
		case Israel:												return "ISR";
		case Italy:													return "ITA";
		case Ivory_Coast:											return "IVO";
		case Jamaica:												return "JAM";
		case Jan_Mayen_and_Svalbard:							return "JMY";
		case Japan:													return "JPN";
		case Jersey:												return "JCI";
		case Jordan:												return "JOR";
		case Kazakhstan:											return "KAZ";
		case Kenya:													return "KEN";
		case Kiribati:												return "KBA";
		case Kosovo:												return "KOS";
		case Kuwait:												return "KUW";
		case Kyrgyzstan:											return "KGZ";
		case Laos:													return "LAO";
		case Latvia:												return "LAT";
		case Lebanon:												return "LBN";
		case Lesotho:												return "LES";
		case Liberia:												return "LBR";
		case Libya:													return "LBY";
		case Liechtenstein:										return "LIE";
		case Lithuania:											return "LTU";
		case Luxembourg:											return "LUX";
		case Macao:													return "MAC";
		case Macedonia:											return "FRM";
		case Madagascar:											return "MAD";
		case Malawi:												return "MWI";
		case Malaysia:												return "MAS";
		case Maldives:												return "MDV";
		case Mali:													return "MLI";
		case Malta:													return "MLT";
		case Marshall_Islands:									return "MSH";
		case Martinique:											return "MFR";
		case Mauritania:											return "MAU";
		case Mauritius:											return "MRI";
		case Mayotte:												return "MYF";
		case Mexico:												return "MEX";
		case Micronesia:											return "MIC";
		case Moldova:												return "MDA";
		case Monaco:												return "MNC";
		case Mongolia:												return "MGL";
		case Montenegro:											return "MNT";
		case Montserrat:											return "MSG";
		case Morocco:												return "MAR";
		case Mozambique:											return "MOZ";
		case Myanmar:												return "MYA";
		case Namibia:												return "NAM";
		case Nauru:													return "NAU";
		case Nepal:													return "NEP";
		case Netherlands:											return "NED";
		case Netherlands_Antilles:								return "AHO";
		case New_Caledonia:										return "NCF";
		case New_Zealand:											return "NZL";
		case Nicaragua:											return "NCA";
		case Niger:													return "NIG";
		case Nigeria:												return "NGR";
		case Niue:													return "NNN";
		case Norfolk_Island:										return "NNA";
		case North_Korea:											return "NKO";
		case Northern_Ireland:									return "NIR";
		case Northern_Mariana_Islands:						return "NMI";
		case Norway:												return "NOR";
		case Oman:													return "OMN";
		case Pakistan:												return "PAK";
		case Palau:													return "PAL";
		case Palestine:											return "PLE";
		case Panama:												return "PAN";
		case Papua_New_Guinea:									return "PNG";
		case Paraguay:												return "PAR";
		case Peru:													return "PER";
		case Philippines:											return "PHI";
		case Pitcairn_Islands:									return "PIG";
		case Poland:												return "POL";
		case Portugal:												return "POR";
		case Puerto_Rico:											return "PUR";
		case Qatar:													return "QAT";
		case Reunion:												return "RUF";
		case Romania:												return "ROM";
		case Russia:												return "RUS";
		case Rwanda:												return "RWA";
		case Saint_Helena:										return "HGB";
		case Saint_Kitts_and_Nevis:							return "SKI";
		case Saint_Lucia:											return "SLU";
		case Saint_Pierre_and_Miquelon:						return "PGB";
		case Saint_Vincent_and_the_Grenadines:				return "SVI";
		case Samoa:													return "SAM";
		case San_Marino:											return "SMR";
		case Sao_Tome_and_Principe:							return "SAO";
		case Saudi_Arabia:										return "SAU";
		case Scotland:												return "SCO";
		case Senegal:												return "SEN";
		case Serbia:												return "SER";
		case Seychelles:											return "SEY";
		case Sierra_Leone:										return "SIE";
		case Singapore:											return "SIN";
		case Slovakia:												return "SVK";
		case Slovenia:												return "SLO";
		case Solomon_Islands:									return "SAL";
		case Somalia:												return "SOM";
		case South_Africa:										return "RSA";
		case South_Georgia_and_South_Sandwich_Islands:	return "GBR";
		case South_Korea:											return "KOR";
		case Soviet_Union:										return "URS";
		case Spain:													return "ESP";
		case Sri_Lanka:											return "SRI";
		case Sudan:													return "SUD";
		case Suriname:												return "SUR";
		case Swaziland:											return "SWA";
		case Sweden:												return "SWE";
		case Switzerland:											return "SUI";
		case Syria:													return "SYR";
		case Tajikistan:											return "TJK";
		case Tanzania:												return "TAN";
		case Thailand:												return "THA";
		case Tibet:													return "CHN";
		case Timor_Leste:											return "OTM";
		case Togo:													return "TOG";
		case Tokelau:												return "TKI";
		case Tonga:													return "TON";
		case Trinidad_and_Tobago:								return "TRI";
		case Tunisia:												return "TUN";
		case Turkey:												return "TUR";
		case Turkmenistan:										return "TKM";
		case Turks_and_Caicos_Islands:						return "TCI";
		case Tuvalu:												return "TUV";
		case US_Virgin_Islands:									return "ISV";
		case Uganda:												return "UGA";
		case Ukraine:												return "UKR";
		case United_Arab_Emirates:								return "UAE";
		case United_States_Minor_Outlying_Islands:		return "USA";
		case United_States_of_America:						return "USA";
		case Uruguay:												return "URU";
		case Uzbekistan:											return "UZB";
		case Vanuatu:												return "VAN";
		case Vatican:												return "VAT";
		case Venezuela:											return "VEN";
		case Vietnam:												return "VIE";
		case Wales:													return "WLS";
		case Wallis_and_Futuna:									return "WFR";
		case West_Germany:										return "GER";
		case Western_Sahara:										return "MAR";
		case Yemen:													return "YEM";
		case Yugoslavia:											return "YUG";
		case Zambia:												return "ZAM";
		case Zanzibar:												return "TAN";
		case Zimbabwe:												return "ZIM";

		case Aboard_Spacecraft:									return "ISS";

		// cannot be mapped:
		case Antarctica:
		case Australasia:
		case Bouvet_Islands:
		case Dominica:
		case Serbia_and_Montenegro:
		case Aboard_Aircraft:
		case At_Sea:
		case The_Internet:
		case Mixed_Team:
		case Unknown:
			break;
	}

	return "";
}


unsigned
country::toRegion(Code code)
{
	switch (code)
	{
		// Region 0 -----------------------------------------------
		case Aboard_Aircraft:									// fallthru
		case Aboard_Spacecraft:									// fallthru
		case At_Sea:												// fallthru
		case Mixed_Team:											// fallthru
		case The_Internet:										// fallthru
		case Unknown:												return 0;

		// Region 1 -----------------------------------------------
		case Aaland_Islands:										// fallthru
		case American_Samoa:										// fallthru
		case Andorra:												// fallthru
		case Anguilla:												// fallthru
		case Antarctica:											// fallthru
		case Antigua:												// fallthru
		case Argentina:											// fallthru
		case Aruba:													// fallthru
		case Australasia:											// fallthru
		case Australia:											// fallthru
		case Austria:												// fallthru
		case Bahamas:												// fallthru
		case Barbados:												// fallthru
		case Basque:												// fallthru
		case Belgium:												// fallthru
		case Belize:												// fallthru
		case Bermuda:												// fallthru
		case Bolivia:												// fallthru
		case Bouvet_Islands:										// fallthru
		case Brazil:												// fallthru
		case British_Indian_Ocean_Territory:				// fallthru
		case British_Virgin_Islands:							// fallthru
		case Canada:												// fallthru
		case Cape_Verde:											// fallthru
		case Catalonia:											// fallthru
		case Cayman_Islands:										// fallthru
		case Chile:													// fallthru
		case Christmas_Island:									// fallthru
		case Cocos_Islands:										// fallthru
		case Colombia:												// fallthru
		case Comoros:												// fallthru
		case Cook_Islands:										// fallthru
		case Costa_Rica:											// fallthru
		case Cuba:													// fallthru
		case Denmark:												// fallthru
		case Dominica:												// fallthru
		case Dominican_Republic:								// fallthru
		case East_Germany:										// fallthru
		case Ecuador:												// fallthru
		case El_Salvador:											// fallthru
		case England:												// fallthru
		case Falkland_Islands:									// fallthru
		case Faroe_Islands:										// fallthru
		case Fiji:													// fallthru
		case Finland:												// fallthru
		case France:												// fallthru
		case French_Guiana:										// fallthru
		case French_Polynesia:									// fallthru
		case French_Southern_Territories:					// fallthru
		case Germany:												// fallthru
		case Gibraltar:											// fallthru
		case Great_Britain:										// fallthru
		case Greenland:											// fallthru
		case Grenada:												// fallthru
		case Guadeloupe:											// fallthru
		case Guam:													// fallthru
		case Guatemala:											// fallthru
		case Guernsey:												// fallthru
		case Guyana:												// fallthru
		case Haiti:													// fallthru
		case Heard_Island_and_McDonald_Islands:			// fallthru
		case Honduras:												// fallthru
		case Iceland:												// fallthru
		case Ireland:												// fallthru
		case Isle_of_Man:											// fallthru
		case Italy:													// fallthru
		case Jamaica:												// fallthru
		case Jan_Mayen_and_Svalbard:							// fallthru
		case Jersey:												// fallthru
		case Liechtenstein:										// fallthru
		case Luxembourg:											// fallthru
		case Macao:													// fallthru
		case Martinique:											// fallthru
		case Mauritius:											// fallthru
		case Mexico:												// fallthru
		case Monaco:												// fallthru
		case Montserrat:											// fallthru
		case Netherlands:											// fallthru
		case Netherlands_Antilles:								// fallthru
		case New_Caledonia:										// fallthru
		case New_Zealand:											// fallthru
		case Nicaragua:											// fallthru
		case Niue:													// fallthru
		case Norfolk_Island:										// fallthru
		case Northern_Ireland:									// fallthru
		case Northern_Mariana_Islands:						// fallthru
		case Norway:												// fallthru
		case Panama:												// fallthru
		case Paraguay:												// fallthru
		case Peru:													// fallthru
		case Pitcairn_Islands:									// fallthru
		case Portugal:												// fallthru
		case Puerto_Rico:											// fallthru
		case Reunion:												// fallthru
		case Saint_Helena:										// fallthru
		case Saint_Kitts_and_Nevis:							// fallthru
		case Saint_Lucia:											// fallthru
		case Saint_Pierre_and_Miquelon:						// fallthru
		case Saint_Vincent_and_the_Grenadines:				// fallthru
		case San_Marino:											// fallthru
		case Sao_Tome_and_Principe:							// fallthru
		case Scotland:												// fallthru
		case Seychelles:											// fallthru
		case South_Georgia_and_South_Sandwich_Islands:	// fallthru
		case Spain:													// fallthru
		case Suriname:												// fallthru
		case Sweden:												// fallthru
		case Switzerland:											// fallthru
		case Tokelau:												// fallthru
		case Trinidad_and_Tobago:								// fallthru
		case Turks_and_Caicos_Islands:						// fallthru
		case US_Virgin_Islands:									// fallthru
		case United_States_Minor_Outlying_Islands:		// fallthru
		case United_States_of_America:						// fallthru
		case Uruguay:												// fallthru
		case Vatican:												// fallthru
		case Venezuela:											// fallthru
		case Wales:													// fallthru
		case Wallis_and_Futuna:									// fallthru
		case West_Germany:										return 1;

		// Region 2 -----------------------------------------------
		case Albania:												// fallthru
		case Armenia:												// fallthru
		case Belarus:												// fallthru
		case Bosnia_and_Herzegovina:							// fallthru
		case Bulgaria:												// fallthru
		case Croatia:												// fallthru
		case Cyprus:												// fallthru
		case Czech_Republic:										// fallthru
		case Czechoslovakia:										// fallthru
		case Estonia:												// fallthru
		case Georgia:												// fallthru
		case Greece:												// fallthru
		case Hungary:												// fallthru
		case Kazakhstan:											// fallthru
		case Kosovo:												// fallthru
		case Kyrgyzstan:											// fallthru
		case Latvia:												// fallthru
		case Lithuania:											// fallthru
		case Macedonia:											// fallthru
		case Moldova:												// fallthru
		case Montenegro:											// fallthru
		case Poland:												// fallthru
		case Romania:												// fallthru
		case Serbia:												// fallthru
		case Serbia_and_Montenegro:							// fallthru
		case Slovakia:												// fallthru
		case Slovenia:												// fallthru
		case Tajikistan:											// fallthru
		case Turkmenistan:										// fallthru
		case Ukraine:												// fallthru
		case Uzbekistan:											// fallthru
		case Yugoslavia:											return 2;

		// Region 3 -----------------------------------------------
		case Algeria:												// fallthru
		case Angola:												// fallthru
		case Bahrain:												// fallthru
		case Benin:													// fallthru
		case Botswana:												// fallthru
		case Brunei:												// fallthru
		case Burkina_Faso:										// fallthru
		case Burundi:												// fallthru
		case Cameroon:												// fallthru
		case Central_African_Republic:						// fallthru
		case Chad:													// fallthru
		case Congo:													// fallthru
		case DR_Congo:												// fallthru
		case Djibouti:												// fallthru
		case Egypt:													// fallthru
		case Equatorial_Guinea:									// fallthru
		case Eritrea:												// fallthru
		case Ethiopia:												// fallthru
		case Gabon:													// fallthru
		case Gambia:												// fallthru
		case Ghana:													// fallthru
		case Guinea:												// fallthru
		case Guinea_Bissau:										// fallthru
		case Iraq:													// fallthru
		case Israel:												// fallthru
		case Ivory_Coast:											// fallthru
		case Jordan:												// fallthru
		case Kenya:													// fallthru
		case Kuwait:												// fallthru
		case Lebanon:												// fallthru
		case Lesotho:												// fallthru
		case Liberia:												// fallthru
		case Libya:													// fallthru
		case Madagascar:											// fallthru
		case Malawi:												// fallthru
		case Mali:													// fallthru
		case Malta:													// fallthru
		case Mauritania:											// fallthru
		case Mayotte:												// fallthru
		case Morocco:												// fallthru
		case Mozambique:											// fallthru
		case Namibia:												// fallthru
		case Niger:													// fallthru
		case Nigeria:												// fallthru
		case Oman:													// fallthru
		case Palestine:											// fallthru
		case Qatar:													// fallthru
		case Rwanda:												// fallthru
		case Saudi_Arabia:										// fallthru
		case Senegal:												// fallthru
		case Sierra_Leone:										// fallthru
		case Somalia:												// fallthru
		case South_Africa:										// fallthru
		case Sudan:													// fallthru
		case Swaziland:											// fallthru
		case Syria:													// fallthru
		case Tanzania:												// fallthru
		case Togo:													// fallthru
		case Tunisia:												// fallthru
		case Uganda:												// fallthru
		case United_Arab_Emirates:								// fallthru
		case Western_Sahara:										// fallthru
		case Yemen:													// fallthru
		case Zambia:												// fallthru
		case Zanzibar:												// fallthru
		case Zimbabwe:												return 3;

		// Region 4 -----------------------------------------------
		case Afghanistan:											// fallthru
		case Azerbaijan:											// fallthru
		case Iran:													// fallthru
		case Russia:												// fallthru
		case Soviet_Union:										// fallthru
		case Turkey:												return 4;

		// Region 5 -----------------------------------------------
		case Bangladesh:											// fallthru
		case Bhutan:												// fallthru
		case Cambodia:												// fallthru
		case China:													// fallthru
		case Chinese_Taipei:										// fallthru
		case Hong_Kong:											// fallthru
		case India:													// fallthru
		case Indonesia:											// fallthru
		case Japan:													// fallthru
		case Kiribati:												// fallthru
		case Laos:													// fallthru
		case Malaysia:												// fallthru
		case Maldives:												// fallthru
		case Marshall_Islands:									// fallthru
		case Micronesia:											// fallthru
		case Mongolia:												// fallthru
		case Myanmar:												// fallthru
		case Nauru:													// fallthru
		case Nepal:													// fallthru
		case North_Korea:											// fallthru
		case Pakistan:												// fallthru
		case Palau:													// fallthru
		case Papua_New_Guinea:									// fallthru
		case Philippines:											// fallthru
		case Samoa:													// fallthru
		case Singapore:											// fallthru
		case Solomon_Islands:									// fallthru
		case South_Korea:											// fallthru
		case Sri_Lanka:											// fallthru
		case Thailand:												// fallthru
		case Tibet:													// fallthru
		case Timor_Leste:											// fallthru
		case Tonga:													// fallthru
		case Tuvalu:												// fallthru
		case Vanuatu:												return 5;

		// Region 6 -----------------------------------------------
		case Vietnam:												return 6;
	}

	return 0;	// satisfies the compiler
}


country::Code
country::fromString(char const* country)
{
	//M_REQUIRE(::strlen(country) >= 3);

	void const* p = ::bsearch(	country,
										NameMap,
										U_NUMBER_OF(NameMap),
										sizeof(NameMap[0]),
										compareNames);

	if (p && ::strncmp(static_cast<Pair const*>(p)->name, country, 3) == 0)
		return static_cast<Pair const*>(p)->code;

	return Unknown;
}


country::Code
country::fromString(mstl::string const& country)
{
	if (country.size() < 3)
		return Unknown;

	return fromString(country.c_str());
}


unsigned
chess960::twin(unsigned idn)
{
	//M_REQUIRE(idn <= 960);
	return chess960::Twins[idn];
}


mstl::string const&
chess960::position(unsigned idn)
{
	//M_REQUIRE(idn <= 960);
	return chess960::PositionTable[idn];
}


mstl::string&
chess960::utf8::position(unsigned idn, mstl::string& result)
{
	mstl::string const& pos = chess960::position(idn);

	for (unsigned i = 0; i < pos.size(); ++i)
		result += piece::utf8::asString(piece::fromLetter(pos[i]));

	return result;
}


mstl::string const&
chess960::identifier()
{
	// this seems to be the most common identifier for chess 960
	static mstl::string const Identifier("chess 960");
	return Identifier;
}


mstl::string
chess960::fen(unsigned idn)
{
	//M_REQUIRE(idn <= 960);

	static mstl::string const Pattern("xxxxxxxx/pppppppp/8/8/8/8/PPPPPPPP/XXXXXXXX w KQkq - 0 1");

	mstl::string result(Pattern);
	mstl::string const& position = chess960::position(idn);

	//M_ASSERT(position.size() >= 8);
	//M_ASSERT(::isupper(position[0]));

	::memcpy(result.data() + 35, position.c_str(), 8);

	result[0] = ::tolower(position[0]);
	result[1] = ::tolower(position[1]);
	result[2] = ::tolower(position[2]);
	result[3] = ::tolower(position[3]);
	result[4] = ::tolower(position[4]);
	result[5] = ::tolower(position[5]);
	result[6] = ::tolower(position[6]);
	result[7] = ::tolower(position[7]);

	return result;
}


unsigned
chess960::lookup(mstl::string const& position)
{
	for (unsigned i = 0; i < U_NUMBER_OF(PositionTable); ++i)
	{
		if (position == PositionTable[i])
			return i;
	}

	return 0;
}


unsigned
shuffle::twin(unsigned idn)
{
	//M_REQUIRE(idn <= 4*960);

	if (idn == 0)
		return 0;

	unsigned f = (idn - 1)/960;

	return chess960::Twins[idn - f*960] + f*960;
}


mstl::string const&
shuffle::identifier()
{
	// this seems to be the best known identifier for Shuffle Chess
	static mstl::string const Identifier("Shuffle Chess");
	return Identifier;
}


mstl::string
shuffle::position(unsigned idn)
{
	//M_REQUIRE(idn <= 4*960);

	if (idn == 0)
		idn = 960;
	else if (idn > 3*960)
		idn -= 3*960;

	mstl::string result = chess960::PositionTable[((idn - 1) % 960) + 1];

	if (idn > 2*960)
	{
		char* r = const_cast<char*>(::strchr(result, 'R'));
		char* k = const_cast<char*>(::strchr(r + 1, 'K'));

		mstl::swap(*r, *k);
	}
	else if (idn > 960)
	{
		char* k = const_cast<char*>(::strchr(result, 'K'));
		char* r = const_cast<char*>(::strchr(k + 1, 'R'));

		mstl::swap(*r, *k);
	}

	return result;
}


mstl::string
shuffle::fen(unsigned idn)
{
	static mstl::string const Pattern = "xxxxxxxx/pppppppp/8/8/8/8/PPPPPPPP/XXXXXXXX w - - 0 1";

	//M_REQUIRE(idn <= 4*960);

	if (idn <= 960)
		return chess960::fen(idn);

	mstl::string result(Pattern);
	mstl::string position(shuffle::position(idn));

	//M_ASSERT(position.size() >= 8);
	//M_ASSERT(::isupper(position[0]));

	::memcpy(result.data() + 35, position.c_str(), 8);

	result[0] = ::tolower(position[0]);
	result[1] = ::tolower(position[1]);
	result[2] = ::tolower(position[2]);
	result[3] = ::tolower(position[3]);
	result[4] = ::tolower(position[4]);
	result[5] = ::tolower(position[5]);
	result[6] = ::tolower(position[6]);
	result[7] = ::tolower(position[7]);

	return result;
}


mstl::string&
shuffle::utf8::position(unsigned idn, mstl::string& result)
{
	mstl::string pos = shuffle::position(idn);

	for (unsigned i = 0; i < pos.size(); ++i)
		result += piece::utf8::asString(piece::fromLetter(pos[i]));

	return result;
}


unsigned
shuffle::lookup(mstl::string const& position)
{
	unsigned i = chess960::lookup(position);

	if (i > 0)
		return i;

	char* k = const_cast<char*>(::strchr(position, 'K'));
	char* r = const_cast<char*>(::strchr(position, 'R'));

	if (k == 0 || r == 0)
		return 0;

	if (k > r)
	{
		if ((r = const_cast<char*>(::strchr(r + 1, 'R'))) == 0 || r > k)
			return 0;

		mstl::swap(*r, *k);
		i = chess960::lookup(position);
		mstl::swap(*r, *k);

		if (i > 0)
			return i + 960;
	}
	else
	{
		mstl::swap(*r, *k);
		i = chess960::lookup(position);
		mstl::swap(*r, *k);

		if (i > 0)
			return i + 2*960;
	}

	return 0;
}


char const*
nag::toSymbol(ID nag)
{
	switch (int(nag))
	{
		/*   1 */ case GoodMove:									return "!";
		/*   2 */ case PoorMove:									return "?";
		/*   3 */ case VeryGoodMove:								return "!!";
		/*   4 */ case VeryPoorMove:								return "??";
		/*   5 */ case SpeculativeMove:							return "!?";
		/*   6 */ case QuestionableMove:							return "?!";
		/*   8 */ case SingularMove:								return "[]";
		/*  10 */ case DrawishPosition:							return "=";
		/*  11 */ case EqualChancesQuietPosition:				return "==";
		/*  12 */ case EqualChancesActivePosition:			return "=~";	// or "&&", "&~", "=&", "~&"
		/*  13 */ case UnclearPosition:							return "~";		// or "~~"
		/*  14 */ case WhiteHasASlightAdvantage:				return "+=";	// or "+/="
		/*  15 */ case BlackHasASlightAdvantage:				return "=+";	// or "=/+"
		/*  16 */ case WhiteHasAModerateAdvantage:			return "+/-";
		/*  17 */ case BlackHasAModerateAdvantage:			return "-/+";
		/*  18 */ case WhiteHasADecisiveAdvantage:			return "+-";
		/*  19 */ case BlackHasADecisiveAdvantage:			return "-+";
		/*  20 */ case WhiteHasACrushingAdvantage:			return "+--";	// or "++--"
		/*  21 */ case BlackHasACrushingAdvantage:			return "--+";	// or "--++"
		/*  22 */ case WhiteIsInZugzwang:						// fallthru
		/*  23 */ case BlackIsInZugzwang:						return "(.)";
		/*  26 */ case WhiteHasAModerateSpaceAdvantage:		// fallthru
		/*  27 */ case BlackHasAModerateSpaceAdvantage:		return "()";
		/*  36 */ case WhiteHasTheInitiative:					// fallthru
		/*  37 */ case BlackHasTheInitiative:					return "|^";	// or "^"
		/*  40 */ case WhiteHasTheAttack:						// fallthru
		/*  41 */ case BlackHasTheAttack:						return "->";
		/*  44 */ case WhiteHasSufficientCompensationForMaterialDeficit:			return "~/=";
		/*  45 */ case BlackHasSufficientCompensationForMaterialDeficit:			return "=/~";
		/*  46 */ case WhiteHasMoreThanAdequateCompensationForMaterialDeficit:	return "+/~";
		/*  47 */ case BlackHasMoreThanAdequateCompensationForMaterialDeficit:	return "-/~";
		/*  50 */ case WhiteHasAModerateCenterControlAdvantage:						// fallthru
		/*  51 */ case BlackHasAModerateCenterControlAdvantage:						return "[+]";
		/* 138 */ case WhiteHasSevereTimeControlPressure:	// fallthru
		/* 139 */ case BlackHasSevereTimeControlPressure:	return "(+)";
		/* 140 */ case WithTheIdea:								return "/\\";
		/* 141 */ case AimedAgainst:								return "\\/";
		/* 142 */ case BetterMove:									return ">=";
		/* 143 */ case WorseMove:									return "<=";
		/* 144 */ case EquivalentMove:							return "=";
		/* 145 */ case EditorsRemark:								return "RR";
		/* 146 */ case Novelty:										return "N";
		/* 147 */ case WeakPoint:									return "><";
		/* 148 */ case Endgame:										return "_|_";
		/* 149 */ case Line:											return "<->";
		/* 150 */ case Diagonal:									return "//";	// or "/^"
		/* 153 */ case BishopsOfOppositeColor:					return "^_";
		/* 154 */ case BishopsOfSameColor:						return "^=";
		/* 155 */ case Diagram:										return "D";
		/* 156 */ case DiagramFromBlack:							return "D";
		/* 157 */ case SeparatedPawns:							return "o/o";
		/* 158 */ case DoublePawns:								return "oo";
		/* 159 */ case UnitedPawns:								return "o.o";	// or "o..o"
		/* 160 */ case PassedPawn:									return "o^";
		/* 165 */ case With:											return "|_";
		/* 166 */ case Without:										return "_|";
		/* 167 */ case Center:										return "[+]";
		/* 168 */ case File:											return "||";
		/* 171 */ case Various:										return "R";
		/* 173 */ case Space:										return "()";
		/* 174 */ case Zeitnot:										return "(+)";
		/* 175 */ case Development:								return "@";
		/* 176 */ case Zugzwang:									return "(.)";
		/* 178 */ case Attack:										return "->";	// or "=>"
		/* 179 */ case Initiative:									return "|^";	// or "^"
		/* 180 */ case Counterplay:								return "<=>";	// or "->/<-", "<=/=>", "=>/<="
		/* 181 */ case WithCompensationForMaterial:			return "=/~";	// or "&/=", "=/&", "~/="
		/* 182 */ case PairOfBishops:								return "^^";
		/* 183 */ case Kingside:									return ">>";
		/* 184 */ case Queenside:									return "<<";
	}

	return 0;
}


nag::ID
nag::fromSymbol(mstl::string const& symbol)
{
	CommentToken const* p = mstl::lower_bound(Map, Map + U_NUMBER_OF(Map), symbol);

	if (	p == Map + U_NUMBER_OF(Map)
		|| symbol.size() != p->token.size()
		|| ::strncmp(symbol, p->token, symbol.size()) != 0)
	{
		return Null;
	}

	return nag::ID(p->value);
}


nag::ID
nag::fromSymbol(char const* symbol, unsigned len)
{
	//M_REQUIRE(symbol);
	//M_REQUIRE(len <= ::strlen(symbol));

	mstl::string str;
	str.hook(const_cast<char*>(symbol), len);
	return fromSymbol(str);
}


nag::ID
nag::fromSymbol(char const* symbol)
{
	//M_REQUIRE(symbol);

	mstl::string str;
	str.hook(const_cast<char*>(symbol), ::strlen(symbol));
	return fromSymbol(str);
}


nag::ID
nag::toScid3(ID nag)
{
	//M_ASSERT(nag < Scidb_Last);

	if (nag >= nag::Scidb_Specific && Scidb_Last > nag)
	{
		switch (nag)
		{
			case Etc:					return Scid3_Etc;
			case DoublePawns:			return Scid3_DoublePawns;
			case SeparatedPawns:		return Scid3_SeparatedPawns;
			case UnitedPawns:			return Scid3_UnitedPawns;
			case HangingPawns:		return Scid3_HangingPawns;
			case BackwardPawns:		return Scid3_BackwardPawns;
			case Diagram:				return Scid3_Diagram;
			case DiagramFromBlack:	return Scid3_Diagram;

			default:						return Null;
		}
	}

	return nag;
}


nag::ID
nag::fromScid3(ID nag)
{
	if (nag >= Scid3_Specific && Scid3_Last > nag)
	{
		switch (nag)
		{
			case Scid3_Etc:				return Etc;
			case Scid3_DoublePawns:		return DoublePawns;
			case Scid3_SeparatedPawns:	return SeparatedPawns;
			case Scid3_UnitedPawns:		return UnitedPawns;
			case Scid3_HangingPawns:	return HangingPawns;
			case Scid3_BackwardPawns:	return BackwardPawns;
			case Scid3_Diagram:			return Diagram;

			default:							return Null;
		}
	}

	return nag;
}


nag::ID
nag::fromChessPad(ID nag)
{
	if (nag >= ChessPad_Specific && ChessPad_Last > nag)
	{
		switch (nag)
		{
			case ChessPad_Diagram:				return Diagram;
			case ChessPad_DiagramFromBlack:	return DiagramFromBlack;
			case ChessPad_SpaceAdvantage:		return Space;
			case ChessPad_File:					return File;
			case ChessPad_Diagonal:				return Diagonal;
			case ChessPad_Center:				return Center;
			case ChessPad_Kingside:				return Kingside;
			case ChessPad_Queenside:			return Queenside;
			case ChessPad_WeakPoint:			return WeakPoint;
			case ChessPad_Ending:				return Endgame;
			case ChessPad_BishopPair:			return PairOfBishops;
			case ChessPad_OppositeBishops:	return BishopsOfOppositeColor;
			case ChessPad_SameBishops:			return BishopsOfSameColor;
			case ChessPad_ConnectedPawns:		return UnitedPawns;
			case ChessPad_IsolatedPawns:		return SeparatedPawns;
			case ChessPad_DoubledPawns:		return DoublePawns;
			case ChessPad_PassedPawn:			return PassedPawn;
			case ChessPad_PawnMajority:		return MorePawns;
			case ChessPad_With:					return With;
			case ChessPad_Without:				return Without;

			default:									return Null;
		}
	}

	return nag;
}


mstl::string&
pawns::print(Progress progress, color::ID color, mstl::string& result)
{
	unsigned size = result.size();

	for (unsigned f = sq::FyleA; f <= sq::FyleH; ++f)
	{
		for (unsigned r = sq::Rank2; r <= sizeof(progress.side[color].rank); ++r)
		{
			if (progress.side[color].test(r, f))
			{
				sq::ID s = sq::make(f, r);

				if (color::isBlack(color))
					s = sq::flipRank(s);

				if (result.size() > size)
					result += ',';
				result += sq::printFyle(s);
				result += sq::printRank(s);
			}
		}
	}

	return result;
}


event::Mode
event::modeFromString(mstl::string const& s)
{
	static_assert(U_NUMBER_OF(ModeLookup) == Composition + 1, "event::Lookup expired");

	mstl::string const Corr("corr");
	mstl::string const Tournament("tournament");

	switch (::toupper(s.c_str()[0]))
	{
		case 'A':
			if (::isCaseEqual(s, ModeLookup[Analysis]))
				return Analysis;
			break;

		case 'C':
			if (::isCaseEqual(s, Corr))
				return PaperMail;
			if (::isCaseEqual(s, ModeLookup[Composition]))
				return Composition;
			break;

		case 'E':
			if (::isCaseEqual(s, ModeLookup[Email]))
				return Email;
			break;

		case 'I':
			if (::isCaseEqual(s, ModeLookup[Internet]))
				return Internet;
			break;

		case 'O':
			if (::isCaseEqual(s, ModeLookup[OverTheBoard]))
				return OverTheBoard;
			break;

		case 'P':
			if (::isCaseEqual(s, ModeLookup[PaperMail]))
				return PaperMail;
			break;

		case 'T':
			if (::isCaseEqual(s, ModeLookup[Telecommunication]) || ::isCaseEqual(s, Tournament))
				return Telecommunication;
			break;
	}

	return Undetermined;
}


mstl::string const&
event::toString(Mode mode)
{
	static_assert(U_NUMBER_OF(ModeLookup) == Composition + 1, "event::Lookup expired");
	//M_ASSERT(mode < int(U_NUMBER_OF(ModeLookup)));

	return ModeLookup[mode];
}


event::Type
event::typeFromString(mstl::string const& s)
{
	switch (::tolower(s.c_str()[0]))
	{
		case 'g':
			if (::isCaseEqual(s, TypeLookup[Game]))
				return Game;
			break;

		case 'm':
			if (::isCaseEqual(s, TypeLookup[Match]))
				return Match;
			break;

		case 't':
			switch (s.c_str()[1])
			{
				case 'o':
					if (::isCaseEqual(s, TypeLookup[Tournament]))
						return Tournament;

				case 'e':
					if (::isCaseEqual(s, TypeLookup[Team]))
						return Team;
					break;
			}
			break;

		case 'k':
			if (::isCaseEqual(s, TypeLookup[Knockout]) || (s.size() == 2 && ::tolower(s[1]) == 'o'))
				return Knockout;

		case 's':
			switch (s.c_str()[1])
			{
				case 'w':
					if (::isCaseEqual(s, TypeLookup[Swiss]))
						return Swiss;
					break;

				case 'i':
					if (::isCaseEqual(s, TypeLookup[Simultan]))
						return Simultan;
					break;

				case 'c':
					if (::isCaseEqual(s, TypeLookup[Schev]))
						return Schev;
					break;
			}
			break;
	}

	return Unknown;
}


mstl::string const&
event::toString(Type type)
{
	//M_ASSERT(type < int(U_NUMBER_OF(TypeLookup)));
	return TypeLookup[type];
}


mstl::string const&
sex::toString(ID sex)
{
	static mstl::string const MaleStr(1, sex::toChar(Male));
	static mstl::string const FemaleStr(1, sex::toChar(Female));

	switch (int(sex))
	{
		case sex::Male:	return MaleStr;
		case sex::Female:	return FemaleStr;
	}

	return mstl::string::empty_string;
}


variant::Type
variant::fromIdn(unsigned idn)
{
	if (idn == 0)								return Unknown;
	if (idn == chess960::StandardIdn)	return Standard;
	if (idn <= 960)							return Chess960;

	return Shuffle;
}

// vi:set ts=3 sw=3:
