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

#ifndef _db_guess_included
#define _db_guess_included

#include "db_board.h"

// null move search is probably not recommendable
//#define USE_NULL_MOVE_SEARCH

namespace db {

class Board;
class MoveList;

class Guess : public Board
{
public:

	enum { MaxDepth = 7, DefaultDepth = 3 };
	enum { IdnStandard = chess960::StandardIdn };

	Guess(Board const& board, unsigned idn);
	~Guess() throw();

	Move search(Square square, unsigned maxDepth = DefaultDepth);
	Move bestMove(Square square, unsigned maxDepth = DefaultDepth);
	Move bestMove(	Square square,
						MoveList const& exclude,
						unsigned maxDepth = DefaultDepth);
	Square bestSquare(Square square, unsigned maxDepth = DefaultDepth);

	struct Score
	{
		Score();
		Score(int score);
		Score(int middleGameScore, int endGameScore);

		Score operator/(int n) const;
		Score operator*(int n) const;
		Score operator+(int n) const;
		Score operator-(int n) const;
		Score operator-() const;

		Score& operator+=(int score);
		Score& operator-=(int score);
		Score& operator+=(Score const& score);
		Score& operator-=(Score const& score);

		int weightedScore(int totalPiecesWhite, int totalPiecesBlack) const;

		int middleGame;
		int endGame;
	};

	class Transposition;

private:

	enum { PawnTableSize = 512 };

	enum WinningChances
	{
		NeitherSideCanWin,	// neither side can win, this is a dead drawn position
		OnlyWhiteCanWin,		// white can win, black can not win
		OnlyBlackCanWin,		// white can not win, black can win
		BothSidesCanWin,		// both white and black can win
	};

#if 0
	enum Recognized
	{
		RecogDraw,
		RecogWin,
		RecogUnknown,
	};
#endif

	typedef sq::ID (*Flip)(sq::ID);

	static int const Infinity		= 32000;
	static int const KingValue		= 32767;
	static int const QueenValue	=   970;
	static int const RookValue		=   500;
	static int const BishopValue	=   325;
	static int const KnightValue	=   300;
	static int const PawnValue		=   100;

	void generateMoves(Square square, MoveList& result) const;

	Move search(MoveList& moves, unsigned maxDepth);
	int quiesce(int alpha, int beta, bool isPromotion);
	int quiesce(int alpha, int beta);

#ifdef USE_NULL_MOVE_SEARCH
	int search(MoveList& moves, unsigned depth, int alpha, int beta, bool allowNull);
	int iterate(MoveList& moves, unsigned depth, int alpha, int beta, bool allowNull);
#else
	int search(MoveList& moves, unsigned depth, int alpha, int beta);
	int iterate(MoveList& moves, unsigned depth, int alpha, int beta);
#endif
	int iterate(MoveList& moves, int alpha, int beta);

	void addKillerMove(Move const& move, int score);
	bool isKillerMove(uint32_t move, unsigned index);

	void preEvaluate();
	int evaluate(int alpha, int beta);
	Score evaluateMaterial();
	int evaluateMaterialDynamic(color::ID side);
	WinningChances evaluateWinningChances();
	bool evaluateWinningChances(color::ID side);
	int evaluateDraws(WinningChances canWin, int score);
	int evaluateMate(color::ID side);
	int evaluateDevelopment(color::ID side);
	Score evaluateKings(color::ID side, Flip flip);
	Score evaluateKnights(color::ID side, Flip flip);
	Score evaluateBishops(color::ID side, Flip flip);
	Score evaluateRooks(color::ID side, Flip flip);
	Score evaluateQueens(color::ID side, Flip flip);
	Score evaluatePawns(color::ID side);
	int evaluateKingsFyle(color::ID side, int whichFyle);
	Score evaluateWeakPawns(color::ID side, int square, uint64_t pawnMoves);
	Score evaluatePassedPawns(color::ID side, Flip flip);
	Score evaluatePassedPawnRaces();
	bool doScorePieces(Score score, int alpha, int beta) const;

	int staticExchangeEvaluator(Move const& move);
	uint64_t addXrayPiece(unsigned from, unsigned target);

#if 0
	Recognized recognize();
	Recognized recogMateInCorner();
	Recognized recogKPK(color::ID side);
	Recognized recogKBPK(color::ID side);
	Recognized recogKQKP(color::ID side);
#endif

	Move setColor(Move move);

	static unsigned minor(Material mat);
	static unsigned major(Material mat);
	static unsigned total(Material mat);

	struct PawnEval
	{
		int8_t	defects[8];
		int8_t	longVsShortScore;
		int8_t	shortVsLongScore;
		uint8_t	openFyle;
		uint8_t	passedPawn;
		uint8_t	candidates;
		uint8_t	all;
	};

	typedef PawnEval Eval[2];

	struct PawnHashEntry
	{
		uint64_t	key;
		Score		score;
		Eval		eval;
	};

	struct Root
	{
		Byte m_castle;
		Byte m_stm;

		castling::Rights castlingRights(color::ID side) const;
		bool canCastle(color::ID side) const;
		color::ID sideToMove() const;
	};

	static int const Piece[8];

	int				m_idn;
	int				m_totalPieces[2];
	int				m_tropism[2];
	int				m_majors;
	int				m_minors;
	PawnHashEntry*	m_pawnData;
	PawnHashEntry	m_pawnTable[PawnTableSize];
	bool				m_dangerous[2];
	bool				m_trojanCheck;
	Root				m_root;
	Transposition*	m_trans;
	uint32_t			m_killer[(2*MaxDepth + 2)*2];
	unsigned			m_ply;
#ifdef USE_NULL_MOVE_SEARCH
	int				m_pvCounter;
#endif
};

Guess::Score operator*(int n, Guess::Score const& score);
Guess::Score operator+(int n, Guess::Score const& score);
Guess::Score operator-(int n, Guess::Score const& score);

} // namespace db

#include "db_guess.ipp"

#endif // _db_guess_included

// vi:set ts=3 sw=3:
