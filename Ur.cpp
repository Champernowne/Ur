// Ur.cpp
// Copyright (c) 2022 Arthur Champernowne
// email: champernowne@hotmail.com
// 

// Ur.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <assert.h>
#include "Ur.h"
#include "index.h"
#include "analysis.h"

unsigned Game(UrStats const& urstat, unsigned ust, UrPlayer & up);
unsigned Move(UrStats const& urstat, unsigned ust, UrPlayer& up, unsigned die);
unsigned MoveTo(UrStats const& urstat, unsigned ust, UrPlayer& up, unsigned die);
bool fPlaying = false;
void Play();
UrStats::winp rgflDie[5] = { .0625, .25, .375, .25, .0625 };
unsigned rguDie[16] = { 0, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4 };
UrPlayer    upToMove = upWhite;
bool fMisere = false;
UrStats urstat;
UrState us(0);
bool fContinue = true;
unsigned die = 5;
const Command Command::s_cmdHelp("help",[] {Command::Help(); });

const Command Command::s_rgcmd[] = {
        {"advice",[]
        {
            unsigned col, row, index;
            col = row = index = 0;  // to keep the compiler happy
            bool fagain;
            switch (upToMove)
            {
            case upWhite:
                index = urstat.BestMove(us, col, row, die, fagain);
                if (index >= maxState)
                {
                    std::cout << "\nNo legal move.";
                    break;
                }
                if (!fagain)
                  index = UrState(UrState(index), true).Pack();
                row += die;
                if (row > 7)
                    col = 1 - col;
                break;
            case upBlack:
                index = UrState(urstat.BestMove(UrState(us, true), col, row, die, fagain)).Pack();
                if (index >= maxState)
                {
                    std::cout << "\nNo legal move.";
                    break;
                }
                if (fagain)
                    index = UrState(UrState(index), true).Pack();
                row += die;
                if (row > 7)
                    col = 1 - col;
                col = 2 - col;
                break;
            }

            if (col == 1)
                row = 7 - row;  
            std::cout << " move to row " << col << " column " << row << UrState(index);
        }},
        {"blocks",[] {BlockStats(urstat, std::cout); }},
        {"CompareStats", CompareStats},
        {"CompareMoves", CompareMoves},
        {"invert",[] {us = UrState(us, true); upToMove = (upToMove == upWhite) ? upBlack : upWhite; }},
        {"move",[]
        {
            if (die >= 5)
                std::cout << "\nDie not thrown.";
            else
                us = UrState(Move(urstat, us.Pack(), upToMove, die));
            die = 5;
        }},
        {"MoveDensity",[] {MoveDensity(urstat, std::cout); }},
        {"moveto",[]
        {
            if (die >= 5)
                std::cout << "\nDie not thrown.";
            else
            {
                unsigned uMovedTo = MoveTo(urstat, us.Pack(), upToMove, die);
                if (uMovedTo < maxState)
                {
                    us = UrState(uMovedTo);
                    die = 5;
                }
                else if (fPlaying)
                {
                    std::cout << "\nDo you want to stop playing y/n?";
                    std::string stryn;
                    std::cin >> stryn;
                    if (stryn == "y")
                        fPlaying = false;
                }
            }
        }},
        {"play", []() {Play(); }},
        {"nikki",[] {Nikki(urstat); }},
        {"pack",[] {std::cout << std::endl << us.Pack(); }},
        {"pwin", []() {std::cout << "\nProbability of a win = " << (((upToMove == upWhite) ? urstat.PWhiteWin(us.Pack()) : 1 - urstat.PWhiteWin(UrState(us, true).Pack()))); }},
        {"pass", []() {
            unsigned col, row;
            bool fRA;
            if (die > 4)
                std::cout << "\nThe dice have not been rolled - you cannot pass.";
            else if(maxState < urstat.BestMove(us, col, row, die, fRA)) std::cout << "\nYou have a legal move - you cannot pass.";
            else {
                die = 5;
                upToMove = (upToMove == upWhite) ? upBlack : upWhite;
               } }},
        {"quit", []() {fContinue = false; }},
        {"repack",[] {us = UrState(us.Pack()); }},
        {"seed",[] {
            std::cout << "\n Seed?";
            int seed;
            std::cin >> seed;
            srand(seed);
        }},
        {"set",[]
        {
            unsigned col, row, player;
            std::cout << "\nrow column player 0=none 1=white 2=black?";
            std::cin >> col;
            std::cin >> row;
            std::cin >> player;
            UrPlayer up = (UrPlayer)(player);
            bool fError = row > 7 || col > 2;
            if(!fError)
                switch (us.s_rgst[col][row])
                {
                case us.stHome:
                case us.stStart:
                    fError = true;
                }
                switch (up)
                {
                case upWhite:
                    if (col > 1)
                        fError = true;
                    break;
                case upBlack:
                    if (col < 1)
                        fError = true;
                    break;
                case upNil:
                    break;
                default:
                    fError = true;
                }
            if (fError)
            {
                std::cout << "\nIllegal position";
                return;
            }
            if (col == 1)
                row = 7 - row;
            us.Set(col, row, up);
        }},
        {"sethome",[]
        {
            unsigned pieces, player;
            std::cout << "\npieces player?";
            std::cin >> pieces;
            std::cin >> player;
            UrPlayer up = (UrPlayer)(player);
            us.SetHome(up, pieces);
        }},
        {"setindex",[] {
            std::cout << "index?";
            unsigned i;
            std::cin >> i;
            us = UrState(i);
        }},
        {"throw",[] {
            if (die > 4)
            {
                die = rguDie[(rand() >> 2) & 0xF];
                std::cout << "\n" << die << " thrown.";
                if (!die)
                {
                    std::cout << "\nNo legal move.";
                    upToMove = (upToMove == upWhite) ? upBlack : upWhite;
                    die = 5;
                }
            }
            else
                std::cout << "\nThe die has already been thrown.";
        }},
        { "throw0",[] {die = 0; } },
        { "throw1",[] {die = 1; } },
        {"throw2",[] {die = 2; }},
        {"throw3",[] {die = 3; }},
        {"throw4",[] {die = 4; }},
        {"validate",[] {if (us.FValid())std::cout << "\nValid"; else std::cout << "\nInvalid"; }},
};
const unsigned Command::s_ccmd = sizeof(Command::s_rgcmd) / sizeof(Command);
int main(int argc, char** argv)
{
    const char* szFile = nullptr;
    const char* szFile2 = nullptr;
#if 0
    for (int iArg = 1; iArg < argc; iArg++)
    {
        if (!_strcmpi("misere", argv[iArg]))
            fMisere = true;
        else if (!szFile)
            szFile = argv[iArg];
        else
            szFile2 = argv[iArg];
    }
#else
    if (argc == 2)
        szFile2 = argv[1];
    if (argc > 2)
    {
        szFile = argv[1];
        szFile2 = argv[2];
    }
#endif
    if (!urstat.FInit(szFile, szFile2, fMisere))
    {
        std::cout << "Cannot initialize from file " << szFile;
        return 0;
    }
    while (fContinue)
    {
        std::cout << us;
        std::cout << " " << ((upToMove == upWhite) ? "White" : "Black") << " to move.";
        if (die > 4)
            std::cout << "Die not thrown.";
        else
            std::cout << "die is " << die << ".";
        std::cout << "\nCommand:";
        std::string str;
        std::cin >> str;
        bool fError = true;
        Command::Get(str).Execute();
    }
}

void Play()
{
    static const Command& cmdThrow = Command::Get("throw");
    static const Command& cmdMove = Command::Get("move");
    static const Command& cmdMoveTo = Command::Get("moveto");
    static const Command& cmdPass = Command::Get("pass");

    UrPlayer    up = upToMove;
    bool fStarted = false;
    fPlaying = true;
    unsigned collocal, rowlocal;
    bool fRA;
    while (fPlaying)
    {
        if (us.Pack() != 0)
            fStarted = true;
        else if (fStarted)
            return;
        while(die >= 5)
            cmdThrow.Execute();
        std::cout << us << " " << die << " thrown";
        UrState usToMove(us, upToMove == upBlack);
        if (maxState <= urstat.BestMove(usToMove, collocal, rowlocal, die, fRA))
        {
            std::cout << "\nNo legal Move";
            cmdPass.Execute();
            continue;
        }
        if (upToMove == up)
            cmdMoveTo.Execute();
        else
            cmdMove.Execute();
    }
 }
unsigned MoveTo(UrStats const& urstat, unsigned ust, UrPlayer& up, unsigned die)
{
    unsigned uStateNext;
    unsigned row;
    unsigned col;
    bool fRollAgain;
    std::cout << "\nWhere do you move? row column:";
    std::cin >> col;
    std::cin >> row;

    if (up == upBlack)  // Reflect to white position
        col = 2 - col;
    
    if (row > 7 || col > 1)
    {
        std::cout << "\nIllegal move";
        return maxState;
    }
    if (col == 1)
        row = 7 - row;
    if (row >= die)
        row -= die;
    else
    {
        row += 8 - die;

        col = 1 - col;
    }
    
    if (up == upBlack) // Reflect
        ust = UrState(UrState(ust), true).Pack();
    
    uStateNext = UrState(ust).Move(col, row, die, fRollAgain);
    if (uStateNext >= maxState)
    {
        std::cout << "\nIllegal move";
        return uStateNext;
    }
    if (uStateNext == uWhiteWin)
    {
        if(fMisere)
            std::cout << "\nYou Lose!!!!!!\n";
        else
            std::cout << "\nYou Win!!!!!!\n";
        up = upWhite;
        return 0;
    }
    
    switch (up)
    {
    case upWhite:
        if (!fRollAgain)
        {
            up = upBlack;
            uStateNext = UrState(UrState(uStateNext), true).Pack();
        }
        break;
    case upBlack:
        if (fRollAgain)
            uStateNext = UrState(UrState(uStateNext), true).Pack();
        else
            up = upWhite;
        break;
    }
    return uStateNext;
}

