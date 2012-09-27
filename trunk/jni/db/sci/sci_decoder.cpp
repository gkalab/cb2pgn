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

#include "sci_decoder.h"
#include "sci_common.h"

#include "db_game_data.h"
#include "db_move_node.h"
#include "db_mark_set.h"
#include "db_move_info_set.h"
#include "db_annotation.h"
#include "db_database_codec.h"
#include "db_tag_set.h"
#include "db_consumer.h"
#include "db_exception.h"

#include "u_byte_stream.h"

#include "m_bit_functions.h"
#include "m_utility.h"
#include "m_assert.h"

#include <string.h>

using namespace db;
using namespace db::sci;
using namespace util;


inline static void
throwCorruptData()
{
	// IO_RAISE(Game, Corrupted, "error while decoding game data");
}


Decoder::Decoder(ByteStream& strm)
	:m_strm(strm)
	,m_ensuredStreamSize(strm.size())
	,m_currentNode(0)
{
}


Decoder::Decoder(ByteStream& strm, unsigned ensuredStreamSize)
	:m_strm(strm)
	,m_ensuredStreamSize(ensuredStreamSize)
	, m_currentNode(0)
{
}

inline
Move
Decoder::decodeKing(sq::ID from, Byte nybble)
{
	static int const Offset[] = { 0, 0, 0, 0, 0, 0, -9, -8, -7, -1, 1, 7, 8, 9, 0, 0 };

	//M_ASSERT(nybble < U_NUMBER_OF(Offset));

	int offset = Offset[nybble];

	if (offset == 0)
	{
		switch (nybble)
		{
			case 5:	return Move::null();
			case 14:	return m_position.makeShortCastlingMove(from);
			case 15:	return m_position.makeLongCastlingMove(from);
		}
	}

	return m_position.makeKingMove(from, (from + offset) & 63);
}


inline
Move
Decoder::decodeQueen(sq::ID from, Byte nybble)
{
	if (nybble >= 8)					// rook-vertical move
		return m_position.makeQueenMove(from, sq::make(sq::fyle(from), sq::Rank(nybble & 7)));

	if (nybble != sq::fyle(from))	// rook-horizontal move
		return m_position.makeQueenMove(from, sq::make(sq::Fyle(nybble), sq::rank(from)));

	//M_ASSERT(m_strm.peek() > token::End_Marker);

	// diagonal move: coded in two bytes
	return m_position.makeQueenMove(from, (int(m_strm.get()) - 64) & 63);
}


inline
Move
Decoder::decodeRook(sq::ID from, Byte nybble)
{
	if (nybble >= 8)	// this is a move along a fyle, to a different rank
		return m_position.makeRookMove(from, sq::make(sq::fyle(from), nybble & 7));

	return m_position.makeRookMove(from, sq::make(nybble, sq::rank(from)));
}


inline
Move
Decoder::decodeBishop(sq::ID from, Byte nybble)
{
	int diff = int(nybble & 7) - int(sq::fyle(from));
	return m_position.makeBishopMove(from, (nybble < 8 ? int(from) + 9*diff : int(from) - 7*diff) & 63);
}


inline
Move
Decoder::decodeKnight(sq::ID from, Byte nybble)
{
	static int const Offset[16] = { 0, -17, -15, -10, -6, 6, 10, 15, 17, 0, 0, 0, 0, 0, 0, 0 };
	return m_position.makeKnightMove(from, (from + Offset[nybble]) & 63);
}


