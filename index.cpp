#include <assert.h>
#include "index.h"

unsigned Choose(unsigned n, unsigned m)
{
	if (!m)
		return 1;
	if (!n)
		return 0;
	return (n * Choose(n - 1, m - 1)) / m;
}

unsigned ChoiceIndex::ISlotGet(unsigned iPiece) const
{
	return m_rgiSlot[iPiece];
}

bool ChoiceIndex::FISlotSet(unsigned iPiece, unsigned iSlot) 
{
	if (iSlot >= m_rgiSlot[iPiece + 1])
		return false;
	if(iPiece && iSlot <= m_rgiSlot[iPiece - 1])
		return false;
	m_rgiSlot[iPiece] = iSlot;
	return true;
}

ChoiceIndex& ChoiceIndex::operator++()
{
	for (unsigned ip = 0; ip < m_cPieces; ip++)
		if ((m_rgiSlot[ip + 1] - m_rgiSlot[ip]) > 1)
		{
			m_rgiSlot[ip]++;
			for (unsigned ip2 = ip; ip2--;)
				m_rgiSlot[ip2] = ip2;
			return *this;
		}
	m_rgiSlot[m_cPieces] = m_cSlots + 1;
	return *this;
}

bool ChoiceIndex::FEnd() const
{
	return m_rgiSlot[m_cPieces] > m_cSlots;
}

unsigned ChoiceIndex::LinearIndex() const
{
	unsigned result = 0;
	unsigned is = 0;
	for (unsigned ip = 0; ip < m_cPieces; ip++)
			result += Choose(m_rgiSlot[ip], ip + 1);
	return result;
}

void ChoiceIndex::Init(unsigned cPiecesUr, unsigned cSlots)
{
	m_cPieces = cPiecesUr;
	m_cSlots = cSlots;
	assert(m_cPieces <= 7);
	assert(m_cPieces <= cSlots);
	for (unsigned i = m_cPieces; i--;)
		m_rgiSlot[i] = i;
	m_rgiSlot[m_cPieces] = m_cSlots;
}

PositionIndex::PositionIndex(const UrState& us)
{
	Init(cPiecesUr - us.GetHome(upWhite), cPiecesUr - us.GetHome(upBlack));
	unsigned col, row, iPiece, iSlot;
	unsigned rgSlot[7];
	iPiece = 0;
	iSlot = 0;
	for (row = 0; row < 8; row++)
	{
		if (row > 1 && row < 4)
			continue;
		if (us.Get(2, row) == upBlack)
		{
			rgSlot[iPiece++] = iSlot;
		}
		iSlot++;
	}
	m_cColumn2Black = iPiece;
	m_ciBlackColumn2.Init(m_cColumn2Black, 6);
	for (iPiece = m_cColumn2Black; iPiece--;)
		if (!m_ciBlackColumn2.FISlotSet(iPiece, rgSlot[iPiece]))
			assert(false);
	iPiece = 0;
	iSlot = 0;
	for (row = 0; row < 8; row++)
	{
		if (us.Get(1, row) == upBlack)
		{
			rgSlot[iPiece++] = iSlot;
		}
		iSlot++;
	}
	m_cColumn1Black = iPiece;
	m_ciBlackColumn1.Init(m_cColumn1Black, 8);
	for (iPiece = m_cColumn1Black; iPiece--;)
		if (!m_ciBlackColumn1.FISlotSet(iPiece, rgSlot[iPiece]))
			assert(false);
	iPiece = 0;
	iSlot = 0;
	for(col = 0; col < 2; col++)
		for (row = 0; row < 8; row++)
		{
			if (col == 0 && row > 1 && row < 4)
				continue;
			switch (us.Get(col, row))
			{
			case upWhite:
				rgSlot[iPiece++] = iSlot;
			case upNil:
				iSlot++;
				break;
			default:
				break;
			}
		}
	

	m_cOnBoardWhite = iPiece;
	m_ciWhite.Init(m_cOnBoardWhite, iSlot);
	for (iPiece = m_cOnBoardWhite; iPiece--;)
		if (!m_ciWhite.FISlotSet(iPiece, rgSlot[iPiece]))
			assert(false);
}

PositionIndex& PositionIndex::operator++()
{
	++m_ciWhite;
	if (m_ciWhite.FEnd())
	{
		++m_ciBlackColumn1;
		if (m_ciBlackColumn1.FEnd())
		{
			++m_ciBlackColumn2;
			if (m_ciBlackColumn2.FEnd())
			{
				m_cColumn2Black++;
				if ((m_cColumn2Black > (m_cPiecesBlack - m_cColumn1Black)) || (m_cColumn2Black > 6))
				{
					m_cColumn1Black++;
					if (m_cColumn1Black > m_cPiecesBlack)
					{
						m_cOnBoardWhite++;
						if (FEnd())
							return *this;
						m_cColumn1Black = 0;
					}
					m_cColumn2Black = 0;
				}
				m_ciBlackColumn2.Init(m_cColumn2Black, 6);
			}
			m_ciBlackColumn1.Init(m_cColumn1Black, 8);
		}
		m_ciWhite.Init(m_cOnBoardWhite, 14 - m_cColumn1Black);
	}
	return *this;
}

void PositionIndex::Init(unsigned cPiecesWhite, unsigned cPiecesBlack)
{
	m_cPiecesWhite = cPiecesWhite;
	m_cPiecesBlack = cPiecesBlack;
	m_ciWhite.Init(0, 14);
	m_ciBlackColumn1.Init(0, 8);
	m_ciBlackColumn2.Init(0, 6);
	m_cOnBoardWhite = 0;
	m_cColumn1Black = 0;
	m_cColumn2Black = 0;
}

bool PositionIndex::FEnd() const
{
	return m_cOnBoardWhite > m_cPiecesWhite;
}

unsigned PositionIndex::LinearIndex() const
{
	unsigned result = 0;
	unsigned cobw, cbc1, cbc2;
	for (cobw = 0; cobw < m_cOnBoardWhite; cobw++)
		for (cbc1 = 0; cbc1 <= m_cPiecesBlack; cbc1++)
			for(cbc2 = 0; (cbc2 <= (m_cPiecesBlack - cbc1)) && (cbc2 < 7); cbc2++)
				result += ChoiceIndex::CConfig(cobw, 14 - cbc1) * ChoiceIndex::CConfig(cbc1, 8) * ChoiceIndex::CConfig(cbc2, 6);
	for (cbc1 = 0; cbc1 < m_cColumn1Black; cbc1++)
		for (cbc2 = 0; (cbc2 <= (m_cPiecesBlack - cbc1)) && (cbc2 < 7); cbc2++)
			result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - cbc1) * ChoiceIndex::CConfig(cbc1, 8) * ChoiceIndex::CConfig(cbc2, 6);
	for (cbc2 = 0; cbc2 < m_cColumn2Black; cbc2++)
		result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - m_cColumn1Black) * ChoiceIndex::CConfig(m_cColumn1Black, 8) * ChoiceIndex::CConfig(cbc2, 6);
	result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - m_cColumn1Black) * ChoiceIndex::CConfig(m_cColumn1Black, 8) * m_ciBlackColumn2.LinearIndex();
	result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - m_cColumn1Black) * m_ciBlackColumn1.LinearIndex();
	result += m_ciWhite.LinearIndex();

	return result;
}

const unsigned rguLog2[] = {
	0, 0, 1, 2, 2, 3, 3, 3
};

unsigned PositionIndex::BitIndex() const
{
	if (m_cOnBoardWhite < 1 || m_cPiecesWhite < 2)
		return 0;
	unsigned result = 0;
	unsigned cbc1, cbc2, cBitsPerPosition;
	for (unsigned cobw = 1; cobw < m_cOnBoardWhite; cobw++)
	{
		cBitsPerPosition = rguLog2[cobw + 1] * 4;
		for (cbc1 = 0; cbc1 <= m_cPiecesBlack; cbc1++)
			for (cbc2 = 0; (cbc2 <= (m_cPiecesBlack - cbc1)) && (cbc2 < 7); cbc2++)
				result += ChoiceIndex::CConfig(cobw, 14 - cbc1) * ChoiceIndex::CConfig(cbc1, 8) * ChoiceIndex::CConfig(cbc2, 6) * cBitsPerPosition;
	}
	cBitsPerPosition = rguLog2[(m_cOnBoardWhite == m_cPiecesWhite) ? m_cOnBoardWhite : m_cOnBoardWhite + 1] * 4;
	for (cbc1 = 0; cbc1 < m_cColumn1Black; cbc1++)
		for (cbc2 = 0; (cbc2 <= (m_cPiecesBlack - cbc1)) && (cbc2 < 7); cbc2++)
			result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - cbc1) * ChoiceIndex::CConfig(cbc1, 8) * ChoiceIndex::CConfig(cbc2, 6) * cBitsPerPosition;
	for (cbc2 = 0; cbc2 < m_cColumn2Black; cbc2++)
		result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - m_cColumn1Black) * ChoiceIndex::CConfig(m_cColumn1Black, 8) * ChoiceIndex::CConfig(cbc2, 6) * cBitsPerPosition;
	result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - m_cColumn1Black) * ChoiceIndex::CConfig(m_cColumn1Black, 8) * m_ciBlackColumn2.LinearIndex() * cBitsPerPosition;
	result += ChoiceIndex::CConfig(m_cOnBoardWhite, 14 - m_cColumn1Black) * m_ciBlackColumn1.LinearIndex() * cBitsPerPosition;
	result += m_ciWhite.LinearIndex() * cBitsPerPosition;

	return result;
}

unsigned PositionIndex::CConfig(unsigned cPiecesWhite, unsigned cPiecesBlack)
{
	unsigned result = 0;
	unsigned cobw, cbc1, cbc2;
	for (cobw = 0; cobw <= cPiecesWhite; cobw++)
		for (cbc1 = 0; cbc1 <= cPiecesBlack; cbc1++)
			for (cbc2 = 0; (cbc2 <= (cPiecesBlack - cbc1)) && (cbc2 < 7); cbc2++)
				result += ChoiceIndex::CConfig(cobw, 14 - cbc1) * ChoiceIndex::CConfig(cbc1, 8) * ChoiceIndex::CConfig(cbc2, 6);
	return result;
}

unsigned PositionIndex::CBitsInPage(unsigned cPiecesWhite, unsigned cPiecesBlack)
{
	unsigned result = 0;
	unsigned cobw, cbc1, cbc2;
	if (cPiecesWhite < 2)
		return 0;
	for (cobw = 1; cobw <= cPiecesWhite; cobw++)
	{
		unsigned cBitsPerPosition = rguLog2[(cobw == cPiecesWhite) ? cobw : cobw + 1] * 4;
		for (cbc1 = 0; cbc1 <= cPiecesBlack; cbc1++)
			for (cbc2 = 0; (cbc2 <= (cPiecesBlack - cbc1)) && (cbc2 < 7); cbc2++)
				result += ChoiceIndex::CConfig(cobw, 14 - cbc1) * ChoiceIndex::CConfig(cbc1, 8) * ChoiceIndex::CConfig(cbc2, 6) * cBitsPerPosition;
	}
	return result;
}

unsigned PositionIndex::CBitsPerMove() const
{
	return rguLog2[(m_cOnBoardWhite == m_cPiecesWhite) ? m_cOnBoardWhite : m_cOnBoardWhite + 1];
}

PositionIndex::operator UrState() const
{
	unsigned rgRow[14];
	unsigned rgCol[14];
	UrState	us;
	us.SetHome(upWhite, cPiecesUr - m_cPiecesWhite);
	us.SetHome(upBlack, cPiecesUr - m_cPiecesBlack);
	unsigned col, row, iPiece, iSlot;

	for (iPiece = m_cColumn2Black; iPiece--;)
	{
		row = m_ciBlackColumn2.ISlotGet(iPiece);
		if (row > 1)
			row += 2;
		us.Set(2, row, upBlack);
	}

	for (iPiece = m_cColumn1Black; iPiece--;)
		us.Set(1, m_ciBlackColumn1.ISlotGet(iPiece), upBlack);

	iSlot = 0;
	unsigned iSlotTarget = m_ciWhite.ISlotGet(m_cOnBoardWhite - 1);
	for(col = 0; col < 2 && iSlot <= iSlotTarget; col++)
		for (row = 0; row < 8 && iSlot <= iSlotTarget; row++)
		{
			switch (us.s_rgst[col][row]) {
			case us.stNormal:
			case us.stDouble:
			case us.stDoubleSafe:
				if (us.Get(col, row) == upNil)
				{
					rgRow[iSlot] = row;
					rgCol[iSlot] = col;
					iSlot++;
				}
				break;
			default:
				break;
			}
		}
	assert(m_cOnBoardWhite == m_ciWhite.CPieces());
	for (iPiece = m_cOnBoardWhite; iPiece--;)
		us.Set(rgCol[m_ciWhite.ISlotGet(iPiece)], rgRow[m_ciWhite.ISlotGet(iPiece)], upWhite);
	us.Set(0, 3, (m_cOnBoardWhite < m_cPiecesWhite) ? upWhite : upNil);
	us.Set(2, 3, (m_cPiecesBlack - m_cColumn1Black - m_cColumn2Black) ? upBlack : upNil);
	return us;
}

bool UrMoveManager::FInit(const char* szFileName, const UrStats* purs)
{
	if (!purs)
		return m_bm.FInit(szFileName, cPiecesUr * cPiecesUr, false);
	if(!m_bm.FInit(szFileName, cPiecesUr * cPiecesUr, true))
		return false;
	return FGenerateMoves(purs);
}

void UrMoveManager::BestMove(const UrState& us, unsigned& col, unsigned& row, unsigned die) const
{
	PositionIndex	pi(us);
	unsigned cBitsMove = pi.CBitsPerMove();
	unsigned uMove = 0;
	if (cBitsMove)
	{
		SetCurrentPage(us.GetHome(upWhite), us.GetHome(upBlack));
		uMove = m_bb.GetBits(pi.BitIndex() + (die - 1) * cBitsMove, cBitsMove);
	}
	MoveFromNumber(us, uMove, col, row);
}

void UrMoveManager::MoveFromNumber(const UrState& us, unsigned uMove, unsigned& col, unsigned& row) const
{
	if (us.GetIdle(upWhite))
	{
		if (!uMove)
		{
			col = 0;
			row = 3;
			return;
		}
		else
			uMove--;
	}
	for (unsigned lcol = 0; lcol < 2; lcol++)
		for (unsigned lrow = 0; lrow < 8; lrow++)
		{
			if (lcol == 0 && lrow > 1 && lrow < 4)
				continue;
			if (us.Get(lcol, lrow) == upWhite)
				if (!uMove--) {
					col = lcol;
					row = lrow;
					return;
				}
		}
}

unsigned UrMoveManager::BestMoves(const GameIndex& gi) const
{
	SetCurrentPage(gi.m_cHomeWhite, gi.m_cHomeBlack);
	unsigned cBitsMove = gi.CBitsPerMove();
	return m_bb.GetBits(gi.m_iBit, cBitsMove * 4);
}

bool UrMoveManager::FGenerateMoves(const UrStats* purs)
{
	PositionIndex pi;

	for (unsigned cHomeWhite = cPiecesUr; cHomeWhite--;)
		for (unsigned cHomeBlack = cPiecesUr; cHomeBlack--;)
		{
//			pi.Init(cPiecesUr - cHomeWhite, cPiecesUr - cHomeBlack);
			unsigned cBitsPage = pi.CBitsInPage();
#ifdef _DEBUG
			unsigned uCount = 0;
#else
			if (!cBitsPage)
				continue;
#endif
			SetCurrentPage(cHomeWhite, cHomeBlack);
			unsigned iBit = 0;
			for(pi.Init(cPiecesUr - cHomeWhite, cPiecesUr - cHomeBlack); !pi.FEnd(); ++pi)
			{
#ifdef _DEBUG
				uCount++;
#endif
				unsigned cBitsMove = pi.CBitsPerMove();
				if (cBitsMove)
				{
					UrState	us = UrState(pi);
					unsigned uMoves = 0;
					for (unsigned die = 1; die < 5; die++)
					{
						unsigned colMove, rowMove, uMoveNumber;
						bool fRollAgain;
						purs->BestMove(us, colMove, rowMove, die, fRollAgain);
						if (colMove == 0 && rowMove == 3)
							uMoveNumber = 0;
						else
						{
							uMoveNumber = pi.FWhitePieceAtStart() ? 1 : 0;
							for (unsigned col = 0; col <= colMove; col++)
								for (unsigned row = 0; (col < colMove && row < 8) || row < rowMove; row++)
								{
									if (col == 0 && row > 1 && row < 4)
										continue;
									if (us.Get(col, row) == upWhite)
										uMoveNumber++;
								}
						}
						assert(!(uMoveNumber >> cBitsMove));
						uMoves = (uMoves << cBitsMove) | uMoveNumber;
					}
					m_bb.SetBits(iBit, cBitsMove * 4, uMoves);
#ifdef _DEBUG
					if (iBit != pi.BitIndex())
						std::cout << "\niBit = " << iBit << " BitIndex = " << pi.BitIndex() << std::endl << us;
					for (unsigned nibble = 4; nibble--; )
					{
						if (m_bb.GetBits(iBit + nibble * cBitsMove, cBitsMove) != (uMoves & ((1 << cBitsMove) - 1)))
							std::cout << "\n Readback error iBit = " << iBit << " nibble = " << nibble << " uMoves = " << uMoves;
						uMoves >>= cBitsMove;
					}

#endif
					iBit += cBitsMove * 4;
				}
			}
#ifdef _DEBUG
			assert(uCount == PositionIndex::CConfig(cPiecesUr - cHomeWhite, cPiecesUr - cHomeBlack));
			assert(iBit == cBitsPage);
#endif
			if (cBitsPage)
				m_bm.FlushBlock(BlockManager::RecordType::PackedMoves, cPiecesUr * cHomeWhite + cHomeBlack, m_bb.PvData(), m_bb.CbData());
		}
	return true;
}

void UrMoveManager::SetCurrentPage(unsigned cWhiteHome, unsigned cBlackHome) const
{
	unsigned iPage = cPiecesUr * cWhiteHome + cBlackHome;
	if (iPage == m_iPageCurrent)
		return;
	unsigned cBitsPage = PositionIndex::CBitsInPage(cPiecesUr - cWhiteHome, cPiecesUr - cBlackHome);
	if (cBitsPage)
	{
		m_bb.Init(cBitsPage);
		unsigned cb = m_bm.CbReadBlock(BlockManager::RecordType::PackedMoves, iPage, m_bb.PvData());
		assert(!cb || cb == m_bb.CbData());
		m_iPageCurrent = iPage;
	}
}

void BitBucket::Init(unsigned cBits)
{
	m_cBits = cBits;
	m_rguData = std::unique_ptr<unsigned[]>(new unsigned[(cBits - 1) / (8 * sizeof(unsigned)) + 1]);
}

static const unsigned cBitsPerLong = sizeof(unsigned) * 8;
void BitBucket::SetBits(unsigned iBitStart, unsigned cBits, unsigned val)
{
	while (cBits)
	{
		unsigned mask = (1 << cBits) - 1;
		assert(val <= mask);
		unsigned u = m_rguData[iBitStart / cBitsPerLong];
		int shift = cBitsPerLong - iBitStart % cBitsPerLong - cBits;
		if (shift >= 0)
		{
			mask <<= shift;
			val <<= shift;
			m_rguData[iBitStart / cBitsPerLong] = (u & ~mask) | val;
			cBits = 0;
		}
		else
		{
			mask >>= -shift;
			m_rguData[iBitStart / cBitsPerLong] = (u & ~mask) | (val >> -shift);
			iBitStart += cBits + shift;
			cBits = -shift;
			val &= (1 << -shift) - 1;
		}
	}
}

unsigned BitBucket::GetBits(unsigned iBitStart, unsigned cBits) const
{
	unsigned result = 0;
	while (cBits)
	{
		unsigned mask = (1 << cBits) - 1;
		unsigned u = m_rguData[iBitStart / cBitsPerLong];
		int shift = cBitsPerLong - iBitStart % cBitsPerLong - cBits;
		if (shift >= 0)
		{
			u >>= shift;
			result |= u & mask;
			cBits = 0;
		}
		else
		{
			mask >>= -shift;
			result |= (u & mask) << -shift;
			iBitStart += cBits + shift;
			assert(!(iBitStart % cBitsPerLong));
			cBits = -shift;
		}
	}
	return result;
}

GameIndex& GameIndex::operator++()
{
	if (FEnd())
		return *this;
	m_iBit += m_pi.CBitsPerMove();
	++m_pi;
	if (m_pi.FEnd()) {
		m_cHomeBlack++;
		if (m_cHomeBlack >= cPiecesUr) {
			m_cHomeWhite++;
			if (FEnd())
				return *this;
			m_cHomeBlack = 0;
		}
		m_pi.Init(cPiecesUr - m_cHomeWhite, cPiecesUr - m_cHomeBlack);
		m_iBit = 0;
	}
	return *this;
}

bool GameIndex::FEnd() const
{
	return m_cHomeWhite >= (cPiecesUr - 1);
}