unsigned Move(UrStats const& urstat, unsigned ust, UrPlayer& up, unsigned die)
{
    unsigned uStateNext = maxState;
    unsigned row;
    unsigned col;
    bool fRollAgain;
    switch (up)
    {
    case upWhite:
        uStateNext = urstat.BestMove(UrState(ust), col, row, die, fRollAgain);
        if (uStateNext >= maxState)
        {
            std::cout << ". No legal move.";
            up = upBlack;
            uStateNext = ust;
            break;
        }
        row += die;
        if (row > 7)
        {
            row -= 8;
            col = 1 - col;
        }
        std::cout << "\nWhite moves to row " << col << " column " << (col ? 7 - row : row);
        if (uStateNext == uWhiteWin)
        {
            if(fMisere)
                std::cout << " and loses!!\n";
            else
                std::cout << " and wins!!\n";
            uStateNext = 0;
            break;
        }
        if (!fRollAgain)
        {
            up = upBlack;
            uStateNext = UrState(UrState(uStateNext), true).Pack();
        }
        break;

    case upBlack:
        uStateNext = urstat.BestMove(UrState(UrState(ust), true), col, row, die, fRollAgain);
        if (uStateNext >= maxState)
        {
            std::cout << ". No legal move.";
            up = upWhite;
            uStateNext = ust;
            break;
        }
        row += die;
        if (row > 7)
        {
            row -= 8;
            col = 1 - col;
        }
        std::cout << "\nBlack moves to row " << (2 - col) << " column " << (col ? 7 - row : row);
        if (uStateNext == uWhiteWin)
        {
            if(fMisere)
                std::cout << " and loses!!\n";
            else
                std::cout << " and wins!!\n";
            uStateNext = 0;
            up = upWhite;
            break;
        }
        if (!fRollAgain)
            up = upWhite;
        else
            uStateNext = UrState(UrState(uStateNext), true).Pack();

        break;
    }
    return uStateNext;
}

const UrState::SquareType	UrState::s_rgst[3][8] = {
    {stNormal, stDouble, stHome, stStart, stNormal, stNormal, stNormal, stDouble},
    {stNormal, stNormal, stNormal, stDoubleSafe, stNormal, stNormal, stNormal, stNormal},
   {stNormal, stDouble, stHome, stStart, stNormal, stNormal, stNormal, stDouble}
};

UrState::UrState(unsigned u)
{
    assert(u < uWhiteWin);
    int rgcIdle[2] = { cPiecesUr, cPiecesUr };
    for (unsigned col = 3; col--;)
        for (unsigned row = 8; row--;)
        {
            if (col == 1)
                m_rgup[col][row] = UrPlayer(UnPackField(upNil, upIllegal, u));
            else
            {
                switch (s_rgst[col][row])
                {
                case stHome:
                case stStart:
                    m_rgup[col][row] = upIllegal;
                    continue;
                default:
                {
                    unsigned fOccupied = UnPackField(0, 2, u);
                    if (!fOccupied)
                        m_rgup[col][row] = upNil;
                    else
                        m_rgup[col][row] = col ? upBlack : upWhite;
                }
                }
            }
            switch (m_rgup[col][row])
            {
            case upWhite:
                rgcIdle[0]--;
                break;
            case upBlack:
                rgcIdle[1]--;
                break;
            case upNil:
                break;
            }
        }
    m_rgcHome[1] = UnPackField(0, cPiecesUr, u);
    rgcIdle[1] -= m_rgcHome[1];
    m_rgcHome[0] = UnPackField(0, cPiecesUr, u);
    rgcIdle[0] -= m_rgcHome[0];
    if (rgcIdle[0] > 0)
        m_rgup[0][3] = upWhite;
    if (rgcIdle[1] > 0)
        m_rgup[2][3] = upBlack;
    m_fValid = rgcIdle[0] >= 0 && rgcIdle[1] >= 0;
}

