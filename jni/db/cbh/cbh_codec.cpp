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

// ChessBase format description:
// http://talkchess.com/forum/viewtopic.php?t=29468&highlight=cbh
// http://talkchess.com/forum/viewtopic.php?topic_view=threads&p=287896&t=29468&sid=a535ba2e9a17395e2582bdddf57c2425

// Required files:	.cba .cbc .cbg .cbh .cbp .cbt
// Used files:			.cba .cbc .cbg .cbh .cbp .cbt .cbs .cbe .cbj .ini .html/*

#include "cbh_codec.h"
#include "cbh_decoder.h"

#include "db_pgn_reader.h"
#include "db_game_data.h"
#include "db_exception.h"

#include "u_progress.h"
#include "u_byte_stream.h"
#include "u_misc.h"

#include "m_limits.h"
#include "m_string.h"
#include "m_byte_order.h"
#include "m_vector.h"
#include "m_stdio.h"
#include "m_bitfield.h"

#include <string.h>
#include <ctype.h>

using namespace db;
using namespace db::cbh;
using namespace db::country;
using namespace db::type;
using namespace util;


enum { Deleted = -999 };


static mstl::bitfield<uint64_t> m_lookup;

static country::Code NationMap[] =
{
	Unknown,										//   0
	Afghanistan,								//   1 AFG
	Albania,										//   2 ALB
	Algeria,										//   3 ALG
	Angola,										//   4 ANG
	Andorra,										//   5 AND
	Antigua,										//   6 ANT
	Argentina,									//   7 ARG
	Armenia,										//   8 ARM
	Australia,									//   9 AUS
	Austria,										//  10 AUT
	Azerbaijan,									//  11 AZE
	Bahamas,										//  12 BAH
	Bahrain,										//  13 BHN
	Bangladesh,									//  14 BAN
	Barbados,									//  15 BAR
	Belarus,										//  16 BLR
	Belgium,										//  17 BEL
	Belize,										//  18 BLZ
	Bermuda,										//  19 BER
	Bolivia,										//  20 BOL
	Bosnia_and_Herzegovina,					//  21 BIH
	Botswana,									//  22 BOT
	Brazil,										//  23 BRA
	British_Virgin_Islands,					//  24 IVB
	Brunei,										//  25 BRU
	Bulgaria,									//  26 BUL
	Burkina_Faso,								//  27 BUR
	Canada,										//  28 CAN
	Chile,										//  29 CHI
	China,										//  30 CHN
	Colombia,									//  31 COL
	Costa_Rica,									//  32 CRI
	Croatia,										//  33 CRO
	Cuba,											//  34 CUB
	Cyprus,										//  35 CYP
	Czech_Republic,							//  36 CZE
	Denmark,										//  37 DEN
	Djibouti,									//  38 DJI
	Dominican_Republic,						//  39 DOM
	Ecuador,										//  40 ECU
	Egypt,										//  41 EGY
	England,										//  42 ENG
	Spain,										//  43 ESP
	Estonia,										//  44 EST
	Ethiopia,									//  45 ETH
	Faroe_Islands,								//  46 FAI
	Fiji,											//  47 FIJ
	Finland,										//  48 FIN
	France,										//  49 FRA
	Macedonia,									//  50 FRM
	Gambia,										//  51 GAM
	Georgia,										//  52 GEO
	Germany,										//  53 GER
	Ghana,										//  54 GHA
	Greece,										//  55 GRE
	Guatemala,									//  56 GUA
	Guernsey,									//  57 GCI
	Guyana,										//  58 GUY
	Haiti,										//  59 HAI
	Honduras,									//  60 HON
	Hong_Kong,									//  61 HKG
	Hungary,										//  62 HUN
	Iceland,										//  63 ISL
	India,										//  64 IND
	Indonesia,									//  65 INA
	Iran,											//  66 IRI
	Iraq,											//  67 IRQ
	Israel,										//  68 ISR
	Ireland,										//  69 IRL
	Italy,										//  70 ITA
	Ivory_Coast,								//  71 IVO
	Jamaica,										//  72 JAM
	Japan,										//  73 JPN
	Jersey,										//  74 JCI
	Jordan,										//  75 JOR
	Kazakhstan,									//  76 KAZ
	Kenya,										//  77 KEN
	South_Korea,								//  78 KOR
	Kyrgyzstan,									//  79 KGZ
	Kuwait,										//  80 KUW
	Latvia,										//  81 LAT
	Lebanon,										//  82 LBN
	Libya,										//  83 LBY
	Liechtenstein,								//  84 LIE
	Lithuania,									//  85 LTU
	Luxembourg,									//  86 LUX
	Macao,										//  87 MAC
	Madagascar,									//  88 MAD
	Malaysia,									//  89 MAS
	Mali,											//  90 MLI
	Malta,										//  91 MLT
	Mauritania,									//  92 MAU
	Mauritius,									//  93 MRI
	Mexico,										//  94 MEX
	Moldova,										//  95 MDA
	Monaco,										//  96 MNC
	Mongolia,									//  97 MGL
	Morocco,										//  98 MAR
	Mozambique,									//  99 MOZ
	Myanmar,										// 100 MYA
	Namibia,										// 101 NAM
	Nepal,										// 102 NEP
	Netherlands,								// 103 NED
	Netherlands_Antilles,					// 104 AHO
	New_Zealand,								// 105 NZL
	Nicaragua,									// 106 NCA
	Nigeria,										// 107 NGR
	Norway,										// 108 NOR
	Pakistan,									// 109 PAK
	Palestine,									// 110 PLE
	Panama,										// 111 PAN
	Papua_New_Guinea,							// 112 PNG
	Paraguay,									// 113 PAR
	Peru,											// 114 PER
	Philippines,								// 115 PHI
	Poland,										// 116 POL
	Portugal,									// 117 POR
	Puerto_Rico,								// 118 PUR
	Qatar,										// 119 QAT
	Romania,										// 120 ROM
	Russia,										// 121 RUS
	El_Salvador,								// 122 ESA
	San_Marino,									// 123 SMR
	Scotland,									// 124 SCO
	Senegal,										// 125 SEN
	Seychelles,									// 126 SEY
	Singapore,									// 127 SIN
	Slovakia,									// 128 SVK
	Slovenia,									// 129 SLO
	South_Africa,								// 130 RSA
	Sri_Lanka,									// 131 SRI
	Sudan,										// 132 SUD
	Suriname,									// 133 SUR
	Sweden,										// 134 SWE
	Switzerland,								// 135 SUI
	Syria,										// 136 SYR
	Tajikistan,									// 137 TJK
	Tanzania,									// 138 TAN
	Thailand,									// 139 THA
	Trinidad_and_Tobago,						// 140 TRI
	Tunisia,										// 141 TUN
	Turkey,										// 142 TUR
	Turkmenistan,								// 143 TKM
	Uganda,										// 144 UGA
	Ukraine,										// 145 UKR
	United_Arab_Emirates,					// 146 UAE
	United_States_of_America,				// 147 USA
	Uruguay,										// 148 URU
	Soviet_Union,								// 149 URS
	Uzbekistan,									// 150 UZB
	Venezuela,									// 151 VEN
	Vietnam,										// 152 VIE
	US_Virgin_Islands,						// 153 ISV
	Wales,										// 154 WLS
	Yemen,										// 155 YEM
	Yugoslavia,									// 156 YUG
	Zambia,										// 157 ZAM
	Zimbabwe,									// 158 ZIM
	DR_Congo,									// 159 ZRE
	East_Germany,								// 160 DDR
	Czechoslovakia,							// 161 CSR
	Cameroon,									// 162 CAM
	Chad,											// 163 CHD
	Cape_Verde,									// 164 KAP
	Kiribati,									// 165 KBA
	Comoros,										// 166 COM
	Congo,										// 167 CON
	North_Korea,								// 168 NKO
	Laos,											// 169 LAO
	Lesotho,										// 170 LES
	Malawi,										// 171 MWI
	Maldives,									// 172 MDV
	Marshall_Islands,							// 173 MSH
	Oman,											// 174 OMN
	Nauru,										// 175 NAU
	Micronesia,									// 176 MIC
	Niger,										// 177 NIG
	Saudi_Arabia,								// 178 SAU
	Togo,											// 179 TOG
	Tonga,										// 180 TON
	Vanuatu,										// 181 VAN
	Vatican,										// 182 VAT
	Tuvalu,										// 183 TUV
	Swaziland,									// 184 SWA
	Sierra_Leone,								// 185 SIE
	Saint_Lucia,								// 186 SLU
	Papua_New_Guinea,							// 187 PAP
	Saint_Vincent_and_the_Grenadines,	// 188 SVI
	Samoa,										// 189 SAM
	Saint_Kitts_and_Nevis,					// 190 SKI
	Solomon_Islands,							// 191 SAL
	Germany,										// 192 GE2 German Empire
	Russia,										// 193 ZAR Russian Empire
	Rwanda,										// 194 RWA
	Liberia,										// 195 LBR
	American_Samoa,							// 196 NET
	Chinese_Taipei,							// 197 TWN
	Anguilla,									// 198 ASU
	Aruba,										// 199 AGG
	Benin,										// 200 BNN
	Bhutan,										// 201 BTN
	Burundi,										// 202 BRI
	Cambodia,									// 203 CMB
	Cayman_Islands,							// 204 CAY
	Central_African_Republic,				// 205 CAR
	Jersey,										// 206 CIB Channel Islands
	Christmas_Island,							// 207 CIA
	Cocos_Islands,								// 208 COA
	Cook_Islands,								// 209 CIN
	Equatorial_Guinea,						// 210 ELG
	Eritrea,										// 211 ERI
	Falkland_Islands,							// 212 FGB
	French_Guiana,								// 213 FRG
	French_Polynesia,							// 214 FRP
	Gabon,										// 215 GAB
	Gibraltar,									// 216 GGB
	Grenada,										// 217 GRA
	Greenland,									// 218 GRL
	Guadeloupe,									// 219 FGA
	Guam,											// 220 GMA
	Guinea,										// 221 GUI
	Guinea_Bissau,								// 222 GUB
	Isle_of_Man,								// 223 IOM
	Jan_Mayen_and_Svalbard,					// 224 JMY Jan Mayen
	Martinique,									// 225 MFR
	Mayotte,										// 226 MYF
	Montserrat,									// 227 MSG
	New_Caledonia,								// 228 NCF
	Niue,											// 229 NNN
	Norfolk_Island,							// 230 NNA
	Northern_Mariana_Islands,				// 231 NMI
	Timor_Leste,								// 232 OTM
	Palau,										// 233 PAL
	Pitcairn_Islands,							// 234 PIG
	Reunion,										// 235 RUF
	Sao_Tome_and_Principe,					// 236 SAO
	Somalia,										// 237 SOM
	Jan_Mayen_and_Svalbard,					// 238 SVN Svalbard
	Saint_Helena,								// 239 HGB
	Saint_Pierre_and_Miquelon,				// 240 PGB
	Tokelau,										// 241 TKI
	Turks_and_Caicos_Islands,				// 242 TCI
	Wallis_and_Futuna,						// 243 WFR
	Northern_Ireland,							// 244 NIR
	Aboard_Spacecraft,						// 245 ISS
	Great_Britain,								// 246 GBR
	Germany,										// 247 SAA Saarland
	Montenegro,									// 248 MNT
	Serbia,										// 249 SER
	Catalonia,									// 250 CAT
	Basque,										// 251 BAS
	Kosovo,										// 252 KOS
};


