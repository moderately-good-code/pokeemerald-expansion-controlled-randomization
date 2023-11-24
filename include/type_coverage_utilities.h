#ifndef GUARD_TYPE_COVERAGE_UTILITIES_H
#define GUARD_TYPE_COVERAGE_UTILITIES_H

#include "constants/pokemon.h"

#define NEUTRAL_EFFECTIVENESS   0
#define WEAK_EFFECTIVENESS      1
#define STRONG_EFFECTIVENESS    2

struct TypeCoverageInfo
{
    u8 weaknesses[NUMBER_OF_MON_TYPES];
    u8 strengths[NUMBER_OF_MON_TYPES];
    u8 maxWeaknessValue; // maximum value in weaknesses, should be low for good coverage
    u8 numberOfStrengths; // number of positive values in strengths, should be high for good coverage
};

u8 GetEffectiveness(u8 moveType, u8 speciesType1, u8 speciesType2);

u8 GetTypeCoverageScore(const struct TypeCoverageInfo* coverage);

void UpdateTypeCoverageForMove(struct TypeCoverageInfo* coverage, u8 moveType);

void UpdateTypeCoverageForSpecies(struct TypeCoverageInfo* coverage, u8 speciesType1,
        u8 speciesType2);

#endif