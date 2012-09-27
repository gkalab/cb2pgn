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

namespace db {

inline Guess::Score::Score() :middleGame(0), endGame(0) {}
inline Guess::Score::Score(int score) :middleGame(score), endGame(score) {}

inline Guess::Score Guess::Score::operator/(int n) const	{ return Score(middleGame/n, endGame/n); }
inline Guess::Score Guess::Score::operator*(int n) const	{ return Score(middleGame*n, endGame*n); }
inline Guess::Score Guess::Score::operator+(int n) const	{ return Score(middleGame + n, endGame + n); }
inline Guess::Score Guess::Score::operator-(int n) const	{ return Score(middleGame - n, endGame - n); }
inline Guess::Score Guess::Score::operator-() const		{ return Score(-middleGame, -endGame); }

inline Guess::Score operator*(int n, Guess::Score const& score) { return score * n; }
inline Guess::Score operator+(int n, Guess::Score const& score) { return score + n; }
inline Guess::Score operator-(int n, Guess::Score const& score) { return score - n; }

inline color::ID Guess::Root::sideToMove() const { return color::ID(m_stm); }


inline
Guess::Score::Score(int middleGameScore, int endGameScore)
	:middleGame(middleGameScore)
	,endGame(endGameScore)
{
}


inline
castling::Rights
Guess::Root::castlingRights(color::ID side) const
{
	return castling::Rights(m_castle & castling::bothSides(side));
}


inline bool Guess::Root::canCastle(color::ID side) const	{ return castlingRights(side); }

} // namespace db

// vi:set ts=3 sw=3:
