// Ur.h
// Copyright (c) 2022 Arthur Champernowne
// email: champernowne@hotmail.com
// 

#pragma once
#include <iostream>
#include <assert.h>
#include <functional>


enum UrPlayer { upNil = 1, upWhite = 2, upBlack = 3, upIllegal};
const unsigned cPiecesUr = 7;
const unsigned cPages = cPiecesUr * cPiecesUr;
const unsigned cBoardsPerPage = 3 * 3 * 3 * 3 * 3 * 3 * 3 * 3 * 4 * 4 * 4 * 4 * 4 * 4;
const unsigned maxState = cBoardsPerPage * cPages + 2;
const unsigned uWhiteWin = maxState - 2;
const unsigned uBlackWin = maxState - 1;

class UrState
{
public:
	UrState(unsigned u = 0);
	UrState(const UrState& us, bool fReverse = false);
	void Set(unsigned col, unsigned row, UrPlayer up) { if (col < 3 && row < 8) m_rgup[col][row] = up; }
	void SetHome(UrPlayer up, unsigned cHome) { if (up == upWhite) m_rgcHome[0] = cHome; else if (up == upBlack) m_rgcHome[1] = cHome; }
	UrPlayer Get(unsigned col, unsigned row) const { if (col < 3 && row < 8) return m_rgup[col][row]; else return upIllegal; }
	unsigned GetHome(UrPlayer up) const { return (up == upWhite) ? m_rgcHome[0] : m_rgcHome[1]; }
	int GetIdle(UrPlayer up) const;
	unsigned Move(unsigned col, unsigned row, unsigned die, bool& fRollAgain) const;
	unsigned Pack() const;
	bool	FValid() const { return(m_fValid); }
	unsigned IPage() const {return m_rgcHome[0] * cPiecesUr + m_rgcHome[1];}
	friend std::ostream& operator << (std::ostream& s, const UrState& us);
	enum SquareType { stNormal = 0, stDouble = 1, stDoubleSafe = 3, stHome = 4, stStart = 8 };
	static const SquareType	s_rgst[3][8];
protected:
	static void	PackField(unsigned value, const unsigned min, const unsigned max, unsigned & sofar) { 
		assert(min < max);
		sofar = sofar * (max - min) + value - min;
	}
	static unsigned	UnPackField(unsigned min, unsigned max, unsigned& sofar) { 
		assert(min < max);
		unsigned result = min + sofar % (max - min);
		sofar /= max - min; 
		return result; 
	}
	UrPlayer	m_rgup[3][8] = { {upNil} };
	unsigned	m_rgcHome[2] = { 0 };
	bool	m_fValid = false;
};

class UrStats
{
public:
	typedef double winp;	// defines the precision of the statistics
	UrStats();
	bool FInit(const char* szFile1, const char* szFile2, bool fMisere = false);
	unsigned BestMove(const UrState& us, unsigned& col, unsigned& row, unsigned die, bool& fRollAgain) const;
	winp PWhiteWin(unsigned index);
	const class UrMoveManager* PUmm() const { return m_purmm.get(); }
protected:
	bool m_fMisere = false;
	unsigned m_cflPage;
	unsigned m_cflAll;
	bool	m_fUseMoveManager = false;
	std::unique_ptr<class UrPageManager>	m_purpm;
	std::unique_ptr<class UrMoveManager>	m_purmm;
	bool FGenerateStats();
};

class BlockManager
{
public:
	~BlockManager() {if(m_pf) fclose(m_pf);}
	bool FInit(const char* szFile, unsigned cBlocks, bool fNew);
	enum RecordType { Empty = 0, FileHeader, ValidityBits, PackedData, PackedMoves};
	void FlushBlock(RecordType rt, unsigned iPage, void * pv, unsigned cb);
	unsigned CbReadBlock(RecordType rt, unsigned iPage, void* pv) const;
protected:
	struct BlockRecord {
		fpos_t m_oRecordStart;
		unsigned __int64 m_u64Check;
		RecordType	m_rt;
		unsigned m_iPage;
		unsigned m_cbRecord;
	};
	int SeekBlock(RecordType rt = Empty, unsigned iPage = 0) const {
		for (unsigned i = m_cBlocks; i--;)
			if (m_rgbr[i].m_rt == rt && m_rgbr[i].m_iPage == iPage)
				return i;
		return -1;
	}
	FILE* m_pf = nullptr;
	unsigned m_cBlocks = 0;
	std::unique_ptr< BlockRecord[]>	m_rgbr;
};

class UrPageManager 
{
public:
	bool FInit(const char* szFile, unsigned cflBlock, unsigned cBlocks, bool fNew);
	UrStats::winp Get(unsigned i) const {
		assert(PflPageOf(i));
		return PflPageOf(i)[i % m_cflPage];
	}
	void Set(unsigned i, UrStats::winp fl) {
		assert(PflPageOf(i));
		PflPageOf(i)[i % m_cflPage] = fl;
	}
	UrStats::winp* PflPageOf(unsigned i) const {
		return m_rgpflPage[i / m_cflPage];
	}
	virtual void FlushPage(unsigned iPage);
	virtual unsigned CbReadPage(unsigned iPage, unsigned iBuffer);
	UrStats::winp* SetPage(unsigned iBlock, unsigned iBuffer);
	void SetCurrentPage(unsigned iBlock);
protected:
	BlockManager m_bm;
	unsigned m_cflPage;
	unsigned m_cPages;
	int m_CurrentPage = -1;
	std::unique_ptr<UrStats::winp[]> m_rgrgfl[4];
	UrStats::winp* m_rgpflPage[cPiecesUr * cPiecesUr] = { nullptr };
	std::unique_ptr<unsigned[]>	m_rgufBuffer;
	unsigned CbCompress(UrStats::winp* pfl, unsigned* rguBits, bool fCreate);
	void DeCompress(UrStats::winp* pfl, unsigned* rguBits, unsigned cb);
};

class Command
{
public:
	Command(const char* sz, std::function<void()> fn) : m_fn(fn), m_sz(sz){};
	void Execute() const { m_fn(); }
	bool operator == (const std::string& str) const { return !m_sz || str == m_sz; }
	friend std::ostream& operator << (std::ostream& s, const Command& c) { return s << c.m_sz; }
	static void Help() {
		std::cout << "\nAvailable commnds are";
		for (unsigned icmd = 0; icmd < s_ccmd; icmd++)
			std::cout << std::endl << s_rgcmd[icmd];
	}
	static const Command& Get(const std::string & str) {
		for (unsigned icmd = 0; icmd < s_ccmd; icmd++)
			if (s_rgcmd[icmd] == str)
				return s_rgcmd[icmd];
		return s_cmdHelp;
	}
protected:
	static const unsigned s_ccmd;
	static const Command s_rgcmd[];
	static const Command s_cmdHelp;
	std::function<void()> m_fn;
	const char* const m_sz;
};

unsigned __int64 FletcherChecksum(void* pData, size_t cbData);