Move
Decoder::decodePawn(sq::ID from, Byte nybble)
{
	static int const Offset[16] = { 7,8,9, 7,8,9, 7,8,9, 7,8,9, 7,8,9, 16 };

	static piece::Type const PromotedPiece[16] =
	{
		piece::None,	piece::None,	piece::None,
		piece::Queen,	piece::Queen,	piece::Queen,
		piece::Rook,	piece::Rook,	piece::Rook,
		piece::Bishop,	piece::Bishop,	piece::Bishop,
		piece::Knight,	piece::Knight,	piece::Knight,
		piece::None,
	};

	int offset = Offset[nybble];

	if (m_position.whiteToMove())
	{
		Square to = (from + offset) & 63;

		switch (offset)
		{
			case 8:
				if (to >= sq::a8)
					return Move::genPromote(from, to, PromotedPiece[nybble]);

				return Move::genOneForward(from, to);

			case 16:
				return Move::genTwoForward(from, to);
		}

		if (to >= sq::a8)
			return Move::genCapturePromote(from, to, PromotedPiece[nybble], m_position.piece(to));

		if (to == m_position.board().enPassantSquare())
			return Move::genEnPassant(from, to);

		return Move::genPawnCapture(from, to, m_position.piece(to));
	}

	// black to move

	Square to = (from - offset) & 63;

	switch (offset)
	{
		case 8:
			if (to <= sq::h1)
				return Move::genPromote(from, to, PromotedPiece[nybble]);

			return Move::genOneForward(from, to);

		case 16:
			return Move::genTwoForward(from, to);
	}

	if (to <= sq::h1)
		return Move::genCapturePromote(from, to, PromotedPiece[nybble], m_position.piece(to));

	if (to == m_position.board().enPassantSquare())
		return Move::genEnPassant(from, to);

	return Move::genPawnCapture(from, to, m_position.piece(to));
}


unsigned
Decoder::decodeMove(Byte value, Move& move)
{
	//M_ASSERT(value > token::End_Marker);

	unsigned	pieceNum = (value >> 4);

	if (m_position.blackToMove())
		pieceNum |= 0x10;

	sq::ID from = sq::ID(m_position[pieceNum]);

	switch (m_position.piece(from))
	{
		case piece::King:		move = decodeKing(from, value & 15); break;
		case piece::Queen:	move = decodeQueen(from, value & 15); break;
		case piece::Rook:		move = decodeRook(from, value & 15); break;
		case piece::Bishop:	move = decodeBishop(from, value & 15); break;
		case piece::Knight:	move = decodeKnight(from, value & 15); break;
		case piece::Pawn:		move = decodePawn(from, value & 15); break;
		case piece::None:		::throwCorruptData();
	}

	Board& board = m_position.board();

	board.prepareUndo(move);
	move.setColor(board.sideToMove());
	//M_ASSERT(board.isValidMove(move));
	move.setLegalMove();
	board.doMove(move);

	return pieceNum;
}


void
Decoder::decodeVariation(ByteStream& data)
{
	unsigned	pieceNum	= 0;	// satisfies the compiler
	Move		move;

	while (true)
	{
		Byte b;

		while ((b = m_strm.get()) > token::End_Marker)
		{
			if (move)
				m_position.doMove(move, pieceNum);

			pieceNum = decodeMove(b, move);

			MoveNode* node = new MoveNode(move);
			m_currentNode->setNext(node);
			m_currentNode = node;
		}

		switch (b)
		{
			case token::End_Marker:
				{
					MoveNode* node = new MoveNode;
					m_currentNode->setNext(node);

					switch (m_strm.get())
					{
						case token::Comment:
							node->setCommentFlag(data.get());
							break;

						case token::End_Marker: break;
						default: return; //IO_RAISE(Game, Corrupted, "unexpected token");
					}
				}
				return;

			case token::Start_Marker:
				{
					MoveNode* current = m_currentNode;

					m_position.push();
					m_position.board().undoMove(move);
					current->addVariation(m_currentNode = new MoveNode);
					decodeVariation(data);
					m_currentNode = current;
					m_position.pop();
				}
				break;

			case token::Nag:
				static_assert(Annotation::Max_Nags >= 7, "Scidb needs at least seven entries");
				{
					nag::ID nag = nag::ID(m_strm.get());

					if (nag == 0)
						move.setLegalMove(false);
					else
						m_currentNode->addAnnotation(nag);
				}
				break;

			case token::Mark:
				if (data.peek() & 0x80)
				{
					MoveInfo info;
					info.decode(data);
					m_currentNode->addMoveInfo(info);
				}
				else
				{
					Mark mark;
					mark.decode(data);
					m_currentNode->addMark(mark);
				}
				break;

			case token::Comment:
				m_currentNode->setCommentFlag(data.get());
				break;
		}
	}
}


