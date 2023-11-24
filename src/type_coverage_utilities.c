#include "global.h"
#include "type_coverage_utilities.h"

extern const uq4_12_t sTypeEffectivenessTable[NUMBER_OF_MON_TYPES][NUMBER_OF_MON_TYPES];

static u8 EffectivenessMultipliersToClassification(uq4_12_t multiplier1, uq4_12_t multiplier2)
{
    if ((multiplier1 == UQ_4_12(0.0)) || (multiplier2 == UQ_4_12(0.0)))
    {
        return WEAK_EFFECTIVENESS;
    }
    switch (multiplier1)
    {
    case UQ_4_12(0.5):
        if (multiplier2 == UQ_4_12(2.0))
        {
            return NEUTRAL_EFFECTIVENESS;
        }
        return WEAK_EFFECTIVENESS;
    case UQ_4_12(1.0):
        switch (multiplier2)
        {
        case UQ_4_12(0.5): return WEAK_EFFECTIVENESS;
        case UQ_4_12(2.0): return STRONG_EFFECTIVENESS;
        case UQ_4_12(1.0): return NEUTRAL_EFFECTIVENESS;
        }
    case UQ_4_12(2.0):
        if (multiplier2 == UQ_4_12(0.5))
        {
            return NEUTRAL_EFFECTIVENESS;
        }
        return STRONG_EFFECTIVENESS;
    }
    return NEUTRAL_EFFECTIVENESS;
}

u8 GetEffectiveness(u8 moveType, u8 speciesType1, u8 speciesType2)
{
    uq4_12_t multiplier2;
    if (speciesType1 == speciesType2)
    {
        multiplier2 = UQ_4_12(1.0);
    }
    else
    {
        multiplier2 = sTypeEffectivenessTable[moveType][speciesType2];
    }
    return EffectivenessMultipliersToClassification(
            sTypeEffectivenessTable[moveType][speciesType1], multiplier2);
}

u8 GetTypeCoverageScore(const struct TypeCoverageInfo* coverage)
{
    return 0xFF - 10*coverage->maxWeaknessValue + coverage->numberOfStrengths;
}

void UpdateTypeCoverageForMove(struct TypeCoverageInfo* coverage, u8 moveType)
{
    u8 type;
    u8 effectiveness;
    u8 maxWeakness;

    maxWeakness = 0;
    for (type=0; type<NUMBER_OF_MON_TYPES; type++)
    {
        effectiveness = GetEffectiveness(moveType, type, type);
        if (effectiveness == STRONG_EFFECTIVENESS)
        {
            if ((coverage->strengths[type])++ == 0)
            {
                coverage->numberOfStrengths++;
            }
        }
        else if (effectiveness == WEAK_EFFECTIVENESS)
        {
            if (++(coverage->weaknesses[type]) > maxWeakness)
            {
                maxWeakness = coverage->weaknesses[type];
            }
        }
    }
    coverage->maxWeaknessValue = maxWeakness;
}

void UpdateTypeCoverageForSpecies(struct TypeCoverageInfo* coverage, u8 speciesType1,
        u8 speciesType2)
{
    u8 moveType;
    u8 effectiveness;
    u8 maxWeakness;

    maxWeakness = 0;
    for (moveType=0; moveType<NUMBER_OF_MON_TYPES; moveType++)
    {
        effectiveness = GetEffectiveness(moveType, speciesType1, speciesType2);
        if (effectiveness == WEAK_EFFECTIVENESS)
        {
            if ((coverage->strengths[moveType])++ == 0)
            {
                coverage->numberOfStrengths++;
            }
        }
        else if (effectiveness == STRONG_EFFECTIVENESS)
        {
            if (++(coverage->weaknesses[moveType]) > maxWeakness)
            {
                maxWeakness = coverage->weaknesses[moveType];
            }
        }
    }
    coverage->maxWeaknessValue = maxWeakness;
}

// static void UpdateTypeCoverageForMoveType(struct TypeCoverageInfo* coverage, u16 moveType)
// {
//     switch (moveType)
//     {
//     case TYPE_NORMAL:
//         coverage->weaknesses[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_GHOST]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
//     case TYPE_FIGHTING:
//         coverage->strengths[TYPE_NORMAL]++;
//         coverage->strengths[TYPE_ICE]++;
//         coverage->weaknesses[TYPE_POISON]++;
//         coverage->weaknesses[TYPE_FLYING]++;
//         coverage->weaknesses[TYPE_PSYCHIC]++;
//         coverage->weaknesses[TYPE_BUG]++;
//         coverage->strengths[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_GHOST]++;
//         coverage->strengths[TYPE_DARK]++;
//         coverage->strengths[TYPE_STEEL]++;
//         coverage->weaknesses[TYPE_FAIRY]++;
//         break;
//     case TYPE_FLYING:
//         coverage->weaknesses[TYPE_ELECTRIC]++;
//         coverage->strengths[TYPE_GRASS]++;
//         coverage->strengths[TYPE_FIGHTING]++;
//         coverage->strengths[TYPE_BUG]++;
//         coverage->weaknesses[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
//     case TYPE_POISON:
//         coverage->strengths[TYPE_GRASS]++;
//         coverage->weaknesses[TYPE_POISON]++;
//         coverage->weaknesses[TYPE_GROUND]++;
//         coverage->weaknesses[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_GHOST]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         coverage->strengths[TYPE_FAIRY]++;
//         break;
//     case TYPE_GROUND:
//         coverage->strengths[TYPE_FIRE]++;
//         coverage->strengths[TYPE_ELECTRIC]++;
//         coverage->weaknesses[TYPE_GRASS]++;
//         coverage->strengths[TYPE_POISON]++;
//         coverage->weaknesses[TYPE_FLYING]++;
//         coverage->weaknesses[TYPE_BUG]++;
//         coverage->strengths[TYPE_ROCK]++;
//         coverage->strengths[TYPE_STEEL]++;
//         break;
//     case TYPE_ROCK:
//         coverage->strengths[TYPE_ROCK]++;
//         coverage->strengths[TYPE_ICE]++;
//         coverage->weaknesses[TYPE_FIGHTING]++;
//         coverage->weaknesses[TYPE_GROUND]++;
//         coverage->strengths[TYPE_FLYING]++;
//         coverage->strengths[TYPE_BUG]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
//     case TYPE_BUG:
//         coverage->weaknesses[TYPE_FIRE]++;
//         coverage->strengths[TYPE_GRASS]++;
//         coverage->weaknesses[TYPE_FIGHTING]++;
//         coverage->weaknesses[TYPE_POISON]++;
//         coverage->weaknesses[TYPE_FLYING]++;
//         coverage->strengths[TYPE_PSYCHIC]++;
//         coverage->weaknesses[TYPE_GHOST]++;
//         coverage->strengths[TYPE_DARK]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         coverage->weaknesses[TYPE_FAIRY]++;
//         break;
//     case TYPE_GHOST:
//         coverage->weaknesses[TYPE_NORMAL]++;
//         coverage->strengths[TYPE_PSYCHIC]++;
//         coverage->strengths[TYPE_GHOST]++;
//         coverage->weaknesses[TYPE_DARK]++;
//         break;
//     case TYPE_STEEL:
//         coverage->weaknesses[TYPE_FIRE]++;
//         coverage->weaknesses[TYPE_WATER]++;
//         coverage->weaknesses[TYPE_ELECTRIC]++;
//         coverage->strengths[TYPE_ICE]++;
//         coverage->strengths[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         coverage->strengths[TYPE_FAIRY]++;
//         break;
//     case TYPE_FIRE:
//         coverage->weaknesses[TYPE_FIRE]++;
//         coverage->weaknesses[TYPE_WATER]++;
//         coverage->strengths[TYPE_GRASS]++;
//         coverage->strengths[TYPE_ICE]++;
//         coverage->strengths[TYPE_BUG]++;
//         coverage->weaknesses[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_DRAGON]++;
//         coverage->strengths[TYPE_STEEL]++;
//         break;
//     case TYPE_WATER:
//         coverage->strengths[TYPE_FIRE]++;
//         coverage->weaknesses[TYPE_WATER]++;
//         coverage->weaknesses[TYPE_GRASS]++;
//         coverage->strengths[TYPE_GROUND]++;
//         coverage->strengths[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_DRAGON]++;
//         break;
//     case TYPE_GRASS:
//         coverage->weaknesses[TYPE_FIRE]++;
//         coverage->strengths[TYPE_WATER]++;
//         coverage->weaknesses[TYPE_GRASS]++;
//         coverage->weaknesses[TYPE_POISON]++;
//         coverage->strengths[TYPE_GROUND]++;
//         coverage->weaknesses[TYPE_FLYING]++;
//         coverage->weaknesses[TYPE_BUG]++;
//         coverage->strengths[TYPE_ROCK]++;
//         coverage->weaknesses[TYPE_DRAGON]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
//     case TYPE_ELECTRIC:
//         coverage->strengths[TYPE_WATER]++;
//         coverage->weaknesses[TYPE_ELECTRIC]++;
//         coverage->weaknesses[TYPE_GRASS]++;
//         coverage->weaknesses[TYPE_GROUND]++;
//         coverage->strengths[TYPE_FLYING]++;
//         coverage->weaknesses[TYPE_DRAGON]++;
//         break;
//     case TYPE_PSYCHIC:
//         coverage->strengths[TYPE_FIGHTING]++;
//         coverage->strengths[TYPE_POISON]++;
//         coverage->weaknesses[TYPE_PSYCHIC]++;
//         coverage->weaknesses[TYPE_DARK]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
//     case TYPE_ICE:
//         coverage->weaknesses[TYPE_FIRE]++;
//         coverage->weaknesses[TYPE_WATER]++;
//         coverage->strengths[TYPE_GRASS]++;
//         coverage->weaknesses[TYPE_ICE]++;
//         coverage->strengths[TYPE_GROUND]++;
//         coverage->strengths[TYPE_FLYING]++;
//         coverage->strengths[TYPE_DRAGON]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
//     case TYPE_DRAGON:
//         coverage->strengths[TYPE_DRAGON]++;
//         coverage->weaknesses[TYPE_STEEL]++;
//         break;
// // #define TYPE_DARK             17
// // #define TYPE_FAIRY            18
//     }
// }