static type::ID const TypeMap[] =
{
	Unspecific,					// Unspecific
	Work,							// Work
	My_Games,					// My games
	Large_Database,			// Large database
	Informant,					// Informant
	Openings,					// Openings
	Unspecific,					// Magazine/Express
	Tournament,					// Classical Tournament
	Tournament,					// Recent Tournament
	Correspondence_Chess,	// Correspondence
	Tactics,						// Tactics
	Analysis,					// Analysis
	Training,					// Training
	Endgames,					// Endings
	Studies,						// Studies
	Blitz_Games,				// Blitz
	Computer_Chess,			// Computer chess
	Problems,					// Problems
	Patzer,						// Patzer
	Gambit,						// Gambit
	Correspondence_Chess,	// BdF
	Match,						// Match
	Player_Collection,		// Biography
	Unspecific,					// Multimedia
	Important,					// Important
	Unspecific,					// Text
	Internet_Chess,			// Internet
	Email_Chess,				// E-Mail
	Openings,					// Opening book
	Unspecific,					// Chess Media
	Unspecific,					// Reference
	Unspecific,					// Repertoire
	Unspecific,					// Update
	Unspecific,					// Strategy
	Unspecific,					// Playchess
	Unspecific,					// Tutorial
	Unspecific,					// Elite Chess
	Jewels,						// Brilliance
	Unspecific,					// Woman Chess
	Unspecific,					// Junior Chess
	Unspecific,					// Simultaneous
	Unspecific,					// Team Tournaments
};


static void
convPlayerName(mstl::string& str)
{
	for (unsigned i = 0; i < str.size(); ++i)
	{
		switch (Byte(str[i]))
		{
			case 0xa2: str[i] = 'K'; break;
			case 0xa3: str[i] = 'Q'; break;
			case 0xa4: str[i] = 'N'; break;
			case 0xa5: str[i] = 'B'; break;
			case 0xa6: str[i] = 'R'; break;
			case 0xa7: str[i] = 'P'; break;
		}
	}
}


inline static unsigned
convertEco(unsigned code)
{
	return code ? (code >> 7) & 0x1ff : 0;
}


inline static void
setDate(Date& result, uint32_t value)
{
	result.setYMD((value >> 9) & 4095, (value >> 5) & 15, value & 31);
}


static void
replaceNewlines(mstl::string& s)
{
	char const*	p = s.begin();
	char const*	e = s.end();
	char*			q = s.data();

	for ( ; p < e; ++p, ++q)
	{
		if (p[0] != '\r')
			*q = *p;
		else if (p[1] == '\n')
			*q = *++p;
		else
			*q = '\n';
	}

	s.resize(q - s.begin());
}


static unsigned
strippedLen(char const* s)
{
	unsigned len = strlen(s);

	while (len > 0 && ::isspace(s[len - 1]))
		--len;

	return len;
}


Codec::Tournament::Tournament() :category(0) ,rounds(0) {}
Codec::Tournament::Tournament(Byte cat, Byte nrounds) :category(cat) ,rounds(nrounds) {}


struct Codec::Source : public NamebaseEntry
{
	Source(mstl::string const& name, Date const& sourceDate);
	Date date;
};


Codec::Source::Source(mstl::string const& name, Date const& sourceDate)
	:NamebaseEntry(name)
	,date(sourceDate)
{
}


unsigned Codec::maxGameRecordLength() const	{ return 0x2fffffff - 1; }
unsigned Codec::maxGameCount() const			{ return (1 << 31)/46; }
unsigned Codec::maxGameLength() const			{ return (1 << 16) - 1; }
unsigned Codec::maxPlayerCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxEventCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxSiteCount() const			{ return (1 << 24) - 1; }
unsigned Codec::maxAnnotatorCount() const		{ return (1 << 24) - 1; }
unsigned Codec::minYear() const					{ return 0; }
unsigned Codec::maxYear() const					{ return mstl::max(Date::MaxYear, uint16_t(4094)); }
unsigned Codec::maxDescriptionLength() const	{ return mstl::numeric_limits<uint32_t>::max() - 1; }


unsigned
Codec::gameFlags() const
{
	return	GameInfo::Flag_Best_Game
			 | GameInfo::Flag_Decided_Tournament
			 | GameInfo::Flag_Model_Game
			 | GameInfo::Flag_Novelty
			 | GameInfo::Flag_Pawn_Structure
			 | GameInfo::Flag_Strategy
			 | GameInfo::Flag_Tactics
			 | GameInfo::Flag_With_Attack
			 | GameInfo::Flag_Sacrifice
			 | GameInfo::Flag_Defense
			 | GameInfo::Flag_Material
			 | GameInfo::Flag_Piece_Play
			 | GameInfo::Flag_End_Game
			 | GameInfo::Flag_Blunder
			 | GameInfo::Flag_User;
}


Codec::Codec()
	:m_teamRecords(0)
	,m_teamRecordSize(0)
	,m_encoding("cp1252")
	,m_allocator(32768)
	,m_sourceBase(Namebase::Annotator)
	,m_illegalEvent(0)
	,m_illegalPlayer(0)
	,m_numGames(0)
	,m_highQuality(false)
{
	if (::m_lookup.none())
	{
		static_assert(tag::ExtraTag <= 8*sizeof(uint64_t), "BitField size exceeded");

		::m_lookup.set(tag::Event);
		::m_lookup.set(tag::Site);
		::m_lookup.set(tag::Date);
		::m_lookup.set(tag::Round);
		::m_lookup.set(tag::White);
		::m_lookup.set(tag::Black);
		::m_lookup.set(tag::Result);
		::m_lookup.set(tag::Annotator);
		::m_lookup.set(tag::Eco);
		::m_lookup.set(tag::WhiteElo);
		::m_lookup.set(tag::BlackElo);
		::m_lookup.set(tag::EventDate);
		::m_lookup.set(tag::EventCountry);
		::m_lookup.set(tag::EventType);
		::m_lookup.set(tag::Mode);
		::m_lookup.set(tag::TimeMode);
	}
}


Codec::~Codec() throw()
{
	for (unsigned i = 0; i < m_teamBase.size(); ++i)
		delete m_teamBase[i];
}


bool
Codec::isWriteable() const
{
	return false;
}


bool
Codec::encodingFailed() const
{
	return false; //M_ASSERT(m_codec);
	//return m_codec->failed();
}


Codec::Format
Codec::format() const
{
	return format::ChessBase;
}


mstl::string const&
Codec::encoding() const
{
	//M_ASSERT(m_codec);
	//return m_codec->encoding();
	return "cp1252";
}


mstl::string const&
Codec::extension() const
{
	static mstl::string const Suffix("cbh");
	return Suffix;
}


void
Codec::Report(char const* charset)
{
	//if (::sys::utf8::Codec::latin1() == charset)
		m_encoding.assign("cp1252");
	//else if (::sys::utf8::Codec::ascii() != charset)
	//	m_encoding.assign(charset);
}


void
Codec::setEncoding(mstl::string const& encoding)
{
	//if (m_codec == 0)
	//	m_codec = new ::sys::utf8::Codec(encoding);
	//else
	//	m_codec->reset(encoding);
}


void
Codec::reset()
{
	// no action
}


void
Codec::toUtf8(mstl::string& str)
{
	//m_codec->toUtf8(str);
//
//	if (!sys::utf8::validate(str))
//		m_codec->forceValidUtf8(str);
}


void
Codec::filterTag(TagSet& tags, tag::ID tag, Section section) const
{
	bool gameTagsOnly = section == GameTags;

	if (::m_lookup.test(tag) == gameTagsOnly)
		tags.remove(tag);
}


void
Codec::mapPlayerName(mstl::string& str)
{
	if (str.empty())
	{
		str = '?';
	}
	else
	{
		mstl::vector<unsigned> indices;

		for (unsigned i = 0; i < str.size(); ++i)
		{
			switch (Byte(str[i]))
			{
				case 0xa2: str[i] = 'K'; indices.push_back(i); break;
				case 0xa3: str[i] = 'Q'; indices.push_back(i); break;
				case 0xa4: str[i] = 'N'; indices.push_back(i); break;
				case 0xa5: str[i] = 'B'; indices.push_back(i); break;
				case 0xa6: str[i] = 'R'; indices.push_back(i); break;
				case 0xa7: str[i] = 'P'; indices.push_back(i); break;
			}
		}

		toUtf8(str);

		mstl::string piece;
		unsigned k = 0;

		for (unsigned i = 0; i < indices.size(); ++i)
		{
			switch (str[indices[i] + k])
			{
				case 'K': piece = piece::utf8::asString(piece::King  ); break;
				case 'Q': piece = piece::utf8::asString(piece::Queen ); break;
				case 'R': piece = piece::utf8::asString(piece::Rook  ); break;
				case 'B': piece = piece::utf8::asString(piece::Bishop); break;
				case 'N': piece = piece::utf8::asString(piece::Knight); break;
				case 'P': piece = piece::utf8::asString(piece::Pawn  ); break;
			}

			str.replace(indices[i] + k, 1, piece);
			k += piece.size() - 1;
		}
	}
}


void
Codec::close()
{
	if (m_gameStream.is_open())
		m_gameStream.close();
	if (m_annotationStream.is_open())
		m_annotationStream.close();
	if (m_teamStream.is_open())
		m_teamStream.close();
}


void
Codec::doOpen(	mstl::string const& rootname,
					mstl::string const& encoding,
					util::Progress& progress)
{
	ProgressWatcher watcher(progress, 0);

	setEncoding(encoding);

	m_numGames = readHeader(rootname);

	mstl::string filename(rootname + ".cbg");
	checkPermissions(filename);
	openFile(m_gameStream, filename, Readonly);

	/*if (!m_codec->hasEncoding())
	{
		preloadPlayerData(rootname, progress);
		preloadAnnotatorData(rootname, progress);
		preloadTournamentData(rootname, progress);
		DataEnd();

		if (m_encoding == sys::utf8::Codec::automatic())
			m_encoding = sys::utf8::Codec::windows(); // cannot be Latin-1
		m_codec->reset(m_encoding);
		useEncoding(m_encoding);
	}*/

	progress.message("read-init");

	//readIniData(rootname);
	readSourceData(rootname, progress);
	readPlayerData(rootname, progress);
	readAnnotatorData(rootname, progress);
	readTournamentData(rootname, progress);
	readTeamData(rootname, progress);
	readIndexData(rootname, progress);

	namebases().setReadonly();

	// delete unused annotators (because we are discarding guiding text)
	namebase(Namebase::Annotator).cleanup();

	// delete unused events (sometimes it contains unused entries)
	namebase(Namebase::Event).cleanup();

	namebases().setReadonly(false);
	namebases().setModified(false);
	namebases().update();
	namebases().setReadonly();

	filename.assign(rootname + ".cba");
	checkPermissions(filename);
	openFile(m_annotationStream, filename, Readonly);
}


void
Codec::readTournamentData(mstl::string const& rootname, util::Progress& progress)
{
	//M_ASSERT(m_codec);

	static unsigned const RecordSize = 99;

	mstl::string	filename(rootname + ".cbt");
	mstl::string	name;
	mstl::string	city;
	mstl::fstream	strm;

	checkPermissions(filename);
	openFile(strm, filename, Readonly);
	name.assign(160, '\0');
	city.assign(160, '\0');

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	uint32_t nrecs = mstl::bo::swapLE(hdr[0]);
	strm.seekg((strm.size() % RecordSize) - sizeof(hdr), mstl::ios_base::cur);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;
	Namebase& eventBase	= namebase(Namebase::Event);
	Namebase& siteBase	= namebase(Namebase::Site);

	progress.start(nrecs);
	progress.message("read-tournament");
	eventBase.reserve(nrecs, (1 << 24) - 1);

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 40);

			name.set_size(::strippedLen(name));
			mapPlayerName(name);
			toUtf8(name);

			strm.read(city.data(), 30);
			city.set_size(::strippedLen(city));
			toUtf8(city);

			if (city.empty())
				city = '?';

			unsigned char buf[6];
			strm.read(buf, 6);

			Date evDate;
			::setDate(evDate, ByteStream::uint24LE(buf));

			Byte eventType = buf[4];

			time::Mode	timeMode		= time::Unknown;
			event::Mode	eventMode	= event::Undetermined;

			switch ((eventType >> 5) & 3)
			{
				case 0: timeMode = time::Normal; break;
				case 1: timeMode = time::Blitz; break;
				case 2: timeMode = time::Rapid; break;

				case 4:
					timeMode = time::Corr;
//					eventMode = event::PaperMail;
					break;
			}

			Byte nationCode = strm.get();
			country::Code countryCode = country::Unknown;

			if (0 < nationCode && nationCode < U_NUMBER_OF(::NationMap))
			{
				countryCode = ::NationMap[nationCode];

				if (	countryCode == country::Jersey
					&& (	::strstr(city, "Guernsey") != 0
						|| ::strstr(city, "Dgernesy") != 0
						|| ::strstr(city, "DgÃšrnÃ©sy") != 0	// "Dgèrnésy"
						|| ::strstr(city, "Peter Port") != 0))
				{
					countryCode = country::Guernsey;
				}
			}

			if (countryCode == country::Unknown)
				countryCode = PgnReader::extractCountryFromSite(city);

			Byte category = strm.get();
			strm.get();	// skip
			Byte rounds = strm.get();

			NamebaseSite* site = siteBase.insertSite(city, countryCode, nrecs);

			NamebaseEvent* event = eventBase.insertEvent(
												name,
												evDate.year(),
												evDate.month(),
												evDate.day(),
												event::Type(mstl::min(eventType & 15, 8)),
												timeMode,
												eventMode,
												nrecs,
												site);

			m_eventMap[i] = event;
			m_tournamentMap[event] = Tournament(category, rounds);

			strm.seekg(RecordSize - 89, mstl::ios_base::cur);
		}
	}

	siteBase.setNextId(nrecs);
	eventBase.setNextId(nrecs);
	strm.close();
}


void
Codec::readPlayerData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 67;

	mstl::fstream	strm;
	mstl::string	str;
	mstl::string	filename(rootname + ".cbp");
	Namebase&		base(namebase(Namebase::Player));

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size() % RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	base.reserve(nrecs, (1 << 24) - 1);
	str.assign(202, '\0');
	progress.start(nrecs);
	progress.message("read-player");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			uint32_t fideID = 0;

			strm.seekg(5, mstl::ios_base::cur);
			strm.read(str.data(), 30);
			str.set_size(::strippedLen(str));

			char* p = str.data() + str.size();

			str.append(", ", 2);
			strm.read(p + 2, 20);
			str.set_size(::strippedLen(p + 2) + str.size());

			if (p + 2 == str.end())
				str.resize(str.size() - 2);

			mapPlayerName(str);

			if (m_highQuality)
			{
				if (Player const* player = Player::findPlayer(str))
				{
					fideID = player->fideID();

					if (::strchr(str, ',') == nullptr)
						str = player->name();
				}
			}

			m_playerMap[i] = base.insertPlayer(str, fideID, nrecs);
			strm.seekg(RecordSize - 59, mstl::ios_base::cur);
		}
	}

	base.setNextId(nrecs);
	strm.close();
}


void
Codec::readAnnotatorData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 62;

	mstl::fstream	strm;
	mstl::string	str;
	mstl::string	filename(rootname + ".cbc");
	Namebase&		base(namebase(Namebase::Annotator));

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	base.reserve(nrecs, (1 << 24) - 1);
	str.assign(200, '\0');
	progress.start(nrecs);
	progress.message("read-annotator");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(str.data(), 45);
			str.set_size(::strippedLen(str));

			if (!str.empty())
			{
				toUtf8(str);
				m_annotatorMap[i] = base.insert(str, nrecs);
			}

			strm.seekg(RecordSize - 54, mstl::ios_base::cur);
		}
	}

	base.setNextId(nrecs);
	strm.close();
}


void
Codec::readSourceData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 68;

	mstl::string filename(rootname + ".cbs");
	mstl::ifstream cbs_stream(filename, mstl::ios_base::in | mstl::ios_base::binary);
	if (!cbs_stream) 
	{
	    return;
	}
	//if (!sys::file::access(filename, sys::file::Readable))
	//	return;

	mstl::fstream	strm;
	mstl::string	str;
	Date				sourceDate;

	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	m_sourceBase.reserve(nrecs, (1 << 24) - 1);
	str.assign(200, '\0');
	progress.start(nrecs);
	progress.message("read-source");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(str.data(), 25);
			str.set_size(::strippedLen(str));

			strm.seekg(20, mstl::ios_base::cur);

			unsigned char buf[3];
			strm.read(buf, 3);

			if (!str.empty())
			{
				toUtf8(str);
				::setDate(sourceDate, ByteStream::uint24LE(buf));
				m_sourceMap2[i] = new Source(str, sourceDate);
			}

			strm.seekg(4, mstl::ios_base::cur); // skip date and version

			if (strm.get() == 1)
				m_highQuality = true;

			strm.seekg(RecordSize - 62, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::readTeamData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 72;

	mstl::string filename(rootname + ".cbe");
	mstl::ifstream cbe_stream(filename, mstl::ios_base::in | mstl::ios_base::binary);
	if (!cbe_stream) 
	{
	    return;
	}
	//if (!sys::file::access(filename, sys::file::Readable))
	//	return;

	mstl::string filenameJ(rootname + ".cbj");
	mstl::ifstream cbj_stream(filenameJ, mstl::ios_base::in | mstl::ios_base::binary);
	if (!cbj_stream) 
	{
	    return;
	}
	//if (!sys::file::access(filenameJ, sys::file::Readable))
	//	return;
	openFile(m_teamStream, filenameJ, Readonly);

	{
		char hdr[12];

		if (!m_teamStream.read(reinterpret_cast<Byte*>(hdr), sizeof(hdr))){return;}
			// IO_RAISE(Index, Read_Error, "unexpected end of file");

		m_teamRecordSize = hdr[4];
		m_teamRecords = mstl::bo::swapLE(*(reinterpret_cast<uint32_t*>(hdr + 8)));

		unsigned expectedSize = m_teamRecords*m_teamRecordSize;
		expectedSize += m_teamStream.size()%m_teamRecordSize;

		if (expectedSize != m_teamStream.size())
		{
			fprintf(
				stderr,
				"File \"%s\" has %u records, but %u records are expected (record size is %u)\n",
				filenameJ.c_str(),
				unsigned(m_teamStream.size()/m_teamRecordSize),
				m_teamRecords,
				m_teamRecordSize);

			m_teamStream.close();
			m_teamRecords = 0;
			return;
		}

		if (m_teamRecords != m_numGames)
		{
			fprintf(
				stderr,
				"File \"%s\" has %u records, but index has %u records (%d missing)\n",
				filenameJ.c_str(),
				m_teamRecords,
				m_numGames,
				int(m_numGames - m_teamRecords));
		}
	}

	mstl::fstream	strm;
	mstl::string	str;
	Date				sourceDate;

	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	m_teamBase.reserve(nrecs);
	str.assign(250, '\0');
	progress.start(nrecs);
	progress.message("read-team");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
			m_teamBase.push_back(0);
		}
		else
		{
			Team* team = new Team;
			m_teamBase.push_back(team);

			strm.seekg(5, mstl::ios_base::cur);
			strm.read(str.data(), 45);
			str.set_size(::strippedLen(str));
			toUtf8(str);

			unsigned char buf[10];
			strm.read(buf, 10);
			ByteStream bstrm(buf, sizeof(buf));

			unsigned number	= bstrm.uint32LE();
			unsigned year2		= bstrm.get();
			unsigned year  	= bstrm.uint16LE();

			if (number)
			{
				str.append(' ');

				if (str.appendRomanNumber(number) == 0)
					str.format("%u", number);
			}

			if (year && !str.empty())
			{
				if (year2)
					str.format(" %u/%02u", year, (year + 1)%100);
				else
					str.format(" %u", year);
			}

			bstrm.skip(2);
			Byte nation = bstrm.get();

			if (0 < nation && nation < U_NUMBER_OF(::NationMap))
				team->nation = ::NationMap[nation];
			else
				team->nation = country::Unknown;

			char* p = m_sourceBase.alloc(str.size());
			::strncpy(p, str, str.size());
			team->title.hook(p, str.size());

			// next 4 bytes: number of games with this team?
			// next 4 bytes: first game id with this team?

			strm.seekg(RecordSize - 54 - sizeof(buf), mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::reloadDescription(mstl::string const& rootname)
{
	readIniData(rootname);
}


void
Codec::reloadNamebases(mstl::string const& rootname, Progress& progress)
{
	ProgressWatcher watcher(progress, 0);

	namebases().setReadonly(false);

	reloadPlayerData(rootname, progress);
	reloadAnnotatorData(rootname, progress);
	reloadTournamentData(rootname, progress);
	reloadTeamData(rootname, progress);
	reloadSourceData(rootname, progress);

	namebases().setReadonly(true);
}


void
Codec::preloadTournamentData(mstl::string const& rootname, util::Progress& progress)
{
	//M_ASSERT(m_codec);

	static unsigned const RecordSize = 99;

	mstl::string	filename(rootname + ".cbt");
	mstl::string	name;
	mstl::string	city;
	mstl::fstream	strm;

	checkPermissions(filename);
	openFile(strm, filename, Readonly);
	name.assign(160, '\0');
	city.assign(160, '\0');

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	uint32_t nrecs = mstl::bo::swapLE(hdr[0]);
	strm.seekg((strm.size() % RecordSize) - sizeof(hdr), mstl::ios_base::cur);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	progress.start(nrecs);
	progress.message("preload-tournament");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 40);
			::convPlayerName(name);
			HandleData(name, ::strlen(name));

			strm.read(city.data(), 30);
			HandleData(city, ::strlen(city));

			strm.seekg(RecordSize - 79, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::preloadPlayerData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 67;

	mstl::string	name;
	mstl::fstream	strm;
	mstl::string	filename(rootname + ".cbp");

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size() % RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	name.assign(202, '\0');
	progress.start(nrecs);
	progress.message("preload-player");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 30);
			::convPlayerName(name);
			HandleData(name, ::strlen(name));

			strm.read(name.data(), 20);
			::convPlayerName(name);
			HandleData(name, ::strlen(name));

			strm.seekg(RecordSize - 59, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::preloadAnnotatorData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 62;

	mstl::string	name;
	mstl::fstream	strm;
	mstl::string	filename(rootname + ".cbc");

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	name.assign(200, '\0');
	progress.start(nrecs);
	progress.message("preload-annotator");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 45);
			HandleData(name, ::strlen(name));

			strm.seekg(RecordSize - 54, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::reloadTournamentData(mstl::string const& rootname, util::Progress& progress)
{
	//M_ASSERT(m_codec);

	static unsigned const RecordSize = 99;

	mstl::string	filename(rootname + ".cbt");
	mstl::string	name;
	mstl::string	city;
	mstl::fstream	strm;

	checkPermissions(filename);
	openFile(strm, filename, Readonly);
	name.assign(160, '\0');
	city.assign(160, '\0');

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	uint32_t nrecs = mstl::bo::swapLE(hdr[0]);
	strm.seekg((strm.size() % RecordSize) - sizeof(hdr), mstl::ios_base::cur);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;
	Namebase& eventBase	= namebase(Namebase::Event);
	Namebase& siteBase	= namebase(Namebase::Site);

	progress.start(nrecs);
	progress.message("load-tournament");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 40);

			name.set_size(::strippedLen(name));
			//m_codec->toUtf8(name);

			//if (!sys::utf8::Codec::is7BitAscii(name))
			{
				/*if (!sys::utf8::validate(name))
					m_codec->forceValidUtf8(name);*/

				eventBase.rename(m_eventMap[i], name);
			}

			strm.read(city.data(), 30);
			city.set_size(::strippedLen(city));
			//m_codec->toUtf8(city);

			/*if (!sys::utf8::Codec::is7BitAscii(city))
			{
				if (!sys::utf8::validate(city))
					m_codec->forceValidUtf8(city);

				siteBase.rename(static_cast<NamebaseEvent*>(m_eventMap[i])->site(), city);
			}*/

			strm.seekg(RecordSize - 79, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::reloadPlayerData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 67;

	mstl::string	name;
	mstl::fstream	strm;
	mstl::string	filename(rootname + ".cbp");
	Namebase&		base(namebase(Namebase::Player));

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size() % RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	name.assign(202, '\0');
	progress.start(nrecs);
	progress.message("read-player");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 30);
			name.set_size(::strippedLen(name));

			char* p = name.data() + name.size();

			name.append(", ", 2);
			strm.read(p + 2, 20);
			name.set_size(::strippedLen(p + 2) + name.size());

			if (p + 2 == name.end())
				name.resize(name.size() - 2);

			mapPlayerName(name);
			base.rename(m_playerMap[i], name);
			strm.seekg(RecordSize - 59, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::reloadAnnotatorData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 62;

	mstl::string	name;
	mstl::fstream	strm;
	mstl::string	filename(rootname + ".cbc");
	Namebase&		base(namebase(Namebase::Annotator));

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	name.assign(200, '\0');
	progress.start(nrecs);
	progress.message("read-annotator");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 45);
			name.set_size(::strippedLen(name));
			//m_codec->toUtf8(name);

			/*if (!sys::utf8::Codec::is7BitAscii(name))
			{
				if (!sys::utf8::validate(name))
					m_codec->forceValidUtf8(name);

				base.rename(m_annotatorMap[i], name);
			}*/

			strm.seekg(RecordSize - 54, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::reloadSourceData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 68;

	mstl::string filename(rootname + ".cbs");

	//if (!sys::file::access(filename, sys::file::Readable))
	//	return;

	mstl::string	name;
	mstl::fstream	strm;

	openFile(strm, filename, Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	name.assign(200, '\0');
	progress.start(nrecs);
	progress.message("read-source");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 25);
			name.set_size(::strippedLen(name));
			//m_codec->toUtf8(name);

			/*if (!sys::utf8::Codec::is7BitAscii(name))
			{
				mstl::string str;

				if (!sys::utf8::validate(name))
					m_codec->forceValidUtf8(name);

				m_sourceBase.rename(m_sourceMap2[i], name);
			}*/

			strm.seekg(RecordSize - 34, mstl::ios_base::cur);
		}
	}

	strm.close();
}


void
Codec::reloadTeamData(mstl::string const& rootname, util::Progress& progress)
{
	static unsigned const RecordSize = 72;

	if (m_teamRecords == 0)
		return;

	mstl::string	name;
	mstl::fstream	strm;

	openFile(strm, rootname + ".cbe", Readonly);

	uint32_t hdr[7];
	strm.read(reinterpret_cast<char*>(hdr), sizeof(hdr));
	strm.seekg((strm.size()%RecordSize) - sizeof(hdr), mstl::ios_base::cur);
	uint32_t nrecs	= mstl::bo::swapLE(hdr[0]);

	unsigned frequency	= progress.frequency(nrecs, 20000);
	unsigned reportAfter	= frequency;

	name.assign(250, '\0');
	progress.start(nrecs);
	progress.message("read-team");

	for (unsigned i = 0; i < nrecs; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		int32_t addr;
		strm.read(reinterpret_cast<char*>(&addr), 4);
		addr = mstl::bo::swapLE(addr);

		if (addr == ::Deleted)
		{
			strm.seekg(RecordSize - 4, mstl::ios_base::cur);
		}
		else
		{
			strm.seekg(5, mstl::ios_base::cur);
			strm.read(name.data(), 45);
			name.set_size(::strippedLen(name));
			//m_codec->toUtf8(name);

			/*if (!sys::utf8::Codec::is7BitAscii(name))
			{
				if (!sys::utf8::validate(name))
					m_codec->forceValidUtf8(name);

				char* p = m_sourceBase.alloc(name.size());
				::strncpy(p, name, name.size());
				m_teamBase[i]->title.hook(p, name.size());
			}*/

			strm.seekg(RecordSize - 54, mstl::ios_base::cur);
		}
	}

	strm.close();
}


unsigned
Codec::readHeader(mstl::string const& rootname)
{
	mstl::string	filename(rootname + ".cbh");
	mstl::fstream	strm;

	checkPermissions(filename);
	openFile(strm, filename, Readonly);

	Byte record[46];
	strm.read(record, sizeof(record));

	if (	record[0] != 0
		|| record[1] != 0
		|| (	record[2] != 0x2c	// CB 9/10/11
			&& record[2] != 0x24)// CB Light
		|| record[3] != 0
		|| record[4] != 0x2e
		|| record[5] != 0x01)
	{
		//IO_RAISE(Index, Open_Failed, "bad magic in '%s'", filename.c_str());
	}

	ByteStream bstrm(record, sizeof(record));

	bstrm.skip(6);
	return bstrm.uint32() - 1;
}


void
Codec::readIndexData(mstl::string const& rootname, util::Progress& progress)
{
	mstl::string	filename(rootname + ".cbh");
	mstl::fstream	strm;

	Byte record[46];
	ByteStream bstrm(record, sizeof(record));

	openFile(strm, filename, Readonly);
	strm.seekg(sizeof(record));

	ProgressWatcher watcher(progress, m_numGames);
	progress.message("read-index");

	unsigned frequency	= progress.frequency(m_numGames, 20000);
	unsigned reportAfter	= frequency;

	GameInfoList& infoList = gameInfoList();

	infoList.reserve(m_numGames);
	m_sourceMap.reserve(unsigned(m_numGames*(100/SourceMap::Load)));
	if (m_teamRecords)
		m_gameIndexLookup.reserve(m_numGames);

	for (unsigned i = 0; i < m_numGames; ++i)
	{
		if (reportAfter == i)
		{
			progress.update(i);
			reportAfter += frequency;
		}

		strm.read(record, sizeof(record));
		bstrm.resetg();

		Byte flags = bstrm.peek();

		if ((flags & 0x2) == 0)
		{
			infoList.push_back(allocGameInfo());
			decodeIndex(bstrm, *infoList.back());

			if (m_teamRecords)
				m_gameIndexLookup[infoList.back()] = i;
		}
		else
		{
			decodeGuidingText(bstrm);
		}
	}

	strm.close();
}


void
Codec::readIniData(mstl::string const& rootname)
{
	mstl::string	filename(rootname + ".ini");
	mstl::string	title;
	mstl::fstream	strm;
	db::type::ID	type;
	mstl::ifstream ini_stream(filename, mstl::ios_base::in | mstl::ios_base::binary);
	if (!ini_stream) 
	{
	    return;
	}

	//if (!sys::file::access(filename, sys::file::Readable))
	//	return;

	openFile(strm, filename, Readonly);
	strm.exceptions(mstl::ios_base::badbit);

	//M_ASSERT(m_codec);
	readIniData(strm, type, title);
	setType(type);
	setDescription(title);
}


void
Codec::readIniData(mstl::fstream& strm, db::type::ID& type, mstl::string& title)
{
	enum Section { AnyOther, DescrCBG , Environ };

	// M_REQUIRE(strm.is_open());

	mstl::string tournTitle;
	mstl::string tournPlace;
	mstl::string tournYear;

	Section section = AnyOther;

	type = Unspecific;

	while (true)
	{
		mstl::string line;

		if (!strm.getline(line))
		{
			strm.close();

			title.trim();
			tournTitle.trim();
			tournPlace.trim();
			tournYear.trim();

			if (!tournTitle.empty())
			{
				title = tournTitle;

				if (tournPlace != "Unknown" && tournPlace != "Unkown" && !tournPlace.empty())
				{
					title.append(", ", 2);
					title += tournPlace;
				}

				if (!tournYear.empty() && ::isdigit(tournYear[0]))
				{
					title.append(", ", 2);
					title += tournYear;
				}
			}

			//codec.toUtf8(title);
			//if (!::sys::utf8::validate(title))
			//	codec.forceValidUtf8(title);

			return;
		}

		if (!line.empty())
		{
			if (line[0] == '[')
			{
				if (::strncmp(line, "[DescrCBG]", 10) == 0)
					section = DescrCBG;
				else if (::strncmp(line, "[Environ]", 9) == 0)
					section = Environ;
				else
					section = AnyOther;
			}
			else
			{
				switch (section)
				{
					case DescrCBG:
						if (::strncmp(line, "Type=", 5) == 0)
						{
							unsigned t = ::strtoul(line.c_str() + 5, nullptr, 10);
							if (t < U_NUMBER_OF(::TypeMap))
								type = ::TypeMap[t];
						}
						else if (::strncmp(line, "Title=", 6) == 0)
						{
							title = line.substr(6);
						}
						break;

					case Environ:
						if (::strncmp(line, "TournTitle=", 11) == 0)
							tournTitle = line.substr(11);
						else if (::strncmp(line, "TournPlace=", 11) == 0)
							tournPlace = line.substr(11);
						else if (::strncmp(line, "TournYear=", 10) == 0)
							tournYear = line.substr(10);
						break;

					case AnyOther:
						break;
				}
			}
		}
	}
}


NamebasePlayer*
Codec::getPlayer(uint32_t ref)
{
	BaseMap::iterator p = m_playerMap.find(ref);

	if (p != m_playerMap.end())
	{
		p->second->ref();
		return static_cast<NamebasePlayer*>(p->second);
	}

	if (m_illegalPlayer == 0)
	{
		m_illegalPlayer = namebase(Namebase::Player).insertPlayer(
									"? (illegal reference)", 0, m_playerMap.size() + 1);
	}

	m_illegalPlayer->ref();
	return m_illegalPlayer;
}


NamebaseEvent*
Codec::getEvent(uint32_t ref)
{
	BaseMap::iterator p = m_eventMap.find(ref);

	if (p != m_eventMap.end())
	{
		NamebaseEvent* event = static_cast<NamebaseEvent*>(p->second);

		if (event->frequency() == 0)
			event->site()->ref();

		event->ref();
		return event;
	}

	if (m_illegalEvent == 0)
	{
		m_illegalEvent = namebase(Namebase::Event).insertEvent(
								"? (illegal reference)",
								m_eventMap.size() + 1,
								namebase(Namebase::Site).insertSite(
									mstl::string::empty_string,
									country::Unknown,
									m_eventMap.size() + 1));
		m_illegalEvent->site()->ref();
	}

	m_illegalEvent->ref();

	return m_illegalEvent;
}


NamebaseEntry*
Codec::getAnnotator(uint32_t ref)
{
	BaseMap::iterator p = m_annotatorMap.find(ref);

	if (p == m_annotatorMap.end())
		return 0;

	p->second->ref();
	return p->second;
}


Codec::Source*
Codec::getSource(uint32_t ref)
{
	BaseMap::iterator p = m_sourceMap2.find(ref);

	if (p == m_sourceMap2.end())
		return 0;

	p->second->ref();

	return static_cast<Source*>(p->second);
}


void
Codec::decodeGuidingText(ByteStream& strm)
{
	// M_ASSERT(m_gameStream.is_open());

if (::getenv("SCIDB_GUIDING_TEXT") == 0) return;

	Byte flags = strm.get();

	if (flags & (1 << 7))
		return; // is deleted

	unsigned offset			= strm.uint32();

//printf("skip(0): %02x %02x %02x\n", Byte(strm.data()[0]), Byte(strm.data()[1]), Byte(strm.data()[2]));
	strm.skip(3);

	__attribute__((unused)) unsigned tournamentId	= strm.uint24();
	__attribute__((unused)) unsigned sourceId			= strm.uint24();
	__attribute__((unused)) unsigned annotatorId		= strm.uint24();;
	__attribute__((unused)) unsigned round				= strm.get();

//printf("offset:     %u\n", offset);
//printf("tournament: %u\n", tournamentId);
//printf("source:     %u\n", sourceId);
//printf("annotator:  %u\n", annotatorId);
//printf("round:      %u\n", round);

#if 0
	if (NamebaseEntry* annotator = getAnnotator(annotatorId))
		send(annotator);
	if (Source* source = getSource(sourceId))
		send(source);
	if (NamebaseEvent* event = getEvent(tournamentId))
		send(event);
	if (round)
		send(round);
#endif

	char header[8];

	if (m_gameStream.seekg(offset, mstl::ios_base::beg) && m_gameStream.read(header, sizeof(header)))
	{
		// Header (8 bytes) -----------------------------------------------------------------------
		// 80 00 20 cd			bit 0-29: Game size (including this, excluding trash bytes)
		// 						bit 30: ?
		// 						bit 31: If set, the data is not encoded
		//
		// 03						type of entries (after titles)?
		// 00						?
		// 01 00					Number of titles (can be 0) (little endian)
		//
		// For each title: ------------------------------------------------------------------------
		// 01 00					Language (0=english, 1=german, 2=france, 3=spain, 4=italian, 5=dutch)
		// 						(little endian) (unknown: italian, portuguese, polish)
		// 05 00					Length of title (little endian)
		// string				Title

		ByteStream bstrm(header, sizeof(header));

		int size = bstrm.uint32();

		if ((size & 0x80000000) == 0)
			return; // data is encoded

		size = (size & 0x3fffffff) - sizeof(header);
//printf("skip(1): %02x %02x\n", Byte(bstrm.data()[0]), Byte(bstrm.data()[1]));

		unsigned type = bstrm.get();

		bstrm.skip(1); // ??

		unsigned numTitles = bstrm.uint16LE();

		char* buf = new char[size];
		char* mem = buf;

		mstl::string str;

		if (m_gameStream.read(buf, size))
		{
			bstrm.setup(buf, size);

			for (unsigned i = 0; i < numTitles; ++i)
			{
				if (bstrm.remaining() <= 4)
					return;

				__attribute__((unused)) unsigned lang		= bstrm.uint16LE();
				__attribute__((unused)) unsigned length	= bstrm.uint16LE();

				if (length > bstrm.remaining())
					return;

				str.clear();
				bstrm.get(str, length);
				::replaceNewlines(str);

//printf("lang=%u, length=%u: '%s'\n", lang, length, str.c_str());
			}
		}

		// PAN-EGC:
		// 01 | 01 00 00 00 | 67 34
		// language | id | length

		// Big2010:
		// 01 | 01 00 00 00 | 20 02
		// language | id | length

		// CB Light Database:
		// 01 | 08 00 00 00 | 6e 09 00 00
		// language | id | length

		// Test1:
		// 01 | 08 00 00 00 | 96 00 00 00
		// language | id | length

		if (type == 1)
		{
			if (bstrm.remaining() > 7)
			{
				__attribute__((unused)) unsigned lang		= bstrm.get();
				__attribute__((unused)) unsigned id			= bstrm.uint32LE();	// which?
				__attribute__((unused)) unsigned length	= bstrm.uint16LE();

				if (length > bstrm.remaining())
					return;

				str.clear();
				bstrm.get(str, length);
				::replaceNewlines(str);
//printf("id=%u, lang=%u: '%s'\n", id, lang, str.c_str());
				bstrm.get(); // nul byte?
			}
		}
		else
		{
			unsigned lang	= bstrm.get();			// really is language?
			unsigned id		= bstrm.uint32LE();	// title id?

			while (bstrm.remaining() > 4)
			{
				unsigned length = bstrm.uint32LE();

				if (length >= bstrm.remaining())
					return;

				str.clear();
				bstrm.get(str, length);
				::replaceNewlines(str);
printf("id=%u, lang=%u: '%s'\n", id, lang, str.c_str());
				bstrm.get(); // nul byte?

				if (bstrm.remaining() > 9)
				{
					id = bstrm.uint32();	// title id?
					lang = bstrm.get();	// really?
				}
			}
		}

		delete [] mem;
	}
}


void
Codec::decodeIndex(ByteStream& strm, GameInfo& info)
{
	// M_ASSERT(strm.size() == 46);

	Byte flags = strm.get();

	if (flags & (1 << 7))
		info.m_gameFlags |= GameInfo::Flag_Deleted;

	info.m_gameOffset = strm.uint32();

	if (unsigned offset = strm.uint32())
		m_annotationMap[info.m_gameOffset] = offset;

	NamebasePlayer* white = getPlayer(strm.uint24());
	NamebasePlayer* black = getPlayer(strm.uint24());

	info.m_player[color::White] = white;
	info.m_player[color::Black] = black;
	info.m_event = getEvent(strm.uint24());

	if (NamebaseEntry* annotator = getAnnotator(strm.uint24()))
		info.m_annotator = annotator;
	if (Source* source = getSource(strm.uint24()))
		m_sourceMap[&info] = source;

	Date date;
	::setDate(date, strm.uint24());

	info.m_dateYear = Date::encodeYearTo10Bits(date.year());
	info.m_dateMonth = date.month();
	info.m_dateDay = date.day();

	switch (strm.get())
	{
		case 0:	info.m_result = result::Black; break;
		case 1:	info.m_result = result::Draw; break;
		case 2:	info.m_result = result::White; break;
		case 7:	info.m_result = result::Lost; break;
		default:	info.m_result = result::Unknown; break;
	}

	strm.skip(1);	// skip line evaluation

	info.m_round = strm.get();
	info.m_subround = strm.get();

	uint16_t whiteElo = mstl::min(uint16_t(rating::Max_Value), strm.uint16());
	uint16_t blackElo = mstl::min(uint16_t(rating::Max_Value), strm.uint16());

	info.m_pd[color::White].elo = whiteElo;
	info.m_pd[color::Black].elo = blackElo;

	if (white != m_illegalPlayer)
		white->setElo(whiteElo);
	if (black != m_illegalPlayer)
		black->setElo(blackElo);

	info.m_eco = ::convertEco(strm.uint16());

	unsigned medals = strm.uint16();

	if (medals & (1 <<  0)) info.m_gameFlags |= GameInfo::Flag_Best_Game;
	if (medals & (1 <<  1)) info.m_gameFlags |= GameInfo::Flag_Decided_Tournament;
	if (medals & (1 <<  2)) info.m_gameFlags |= GameInfo::Flag_Model_Game;
	if (medals & (1 <<  3)) info.m_gameFlags |= GameInfo::Flag_Novelty;
	if (medals & (1 <<  4)) info.m_gameFlags |= GameInfo::Flag_Pawn_Structure;
	if (medals & (1 <<  5)) info.m_gameFlags |= GameInfo::Flag_Strategy;
	if (medals & (1 <<  6)) info.m_gameFlags |= GameInfo::Flag_Tactics;
	if (medals & (1 <<  7)) info.m_gameFlags |= GameInfo::Flag_With_Attack;
	if (medals & (1 <<  8)) info.m_gameFlags |= GameInfo::Flag_Sacrifice;
	if (medals & (1 <<  9)) info.m_gameFlags |= GameInfo::Flag_Defense;
	if (medals & (1 << 10)) info.m_gameFlags |= GameInfo::Flag_Material;
	if (medals & (1 << 11)) info.m_gameFlags |= GameInfo::Flag_Piece_Play;
	if (medals & (1 << 12)) info.m_gameFlags |= GameInfo::Flag_End_Game;
	if (medals & (1 << 13)) info.m_gameFlags |= GameInfo::Flag_Blunder;	// Flag_Tactical_Blunder
	if (medals & (1 << 14)) info.m_gameFlags |= GameInfo::Flag_Blunder;	// Flag_Strategical_Blunder
	if (medals & (1 << 15)) info.m_gameFlags |= GameInfo::Flag_User;

	strm.skip(3);
	flags = strm.get();
	info.m_positionId = flags & (1 << 0) ? 0 : chess960::StandardIdn;

	if (flags & (1 << 1)) info.m_variationCount = 5;
	if (flags & (1 << 2)) info.m_commentCount = 5;
	if (flags & (7 << 3)) info.m_annotationCount = 5;

	strm.skip(1);

	flags = strm.get();
	if (flags & (2 << 0)) info.m_variationCount = 15;
	if (flags & (1 << 2)) info.m_commentCount = 15;
	if (flags & (7 << 3)) info.m_annotationCount = 15;

	info.m_plyCount = mstl::min(	GameInfo::MaxPlyCount,
											unsigned(mstl::max(0, mstl::mul2(int(strm.get())) - 1)));
}


void
Codec::startDecoding(ByteStream& gameStream,
							ByteStream& annotationStream,
							GameInfo const& info,
							bool& isChess960)
{
	if (!info.gameOffset()){return;}
		// IO_RAISE(Index, Corrupted, "no game data");
	if (!m_gameStream.seekg(info.gameOffset(), mstl::ios_base::beg)){return;}
		// IO_RAISE(Game, Corrupted, "unexpected end of file");
	if (!m_gameStream.read(gameStream.base(), 4)){return;}
		// IO_RAISE(Game, Corrupted, "unexpected end of file");

	unsigned word = gameStream.uint32();

	if (word & 0x80000000)
	{
		//M_THROW(DecodingFailedException());
return;
		// TODO: we have something special to do, but what?
		// look at Big2010, #1964391, Giffard, Nicalas - Castlagliola, Marina
	}

	isChess960 = bool(word & 0x0a000000);
	unsigned size = word & 0x00ffffff;

	gameStream.resetg();
	gameStream.reserve(size);
	gameStream.provide(size);

	if (!m_gameStream.seekg(-4, mstl::ios_base::cur)){return;}
		// IO_RAISE(Game, Unknown_Error_Type, "seek failed");
	if (!m_gameStream.read(gameStream.base(), size)){return;}
		// IO_RAISE(Game, Corrupted, "unexpected end of file");

	AnnotationMap::const_iterator i = m_annotationMap.find(info.gameOffset());

	if (i != m_annotationMap.end())
	{
		uint32_t address = i->second;

		if (!m_annotationStream.seekg(address + 10, mstl::ios_base::beg)){return;}
			// IO_RAISE(Annotation, Corrupted, "unexpected end of file");

		if (!m_annotationStream.read(annotationStream.base(), 4)){return;}
			// IO_RAISE(Annotation, Corrupted, "unexpected end of file");

		size = annotationStream.uint32() - 14;

		annotationStream.resetg();
		annotationStream.reserve(size);
		annotationStream.provide(size);

		if (!m_annotationStream.read(annotationStream.base(), size)){return;}
			// IO_RAISE(Annotation, Corrupted, "unexpected end of file");
	}
	else
	{
		annotationStream.provide(0);
	}
}


void
Codec::addSourceTags(TagSet& tags, GameInfo const& info)
{
	if (SourceMap::const_pointer s = m_sourceMap.find(&info))
	{
		Source const* source = *s;

		tags.set(tag::Source, source->name());

		if (source->date)
			tags.set(tag::SourceDate, source->date.asString());
	}
}


void
Codec::addEventTags(TagSet& tags, GameInfo const& info)
{
	TournamentMap::const_iterator t = m_tournamentMap.find(info.eventEntry());

	if (t != m_tournamentMap.end())
	{
		Tournament tournament = t->second;

		if (tournament.category)
			tags.set(tag::EventCategory, tournament.category);

		if (tournament.rounds)
			tags.set(tag::EventRounds, tournament.rounds);
	}
}


void
Codec::addTeamTags(TagSet& tags, GameInfo const& info)
{
	if (m_teamRecords == 0)
		return;

	// M_ASSERT(m_gameIndexLookup.find(&info) != m_gameIndexLookup.end());

	unsigned gameIndex = m_gameIndexLookup.find(&info)->second;

	if (m_teamRecords <= gameIndex)
		return;

	uint32_t buf[2];
	unsigned offset = m_teamStream.size()%m_teamRecordSize + gameIndex*m_teamRecordSize;

	if (!m_teamStream.seekg(offset, mstl::ios_base::beg)){return;}
		// IO_RAISE(Index, Read_Error, "seek failed");
	if (!m_teamStream.read(reinterpret_cast<Byte*>(buf), sizeof(buf))){return;}
		// IO_RAISE(Index, Read_Error, "unexpected end of file");

	unsigned whiteTeamRef = mstl::bo::swapLE(buf[0]);
	unsigned blackTeamRef = mstl::bo::swapLE(buf[1]);

	if (whiteTeamRef < m_teamBase.size())
	{
		if (Team const* team = m_teamBase[whiteTeamRef])
		{
			if (!team->title.empty())
			{
				tags.set(tag::WhiteTeam, team->title);

				if (team->nation != country::Unknown)
					tags.set(tag::WhiteTeamCountry, country::toString(team->nation));

			}
		}
	}

	if (blackTeamRef < m_teamBase.size())
	{
		if (Team const* team = m_teamBase[blackTeamRef])
		{
			if (!team->title.empty())
			{
				tags.set(tag::BlackTeam, team->title);

				if (team->nation != country::Unknown)
					tags.set(tag::BlackTeamCountry, country::toString(team->nation));

			}
		}
	}
}


void
Codec::doDecoding(GameData& data, GameInfo& info, mstl::string*)
{
	Byte buf[2][32768];

	ByteStream gStrm(buf[0], sizeof(buf[0]));
	ByteStream aStrm(buf[1], sizeof(buf[1]));

	bool isChess960;

	startDecoding(gStrm, aStrm, info, isChess960);

	addSourceTags(data.m_tags, info);
	addEventTags(data.m_tags, info);
	addTeamTags(data.m_tags, info);

	Decoder decoder(gStrm, aStrm, isChess960);
	info.m_plyCount = mstl::min(GameInfo::MaxPlyCount, decoder.doDecoding(data));
}


save::State
Codec::doDecoding(Consumer& consumer, TagSet& tags, GameInfo const& info)
{
	Byte buf[2][32768];

	ByteStream gStrm(buf[0], sizeof(buf[0]));
	ByteStream aStrm(buf[1], sizeof(buf[1]));

	bool isChess960;

	startDecoding(gStrm, aStrm, info, isChess960);

	addSourceTags(tags, info);
	addEventTags(tags, info);
	addTeamTags(tags, info);

	Decoder decoder(gStrm, aStrm, isChess960);
	save::State state = decoder.doDecoding(consumer, tags, info);

	return state;
}


// NOTE: currently not used!
Move
Codec::findExactPositionAsync(GameInfo const& info, Board const& position, bool skipVariations)
{
	Byte buf[32768];

	ByteStream gStrm(buf, sizeof(buf));
	ByteStream aStrm(static_cast<Byte*>(0), static_cast<Byte*>(0));

	bool isChess960;

	startDecoding(gStrm, aStrm, info, isChess960);

	Decoder decoder(gStrm, aStrm, isChess960);
	return decoder.findExactPosition(position, skipVariations);
}


bool
Codec::getAttributes(mstl::string const& filename,
							int& numGames,
							db::type::ID& type,
							mstl::string* description)
{
	mstl::fstream strm(filename, mstl::ios_base::in | mstl::ios_base::binary);
	mstl::string rootname(misc::file::rootname(filename));

	if (!strm)
		return false;

	Byte record[46];

	if (!strm.read(record, sizeof(record)))
		return false;

	if (	record[0] != 0
		|| record[1] != 0
		|| (	record[2] != 0x2c		// CB 9/10/11
			&& record[2] != 0x24)	// CB Light
		|| record[3] != 0
		|| record[4] != 0x2e
		|| record[5] != 0x01)
	{
		return false;
	}

	ByteStream bstrm(record, sizeof(record));

	bstrm.skip(6);
	numGames = bstrm.uint32() - 1;

	if (description)
	{
		strm.close();
		strm.open(rootname + ".ini", mstl::ios_base::in);

		if (strm)
		{
			//sys::utf8::Codec codec(sys::utf8::Codec::windows());
			readIniData(strm, type, *description);
			strm.close();
		}
	}

	strm.close();
	return true;
}


void
Codec::getSuffixes(mstl::string const&, StringList& result)
{
	result.push_back("cba");
	result.push_back("cbb");
	result.push_back("cbc");
	result.push_back("cbe");
	result.push_back("cbg");
	result.push_back("cbgi");
	result.push_back("cbh");
	result.push_back("cbj");
	result.push_back("cbm");
	result.push_back("cbp");
	result.push_back("cbs");
	result.push_back("cbt");
	result.push_back("cbtt");
	result.push_back("cib");
	result.push_back("cit");
	result.push_back("ck1");
	result.push_back("ck2");
	result.push_back("ck3");
	result.push_back("ckn");
	result.push_back("cko");
	result.push_back("cp1");
	result.push_back("cp2");
	result.push_back("cp3");
	result.push_back("cpn");
	result.push_back("cpo");
	result.push_back("flags");
	result.push_back("html");
	result.push_back("ini");
	result.push_back("rb0");
	result.push_back("rb1");
	result.push_back("rb2");
}

// vi:set ts=3 sw=3:
