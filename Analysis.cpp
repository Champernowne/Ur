#include "Analysis.h"
#include "index.h"

struct PossibleDouble
{
	unsigned col;
	unsigned row;
	unsigned coldouble;
	unsigned rowdouble;
	bool ftakeallowed;
};

PossibleDouble	rgrgpd[4][3] = { {
	{0,6,0,7,true},
	{1,3,1,4,false},
	{0,0,0,1,true}, }, {
	{0,5,0,7,true},
	{1,2,1,4,false },
	{1,7,0,1,true }, }, {
	{0,4,0,7,true},
	{1,1,1,4,false},
	{1,6,0,1,true}, }, {
	{0,3,0,7,true},
	{1,0,1,4,false},
	{1,5,0,1,true}, }
};

void Nikki(UrStats& urs)
{
	for (unsigned index = uWhiteWin; index--;)
	{
		if (urs.PWhiteWin(index) < 0.)
			continue;
		UrState us(index);
		for (unsigned dieminus1 = 4; dieminus1--;)
		{
			if (!dieminus1 && us.Get(0, 1) == upWhite)	//Exception 2 Move off the last square whenever possible
				continue;
			for (const PossibleDouble& pd : rgrgpd[dieminus1])
				switch (us.Get(pd.col, pd.row))
				{
				case upIllegal:
					if (!us.GetIdle(upWhite))
						break;
				case upWhite:
					switch (us.Get(pd.coldouble, pd.rowdouble))
					{
					case upBlack:
						if (!pd.ftakeallowed)
							continue;
					case upNil:
					{
						if (pd.rowdouble == 1 && us.GetHome(upWhite) > 4)	//Exception 1 Avoid the last square before home
							continue;
						unsigned col, row;
						bool fDouble;
						unsigned ubest = urs.BestMove(us, col, row, dieminus1 + 1, fDouble);
						if (!fDouble)
						{
							std::cout << us << "\ndie = " << dieminus1 + 1 << " index = " << index;
							std::cout << UrState(UrState(ubest), true) << "\nRecommended with pwin = " << 1. - urs.PWhiteWin(us.Pack());
						}
						goto NextDie;
					}
					}
				}
		NextDie:;
		}
	}
}

void BlockStats(UrStats &urs, std::ostream &os)
{
	os << "\nHomeWhite, HomeBlack, #Valid, %Valid, MeanP, SigmaP\n";
	for (unsigned iPage = 0; iPage < cPages; iPage++) {
		unsigned iPageBase = iPage * cBoardsPerPage;
		unsigned cValid = 0;
		UrStats::winp	mean = 0.;
		UrStats::winp	var = 0.;
		for (unsigned index = cBoardsPerPage; index--;) {
			UrStats::winp flP = urs.PWhiteWin(index + iPageBase);
			if (flP >= 0.) {
				cValid++;
				mean += flP;
				var += flP * flP;
			}
		}
		mean /= cValid;
		var /= (cValid - 1);
		var -= mean * mean;
		os << iPage / cPiecesUr << ", " << iPage % cPiecesUr << ", " << cValid << ", " << 100. * UrStats::winp(cValid) / UrStats::winp(cBoardsPerPage) << ", " << mean << ", " << sqrt(var) << std::endl;
	}
}

void CompareStats()
{
	std::string file1, file2;
	std::cout << "\nFirst Stats Filename:";
	std::cin >> file1;
	std::unique_ptr <UrStats> purs1(new UrStats());
	if (!purs1->FInit(file1.c_str(), nullptr))
		return;
	std::cout << "\nSecond Stats Filename:";
	std::cin >> file2;
	std::unique_ptr <UrStats> purs2(new UrStats());
	if (!purs2->FInit(file2.c_str(), nullptr))
		return;

	UrStats::winp sump1 = 0;
	UrStats::winp sump2 = 0;
	UrStats::winp sump1squared = 0;
	UrStats::winp sump2squared = 0;
	UrStats::winp sumerr = 0;
	UrStats::winp sumerr2 = 0;
	unsigned cvalid = 0;
	for (int i = 0; i < uWhiteWin; i++) {
		UrStats::winp p1 = purs1->PWhiteWin(i);
		if (p1 < 0) {
			assert(purs2->PWhiteWin(i) < 0);
			continue;
		}
		UrStats::winp p2 = purs2->PWhiteWin(i);
		UrStats::winp err = p2 - p1;
		sump1 += p1;
		sump2 += p2;
		sump1squared += p1 * p1;
		sump2squared += p1 * p1;
		sumerr += err;
		sumerr2 += err * err;

		cvalid++;
	}
	sump1 /= cvalid;
	sump2 /= cvalid;
	sumerr /= cvalid;
	std::cout << "\n" << file1 << " mean = " << sump1 << " variance = " << (sump1squared / cvalid - sump1 * sump1);
	std::cout << "\n" << file2 << " mean = " << sump2 << " variance = " << (sump2squared / cvalid - sump2 * sump2);
	std::cout << "\nmean diference = " << sumerr << " variance = " << (sumerr2 / cvalid - sumerr * sumerr);
}

void CompareMoves()
{
	std::string file1, file2;
	std::cout << "\nFirst Move Filename:";
	std::cin >> file1;
	std::unique_ptr <UrMoveManager> purmm1(new UrMoveManager());
	if (!purmm1->FInit(file1.c_str(), nullptr))
		return;
	std::cout << "\nSecond Move Filename:";
	std::cin >> file2;
	std::unique_ptr <UrMoveManager> purmm2(new UrMoveManager());
	if (!purmm2->FInit(file2.c_str(), nullptr))
		return;

	unsigned cdifferent = 0;
	unsigned cmoves = 0;

	for (GameIndex gi; !gi.FEnd(); ++gi) {
		if (gi.CBitsPerMove()) {
			unsigned xormoves = purmm1->BestMoves(gi) ^ purmm2->BestMoves(gi);
			if (xormoves) {
				unsigned cBitsPerMove = gi.CBitsPerMove();
				unsigned mask = (1 << cBitsPerMove) - 1;
				while (xormoves) {
					if (xormoves & mask)
						cdifferent++;
					xormoves >>= cBitsPerMove;
				}
			}
			cmoves += 4;
		}
	}
	std::cout << "\n" << cdifferent << " moves different out of " << cmoves << " = " << float(cdifferent) * 100. / float(cmoves);
}

void MoveDensity(const UrStats& urs, std::ostream& os) {
	unsigned rgcBest[7][7][2][8][4] = { 0 };
	unsigned rgcAny[7][7][2][8][4] = { 0 };
	unsigned col = 0;
	unsigned row = 0;
	unsigned die = 0;
	bool f = false;
	for (GameIndex gi; !gi.FEnd(); ++gi) {
		unsigned cBitsPerMove = gi.CBitsPerMove();
		if (cBitsPerMove) {
			unsigned mask = (1 << cBitsPerMove) - 1;
			unsigned umoves = urs.PUmm()->BestMoves(gi);
			const UrState	us = gi.m_pi;
			for (die = 4; die > 0; die--, umoves >>= cBitsPerMove) {
				urs.PUmm()->MoveFromNumber(us, mask & umoves, col, row);
				if(us.Move(col, row, die, f) < maxState)
					rgcBest[gi.m_cHomeWhite][gi.m_cHomeBlack][col][row][die-1]++;
				for(col = 2; col--;)
					for(row = 8; row--;)
						if (us.Move(col, row, die, f) < maxState)
							rgcAny[gi.m_cHomeWhite][gi.m_cHomeBlack][col][row][die - 1]++;
			}
		}
	}
	os << "\ncHomeW, cHomeB, square#, die, countBest, countLegal, best/legal";
	for (unsigned cHomeW = 6; cHomeW--;)
		for (unsigned cHomeB = 7; cHomeB--;)
			for (col = 2; col--;)
				for (row = 8; row--;) {
					if ((col == 0) && (row == 2))
						continue;
					for (die = 1; die < 5; die++)
						os << "\n" << cHomeW << ", " << cHomeB << ", " <<
						((col == 1) ? (row + 5) : ((row > 2) ? (row - 3) : (row + 13))) << ", " << die << ", " <<
						rgcBest[cHomeW][cHomeB][col][row][die - 1] << ", " <<
						rgcAny[cHomeW][cHomeB][col][row][die - 1] << ", " <<
						float(rgcBest[cHomeW][cHomeB][col][row][die - 1]) / float(rgcAny[cHomeW][cHomeB][col][row][die - 1]); ;
				}
}