void
Decoder::decodeVariation(Consumer& consumer, util::ByteStream& data, ByteStream& text)
{
	MarkSet			marks;
	MoveInfoSet		moveInfo;
	Annotation		annotation;
	mstl::string	buf;
	Comment			comment;
	Comment			preComment;
	bool				hasNote(false);	// satisfies the compiler
	unsigned			pieceNum(0);		// satisfies the compiler
	Move				move;
	Move				lastMove;

	while (true)
	{
		Byte b = m_strm.get();

		if (__builtin_expect(b > token::Last, 1))
		{
			if (move)
			{
				if (hasNote)
				{
					consumer.putMove(move, annotation, preComment, comment, marks);
					if (!moveInfo.isEmpty())
					{
						consumer.putMoveInfo(moveInfo);
						moveInfo.clear();
					}
					marks.clear();
					annotation.clear();
					comment.clear();
					preComment.clear();
					hasNote = false;
				}
				else
				{
					consumer.putMove(move);
				}

				m_position.doMove(move, pieceNum);
			}
			else
			{
				if (lastMove)
				{
					m_position.doMove(lastMove, pieceNum);
					lastMove.clear();
				}

				if (hasNote)
				{
					consumer.putPrecedingComment(comment, annotation, marks);
					marks.clear();
					annotation.clear();
					comment.clear();
					hasNote = false;
				}
			}

			pieceNum = decodeMove(b, move);
		}
		else
		{
			switch (b)
			{
				case token::End_Marker:
					if (move)
					{
						if (hasNote)
						{
							consumer.putMove(move, annotation, preComment, comment, marks);
							if (!moveInfo.isEmpty())
								consumer.putMoveInfo(moveInfo);
						}
						else
						{
							consumer.putMove(move);
						}
					}
					else if (hasNote)
					{
						consumer.putPrecedingComment(comment, annotation, marks);
						hasNote = false;
					}
					switch (m_strm.get())
					{
						case token::Comment:
							{
								uint8_t flag = data.get();

								buf.clear();
								text.get(buf);
								comment.swap(buf, bool(flag & comm::Ante_Eng), bool(flag & comm::Ante_Oth));
								consumer.putTrailingComment(comment);
								break;
							}

						case token::End_Marker: break;
						default: return; //IO_RAISE(Game, Corrupted, "unexpected token");
					}
					return;

				case token::Start_Marker:
					if (move)
					{
						if (hasNote)
						{
							consumer.putMove(move, annotation, preComment, comment, marks);
							if (!moveInfo.isEmpty())
							{
								consumer.putMoveInfo(moveInfo);
								moveInfo.clear();
							}
							marks.clear();
							annotation.clear();
							comment.clear();
							preComment.clear();
							hasNote = false;
						}
						else
						{
							consumer.putMove(move);
						}

						lastMove = move;
						move.clear();
					}

					//M_ASSERT(!hasNote);

					m_position.push();
					m_position.board().undoMove(lastMove);
					consumer.startVariation();
					decodeVariation(consumer, data, text);
					consumer.finishVariation();
					m_position.pop();
					break;

				case token::Nag:
					{
						nag::ID nag = nag::ID(m_strm.get());

						if (nag == 0)
						{
							move.setLegalMove(false);
						}
						else
						{
							annotation.add(nag);
							hasNote = true;
						}
					}
					break;

				case token::Mark:
					if (data.peek() & 0x80)
						moveInfo.add().decode(data);
					else
						marks.add().decode(data);
					hasNote = true;
					break;

				case token::Comment:
					{
						uint8_t flag = data.get();

						if (flag & comm::Ante)
						{
							buf.clear();
							text.get(buf);
							preComment.swap(buf, bool(flag & comm::Ante_Eng), bool(flag & comm::Ante_Oth));
						}

						if (flag & comm::Post)
						{
							buf.clear();
							text.get(buf);
							comment.swap(buf, bool(flag & comm::Post_Eng), bool(flag & comm::Post_Oth));
						}

						hasNote = true;
					}
					break;
			}
		}
	}
}


void
Decoder::decodeRun(unsigned count)
{
	Move move;

	for ( ; count > 0; --count)
	{
		static_assert((1 << 16)/* Maximal Run Size */ <= Block_Size, "unsafeGet() is unsafe");

		// unsafeGet() is ok because the block file is buffered with doubled size
		m_position.doMove(move, decodeMove(m_strm.unsafeGet(), move));
		MoveNode* node = new MoveNode(move);
		m_currentNode->setNext(node);
		m_currentNode = node;
	}
}


void
Decoder::decodeRun(unsigned count, Consumer& consumer)
{
	Move move;

	for (unsigned i = 0; i < count; ++i)
	{
		static_assert((1 << 16)/* Maximal Run Size */ <= Block_Size, "unsafeGet() is unsafe");

		// unsafeGet() is ok because the block file is buffered with doubled size
		unsigned pieceNum = decodeMove(m_strm.unsafeGet(), move);
		m_position.doMove(move, pieceNum);
		consumer.putMove(move);
	}
}


void
Decoder::decodeTextSection(MoveNode* node, ByteStream& text)
{
	for ( ; node; node = node->next())
	{
		if (node->hasSupplement())
		{
			if (uint8_t flag = node->commentFlag())
			{
				mstl::string	buf;
				Comment			comment;

				if (flag & comm::Ante)
				{
					text.get(buf);
					comment.swap(buf, bool(flag & comm::Ante_Eng), bool(flag & comm::Ante_Oth));
					node->setComment(comment, move::Ante);
				}

				if (flag & comm::Post)
				{
					text.get(buf);
					comment.swap(buf,  bool(flag & comm::Post_Eng), bool(flag & comm::Post_Oth));
					node->setComment(comment, move::Post);
				}
			}

			for (unsigned i = 0; i < node->variationCount(); ++i)
				decodeTextSection(node->variation(i), text);
		}
	}
}


void
Decoder::decodeTags(ByteStream& strm, TagSet& tags)
{
	//M_ASSERT(strm.remaining());

	mstl::string name;
	mstl::string value;

	for (tag::ID id = tag::ID(strm.get()); id; id = tag::ID(strm.get()))
	{
		if (id == tag::ExtraTag)
		{
			name.clear();
			value.clear();
			strm.get(name);
			strm.get(value);
			tags.setExtra(name, value);
		}
		else
		{
			value.clear();
			strm.get(value);
			tags.set(id, value);

			if (tag::isRatingTag(id))
				tags.setSignificance(id, 0);
		}

#define SCI_TAGS_FIX
#ifdef SCI_TAGS_FIX
		if (strm.remaining() == 0)
			return;
#endif
	}
}


void
Decoder::decodeEngines(ByteStream& strm, EngineList& engines)
{
	uint8_t count = strm.get();

	engines.reserve(count);

	for (unsigned i = 0; i < count; ++i)
	{
		mstl::string engine;
		strm.get(engine);
		engines.addEngine(engine);
	}
}


save::State
Decoder::doDecoding(db::Consumer& consumer, TagSet& tags)
{
	uint16_t flags	= m_strm.uint16();
	uint16_t idn	= flags & 0x0fff;

	if (idn)
	{
		m_position.setup(idn);
	}
	else
	{
		mstl::string fen;
		m_strm.get(fen);
		m_position.setup(fen);
		tags.set(tag::SetUp, "1");	// bad PGN design
		tags.set(tag::Fen, fen);
	}

	if (!consumer.startGame(tags, m_position.board()))
		return save::UnsupportedVariant;

	Byte* dataSection = m_strm.base() + m_strm.uint24();
	ByteStream text, data;

	if (flags & 0x8000)
	{
		unsigned size = ByteStream::uint24(dataSection);

		text.setup(dataSection + 3, size);
		data.setup(text.end(), m_strm.end());
	}
	else
	{
		data.setup(dataSection, m_strm.end());
	}

	if (flags & 0x7000)
	{
		EngineList engines;
		decodeEngines(data, engines);
		consumer.swapEngines(engines);
	}

	consumer.startMoveSection();
	decodeRun(m_strm.uint16(), consumer);
	decodeVariation(consumer, data, text);
	consumer.finishMoveSection(result::fromString(tags.value(tag::Result)));

	if (data.remaining())
		decodeTags(data, tags);

	return consumer.finishGame(tags);
}


