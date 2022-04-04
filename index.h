// index.h
// Copyright (c) 2022 Arthur Champernowne
// email: champernowne@hotmail.com
// 

#pragma once
#include "Ur.h"
unsigned Choose(unsigned n, unsigned m);

class ChoiceIndex
{
public:
	unsigned ISlotGet(unsigned iPiece) const;
	bool FISlotSet(unsigned iPiece, unsigned iSlot);
	ChoiceIndex& operator ++();
	void Init(unsigned cPieces, unsigned cSlots);
	unsigned CSlots() const { return m_cSlots; }
	unsigned CPieces() const { return m_cPieces; }
	bool FEnd() const;
	unsigned CConfig() const { return CConfig(m_cPieces, m_cSlots); }
	unsigned LinearIndex() const;
	static unsigned CConfig(unsigned cPieces, unsigned cSlots) { return Choose(cSlots, cPieces); }
protected:
	unsigned m_cPieces;
	unsigned m_cSlots;
	unsigned m_rgiSlot[8];
};

class BitBucket
{
public:
	void Init(unsigned cBits);
	void SetBits(unsigned iBitStart, unsigned cBits, unsigned val);
	unsigned GetBits(unsigned iBitStart, unsigned cBits) const;
	void* PvData() {return m_rguData.get();	}
	unsigned CbData() const { return (m_cBits - 1) / 8 + 1; }
protected:
	unsigned m_cBits = 0;
	std::unique_ptr<unsigned[]>	m_rguData;
};

class PositionIndex
{
public:
	PositionIndex() { Init(0, 0); };
	PositionIndex(const UrState& us);
	PositionIndex& operator ++();
	void Init(unsigned cPiecesWhite, unsigned cPiecesBlack);
	bool FEnd() const;
	unsigned LinearIndex() const;
	unsigned BitIndex() const;
	static unsigned CConfig(unsigned cPiecesWhite, unsigned cPiecesBlack);
	static unsigned CBitsInPage(unsigned cPiecesWhite, unsigned cPiecesBlack);
	unsigned CBitsPerMove() const;
	unsigned CBitsInPage() const { return CBitsInPage(m_cPiecesWhite, m_cPiecesBlack); }
	bool FWhitePieceAtStart() const { return !!(m_cPiecesWhite - m_cOnBoardWhite); }
	operator UrState() const;
protected:
	unsigned	m_cPiecesWhite;
	unsigned	m_cPiecesBlack;
	unsigned	m_cOnBoardWhite;
	unsigned	m_cColumn1Black;
	unsigned	m_cColumn2Black;
	ChoiceIndex	m_ciWhite;
	ChoiceIndex m_ciBlackColumn1;
	ChoiceIndex m_ciBlackColumn2;
};

class GameIndex
{
public:
	GameIndex() {
		m_pi.Init(cPiecesUr - m_cHomeWhite, cPiecesUr - m_cHomeBlack);
	}
	GameIndex& operator ++();
	bool FEnd() const;
	unsigned CBitsPerMove() const { return m_pi.CBitsPerMove(); }
	unsigned m_cHomeWhite = 0;
	unsigned m_cHomeBlack = 0;
	unsigned m_iBit = 0;
	PositionIndex m_pi;
};

class UrMoveManager
{
public:
	bool FInit(const char* szFileName, const UrStats* purs);
	void BestMove(const UrState& us, unsigned& col, unsigned& row, unsigned die) const;
	void MoveFromNumber(const UrState& us, unsigned iMove, unsigned& col, unsigned& row) const;
	unsigned BestMoves(const GameIndex& gi) const;
protected:
	mutable unsigned m_iPageCurrent = 0xffffffff;
	mutable BitBucket	m_bb;
	BlockManager m_bm;
	bool FGenerateMoves(const UrStats* purs);
	void SetCurrentPage(unsigned cWhiteHome, unsigned cBlackHome) const;
};