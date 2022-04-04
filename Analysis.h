// Analysis.h
// Copyright (c) 2022 Arthur Champernowne
// email: champernowne@hotmail.com
// 

#pragma once
#include "ur.h"
void Nikki(UrStats & urs);
void BlockStats(UrStats&, std::ostream&);
void CompareStats();
void CompareMoves();
void MoveDensity(const UrStats& urs, std::ostream& os);
