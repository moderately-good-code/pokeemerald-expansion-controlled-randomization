#include "global.h"
#include "randomization_npcs.h"
#include "randomization_starters.h"
#include "randomization_utilities.h"
#include "type_coverage_utilities.h"
#include "constants/abilities.h"
#include "constants/moves.h"
#include "constants/trainers.h"
#include "data/pokemon/smogon_gen8lc.h"
#include "data/pokemon/smogon_gen8zu.h"
#include "data/pokemon/smogon_gen8uu.h"
#include "data/pokemon/smogon_gen8ou.h"
#include "data/pokemon/smogon_gen8balancedhackmons.h"

#define NUM_TRAINER_RANDOMIZATION_TRIES         200
#define NUM_SPECIES_RANDOMIZATION_CANDIDATES    3
#define NUM_MOVE_RANDOMIZATION_CANDIDATES       3
#define STATUS_MOVE_CHANCE_PER_CANDIDATE        25

#define SMOGON_GEN8LC_SPECIES_COUNT (SMOGON_BINACLE_INDEX_GEN8LC + 1)
#define SMOGON_GEN8ZU_SPECIES_COUNT (SMOGON_WOBBUFFET_INDEX_GEN8ZU + 1)
#define SMOGON_GEN8UU_SPECIES_COUNT (SMOGON_LINOONE_GALARIAN_INDEX_GEN8UU + 1)
#define SMOGON_GEN8OU_SPECIES_COUNT (SMOGON_BARBARACLE_INDEX_GEN8OU + 1)
#define SMOGON_GEN8BH_SPECIES_COUNT (SMOGON_HATTERENE_INDEX_GEN8BALANCEDHACKMONS + 1)

#define MIN_ABILITY_USAGE_VALUE                 50

#define SECONDARY_TIER_FLAG                     0x8000

#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_1    120
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_2    140
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_3    170
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_4    180
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_5    180
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_6    190
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_1      120
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_2      125
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_3      140
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_4      160
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_5      173
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_6      190

#define GENERALLY_USEFUL_ABILITY_COUNT          18
const u16 generallyUsefulAbilities[GENERALLY_USEFUL_ABILITY_COUNT] = {
    ABILITY_MAGIC_BOUNCE,
    ABILITY_UNAWARE,
    ABILITY_STURDY,
    ABILITY_REGENERATOR,
    ABILITY_SHEER_FORCE,
    ABILITY_ADAPTABILITY,
    ABILITY_SAP_SIPPER,
    ABILITY_STORM_DRAIN,
    ABILITY_DRY_SKIN,
    ABILITY_FLASH_FIRE,
    ABILITY_VOLT_ABSORB,
    ABILITY_LEVITATE,
    ABILITY_MOODY,
    ABILITY_PROTEAN,
    ABILITY_STAKEOUT,
    ABILITY_SPEED_BOOST,
    ABILITY_INNARDS_OUT,
    ABILITY_ILLUSION
};

static u16 GetRandomizedBossTrainerMonSpecies(const struct Smogon* preferredTier,
        u16 preferredTierMonCount, const struct Smogon* secondaryTier,
        u16 secondaryTierMonCount, u8 preferredType, u8 level,
        union CompactRandomState* seed, struct TypeCoverageInfo* coverage,
        bool8 preferMega)
{ // returns smogon ID of mon if preferred tier, else smogon ID | (0b1000000000000000)
    struct TypeCoverageInfo coverageTemporary;
    u8 i, currentCandidateNumber, currentCoverageScore, bestCoverageScore;
    u16 smogonId;
    u16 speciesId;
    u16 bestSmogonId = 0;
    bool8 fromPreferredTier = FALSE;

    // search in preferred tier
    currentCandidateNumber = 0;
    bestCoverageScore = 0;
    for (i=0; i<NUM_TRAINER_RANDOMIZATION_TRIES; i++)
    {
        seed->state = CompactRandom(seed);
        smogonId = seed->state % preferredTierMonCount;
        speciesId = preferredTier[smogonId].species;
        if ((preferredType != TYPE_NONE)
                && (gSpeciesInfo[speciesId].types[0] != preferredType)
                && (gSpeciesInfo[speciesId].types[1] != preferredType))
        {
            continue;
        }
        if (DoesSpeciesMatchLevel(speciesId, level) && IsSpeciesValidWildEncounter(speciesId))
        {
            // valid candidate found
            fromPreferredTier = TRUE;

            // are megas preferred?
            if (preferMega && (GetSpeciesMegaStone(speciesId) != ITEM_NONE))
            {
                bestSmogonId = smogonId;
                break;
            }

            // assess coverage
            coverageTemporary = *coverage;
            UpdateTypeCoverageForSpecies(&coverageTemporary, gSpeciesInfo[speciesId].types[0],
                    gSpeciesInfo[speciesId].types[1]);
            currentCoverageScore = GetTypeCoverageScore(&coverageTemporary);
            if (currentCoverageScore > bestCoverageScore)
            {
                bestSmogonId = smogonId;
                bestCoverageScore = currentCoverageScore;
            }

            // enough candidates checked?
            if (++currentCandidateNumber == NUM_SPECIES_RANDOMIZATION_CANDIDATES)
            {
                break;
            }
        }
    }

    // TODO: search in secondary tier

    if (fromPreferredTier)
    {
        speciesId = preferredTier[smogonId].species;
    }
    else
    {
        speciesId = secondaryTier[smogonId].species;
        bestSmogonId |= SECONDARY_TIER_FLAG;
    }
    UpdateTypeCoverageForSpecies(coverage, gSpeciesInfo[speciesId].types[0], gSpeciesInfo[speciesId].types[1]);
    return bestSmogonId;
}

static u16 GetRandomizedTrainerMonSpecies(const struct Smogon* preferredTier,
        u16 preferredTierMonCount, const struct Smogon* secondaryTier,
        u16 secondaryTierMonCount, u8 preferredType, u8 level,
        union CompactRandomState* seed)
{ // returns smogon ID of mon if preferred tier, else smogon ID | (0b1000000000000000)
    u8 i;
    u16 smogonId;
    u16 speciesId;

    // try to generate based on preferred tier
    for (i=0; i<NUM_TRAINER_RANDOMIZATION_TRIES; i++)
    {
        seed->state = CompactRandom(seed);
        smogonId = seed->state % preferredTierMonCount;
        speciesId = preferredTier[smogonId].species;
        if ((preferredType != TYPE_NONE)
                && (gSpeciesInfo[speciesId].types[0] != preferredType)
                && (gSpeciesInfo[speciesId].types[1] != preferredType))
        {
            continue;
        }
        if (DoesSpeciesMatchLevel(speciesId, level) && IsSpeciesValidWildEncounter(speciesId))
        {
            return smogonId;
        }
    }

    for (i=0; i<NUM_TRAINER_RANDOMIZATION_TRIES; i++)
    {
        seed->state = CompactRandom(seed);
        smogonId = seed->state % secondaryTierMonCount;
        speciesId = secondaryTier[smogonId].species;
        if ((preferredType != TYPE_NONE)
                && (gSpeciesInfo[speciesId].types[0] != preferredType)
                && (gSpeciesInfo[speciesId].types[1] != preferredType))
        {
            continue;
        }
        if (DoesSpeciesMatchLevel(speciesId, level))
        {
            return (smogonId | SECONDARY_TIER_FLAG);
        }
    }
    return (smogonId | SECONDARY_TIER_FLAG);
}

static void SetEvSpread(struct Pokemon* mon, const u8* evSpread)
{
    SetMonData(mon, MON_DATA_HP_EV, evSpread);
    SetMonData(mon, MON_DATA_ATK_EV, evSpread+1);
    SetMonData(mon, MON_DATA_DEF_EV, evSpread+2);
    SetMonData(mon, MON_DATA_SPATK_EV, evSpread+3);
    SetMonData(mon, MON_DATA_SPDEF_EV, evSpread+4);
    SetMonData(mon, MON_DATA_SPEED_EV, evSpread+5);
}

static void SetPerfectIvs(struct Pokemon* mon)
{
    u8 iv = 31;
    SetMonData(mon, MON_DATA_HP_IV, &iv);
    SetMonData(mon, MON_DATA_ATK_IV, &iv);
    SetMonData(mon, MON_DATA_DEF_IV, &iv);
    SetMonData(mon, MON_DATA_SPEED_IV, &iv);
    SetMonData(mon, MON_DATA_SPATK_IV, &iv);
    SetMonData(mon, MON_DATA_SPDEF_IV, &iv);
}

static void SetRandomizedMoves(struct Pokemon* originalMon, u16 smogonId,
        const struct Smogon* gSmogon, union CompactRandomState* seed)
{
    struct TypeCoverageInfo coverage = { 0, };
    struct TypeCoverageInfo coverageTemporary = { 0, };
    u16 moves[MAX_MON_MOVES] = {
        MOVE_NONE,
        MOVE_NONE,
        MOVE_NONE,
        MOVE_NONE
    };
    u16 randomized;
    u8 i, j, currentCandidateNumber, currentCoverageScore, bestCoverageScore, numStatusMoves;

    numStatusMoves = 0;
    for (i=0; i<MAX_MON_MOVES; i++)
    {
        if (i >= gSmogon[smogonId].movesCount)
        {
            break;
        }
        currentCandidateNumber = 0;
        bestCoverageScore = 0;
        for (j=0; j<NUM_TRAINER_RANDOMIZATION_TRIES; j++)
        {
            // get randomized move candidate
            (*seed).state = CompactRandom(seed);
            randomized = (*seed).state % gSmogon[smogonId].movesCount;
            randomized = gSmogon[smogonId].moves[randomized].move;

            // make sure that move has not been assigned to previous slot
            if ((randomized != moves[0]) && (randomized != moves[1])
                    && (randomized != moves[2]) && (randomized != moves[3])
                    // AI doesn't handle these well:
                    && (randomized != MOVE_SWITCHEROO) && (randomized != MOVE_TRICK)
                    && (randomized != MOVE_HEAL_PULSE))
            {
                // move is valid candidate
                if (gBattleMoves[randomized].split == SPLIT_STATUS)
                {
                    // accept status move candidates with constant probability
                    (*seed).state = CompactRandom(seed);
                    if ((*seed).state % 100 < (STATUS_MOVE_CHANCE_PER_CANDIDATE / (numStatusMoves+1)))
                    {
                        // candidate accepted
                        moves[i] = randomized;
                        numStatusMoves++;
                        break;
                    }
                }
                else
                {
                    // attack moves are accepted depending on their type coverage
                    coverageTemporary = coverage;
                    UpdateTypeCoverageForMove(&coverageTemporary, gBattleMoves[randomized].type);
                    currentCoverageScore = GetTypeCoverageScore(&coverageTemporary);
                    if (currentCoverageScore > bestCoverageScore)
                    {
                        // candidate accepted
                        moves[i] = randomized;
                        bestCoverageScore = currentCoverageScore;
                    }
                }

                if (++currentCandidateNumber == NUM_MOVE_RANDOMIZATION_CANDIDATES)
                {
                    if (gBattleMoves[randomized].split != SPLIT_STATUS)
                    {
                        UpdateTypeCoverageForMove(&coverage, gBattleMoves[randomized].type);
                    }
                    if (moves[i] == MOVE_NONE)
                    {
                        moves[i] = randomized;
                    }
                    break;
                }
            }
        }

        // assign randomized move
        SetMonData(originalMon, MON_DATA_MOVE1 + i, &randomized);
        SetMonData(originalMon, MON_DATA_PP1 + i, &gBattleMoves[randomized].pp);
    }
}

static void CreateMonFromSmogonStats(struct Pokemon* originalMon, u16 smogonId,
        const struct Smogon* gSmogon, union CompactRandomState* seed)
{
    u8 i, j;
    u16 randomized;

    // create mon
    CreateMon(originalMon, gSmogon[smogonId].species, originalMon->level, 0/*TODO*/, TRUE,
            originalMon->box.personality, 0, 0);

    // TODO: set better nature, set moves, set item

    // select moves
    SetRandomizedMoves(originalMon, smogonId, gSmogon, seed);

    // assign item with probability depending on level
    if ((*seed).state % 100 < originalMon->level)
    {
        randomized = (*seed).state % gSmogon[smogonId].itemsCount;
        randomized = gSmogon[smogonId].items[randomized].item;
        // current AI can't handle choice items :/
        if ((randomized != ITEM_CHOICE_SCARF) && (randomized != ITEM_CHOICE_BAND)
                && (randomized != ITEM_CHOICE_SPECS))
        {
            SetMonData(originalMon, MON_DATA_HELD_ITEM, &randomized);
        }
    }

    // assign most used nature (let's not overcomplicate things)
    // TODO

    // set EVs and IVs
    SetEvSpread(originalMon, gSmogon[smogonId].spreads->spread);
    SetPerfectIvs(originalMon);

    // assign ability
    (*seed).state = CompactRandom(seed);
    randomized = (*seed).state % gSmogon[smogonId].abilitiesCount;
    if (gSmogon[smogonId].abilities[randomized].usage < MIN_ABILITY_USAGE_VALUE)
    {
        // abilities with very low usage are often trash and require specific playstyle
        randomized = 0;
    }
    randomized = gSmogon[smogonId].abilities[randomized].ability;
    for (j=0; j<NUM_ABILITY_SLOTS; j++)
    {
        if (gSpeciesInfo[gSmogon[smogonId].species].abilities[j] == randomized)
        {
            SetMonData(originalMon, MON_DATA_ABILITY_NUM, &j);
            break;
        }
    }
}

static u8 GetBossMonLevelIncrease(u8 level, u8 badges)
{
    switch (badges)
    {
    case 0:
        return (level* BOSS_NPC_LEVEL_INCREASE_TO_BADGE_1) / 100;
    case 1:
        return (level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_2) / 100;
    case 2:
        return (level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_3) / 100;
    case 3:
        return (level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_4) / 100;
    case 4:
        return (level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_5) / 100;
    case 5:
        return (level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_6) / 100;
    }
    return level;
}

static u8 GetNormalMonLevelIncrease(u8 level, u8 badges)
{
    switch (badges)
    {
    case 0:
        if (level < 5)
        {
            return 5;
        }
        return (level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_1) / 100;
    case 1:
        return (level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_2) / 100;
    case 2:
        return (level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_3) / 100;
    case 3:
        level = (level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_4) / 100;
        return (level > 39 ? 39 : level);
    case 4:
        level = (level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_5) / 100;
        return (level > 45 ? 45 : level);
    case 5:
        level = (level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_6) / 100;
        return (level > 55 ? 55 : level);
    }
    return level;
}

static void RandomizeBossNPCTrainerParty(struct Pokemon* party, u16 trainerNum,
        const struct Smogon* preferredTier, u16 preferredTierMonCount,
        const struct Smogon* secondaryTier, u16 secondaryTierMonCount, u8 preferredType,
        u8 badges)
{
    struct TypeCoverageInfo coverage = { 0, };
    union CompactRandomState seed;
    u16 randomizedSpecies;
    u16 randomized; // just for item, outsource to other function later
    u8 i;
    bool8 hasMega;

    hasMega = FALSE;

    seed.state = trainerNum
        + (((u16) gSaveBlock2Ptr->playerTrainerId[0]) << 8)
        + (((u16) gSaveBlock2Ptr->playerTrainerId[1])     )
        + (((u16) gSaveBlock2Ptr->playerTrainerId[2]) << 8)
        + (((u16) gSaveBlock2Ptr->playerTrainerId[3])     );
    
    for (i = 0; i<PARTY_SIZE; i++)
    {
        // always use six mons
        if (party[i].level == 0)
        {
            party[i].level = party[0].level;
        }
        else
        {
            // increase level slightly for more difficulty
            party[i].level = GetBossMonLevelIncrease(party[i].level, badges);
        }

        // don't randomize starters for May/Brandon,
        // but take randomization of starters into account
        switch (GetBoxMonData(&(party[i].box), MON_DATA_SPECIES))
        {
        case SPECIES_TREECKO:
        case SPECIES_GROVYLE:
        case SPECIES_SCEPTILE:
        case SPECIES_TORCHIC:
        case SPECIES_COMBUSKEN:
        case SPECIES_BLAZIKEN:
        case SPECIES_MUDKIP:
        case SPECIES_MARSHTOMP:
        case SPECIES_SWAMPERT:
            randomizedSpecies = GetRivalStarterSpecies();
            randomizedSpecies = GetEvolvedWildMonSpecies(randomizedSpecies, party[i].level);
            CreateMon(&(party[i]), randomizedSpecies, party[i].level, 0/*TODO*/, TRUE,
                    party[i].box.personality, 0, 0);
            continue;
        }

        // select randomized species
        // TODO: different distribution for boss battles
        // randomizedSpecies = GetRandomizedTrainerMonSpecies(preferredTier, preferredTierMonCount,
        //         secondaryTier, secondaryTierMonCount, preferredType, party[i].level, &seed);
        randomizedSpecies = GetRandomizedBossTrainerMonSpecies(preferredTier, preferredTierMonCount,
                secondaryTier, secondaryTierMonCount, preferredType, party[i].level, &seed,
                &coverage, !hasMega);
        
        // create mon
        // TODO: different distribution for boss battles
        if (randomizedSpecies & SECONDARY_TIER_FLAG)
        {
            randomizedSpecies -= SECONDARY_TIER_FLAG;
            CreateMonFromSmogonStats(&(party[i]), randomizedSpecies, secondaryTier, &seed);

            // boss battles always use items, if possible mega stone
            randomized = ITEM_NONE;
            if (!hasMega)
            {
                randomized = GetSpeciesMegaStone(secondaryTier[randomizedSpecies].species);
                hasMega = (randomized != ITEM_NONE);
            }
            if (randomized == ITEM_NONE)
            {
                randomized = seed.state % secondaryTier[randomizedSpecies].itemsCount;
                randomized = secondaryTier[randomizedSpecies].items[randomized].item;
            }
            if ((randomized != ITEM_CHOICE_SCARF) && (randomized != ITEM_CHOICE_BAND)
                    && (randomized != ITEM_CHOICE_SPECS))
            {
                SetMonData(&(party[i]), MON_DATA_HELD_ITEM, &randomized);
            }
        }
        else
        {
            CreateMonFromSmogonStats(&(party[i]), randomizedSpecies, preferredTier, &seed);

            // boss battles always use items, if possible mega stone
            randomized = ITEM_NONE;
            if (!hasMega)
            {
                randomized = GetSpeciesMegaStone(preferredTier[randomizedSpecies].species);
                hasMega = (randomized != ITEM_NONE);
            }
            if (randomized == ITEM_NONE)
            {
                randomized = seed.state % preferredTier[randomizedSpecies].itemsCount;
                randomized = preferredTier[randomizedSpecies].items[randomized].item;
            }
            if ((randomized != ITEM_CHOICE_SCARF) && (randomized != ITEM_CHOICE_BAND)
                    && (randomized != ITEM_CHOICE_SPECS))
            {
                SetMonData(&(party[i]), MON_DATA_HELD_ITEM, &randomized);
            }
        }
    }
}

static void RandomizeNormalNPCTrainerParty(struct Pokemon* party, u16 trainerNum,
        const struct Smogon* preferredTier, u16 preferredTierMonCount,
        const struct Smogon* secondaryTier, u16 secondaryTierMonCount, u8 preferredType,
        u8 badges, u8 minMonCount)
{
    union CompactRandomState seed;
    u16 randomizedSpecies;
    u8 i;

    seed.state = trainerNum
            + (((u16) gSaveBlock2Ptr->playerTrainerId[0]) << 8)
            + (((u16) gSaveBlock2Ptr->playerTrainerId[1])     )
            + (((u16) gSaveBlock2Ptr->playerTrainerId[2]) << 8)
            + (((u16) gSaveBlock2Ptr->playerTrainerId[3])     );
    
    for (i = 0; i<PARTY_SIZE; i++)
    {
        // stop if no more mons in team, but always at least 3
        if (party[i].level == 0)
        {
            if (i >= minMonCount)
            {
                break;
            }
            party[i].level = party[0].level;
        }
        else
        {
            // increase level slightly for more difficulty
            party[i].level = GetNormalMonLevelIncrease(party[i].level, badges);
        }

        // select randomized species
        randomizedSpecies = GetRandomizedTrainerMonSpecies(preferredTier, preferredTierMonCount,
                secondaryTier, secondaryTierMonCount, preferredType, party[i].level, &seed);
        
        if (randomizedSpecies & SECONDARY_TIER_FLAG)
        {
            randomizedSpecies -= SECONDARY_TIER_FLAG;
            CreateMonFromSmogonStats(&(party[i]), randomizedSpecies, secondaryTier, &seed);
        }
        else
        {
            CreateMonFromSmogonStats(&(party[i]), randomizedSpecies, preferredTier, &seed);
        }
    }
}

static void SetGymType(u8* gymType)
{
    u16 currentMapId;
    currentMapId = ((gSaveBlock1Ptr->location.mapGroup) << 8 | gSaveBlock1Ptr->location.mapNum);
    switch (currentMapId)
    {
    case MAP_DEWFORD_TOWN_GYM:
        *gymType = TYPE_FIGHTING;
        break;
    case MAP_LAVARIDGE_TOWN_GYM_1F:
    case MAP_LAVARIDGE_TOWN_GYM_B1F:
        *gymType = TYPE_FIRE;
        break;
    case MAP_PETALBURG_CITY_GYM:
        *gymType = TYPE_NORMAL;
        break;
    case MAP_MAUVILLE_CITY_GYM:
        *gymType = TYPE_ELECTRIC;
        break;
    case MAP_RUSTBORO_CITY_GYM:
        *gymType = TYPE_ROCK;
        break;
    case MAP_FORTREE_CITY_GYM:
        *gymType = TYPE_FLYING;
        break;
    case MAP_MOSSDEEP_CITY_GYM:
        *gymType = TYPE_PSYCHIC;
        break;
    case MAP_SOOTOPOLIS_CITY_GYM_1F:
    case MAP_SOOTOPOLIS_CITY_GYM_B1F:
        *gymType = TYPE_WATER;
        break;
    }
}

void RandomizeTrainerParty(struct Pokemon* party, u16 trainerNum, u8 trainerClass)
{
    u16 preferredTierMonCount;
    u16 secondaryTierMonCount;
    const struct Smogon* preferredTier;
    const struct Smogon* secondaryTier;
    u8 level;
    u8 preferredType;
    u8 badges;

    badges = GetNumOwnedBadges();

    switch (trainerNum) // cases for boss battles
    {
    case TRAINER_BRENDAN_ROUTE_103_MUDKIP:
    case TRAINER_BRENDAN_ROUTE_103_TREECKO:
    case TRAINER_BRENDAN_ROUTE_103_TORCHIC:
    case TRAINER_MAY_ROUTE_103_MUDKIP:
    case TRAINER_MAY_ROUTE_103_TREECKO:
    case TRAINER_MAY_ROUTE_103_TORCHIC:
        CreateMon(&(party[0]), GetRivalStarterSpecies(), party[0].level, 0/*TODO*/, TRUE,
                party[0].box.personality, 0, 0);
        break;
    case TRAINER_BRENDAN_RUSTBORO_TREECKO:
    case TRAINER_BRENDAN_RUSTBORO_MUDKIP:
    case TRAINER_BRENDAN_RUSTBORO_TORCHIC:
    case TRAINER_MAY_RUSTBORO_MUDKIP:
    case TRAINER_MAY_RUSTBORO_TREECKO:
    case TRAINER_MAY_RUSTBORO_TORCHIC:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8lc, SMOGON_GEN8LC_SPECIES_COUNT,
                gSmogon_gen8zu, SMOGON_GEN8ZU_SPECIES_COUNT, TYPE_NONE, badges);
        break;
    case TRAINER_BRENDAN_ROUTE_110_MUDKIP:
    case TRAINER_BRENDAN_ROUTE_110_TREECKO:
    case TRAINER_BRENDAN_ROUTE_110_TORCHIC:
    case TRAINER_MAY_ROUTE_110_MUDKIP:
    case TRAINER_MAY_ROUTE_110_TREECKO:
    case TRAINER_MAY_ROUTE_110_TORCHIC:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8zu, SMOGON_GEN8ZU_SPECIES_COUNT,
                gSmogon_gen8lc, SMOGON_GEN8LC_SPECIES_COUNT, TYPE_NONE, badges);
        break;
    case TRAINER_ROXANNE_1:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8zu, SMOGON_GEN8ZU_SPECIES_COUNT,
                gSmogon_gen8lc, SMOGON_GEN8LC_SPECIES_COUNT, TYPE_ROCK, badges);
        break;
    case TRAINER_BRAWLY_1:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8zu, SMOGON_GEN8ZU_SPECIES_COUNT,
                gSmogon_gen8lc, SMOGON_GEN8LC_SPECIES_COUNT, TYPE_FIGHTING, badges);
        break;
    case TRAINER_WATTSON_1:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8uu, SMOGON_GEN8UU_SPECIES_COUNT,
                gSmogon_gen8zu, SMOGON_GEN8ZU_SPECIES_COUNT, TYPE_ELECTRIC, badges);
        break;
    case TRAINER_FLANNERY_1:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8ou, SMOGON_GEN8OU_SPECIES_COUNT,
                gSmogon_gen8uu, SMOGON_GEN8UU_SPECIES_COUNT, TYPE_FIRE, badges);
        break;
    case TRAINER_NORMAN_1:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8ou, SMOGON_GEN8OU_SPECIES_COUNT,
                gSmogon_gen8uu, SMOGON_GEN8UU_SPECIES_COUNT, TYPE_NORMAL, badges);
        break;

    case TRAINER_MAXIE_MT_CHIMNEY:
        RandomizeBossNPCTrainerParty(party, trainerNum, gSmogon_gen8ou, SMOGON_GEN8OU_SPECIES_COUNT,
                gSmogon_gen8uu, SMOGON_GEN8UU_SPECIES_COUNT, TYPE_NONE, badges);
        break;
    // rematches:
    case TRAINER_ROXANNE_2:
    case TRAINER_ROXANNE_3:
    case TRAINER_ROXANNE_4:
    case TRAINER_ROXANNE_5:
        // TODO ...

    default: // case for normal NPCs
        // for normal NPCs, tiers are decided by level
        level = GetNormalMonLevelIncrease(party[0].level, badges);
        if (level <= 19)
        {
            preferredTier = gSmogon_gen8lc;
            preferredTierMonCount = SMOGON_GEN8LC_SPECIES_COUNT;
            secondaryTier = gSmogon_gen8zu;
            secondaryTierMonCount = SMOGON_GEN8ZU_SPECIES_COUNT;
        }
        else if (level <= 34)
        {
            preferredTier = gSmogon_gen8zu;
            preferredTierMonCount = SMOGON_GEN8ZU_SPECIES_COUNT;
            secondaryTier = gSmogon_gen8lc;
            secondaryTierMonCount = SMOGON_GEN8LC_SPECIES_COUNT;
        }
        else if (level <= 49)
        {
            preferredTier = gSmogon_gen8uu;
            preferredTierMonCount = SMOGON_GEN8UU_SPECIES_COUNT;
            secondaryTier = gSmogon_gen8zu;
            secondaryTierMonCount = SMOGON_GEN8ZU_SPECIES_COUNT;
        }
        else
        {
            preferredTier = gSmogon_gen8ou;
            preferredTierMonCount = SMOGON_GEN8OU_SPECIES_COUNT;
            secondaryTier = gSmogon_gen8uu;
            secondaryTierMonCount = SMOGON_GEN8UU_SPECIES_COUNT;
        }

        // randomize based on trainer class
        // TODO: allow a second preferred type
        switch (trainerClass)
        {
        case TRAINER_CLASS_HIKER:
            preferredType = TYPE_GROUND;
            break;
        case TRAINER_CLASS_BIRD_KEEPER:
            preferredType = TYPE_FLYING;
            break;
        case TRAINER_CLASS_SWIMMER_M:
        case TRAINER_CLASS_SWIMMER_F:
        case TRAINER_CLASS_SAILOR:
        case TRAINER_CLASS_TUBER_F:
        case TRAINER_CLASS_TUBER_M:
        case TRAINER_CLASS_FISHERMAN:
        case TRAINER_CLASS_SIS_AND_BRO:
            preferredType = TYPE_WATER;
            break;
        case TRAINER_CLASS_EXPERT:
        case TRAINER_CLASS_BLACK_BELT:
        case TRAINER_CLASS_BATTLE_GIRL:
            preferredType = TYPE_FIGHTING;
            break;
        case TRAINER_CLASS_HEX_MANIAC:
            preferredType = TYPE_GHOST;
            break;
        case TRAINER_CLASS_AROMA_LADY:
            preferredType = TYPE_GRASS;
            break;
        case TRAINER_CLASS_RUIN_MANIAC:
            preferredType = TYPE_ROCK;
            break;
        case TRAINER_CLASS_KINDLER:
            preferredType = TYPE_FIRE;
            break;
        case TRAINER_CLASS_BEAUTY:
            preferredType = TYPE_FAIRY;
            break;
        case TRAINER_CLASS_BUG_MANIAC:
        case TRAINER_CLASS_BUG_CATCHER:
            preferredType = TYPE_BUG;
            break;
        case TRAINER_CLASS_PSYCHIC:
            preferredType = TYPE_PSYCHIC;
            break;
        case TRAINER_CLASS_DRAGON_TAMER:
            preferredType = TYPE_DRAGON;
            break;
        case TRAINER_CLASS_NINJA_BOY:
            preferredType = TYPE_POISON;
            break;

        // special randomization for grunts
        case TRAINER_CLASS_TEAM_AQUA:
        case TRAINER_CLASS_TEAM_MAGMA:
            // dont do balanced hackmons for first few grunts:
            if ((trainerNum != TRAINER_GRUNT_PETALBURG_WOODS)
                    && (trainerNum != TRAINER_GRUNT_RUSTURF_TUNNEL)
                    && (trainerNum != TRAINER_GRUNT_MUSEUM_1)
                    && (trainerNum != TRAINER_GRUNT_MUSEUM_2))
            {
                secondaryTier = preferredTier;
                secondaryTierMonCount = preferredTierMonCount;
                preferredTier = gSmogon_gen8balancedhackmons;
                preferredTierMonCount = SMOGON_GEN8BH_SPECIES_COUNT;
            }
            break;

        default: // trainer class with no preference
            preferredType = TYPE_NONE;
        }

        // overwrite if in gym
        SetGymType(&preferredType);

        if ((trainerClass == TRAINER_CLASS_COOLTRAINER)
                || (trainerClass == TRAINER_CLASS_SIS_AND_BRO)
                || (trainerClass == TRAINER_CLASS_INTERVIEWER))
        {
            RandomizeNormalNPCTrainerParty(party, trainerNum, preferredTier, preferredTierMonCount,
                    secondaryTier, secondaryTierMonCount, preferredType, badges, 6);
        }
        else
        {
            RandomizeNormalNPCTrainerParty(party, trainerNum, preferredTier, preferredTierMonCount,
                    secondaryTier, secondaryTierMonCount, preferredType, badges, 3);
        }
    }
}

void SetRandomizedAbility(struct BattlePokemon* battleMon, u16 trainerNum_A,
        u8 trainerClass_A, u16 trainerNum_B, u8 trainerClass_B)
{
    // u8 escape_attacks;
    // u8 status_attacks;
    // u8 physical_attacks;
    // u8 special_attacks;
    // u8 i;
    u16 smogonId;
    u16 ability;
    union CompactRandomState seed;

    // only grunts and grunt bosses get illegal abilities
    if ((trainerClass_A != TRAINER_CLASS_TEAM_AQUA) && (trainerClass_A != TRAINER_CLASS_AQUA_ADMIN)
            && (trainerClass_A != TRAINER_CLASS_AQUA_LEADER)
            && (trainerClass_A != TRAINER_CLASS_TEAM_MAGMA)
            && (trainerClass_A != TRAINER_CLASS_MAGMA_ADMIN)
            && (trainerClass_A != TRAINER_CLASS_MAGMA_LEADER)
            && (trainerClass_B != TRAINER_CLASS_TEAM_AQUA)
            && (trainerClass_B != TRAINER_CLASS_AQUA_ADMIN)
            && (trainerClass_B != TRAINER_CLASS_AQUA_LEADER)
            && (trainerClass_B != TRAINER_CLASS_TEAM_MAGMA)
            && (trainerClass_B != TRAINER_CLASS_MAGMA_ADMIN)
            && (trainerClass_B != TRAINER_CLASS_MAGMA_LEADER))
    {
        return;
    }

    // TODO: the grunts encountered before mount chimney don't get illegal abilities
    if ((trainerNum_A == TRAINER_GRUNT_PETALBURG_WOODS)
            || (trainerNum_A == TRAINER_GRUNT_RUSTURF_TUNNEL)
            || (trainerNum_A == TRAINER_GRUNT_MUSEUM_1)
            || (trainerNum_A == TRAINER_GRUNT_MUSEUM_2)
            || (trainerNum_B == TRAINER_GRUNT_PETALBURG_WOODS)
            || (trainerNum_B == TRAINER_GRUNT_RUSTURF_TUNNEL)
            || (trainerNum_B == TRAINER_GRUNT_MUSEUM_1)
            || (trainerNum_B == TRAINER_GRUNT_MUSEUM_2))
    {
        return;
    }

    // special cases
    if (battleMon->item == ITEM_TOXIC_ORB)
    {
        battleMon->ability = ABILITY_POISON_HEAL;
        return;
    }
    else if (battleMon->item == ITEM_FLAME_ORB)
    {
        battleMon->ability = ABILITY_GUTS;
        return;
    }
    if ((battleMon->species == SPECIES_CHANSEY) || (battleMon->species == SPECIES_BLISSEY)) // sus species
    {
        battleMon->ability = ABILITY_IMPOSTER;
        return;
    }

    smogonId = gSmogonSpeciesMappinggen8balancedhackmons[battleMon->species];
    seed.state = trainerNum_A + smogonId;
    seed.state = CompactRandom(&seed);
    if (smogonId != 0xFFFF) // exists in smogon BH struct
    {
        ability = seed.state % (gSmogon_gen8balancedhackmons[smogonId].abilitiesCount);
        ability = gSmogon_gen8balancedhackmons[smogonId].abilities[ability].ability;
    }
    else // not in smogon BH struct, set good ability manually
    {
        ability = seed.state % GENERALLY_USEFUL_ABILITY_COUNT;
        ability = generallyUsefulAbilities[ability];
    }
    battleMon->ability = ability;

    // // find abilities that fit mon's moves
    // escape_attacks = 0;
    // status_attacks = 0;
    // physical_attacks = 0;
    // special_attacks = 0;
    // for (i=0; i<MAX_MON_MOVES; i++)
    // {
    //     if (battleMon->moves[i] == MOVE_NONE)
    //     {
    //         continue;
    //     }
    //     switch (gBattleMoves[battleMon->moves[i]].split)
    //     {
    //     case SPLIT_PHYSICAL:
    //         physical_attacks++;
    //         break;
    //     case SPLIT_SPECIAL:
    //         special_attacks++;
    //         break;
    //     case SPLIT_STATUS:
    //         status_attacks++;
    //         break;
    //     }
    //     if (gBattleMoves[battleMon->moves[i]].effect == EFFECT_HIT_ESCAPE)
    //     {
    //         escape_attacks++;
    //     }
    // }
    // if (escape_attacks >= 2)
    // {
    //     battleMon->ability = ABILITY_REGENERATOR;
    // }
    // if (status_attacks >= 3)
    // {
    //     battleMon->ability = ABILITY_PRANKSTER;
    // }

    // // type multiplier ( ... GetTypeModifier ...)

    // // less urgent special cases
    // if (battleMon->item == ITEM_LIFE_ORB)
    // {
    //     battleMon->ability = ABILITY_SHEER_FORCE;
    // }

    // // get randomized ability based on balanced hackmons tier
    // battleMon->ability = ABILITY_SAP_SIPPER;

    // // TODO: if mon exists in balanced hackmons tier, then 
}