void
Decoder::doDecoding(GameData& gameData)
{
	uint16_t flags	= m_strm.uint16();
	uint16_t idn	= flags & 0x0fff;

	if (idn)
	{
		m_position.setup(idn);
	}
	else
	{
		mstl::string fen;
		m_strm.get(fen);
		m_position.setup(fen);
	}

	gameData.m_startBoard = m_position.board();
	m_currentNode = gameData.m_startNode;

	//M_ASSERT(m_currentNode);

	Byte* dataSection = m_strm.base() + m_strm.uint24();
	ByteStream data, text;

	if (flags & 0x8000)
	{
		unsigned size = ByteStream::uint24(dataSection);

		text.setup(dataSection + 3, size);
		data.setup(text.end(), m_strm.end());
	}
	else
	{
		data.setup(dataSection, m_strm.end());
	}

	decodeRun(m_strm.uint16());

	if (flags & 0x7000)
		decodeEngines(data, gameData.m_engines);

	decodeVariation(data);
	if (text.remaining())
		decodeTextSection(gameData.m_startNode, text);

	if (data.remaining())
		decodeTags(data, gameData.m_tags);
}


void
Decoder::skipVariations()
{
	unsigned count = 0;

	while (true)
	{
		Byte b = m_strm.get();

		if (__builtin_expect(b <= token::Last, 0))
		{
			switch (b)
			{
				case token::Start_Marker:
					++count;
					break;

				case token::End_Marker:
					m_strm.skip(1);

					if (count > 0)
						--count;
					else if (m_strm.peek() == token::Start_Marker)
						m_strm.skip(1);
					else
						return;
					break;

				case token::Nag:
					m_strm.skip(1);
					break;
			}
		}
	}
}


Move
Decoder::nextMove(unsigned runLength)
{
	Move move;

	if (runLength)
	{
		m_position.doMove(move, decodeMove(m_strm.get(), move));
	}
	else
	{
		while (true)
		{
			Byte b = m_strm.get();

			if (__builtin_expect(b > token::Last, 1))
			{
				m_position.doMove(move, decodeMove(b, move));
				return move;
			}
			else
			{
				switch (b)
				{
					case token::End_Marker:
						return Move::empty();

					case token::Start_Marker:
						skipVariations();
						break;

					case token::Nag:
						m_strm.skip(1);
						break;
				}
			}
		}
	}

	return move;
}


Move
Decoder::findExactPosition(Board const& position, bool skipVariations)
{
	uint16_t idn = m_strm.uint16() & 0x0fff;

	if (idn)
	{
		m_position.setup(idn);
	}
	else
	{
		mstl::string fen;
		m_strm.get(fen);
		m_position.setup(fen);
	}

	m_strm.skip(3);	// skip offset to text section

	unsigned runLength = m_strm.uint16();

	if (position.isEqualPosition(m_position.board()))
		return nextMove(runLength);

	Move move;

	for ( ; runLength > 0; --runLength)
	{
		static_assert((1 << 16)/* Maximal Run Size */ <= Block_Size, "unsafeGet() is unsafe");

		// unsafeGet() is ok because the block file is buffered with doubled size
		m_position.doMove(move, decodeMove(m_strm.unsafeGet(), move));

		if (position.isEqualPosition(m_position.board()))
			return nextMove(runLength - 1);

		if (!m_position.board().signature().isReachablePawns(position.signature()))
			return Move::invalid();
	}

	return searchForPosition(position, skipVariations);
}


Move
Decoder::searchForPosition(Board const& position, bool skipVariations)
{
	Move move;

	while (true)
	{
		Byte b = m_strm.get();

		if (__builtin_expect(b > token::Last, 1))
		{
			m_position.doMove(move, decodeMove(b, move));

			if (position.isEqualPosition(m_position.board()))
				return nextMove();

			if (!m_position.board().signature().isReachablePawns(position.signature()))
				return Move::invalid();
		}
		else
		{
			switch (b)
			{
				case token::End_Marker:
					return Move::invalid();

				case token::Start_Marker:
					if (skipVariations)
					{
						this->skipVariations();
					}
					else
					{
						m_position.push();
						m_position.board().undoMove(move);
						move = findExactPosition(position, false);
						m_position.pop();

						if (move)
							return move;

						m_strm.skip(1);
					}
					break;

				case token::Nag:
					m_strm.skip(1);
					break;
			}
		}
	}

	return move;	// never reached
}

// vi:set ts=3 sw=3:
