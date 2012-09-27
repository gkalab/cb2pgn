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

#ifndef _cbh_codec_included
#define _cbh_codec_included

#include "db_database_codec.h"
#include "db_date.h"

#include "nsUniversalDetector.h"

#include "m_fstream.h"
#include "m_map.h"
#include "m_vector.h"
#include "m_hash.h"
#include "m_chunk_allocator.h"

namespace mstl { class fstream; }
namespace util { class ByteStream; };

namespace db {

class NamebaseEntry;
class NamebaseEvent;
class NamebasePlayer;
class TagSet;
class GameInfo;
class Date;

namespace cbh {

class Codec : public DatabaseCodec, public nsUniversalDetector
{
public:

	Codec();
	~Codec() throw();

	bool isWriteable() const override;
	bool encodingFailed() const override;

	Format format() const override;

	unsigned maxGameRecordLength() const override;
	unsigned maxGameLength() const override;
	unsigned maxGameCount() const override;
	unsigned maxPlayerCount() const override;
	unsigned maxEventCount() const override;
	unsigned maxSiteCount() const override;
	unsigned maxAnnotatorCount() const override;
	unsigned minYear() const override;
	unsigned maxYear() const override;
	unsigned maxDescriptionLength() const override;

	unsigned gameFlags() const override;

	mstl::string const& extension() const override;
	mstl::string const& encoding() const override;
	void filterTag(TagSet& tags, tag::ID tag, Section section) const override;

	void doOpen(mstl::string const& rootname,
					mstl::string const& encoding,
					util::Progress& progress) override;
	void reloadDescription(mstl::string const& rootname) override;
	void reloadNamebases(mstl::string const& rootname, util::Progress& progress) override;

	void close() override;

	void doDecoding(GameData& data, GameInfo& info, mstl::string*) override;
	save::State doDecoding(Consumer& consumer, TagSet& tags, GameInfo const& info) override;

	void reset() override;
	void setEncoding(mstl::string const& encoding) override;

	Move findExactPositionAsync(	GameInfo const& info,
											Board const& position,
											bool skipVariations) override;

	static bool getAttributes(	mstl::string const& filename,
										int& numGames,
										db::type::ID& type,
										mstl::string* description);
	static void getSuffixes(mstl::string const& filename, StringList& result);

private:

	class Source;

	struct Team
	{
		mstl::string	title;
		country::Code	nation;
	};

	struct Tournament
	{
		Tournament();
		Tournament(Byte cat, Byte nrounds);

		Byte category;
		Byte rounds;
	}
	__attribute__((packed));

	typedef mstl::map<uint32_t,NamebaseEntry*>			BaseMap;
	typedef mstl::map<uint32_t,uint32_t>					AnnotationMap;
	typedef mstl::hash<GameInfo const*,Source*>			SourceMap;
	typedef mstl::map<NamebaseEvent const*,Tournament>	TournamentMap;
	typedef mstl::chunk_allocator<NamebaseEvent>			Allocator;
	typedef mstl::vector<Team*>								TeamBase;
	typedef mstl::map<GameInfo const*,unsigned>			GameIndexLookup;

	void startDecoding(	util::ByteStream& gameStream,
								util::ByteStream& annotationStream,
								GameInfo const& info,
								bool& isChess960);
	void decodeIndex(util::ByteStream& strm, GameInfo& info);
	void decodeGuidingText(util::ByteStream& strm);

	unsigned readHeader(mstl::string const& rootname);

	void readIniData(mstl::string const& rootname);
	void readPlayerData(mstl::string const& rootname, util::Progress& progress);
	void readTournamentData(mstl::string const& rootname, util::Progress& progress);
	void readAnnotatorData(mstl::string const& rootname, util::Progress& progress);
	void readSourceData(mstl::string const& rootname, util::Progress& progress);
	void readTeamData(mstl::string const& rootname, util::Progress& progress);
	void readIndexData(mstl::string const& rootname, util::Progress& progress);
	void reloadPlayerData(mstl::string const& rootname, util::Progress& progress);
	void reloadTournamentData(mstl::string const& rootname, util::Progress& progress);
	void reloadAnnotatorData(mstl::string const& rootname, util::Progress& progress);
	void reloadSourceData(mstl::string const& rootname, util::Progress& progress);
	void reloadTeamData(mstl::string const& rootname, util::Progress& progress);
	void reloadIndexData(mstl::string const& rootname, util::Progress& progress);
	void preloadPlayerData(mstl::string const& rootname, util::Progress& progress);
	void preloadTournamentData(mstl::string const& rootname, util::Progress& progress);
	void preloadAnnotatorData(mstl::string const& rootname, util::Progress& progress);

	void addSourceTags(TagSet& tags, GameInfo const& info);
	void addEventTags(TagSet& tags, GameInfo const& info);

	NamebasePlayer* getPlayer(uint32_t ref);
	NamebaseEvent* getEvent(uint32_t ref);
	NamebaseEntry* getAnnotator(uint32_t ref);
	Source* getSource(uint32_t ref);

	void addTeamTags(TagSet& tags, GameInfo const& info);
	void mapPlayerName(mstl::string& str);
	void toUtf8(mstl::string& str);

	void Report(char const* charset);

	static void readIniData(mstl::fstream& strm,
									db::type::ID& type,
									mstl::string& title);

	mstl::fstream		m_gameStream;
	mstl::fstream		m_annotationStream;
	mstl::fstream		m_teamStream;
	unsigned				m_teamRecords;
	unsigned				m_teamRecordSize;
	AnnotationMap		m_annotationMap;
	BaseMap				m_playerMap;
	BaseMap				m_eventMap;
	BaseMap				m_annotatorMap;
	SourceMap			m_sourceMap;
	BaseMap				m_sourceMap2;
	TournamentMap		m_tournamentMap;
	TeamBase				m_teamBase;
	GameIndexLookup	m_gameIndexLookup;
	mstl::string		m_encoding;
	Allocator			m_allocator;
	Namebase				m_sourceBase;
	NamebaseEvent*		m_illegalEvent;
	NamebasePlayer*	m_illegalPlayer;
	unsigned				m_numGames;
	bool					m_highQuality;
};

} // namespace cbh
} // namespace db

#endif // _cbh_codec_included

// vi:set ts=3 sw=3:
