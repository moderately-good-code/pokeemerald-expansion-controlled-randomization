#include "global.h"
#include "randomization_utilities.h"
#include "battle_util.h"
#include "pokemon.h"
#include "event_data.h"
#include "constants/form_change_types.h"

#define MIN_ENCOUNTER_LVL_NON_EVOLVERS          22
#define MIN_ENCOUNTER_LVL_FRIENDSHIP_EVOLVERS   28
#define MIN_ENCOUNTER_LVL_TRADE_EVOLVERS        38
#define MIN_ENCOUNTER_LVL_ITEM_EVOLVERS         32

#define MAX_LEVEL_OVER_EVOLUTION_LEVEL          7

extern struct Evolution gEvolutionTable[][EVOS_PER_MON];

static const u16 sBadgeFlags[8] = {
    FLAG_BADGE01_GET, FLAG_BADGE02_GET, FLAG_BADGE03_GET, FLAG_BADGE04_GET,
    FLAG_BADGE05_GET, FLAG_BADGE06_GET, FLAG_BADGE07_GET, FLAG_BADGE08_GET,
};

// taken from match_call.c
u8 GetNumOwnedBadges(void)
{
    u8 i;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (!FlagGet(sBadgeFlags[i]))
            break;
    }

    return i;
}

u16 GetEvolvedWildMonSpecies(u16 currentSpecies, u8 level)
{
    u8 i;

    for (i = 0; i < EVOS_PER_MON; i++)
    {
        switch (gEvolutionTable[currentSpecies][i].method)
        {
        // case already evolved:
        case 0:
            if ((i == 0) && (level < MIN_ENCOUNTER_LVL_NON_EVOLVERS))
            {
                return currentSpecies;
            }
            break;

        // case evolves via level:
        case EVO_LEVEL:
        case EVO_LEVEL_ATK_GT_DEF:
        case EVO_LEVEL_ATK_EQ_DEF:
        case EVO_LEVEL_ATK_LT_DEF:
        case EVO_LEVEL_SILCOON:
        case EVO_LEVEL_CASCOON:
        case EVO_LEVEL_NINJASK:
        case EVO_LEVEL_SHEDINJA:
        // case EVO_LEVEL_FEMALE:
        // case EVO_LEVEL_MALE:
        case EVO_LEVEL_NIGHT:
        case EVO_LEVEL_DAY:
        case EVO_LEVEL_DUSK:
        case EVO_LEVEL_RAIN:
        case EVO_LEVEL_DARK_TYPE_MON_IN_PARTY:
        case EVO_LEVEL_NATURE_AMPED:
        case EVO_LEVEL_NATURE_LOW_KEY:
        case EVO_LEVEL_FOG:
            if (gEvolutionTable[currentSpecies][i].param <= level)
            {
                return gEvolutionTable[currentSpecies][i].targetSpecies;
            }
            break;
        
        // case evolves via friendship:
        case EVO_FRIENDSHIP:
        case EVO_FRIENDSHIP_DAY:
        case EVO_FRIENDSHIP_NIGHT:
        case EVO_FRIENDSHIP_MOVE_TYPE:
            if (level >= MIN_ENCOUNTER_LVL_FRIENDSHIP_EVOLVERS)
            {
                return gEvolutionTable[currentSpecies][i].targetSpecies;
            }
            break;

        // case evolves via trade:
        case EVO_TRADE:
        case EVO_TRADE_ITEM:
        case EVO_TRADE_SPECIFIC_MON:
            if (level >= MIN_ENCOUNTER_LVL_TRADE_EVOLVERS)
            {
                return gEvolutionTable[currentSpecies][i].targetSpecies;
            }
            break;


        // case evolves via item:
        case EVO_ITEM:
        case EVO_ITEM_HOLD_DAY:
        case EVO_ITEM_HOLD_NIGHT:
        case EVO_ITEM_MALE:
        case EVO_ITEM_FEMALE:
        case EVO_ITEM_NIGHT:
        case EVO_ITEM_DAY:
        case EVO_ITEM_HOLD:
            if (level >= MIN_ENCOUNTER_LVL_ITEM_EVOLVERS)
            {
                return gEvolutionTable[currentSpecies][i].targetSpecies;
            }
            break;
        }
    }

    return currentSpecies;
}

// This is the least efficient of the tests and should therefore be done last.
bool8 DoesSpeciesMatchLevel(u16 species, u8 level)
{
    u16 j;
    u8 i, k;

    for (i = 0; i < EVOS_PER_MON; i++)
    {
        switch (gEvolutionTable[species][i].method)
        {
        // case already evolved:
        case 0:
            if ((i == 0) && (level < MIN_ENCOUNTER_LVL_NON_EVOLVERS))
            {
                return FALSE;
            }
            break;
        
        // case evolves via level:
        case EVO_LEVEL:
        case EVO_LEVEL_ATK_GT_DEF:
        case EVO_LEVEL_ATK_EQ_DEF:
        case EVO_LEVEL_ATK_LT_DEF:
        case EVO_LEVEL_SILCOON:
        case EVO_LEVEL_CASCOON:
        case EVO_LEVEL_NINJASK:
        case EVO_LEVEL_SHEDINJA:
        case EVO_LEVEL_FEMALE:
        case EVO_LEVEL_MALE:
        case EVO_LEVEL_NIGHT:
        case EVO_LEVEL_DAY:
        case EVO_LEVEL_DUSK:
        case EVO_LEVEL_RAIN:
        case EVO_LEVEL_DARK_TYPE_MON_IN_PARTY:
        case EVO_LEVEL_NATURE_AMPED:
        case EVO_LEVEL_NATURE_LOW_KEY:
        case EVO_LEVEL_FOG:
            if (gEvolutionTable[species][i].param < level + MAX_LEVEL_OVER_EVOLUTION_LEVEL)
            {
                return FALSE;
            }
            break;
        }
    }

    // check if previous evolution aready evolved at this level:
    for (j=0; j<NUM_SPECIES; j++)
    {
        for (i=0; i<EVOS_PER_MON; i++)
        {
            if (gEvolutionTable[j][i].targetSpecies == species)
            {
                switch (gEvolutionTable[j][i].method)
                {
                // case evolves via level:
                case EVO_LEVEL:
                case EVO_LEVEL_ATK_GT_DEF:
                case EVO_LEVEL_ATK_EQ_DEF:
                case EVO_LEVEL_ATK_LT_DEF:
                case EVO_LEVEL_SILCOON:
                case EVO_LEVEL_CASCOON:
                case EVO_LEVEL_NINJASK:
                case EVO_LEVEL_SHEDINJA:
                case EVO_LEVEL_FEMALE:
                case EVO_LEVEL_MALE:
                case EVO_LEVEL_NIGHT:
                case EVO_LEVEL_DAY:
                case EVO_LEVEL_DUSK:
                case EVO_LEVEL_RAIN:
                case EVO_LEVEL_DARK_TYPE_MON_IN_PARTY:
                case EVO_LEVEL_NATURE_AMPED:
                case EVO_LEVEL_NATURE_LOW_KEY:
                case EVO_LEVEL_FOG:
                    if (gEvolutionTable[j][i].param <= level)
                    {
                        return TRUE;
                    }
                    return FALSE;
                
                // case evolves via friendship:
                case EVO_FRIENDSHIP:
                case EVO_FRIENDSHIP_DAY:
                case EVO_FRIENDSHIP_NIGHT:
                case EVO_FRIENDSHIP_MOVE_TYPE:
                    if (level >= MIN_ENCOUNTER_LVL_FRIENDSHIP_EVOLVERS)
                    {
                        return TRUE;
                    }
                    return FALSE;

                // case evolves via trade:
                case EVO_TRADE:
                case EVO_TRADE_ITEM:
                case EVO_TRADE_SPECIFIC_MON:
                    if (level >= MIN_ENCOUNTER_LVL_TRADE_EVOLVERS)
                    {
                        return TRUE;
                    }
                    return FALSE;


                // case evolves via item:
                case EVO_ITEM:
                case EVO_ITEM_HOLD_DAY:
                case EVO_ITEM_HOLD_NIGHT:
                case EVO_ITEM_MALE:
                case EVO_ITEM_FEMALE:
                case EVO_ITEM_NIGHT:
                case EVO_ITEM_DAY:
                case EVO_ITEM_HOLD:
                    if (level >= MIN_ENCOUNTER_LVL_ITEM_EVOLVERS)
                    {
                        return TRUE;
                    }
                    return FALSE;
                }
            }
        }
    }

    return TRUE;
}

bool8 IsSpeciesValidWildEncounter(u16 species)
{
    u16 flags;

    flags = gSpeciesInfo[species].flags;
    if ((flags & SPECIES_FLAG_LEGENDARY) 
            || (flags & SPECIES_FLAG_MYTHICAL)
            || (flags & SPECIES_FLAG_MEGA_EVOLUTION)
            || (flags & SPECIES_FLAG_PRIMAL_REVERSION)
            || (flags & SPECIES_FLAG_ULTRA_BEAST))
    {
        return FALSE;
    }

    // anything else?

    return TRUE;
}

u16 GetSpeciesMegaStone(u16 species)
{
    u32 i;
    const struct FormChange *formChanges = gFormChangeTablePointers[species];

    if (formChanges != NULL)
    {
        for (i = 0; formChanges[i].method != FORM_CHANGE_TERMINATOR; i++)
        {
            if (formChanges[i].method == FORM_CHANGE_BATTLE_MEGA_EVOLUTION_ITEM)
            {
                return formChanges[i].param1;
            }
        }
    }
    return ITEM_NONE;
}