UrState::UrState(const UrState& us, bool fReverse)
{
    if (!fReverse)
    {
        memcpy(this, &us, sizeof(UrState));
        return;
    }
    m_rgcHome[0] = us.m_rgcHome[1];
    m_rgcHome[1] = us.m_rgcHome[0];
    m_fValid = us.m_fValid;
    for (unsigned col = 0; col < 3; col++)
        for (unsigned row = 0; row < 8; row++)
        {
            UrPlayer    up = us.m_rgup[col][row];
            switch (up)
            {
            case upWhite:
                up = upBlack;
                break;
            case upBlack:
                up = upWhite;
                break;
            }
            m_rgup[2 - col][row] = up;
        }
}

int UrState::GetIdle(UrPlayer up) const
{
    int result = cPiecesUr - m_rgcHome[(up == upWhite) ? 0 : 1];
    for (unsigned col = 3; col--;)
        for (unsigned row = 8; row--;)
            if (m_rgup[col][row] == up && s_rgst[col][row] != stStart)
                result--;
    return result;
}

unsigned UrState::Move(unsigned col, unsigned row, unsigned die, bool & fRollAgain) const
{
    unsigned colLanding = 0;
    unsigned rowLanding = 0;
  
    if (col >= 2 || row >= 8 || die >= 5)
        return maxState;
    if (m_rgup[col][row] != upWhite)
        return maxState;
    rowLanding = row + die;
    switch (col)
    {
    case 0:
        if (row < 3 && rowLanding > 2)
            return maxState;
        if (rowLanding < 8)
            colLanding = 0;
        else
        {
            colLanding = 1;
            rowLanding -= 8;
        }
        break;
    case 1:
        if (rowLanding < 8)
            colLanding = 1;
        else
        {
            colLanding = 0;
            rowLanding -= 8;
            if (rowLanding > 2)
                return maxState;
        }
        break;
    default:
        return maxState;
    }
    if (m_rgup[colLanding][rowLanding] == upWhite)
        return maxState;
    switch (s_rgst[colLanding][rowLanding])
    {
    case stNormal:
    {
        fRollAgain = false;
        UrState usMoved(*this, true);
        usMoved.m_rgup[2 - colLanding][rowLanding] = upBlack;
        assert(usMoved.m_rgup[2 - col][row] == upBlack);
        usMoved.m_rgup[2 - col][row] = upNil;
        return usMoved.Pack();
    }
    case stDoubleSafe:
        if (m_rgup[colLanding][rowLanding] == upBlack)
            return maxState;
    case stDouble:
    {
        fRollAgain = true;
        UrState usMoved(*this, false);
        usMoved.m_rgup[colLanding][rowLanding] = upWhite;
        assert(usMoved.m_rgup[col][row] == upWhite);
        usMoved.m_rgup[col][row] = upNil;
        return usMoved.Pack();
    }
    case stHome:
    {
        fRollAgain = false;
        UrState usMoved(*this, true);
        if((usMoved.m_rgcHome[1] += 1) >= cPiecesUr)
            return uWhiteWin;
        usMoved.m_rgup[2 - col][row] = upNil;
        return usMoved.Pack();
    }
    default :
        assert(false);
    }
    return maxState;
}

unsigned UrState::Pack() const
{
    unsigned result = 0;
    PackField(m_rgcHome[0], 0, cPiecesUr, result);    
    PackField(m_rgcHome[1], 0, cPiecesUr, result);
    for(unsigned col = 0; col < 3; col++)
        for (unsigned row = 0; row < 8; row++)
        {
            if (col == 1)
                PackField(m_rgup[col][row], upNil, upIllegal, result);
            else
            {
                switch (s_rgst[col][row])
                {
                case stHome:
                case stStart:
                    continue;
                default:
                    PackField(m_rgup[col][row] == upNil ? 0 : 1, 0, 2, result);
                }
            }
        }
    return result;
}

std::ostream& operator<<(std::ostream& s, const UrState& us)
{
    s << "\n_____________     ________";
    for (unsigned col = 0; col < 3; col++)
    {
        s << "\n|";
        for (unsigned row = 8; row--;)
        {
            unsigned rrow = row;
            if (col == 1)
                rrow = 7 - rrow;
            
            switch (UrState::s_rgst[col][rrow])
            {
            case UrState::stHome:
                s << "  |";
                continue;
            case UrState::stStart:
                s << "   ";
                continue;
            case UrState::stNormal:
                s << " ";
                break;
            case UrState::stDouble:
                s << "*";
                break;
            case UrState::stDoubleSafe:
                s << "X";
                break;
            default:
                assert(false);
            }
            switch (us.m_rgup[col][rrow])
            {
            case upWhite:
                s << "W|";
                break;
            case upBlack:
                s << "B|";
                break;
            case upNil:
                s << " |";
                break;
            default:
                assert(false);
            }
        }
        s << "\n-------------     --------";
    }
    s << "\nHome White = " << us.m_rgcHome[0] << " Black = " << us.m_rgcHome[1];
    return s;
}

unsigned __int64 FletcherChecksum(void* pData, size_t cbData)
{
    unsigned* pu = (unsigned *) pData;
    unsigned u1 = 0;
    unsigned u2 = 0;
    while (cbData >= sizeof(unsigned))
    {
        u1 += *(pu++);
        u2 += u1;
        cbData -= sizeof(unsigned);
    }
    unsigned __int64 result = u1;
    result <<= 32;
    result += u2;
    return result;
}

UrStats::UrStats() 
{
    m_cflPage = cBoardsPerPage;
    m_cflAll = cPiecesUr * cPiecesUr * m_cflPage;
}

bool UrStats::FInit(const char* szFile, const char *szFile2, bool fMisere) {
    m_fMisere = fMisere;
    if (szFile)
    {
        m_purpm = std::unique_ptr<UrPageManager>(new UrPageManager);
        if (!m_purpm->FInit(szFile, m_cflPage, cPiecesUr * cPiecesUr, false))
        {
            if (!m_purpm->FInit(szFile, m_cflPage, cPiecesUr * cPiecesUr, true))
                std::cout << "\nCannot open " << szFile << std::endl;
            if (!FGenerateStats())
                return false;
        }
    }
    if (szFile2)
    {
        m_purmm = std::unique_ptr<UrMoveManager>(new UrMoveManager);
        if (m_purmm->FInit(szFile2, nullptr))
        {
            m_fUseMoveManager = true;
            return true;
        }
        return m_fUseMoveManager = m_purmm->FInit(szFile2, this);
    }
    return !!m_purpm.get();
}

unsigned UrStats::BestMove(const UrState& us, unsigned& colBest, unsigned& rowBest, unsigned die, bool& fRollAgainBest) const
{
    unsigned result = maxState;
    UrStats::winp flPWinDie = -1;
    if (!die || die > 4)
        return result;
#ifndef _DEBUG
    if (!m_fUseMoveManager)
#endif
    {
        m_purpm->SetCurrentPage(us.IPage());
        for (unsigned col = 0; col < 2; col++)
            for (unsigned row = 0; row < 8; row++)
            {
                bool fRollAgain;
                unsigned iStatNext = us.Move(col, row, die, fRollAgain);
                if (iStatNext >= maxState)
                    continue;                   // Illegal move
                if (iStatNext == uWhiteWin)
                {
                    fRollAgainBest = fRollAgain;
                    result = iStatNext;
                    colBest = col;
                    rowBest = row;
                    goto Win;
                }
                unsigned ifl = iStatNext;
                assert(iStatNext < cPiecesUr* cPiecesUr* m_cflPage);
                UrStats::winp fl = m_purpm->Get(iStatNext);
                assert(fl >= 0.);
                if (!fRollAgain)
                    fl = 1 - fl;
                if (fl > flPWinDie)
                {
                    fRollAgainBest = fRollAgain;
                    result = iStatNext;
                    colBest = col;
                    rowBest = row;
                    flPWinDie = fl;
                }
            }
    Win:;
    }
    if (m_fUseMoveManager)
    {
#ifdef _DEBUG
        unsigned colTest = colBest;
        unsigned rowTest = rowBest;
        bool fTest = fRollAgainBest;
#endif
        m_purmm->BestMove(us, colBest, rowBest, die);
#ifdef _DEBUG
        if (colTest != colBest || rowTest != rowBest || fTest != fRollAgainBest)
            std::cout << "\nMoveManager says move " << colBest << ", " << rowBest << " but best move is " << colTest << ", " << rowTest;
#endif
        return us.Move(colBest, rowBest, die, fRollAgainBest);
    }
    return result;
}

UrStats::winp UrStats::PWhiteWin(unsigned index)
{
    m_purpm->SetCurrentPage(index / m_cflPage); 
    return m_purpm->Get(index); 
}

bool UrStats::FGenerateStats()
{
    unsigned i;

    for (unsigned cHomeWhite = cPiecesUr; cHomeWhite--;)
        for (unsigned cHomeBlack = cHomeWhite + 1; cHomeBlack--;)
        {
            unsigned iPage = cHomeWhite * cPiecesUr + cHomeBlack;
            unsigned iPageReverse = cHomeBlack * cPiecesUr + cHomeWhite;
            m_purpm->SetCurrentPage(iPage);
            unsigned rgiflPage[2] = { iPage * m_cflPage , iPageReverse * m_cflPage };
            unsigned cPages = (cHomeWhite == cHomeBlack) ? 1 : 2;
            for (unsigned ii = m_cflPage; ii--;)
            {
                for (unsigned iii = cPages; iii--;)
                {
                    i = rgiflPage[iii] + ii;
                    UrState us(i);
                    if (us.FValid())
                        m_purpm->Set(i, 0.5);
                    else
                        m_purpm->Set(i, -1.);
                }
            }
            UrStats::winp err = 1.;
            while (err >= .01)
            {
                err = 0.;
                for (unsigned ii = m_cflPage; ii--;)
                {
                    for (unsigned iii = cPages; iii--;)
                    {
                        i = rgiflPage[iii] + ii;
                        if (m_purpm->Get(i) == -1.)
                            continue;
                        UrStats::winp flPWin = 0;
                        UrState us(i);
                        UrState usReverse(us, true);
                        unsigned iflReverse = usReverse.Pack();  // swap players
                        assert(iflReverse >= 0);
                        assert(iflReverse < m_cflAll);
                        UrStats::winp flPWinBlackMove = m_purpm->Get(iflReverse);
                        assert(flPWinBlackMove >= 0.);
                        assert(flPWinBlackMove <= 1.);
                        for (unsigned die = 0; die < 5; die++)
                        {
                            UrStats::winp flPWinDie;
                            unsigned col;
                            unsigned row;
                            bool fRollAgain;
                            unsigned iStatNext = BestMove(us, col, row, die, fRollAgain);
                            if (iStatNext >= maxState)
                                flPWinDie = 1 - flPWinBlackMove;  // No legal move
                            else if (iStatNext == uWhiteWin)
                                flPWinDie = m_fMisere ? 0. : 1.;
                            else
                            {
                                assert(iStatNext < m_cflAll);
                                flPWinDie = m_purpm->Get(iStatNext);
                                assert(flPWinDie >= 0.);
                                assert(flPWinDie <= 1.);
                                if (!fRollAgain)
                                    flPWinDie = 1 - flPWinDie;
                            }
                            flPWin += flPWinDie * rgflDie[die];
                            assert(flPWin >= 0.);
                            assert(flPWin <= 1.);
                        }
                        assert(flPWin >= 0.);
                        assert(flPWin <= 1.);
                        UrStats::winp thiserr = fabs(flPWin - m_purpm->Get(i));
                        if (thiserr > err)
                        {
                            assert(thiserr < 1);
                            err = thiserr;
                        }
                        m_purpm->Set(i, flPWin);
                    }
                }
            }
            m_purpm->FlushPage(iPage);
            if (cHomeWhite != cHomeBlack)
                m_purpm->FlushPage(cHomeWhite + cPiecesUr * cHomeBlack);
        }

    return true;
}

UrStats::winp* UrPageManager::SetPage(unsigned iPage, unsigned iBuffer)
{
    UrStats::winp* pfl = m_rgrgfl[iBuffer].get();
    if(pfl)
        for (int i = cPiecesUr * cPiecesUr; i--;)
            if (m_rgpflPage[i] == pfl)
                m_rgpflPage[i] = nullptr;
        m_rgpflPage[iPage] = pfl;
    return pfl;
}

void UrPageManager::SetCurrentPage(unsigned iPage) 
{
    unsigned cPiecesWhite = iPage / cPiecesUr;
    unsigned cPiecesBlack = iPage % cPiecesUr;
    if (cPiecesBlack > cPiecesWhite)
    {
        cPiecesBlack = cPiecesWhite;
        cPiecesWhite = iPage % cPiecesUr;
        iPage = cPiecesWhite  * cPiecesUr + cPiecesBlack;
    }
    if (m_CurrentPage == iPage)
        return;
    CbReadPage(iPage, 0);
    if (cPiecesWhite < (cPiecesUr - 1))
        CbReadPage(cPiecesBlack * cPiecesUr + cPiecesWhite + 1, 3);
    if (iPage % (cPiecesUr + 1))
    {
        CbReadPage(cPiecesBlack * cPiecesUr + cPiecesWhite, 1);
        if (cPiecesBlack < (cPiecesUr - 1))
            CbReadPage(iPage + 1, 2);
    }
    m_CurrentPage = iPage;
}

bool UrPageManager::FInit(const char* szFile, unsigned cflPage, unsigned cPages, bool fNew)
{
    m_cflPage = cflPage;
    m_cPages = cPages;
    if (!m_bm.FInit(szFile, 2 * m_cPages, fNew))
        return false;
    for (int i = 4; i--;)
        m_rgrgfl[i] = std::unique_ptr<UrStats::winp[]>(new UrStats::winp[cflPage]);
    m_rgufBuffer = std::unique_ptr<unsigned[]>(new unsigned[(m_cflPage -1)/(8 * sizeof(unsigned)) + 1]);
    return true;
}

void UrPageManager::FlushPage(unsigned iPage)
{
    UrStats::winp* pfl = m_rgpflPage[iPage];
    unsigned cb = m_bm.CbReadBlock(m_bm.ValidityBits, iPage, m_rgufBuffer.get());
    bool fNew = !cb;
    cb = CbCompress(pfl, m_rgufBuffer.get(), fNew);
    if (fNew)
        m_bm.FlushBlock(m_bm.ValidityBits, iPage, m_rgufBuffer.get(), ((m_cflPage - 1) / (sizeof(unsigned) * 8) + 1) * sizeof(unsigned));
    m_bm.FlushBlock(m_bm.PackedData, iPage, pfl, cb);
    DeCompress(pfl, m_rgufBuffer.get(), cb);
}

unsigned UrPageManager::CbReadPage(unsigned iPage, unsigned iBuffer)
{
    UrStats::winp* pfl = SetPage(iPage, iBuffer);
    unsigned cb = m_bm.CbReadBlock(m_bm.ValidityBits, iPage, m_rgufBuffer.get());
    if(cb)
       cb = m_bm.CbReadBlock(m_bm.PackedData, iPage, pfl);
    if (cb)
    {
        DeCompress(pfl, m_rgufBuffer.get(), cb);
        cb = m_cflPage * sizeof(UrStats::winp);
    }
    return cb;
}

/*
void UrPageManager::FlushBlock(unsigned iBlock, bool fNew)
{
    BlockRecord& br = m_rgbr[iBlock];
    void* pvBuffer = nullptr;
    switch (br.m_rt)
    {
    case FileHeader:
        pvBuffer = m_rgbr.get();
        break;
    case ValidityBits:
        pvBuffer = m_rgufBuffer.get();
        break;
    case PackedData:
        pvBuffer = m_rgpflPage[br.m_iPage];;
        break;
    default:
        assert(false);
    }
    if (fNew)
    {
        fseek(m_pf, 0, SEEK_END);
        fgetpos(m_pf, &br.m_oRecordStart);
    }
    else
        fsetpos(m_pf, &br.m_oRecordStart);
    if(br.m_rt != FileHeader)
        br.m_u64Check = FletcherChecksum(pvBuffer, br.m_cbRecord);
       
    unsigned cb = fwrite(pvBuffer, br.m_cbRecord, 1, m_pf);
    if (!cb)
        std::cout << "\nRead Error Block " << iBlock << " feof = " << feof(m_pf) << " ferror = " << ferror(m_pf) << std::endl;
    
    assert(cb == 1);
    std::cout << "\n Write type " << br.m_rt << " page " << br.m_iPage << " length " << br.m_cbRecord << " checksum " << br.m_u64Check;
    if (fNew)
        FlushBlock(0, false);
}

void UrPageManager::ReadBlock(unsigned iBlock, unsigned iBuffer)
{
    BlockRecord &br	= m_rgbr[iBlock];
    void* pvBuffer = nullptr;
    switch (br.m_rt)
    {
    case FileHeader:
        pvBuffer = m_rgbr.get();
        break;
    case ValidityBits:
        pvBuffer = m_rgufBuffer.get();
        break;
    case PackedData:
        pvBuffer = m_rgrgfl[iBuffer].get();
        break;
    default:
        assert(false);
    }
    fsetpos(m_pf, &br.m_oRecordStart);
    unsigned cb = fread(pvBuffer, br.m_cbRecord, 1, m_pf);
    if (!cb)
        std::cout << "\nRead Error Block " << iBlock << " feof = " << feof(m_pf) << " ferror = " << ferror(m_pf) << std::endl;
    assert(cb == 1);
    if (br.m_rt != FileHeader) {
        unsigned __int64 u64check = FletcherChecksum(pvBuffer, br.m_cbRecord);
        //    std::cout << "\n read checksum " << u64check;
        if (br.m_u64Check != u64check)
        {
            std::cout << "\nChecksum Error Block " << iBlock << " was = " << br.m_u64Check << " but now = " << u64check << std::endl;
            assert(false);
        }
    }
}
*/

unsigned UrPageManager::CbCompress(UrStats::winp* pfl, unsigned* rguBits, bool fCreate)
{
    unsigned* puFlags = m_rgufBuffer.get();
    UrStats::winp* pflSoFar = pfl;
    unsigned mask = 1;
    unsigned uFlags = 0;
    for (unsigned ifl = 0; ifl < m_cflPage; ifl++, mask <<= 1)
    {
        if (!mask)
        {
            mask = 1;
            if (fCreate)
            {
                *(puFlags++) = uFlags;
                uFlags = 0;
            }
            else
                uFlags = *(puFlags++);
        }
        if (fCreate)
        {
            if (pfl[ifl] >= 0.)
            {
                uFlags |= mask;
                *(pflSoFar++) = pfl[ifl];
            }
        }
        else
        {
            if (mask & uFlags)
                *(pflSoFar++) = pfl[ifl];
        }
    }
    if(fCreate)
        *(puFlags++) = uFlags;
    return (pflSoFar - pfl) * sizeof(UrStats::winp);
}

void UrPageManager::DeCompress(UrStats::winp* pfl, unsigned* rguBits, unsigned cb)
{
    unsigned* puFlags = &m_rgufBuffer[(m_cflPage - 1) / (8 * sizeof(unsigned))];
    UrStats::winp* pflSoFar = pfl + (cb / sizeof(UrStats::winp) - 1);
    unsigned mask = 1 << (m_cflPage - 1) % (8 * sizeof(unsigned));
    unsigned uFlags = *(puFlags--);
    for (unsigned ifl = m_cflPage; ifl--; mask >>= 1)
    {
        if (!mask)
        {
            mask = 0x80000000;
            uFlags = *(puFlags--);
        }
        if (mask & uFlags)
            pfl[ifl] = *(pflSoFar--);
        else
            pfl[ifl] = -1.;
    }
    assert((pfl - pflSoFar) == 1);
}

bool BlockManager::FInit(const char* szFile, unsigned cBlocks, bool fNew)
{
    if (m_pf)
        fclose(m_pf);
    m_pf = nullptr;
    errno_t errn;
    if (fNew)
        errn = fopen_s(&m_pf, szFile, "w+b");
    else
        errn = fopen_s(&m_pf, szFile, "rb");
    if(errn)
        return false;

    m_cBlocks = cBlocks + 1;
    m_rgbr = std::unique_ptr< BlockRecord[]>(new BlockRecord[m_cBlocks]);
    memset(&m_rgbr[0], 0, sizeof(BlockRecord) * m_cBlocks);
    m_rgbr[0] = { 0, 0, FileHeader, 0, sizeof(BlockRecord) * m_cBlocks };
    if (!fNew)
        CbReadBlock(FileHeader, 0, m_rgbr.get());
    else
        FlushBlock(FileHeader, 0, m_rgbr.get(), sizeof(BlockRecord) * m_cBlocks);
    return true;
}

void BlockManager::FlushBlock(RecordType rt, unsigned iPage, void* pv, unsigned cb)
{
    int iBlock = SeekBlock(rt, iPage);
    bool fNew = iBlock < 0;
    if (fNew)
    {
        if (rt == FileHeader)
            iBlock = 0;
        else
        {
            iBlock = SeekBlock();
            assert(iBlock > 0);
        }
        m_rgbr[iBlock] = { 0, 0, rt, iPage, cb };
    }
    BlockRecord& br = m_rgbr[iBlock];
    assert(cb == br.m_cbRecord);

    if (fNew)
    {
        fseek(m_pf, 0, SEEK_END);
        fgetpos(m_pf, &br.m_oRecordStart);
    }
    else
        fsetpos(m_pf, &br.m_oRecordStart);
    if (br.m_rt != FileHeader)
        br.m_u64Check = FletcherChecksum(pv, br.m_cbRecord);

    cb = fwrite(pv, br.m_cbRecord, 1, m_pf);
    if (!cb)
        std::cout << "\nWrite Error Block " << iBlock << " feof = " << feof(m_pf) << " ferror = " << ferror(m_pf) << std::endl;

    assert(cb == 1);
    std::cout << "\n Write type " << br.m_rt << " page " << br.m_iPage << " length " << br.m_cbRecord << " checksum " << br.m_u64Check;
    if(br.m_rt != FileHeader)
        FlushBlock(FileHeader, 0, m_rgbr.get(), sizeof(BlockRecord) * m_cBlocks);
}

unsigned BlockManager::CbReadBlock(RecordType rt, unsigned iPage, void* pv) const
{
    int iBlock = SeekBlock(rt, iPage);
    if (iBlock < 0)
        return 0;
    BlockRecord& br = m_rgbr[iBlock];
    fsetpos(m_pf, &br.m_oRecordStart);
    unsigned cb = fread(pv, br.m_cbRecord, 1, m_pf);
    if (!cb)
        std::cout << "\nRead Error Block " << iBlock << " feof = " << feof(m_pf) << " ferror = " << ferror(m_pf) << std::endl;
    assert(cb == 1);
    if (br.m_rt != FileHeader) {
        unsigned __int64 u64check = FletcherChecksum(pv, br.m_cbRecord);
        //    std::cout << "\n read checksum " << u64check;
        if (br.m_u64Check != u64check)
        {
            std::cout << "\nChecksum Error Block " << iBlock << " was = " << br.m_u64Check << " but now = " << u64check << std::endl;
            assert(false);
        }
    }
    return br.m_cbRecord;
}
