#include "global.h"
#include "randomization_utilities.h"
#include "battle_util.h"
#include "pokemon.h"
#include "random.h"
#include "event_data.h"
#include "constants/abilities.h"
#include "constants/battle_move_effects.h"
#include "constants/map_groups.h"
#include "constants/moves.h"
#include "constants/pokemon.h"
#include "constants/species.h"

// #include "constants/battle_ai.h"
// #include "data.h"
#include "constants/trainers.h"
#include "data/pokemon/smogon_gen8lc.h"
#include "data/pokemon/smogon_gen8zu.h"
#include "data/pokemon/smogon_gen8uu.h"
#include "data/pokemon/smogon_gen8ou.h"
#include "data/pokemon/smogon_gen8balancedhackmons.h"
// #include "data/trainer_parties.h"
// #include "data/trainers.h"

#define NUM_ENCOUNTER_RANDOMIZATION_TRIES       200
#define NUM_TRAINER_RANDOMIZATION_TRIES         200

#define MIN_ENCOUNTER_LVL_NON_EVOLVERS          22
#define MIN_ENCOUNTER_LVL_FRIENDSHIP_EVOLVERS   28
#define MIN_ENCOUNTER_LVL_TRADE_EVOLVERS        38
#define MIN_ENCOUNTER_LVL_ITEM_EVOLVERS         32

#define MAX_LEVEL_OVER_EVOLUTION_LEVEL          7

#define SMOGON_GEN8LC_SPECIES_COUNT (SMOGON_BINACLE_INDEX_GEN8LC + 1)
#define SMOGON_GEN8ZU_SPECIES_COUNT (SMOGON_WOBBUFFET_INDEX_GEN8ZU + 1)
#define SMOGON_GEN8UU_SPECIES_COUNT (SMOGON_LINOONE_GALARIAN_INDEX_GEN8UU + 1)
#define SMOGON_GEN8OU_SPECIES_COUNT (SMOGON_BARBARACLE_INDEX_GEN8OU + 1)
#define SMOGON_GEN8BH_SPECIES_COUNT (SMOGON_HATTERENE_INDEX_GEN8BALANCEDHACKMONS + 1)

#define SECONDARY_TIER_FLAG                     0x8000

#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_1    1.2
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_2    1.4
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_3    1.7
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_4    1.8
#define NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_5    1.9
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_1      1.2
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_2      1.25
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_3      1.4
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_4      1.6
#define BOSS_NPC_LEVEL_INCREASE_TO_BADGE_5      1.85

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

extern struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern struct SaveBlock2 *gSaveBlock2Ptr;

enum { // stolen from wild_encounter.c - find better solution than defining it here a 2nd time...
    WILD_AREA_LAND,
    WILD_AREA_WATER,
    WILD_AREA_ROCKS,
    WILD_AREA_FISHING,
};

static const u16 sBadgeFlags[8] = {
    FLAG_BADGE01_GET, FLAG_BADGE02_GET, FLAG_BADGE03_GET, FLAG_BADGE04_GET,
    FLAG_BADGE05_GET, FLAG_BADGE06_GET, FLAG_BADGE07_GET, FLAG_BADGE08_GET,
};

// taken from match_call.c
static u8 GetNumOwnedBadges(void)
{
    u8 i;

    for (i = 0; i < NUM_BADGES; i++)
    {
        if (!FlagGet(sBadgeFlags[i]))
            break;
    }

    return i;
}

// This is the least efficient of the tests and should therefore be done last.
static bool8 DoesSpeciesMatchLevel(u16 species, u8 level)
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

static bool8 DoesSpeciesMatchLandGeneralNature(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_DRAGON:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_MONSTER:
        case EGG_GROUP_BUG:
        case EGG_GROUP_FLYING:
        case EGG_GROUP_FIELD:
        case EGG_GROUP_FAIRY:
        case EGG_GROUP_GRASS:
        case EGG_GROUP_DITTO:
            match = TRUE;
            break;
        }
    }

    return match;
}

static bool8 DoesSpeciesMatchLandNearWater(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_DRAGON:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_MONSTER:
        case EGG_GROUP_BUG:
        case EGG_GROUP_FLYING:
        case EGG_GROUP_FIELD:
        case EGG_GROUP_FAIRY:
        case EGG_GROUP_GRASS:
        case EGG_GROUP_DITTO:
        case EGG_GROUP_WATER_1:
            match = TRUE;
            break;
        }
    }

    return match;
}

static bool8 DoesSpeciesMatchLandInWater(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_DRAGON:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_WATER_1:
            match = TRUE;
            break;
        }
    }
    if ((gSpeciesInfo[species].types[0] == TYPE_WATER)
            || (gSpeciesInfo[species].types[1] == TYPE_WATER))
    {
        match = TRUE;
    }

    return match;
}

static bool8 DoesSpeciesMatchLandInForest(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_DRAGON:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_GRASS:
        case EGG_GROUP_BUG:
        case EGG_GROUP_AMORPHOUS:
            match = TRUE;
            break;
        }
    }

    return match;
}

static bool8 DoesSpeciesMatchLandOnMountain(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_DRAGON:
        case EGG_GROUP_HUMAN_LIKE:
        case EGG_GROUP_FLYING:
            match = TRUE;
            break;
        }
    }

    return match;
}

static bool8 DoesSpeciesMatchLandInNormalCave(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    switch (species)
    {
    // allow all bat-like mons
    case SPECIES_ZUBAT:
    case SPECIES_GOLBAT:
    case SPECIES_CROBAT:
    case SPECIES_NOIBAT:
    case SPECIES_NOIVERN:
    case SPECIES_WOOBAT:
    case SPECIES_SWOOBAT:
    case SPECIES_GLIGAR:
    case SPECIES_GLISCOR:
        return TRUE;

    // these are minerals, but don't fit into caves:
    case SPECIES_TRUBBISH:
    case SPECIES_GARBODOR:
    // these are amorphous, but don't fit into caves:
    case SPECIES_FRILLISH:
    case SPECIES_JELLICENT:
    case SPECIES_DRIFLOON:
    case SPECIES_DRIFBLIM:
    case SPECIES_PHANTUMP:
    case SPECIES_TREVENANT:
    case SPECIES_PUMPKABOO:
    case SPECIES_PUMPKABOO_SMALL:
    case SPECIES_PUMPKABOO_LARGE:
    case SPECIES_PUMPKABOO_SUPER:
    case SPECIES_GOURGEIST:
    case SPECIES_GOURGEIST_SMALL:
    case SPECIES_GOURGEIST_LARGE:
    case SPECIES_GOURGEIST_SUPER:
    case SPECIES_SHELLOS:
    case SPECIES_SHELLOS_EAST_SEA:
    case SPECIES_GASTRODON:
    case SPECIES_GASTRODON_EAST_SEA:
    // these are dragons but don't fit into caves:
    case SPECIES_APPLIN:
    case SPECIES_FLAPPLE:
    case SPECIES_APPLETUN:
    case SPECIES_FEEBAS:
    case SPECIES_MILOTIC:
    case SPECIES_HORSEA:
    case SPECIES_SEADRA:
    case SPECIES_KINGDRA:
        return FALSE;
    }

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_DRAGON:
            // dragons allowed in caves, but not flying ones
            if ((gSpeciesInfo[species].types[0] == TYPE_FLYING)
                    || (gSpeciesInfo[species].types[1] == TYPE_FLYING))
            {
                return FALSE;
            }
        case EGG_GROUP_MINERAL:
        case EGG_GROUP_AMORPHOUS:
            match = TRUE;
            break;
        }
    }

    return match;
}

static bool8 DoesSpeciesMatchLandInIcyCave(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_MINERAL:
            match = TRUE;
            break;
        }
    }

    // also allow all ice types that are not fish
    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return TRUE;
    }

    return match;
}

static bool8 DoesSpeciesMatchLandInDesert(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    switch (species)
    {
    // these have sand-related abilities, but don't fit into deserts:
    case SPECIES_LILLIPUP:
    case SPECIES_HERDIER:
    case SPECIES_STOUTLAND:
        return FALSE;
    }

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].types[i])
        {
        case TYPE_ICE:
        case TYPE_WATER:
        case TYPE_FIRE:
            return FALSE;
        case TYPE_GROUND:
            match = TRUE;
        }
    }
    if (match)
    {
        return TRUE;
    }

    // also check for sand-related abilities
    for (i=0; i<NUM_ABILITY_SLOTS; i++)
    {
        switch (gSpeciesInfo[species].abilities[i])
        {
            case ABILITY_SAND_STREAM:
            case ABILITY_SAND_FORCE:
            case ABILITY_SAND_RUSH:
            case ABILITY_SAND_VEIL:
                match = TRUE;
        }
    }

    return match;
}

static bool8 DoesSpeciesMatchLandNearVolcano(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    // no flying or ice mons near volcano
    if ((gSpeciesInfo[species].types[0] == TYPE_FLYING)
            || (gSpeciesInfo[species].types[1] == TYPE_FLYING)
            || (gSpeciesInfo[species].types[0] == TYPE_ICE)
            || (gSpeciesInfo[species].types[1] == TYPE_ICE))
    {
        return FALSE;
    }

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_1:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_DRAGON:
        case EGG_GROUP_MINERAL:
            match = TRUE;
            break;
        }
    }

    // also allow all fire types
    if ((gSpeciesInfo[species].types[0] == TYPE_FIRE)
            || (gSpeciesInfo[species].types[1] == TYPE_FIRE))
    {
        match = TRUE;
    }

    return match;
}

static bool8 DoesSpeciesMatchLandInCreepyArea(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_1:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_DITTO:
        case EGG_GROUP_HUMAN_LIKE:
        case EGG_GROUP_AMORPHOUS:
            if (species != SPECIES_JELLICENT)
            {
                match = TRUE;
            }
        }
    }

    return match;            
}

static bool8 DoesSpeciesMatchLandInIndustrialArea(u16 species)
{
    u8 i;
    bool8 match;

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_1:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
        case EGG_GROUP_UNDISCOVERED:
            return FALSE;
        case EGG_GROUP_MINERAL:
            match = TRUE;
        }
    }

    // also allow all electric and steel types
    if ((gSpeciesInfo[species].types[0] == TYPE_STEEL)
            || (gSpeciesInfo[species].types[1] == TYPE_STEEL)
            || (gSpeciesInfo[species].types[0] == TYPE_ELECTRIC)
            || (gSpeciesInfo[species].types[1] == TYPE_ELECTRIC))
    {
        return TRUE;
    }

    return match;
}

static bool8 DoesSpeciesMatchCurrentMap_Land(u16 species, u16 currentMapId)
{
    switch (currentMapId)
    {
    // general nature:
    case MAP_ROUTE101:
    case MAP_ROUTE102:
    case MAP_ROUTE116:
    case MAP_ROUTE117:
    case MAP_ROUTE120:
    case MAP_ROUTE121:
    case MAP_ROUTE123:
    case MAP_SAFARI_ZONE_NORTHWEST:
    case MAP_SAFARI_ZONE_NORTH:
    case MAP_SAFARI_ZONE_SOUTHWEST:
    case MAP_SAFARI_ZONE_SOUTH:
    case MAP_SAFARI_ZONE_NORTHEAST:
    case MAP_SAFARI_ZONE_SOUTHEAST:
        return DoesSpeciesMatchLandGeneralNature(species);
    
    // forest near water:
    case MAP_ROUTE103:
    case MAP_ROUTE104:
    case MAP_ROUTE118:
    case MAP_MOSSDEEP_CITY:
        return DoesSpeciesMatchLandNearWater(species);

    // sea:
    case MAP_ROUTE105:
    case MAP_ROUTE106:
    case MAP_ROUTE107:
    case MAP_ROUTE108:
    case MAP_ROUTE109:
    case MAP_ROUTE110:
    case MAP_ROUTE122:
    case MAP_ROUTE124:
    case MAP_ROUTE125:
    case MAP_ROUTE126:
    case MAP_ROUTE127:
    case MAP_ROUTE128:
    case MAP_ROUTE129:
    case MAP_ROUTE130:
    case MAP_ROUTE131:
    case MAP_ROUTE132:
    case MAP_ROUTE133:
    case MAP_ROUTE134:
        return DoesSpeciesMatchLandInWater(species);

    // deep forest:
    case MAP_PETALBURG_WOODS:
    case MAP_ROUTE119:
        return DoesSpeciesMatchLandInForest(species);

    // mountain-like:
    case MAP_ROUTE112:
    case MAP_ROUTE113: // TODO: this is the ash-filled one, maybe do something special with it
    case MAP_ROUTE114:
    case MAP_ROUTE115:
    case MAP_JAGGED_PASS:
    case MAP_MT_PYRE_EXTERIOR:
        return DoesSpeciesMatchLandOnMountain(species);

    // normal cave:
    case MAP_METEOR_FALLS_1F_1R:
    case MAP_METEOR_FALLS_1F_2R:
    case MAP_METEOR_FALLS_B1F_1R:
    case MAP_METEOR_FALLS_B1F_2R:
    case MAP_RUSTURF_TUNNEL:
    case MAP_GRANITE_CAVE_1F:
    case MAP_GRANITE_CAVE_B1F:
    case MAP_GRANITE_CAVE_B2F:
    case MAP_GRANITE_CAVE_STEVENS_ROOM:
    case MAP_CAVE_OF_ORIGIN_ENTRANCE:
    case MAP_CAVE_OF_ORIGIN_1F:
    case MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP1:
    case MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP2:
    case MAP_CAVE_OF_ORIGIN_UNUSED_RUBY_SAPPHIRE_MAP3:
    case MAP_CAVE_OF_ORIGIN_B1F:
    case MAP_VICTORY_ROAD_1F:
    case MAP_VICTORY_ROAD_B1F:
    case MAP_VICTORY_ROAD_B2F:
    case MAP_ARTISAN_CAVE_B1F:
    case MAP_ARTISAN_CAVE_1F:
    case MAP_ALTERING_CAVE:
    case MAP_METEOR_FALLS_STEVENS_CAVE:
        return DoesSpeciesMatchLandInNormalCave(species);

    // icy cave:
    case MAP_SHOAL_CAVE_LOW_TIDE_ENTRANCE_ROOM:
    case MAP_SHOAL_CAVE_LOW_TIDE_INNER_ROOM:
    case MAP_SHOAL_CAVE_LOW_TIDE_STAIRS_ROOM:
    case MAP_SHOAL_CAVE_LOW_TIDE_LOWER_ROOM:
    case MAP_SHOAL_CAVE_HIGH_TIDE_ENTRANCE_ROOM:
    case MAP_SHOAL_CAVE_HIGH_TIDE_INNER_ROOM:
    case MAP_SHOAL_CAVE_LOW_TIDE_ICE_ROOM:
        return DoesSpeciesMatchLandInIcyCave(species);

    // desert:
    case MAP_ROUTE111:
    case MAP_MIRAGE_TOWER_1F:
    case MAP_MIRAGE_TOWER_2F:
    case MAP_MIRAGE_TOWER_3F:
    case MAP_MIRAGE_TOWER_4F:
    case MAP_DESERT_UNDERPASS:
        return DoesSpeciesMatchLandInDesert(species);

    // near volcano:
    case MAP_FIERY_PATH:
    case MAP_MT_CHIMNEY:
    case MAP_MAGMA_HIDEOUT_1F:
    case MAP_MAGMA_HIDEOUT_2F_1R:
    case MAP_MAGMA_HIDEOUT_2F_2R:
    case MAP_MAGMA_HIDEOUT_3F_1R:
    case MAP_MAGMA_HIDEOUT_3F_2R:
    case MAP_MAGMA_HIDEOUT_4F:
    case MAP_MAGMA_HIDEOUT_3F_3R:
    case MAP_MAGMA_HIDEOUT_2F_3R:
        return DoesSpeciesMatchLandNearVolcano(species);

    // creepy area:
    case MAP_MT_PYRE_1F:
    case MAP_MT_PYRE_2F:
    case MAP_MT_PYRE_3F:
    case MAP_MT_PYRE_4F:
    case MAP_MT_PYRE_5F:
    case MAP_MT_PYRE_6F:
    case MAP_MT_PYRE_SUMMIT:
    case MAP_SKY_PILLAR_ENTRANCE:
    case MAP_SKY_PILLAR_OUTSIDE:
    case MAP_SKY_PILLAR_1F:
    case MAP_SKY_PILLAR_2F:
    case MAP_SKY_PILLAR_3F:
    case MAP_SKY_PILLAR_4F:
    case MAP_SKY_PILLAR_5F:
    case MAP_SKY_PILLAR_TOP:
        return DoesSpeciesMatchLandInCreepyArea(species);

    // industrial area:
    case MAP_NEW_MAUVILLE_ENTRANCE:
    case MAP_NEW_MAUVILLE_INSIDE:
        return DoesSpeciesMatchLandInIndustrialArea(species);
    }

    // TODO: return FALSE is only for debugging, will cause problems for undefined map segments
    return FALSE;
}

static bool8 DoesSpeciesMatchWater(species)
{
    u8 i;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_WATER_1:
        case EGG_GROUP_WATER_2:
        case EGG_GROUP_WATER_3:
            return TRUE;
        case EGG_GROUP_FLYING:
            if ((gSpeciesInfo[species].types[0] == TYPE_WATER)
                    || (gSpeciesInfo[species].types[1] == TYPE_WATER))
            {
                return TRUE;
            }
        }
    }

    return FALSE;
}

static bool8 DoesSpeciesMatchRocks(species)
{
    u8 i, j;

    for (i=0; i<2; i++)
    {
        if (gSpeciesInfo[species].eggGroups[i] == EGG_GROUP_MINERAL)
        {
            for (j=0; j<2; j++)
            {
                if ((gSpeciesInfo[species].types[j] == TYPE_STEEL)
                        || (gSpeciesInfo[species].types[j] == TYPE_FLYING)
                        || (gSpeciesInfo[species].types[j] == TYPE_WATER))
                {
                    return FALSE;
                }
            }
            return TRUE;
        }
    }

    // TODO: mineral group, but no steel, flying or fish types types
    return FALSE;
}

static bool8 DoesSpeciesMatchFishing(species)
{
    u8 i;

    for (i=0; i<2; i++)
    {
        if (gSpeciesInfo[species].eggGroups[i] == EGG_GROUP_WATER_2)
        {
            return TRUE;
        }
    }
    // TODO: only specific kinds of fish
    return FALSE;
}

static bool8 DoesSpeciesMatchCurrentMap(u16 species, u8 areaType, u16 currentMapId)
{
    switch (areaType)
    {
    case WILD_AREA_LAND:
        return DoesSpeciesMatchCurrentMap_Land(species, currentMapId);
    case WILD_AREA_WATER:
        return DoesSpeciesMatchWater(species);
    case WILD_AREA_ROCKS:
        return DoesSpeciesMatchRocks(species);
    case WILD_AREA_FISHING:
        return DoesSpeciesMatchFishing(species);
    }

    // area type should always be one of the above
    return FALSE;
}

static bool8 IsSpeciesValidWildEncounter(u16 species)
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

u16 GetRandomizedSpecies(u16 seedSpecies, u8 level, u8 areaType)
{
    u16 currentMapId;
    u16 randomizedSpecies;
    // u16 seed;
    union CompactRandomState seed;
    u8 i;

    // create map ID early to use in RNG seed
    currentMapId = ((gSaveBlock1Ptr->location.mapGroup) << 8 | gSaveBlock1Ptr->location.mapNum);

    // create temporary random seed
    seed.state = areaType + (seedSpecies << 4) + currentMapId
            + (((u16) gSaveBlock2Ptr->playerTrainerId[0]) << 8)
            + (((u16) gSaveBlock2Ptr->playerTrainerId[1])     )
            + (((u16) gSaveBlock2Ptr->playerTrainerId[2]) << 8)
            + (((u16) gSaveBlock2Ptr->playerTrainerId[3])     );
    // (The bit shift should lead to more diversity. Otherwise, different values might lead to the
    // same sum.)

    // sample random species until a valid match appears
    for (i=0; i<NUM_ENCOUNTER_RANDOMIZATION_TRIES; i++)
    {
        seed.state = CompactRandom(&seed);
        randomizedSpecies = seed.state % NUM_SPECIES;
        if (IsSpeciesValidWildEncounter(randomizedSpecies)
                && DoesSpeciesMatchCurrentMap(randomizedSpecies, areaType, currentMapId)
                // check level last because it is least efficient check:
                && DoesSpeciesMatchLevel(randomizedSpecies, level)) 
        {
            return randomizedSpecies;
        }
    }

    // no match found
    return SPECIES_UMBREON;
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
            continue; // TODO: also take level into account
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
            continue; // TODO: also take level into account
        }
        if (DoesSpeciesMatchLevel(speciesId, level))
        {
            return (smogonId | SECONDARY_TIER_FLAG);
        }
    }
    return (smogonId | SECONDARY_TIER_FLAG);
}

static void CreateMonFromSmogonStats(struct Pokemon* originalMon, u16 smogonId,
        const struct Smogon* gSmogon, union CompactRandomState* seed)
{
    u8 i, j;
    u16 randomized;
    u16 moves[MAX_MON_MOVES] = {
        MOVE_NONE,
        MOVE_NONE,
        MOVE_NONE,
        MOVE_NONE
    };

    // create mon
    CreateMon(originalMon, gSmogon[smogonId].species, originalMon->level, 0/*TODO*/, TRUE,
            originalMon->box.personality, 0, 0);

    // TODO: set better nature, set moves, set item

    // select moves
    for (i=0; i<MAX_MON_MOVES; i++)
    {
        for (j=0; j<NUM_TRAINER_RANDOMIZATION_TRIES; j++)
        {
            // get randomized move
            (*seed).state = CompactRandom(seed);
            randomized = (*seed).state % gSmogon[smogonId].movesCount;
            randomized = gSmogon[smogonId].moves[randomized].move;

            // make sure that move has not been assigned to previous slot
            if ((randomized != moves[0]) && (randomized != moves[1])
                    && (randomized != moves[2]) && (randomized != moves[3]))
            {
                moves[i] = randomized;
                break;
            }
        }

        // assign randomized move
        SetMonData(originalMon, MON_DATA_MOVE1 + i, &randomized);
        SetMonData(originalMon, MON_DATA_PP1 + i, &gBattleMoves[randomized].pp);
    }

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
    // TODO...

    // assign ability
    (*seed).state = CompactRandom(seed);
    randomized = (*seed).state % gSmogon[smogonId].abilitiesCount;
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
        return level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_1;
    case 1:
        return level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_2;
    case 2:
        return level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_3;
    case 3:
        return level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_4;
    case 4:
        return level * BOSS_NPC_LEVEL_INCREASE_TO_BADGE_5;
    }
    return level;
}

static u8 GetNormalMonLevelIncrease(u8 level, u8 badges)
{
    switch (badges)
    {
    case 0:
        return level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_1;
    case 1:
        return level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_2;
    case 2:
        return level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_3;
    case 3:
        level *= NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_4;
        return (level > 39 ? 39 : level);
    case 4:
        return level * NORMAL_NPC_LEVEL_INCREASE_TO_BADGE_5;
    }
    return level;
}

static void RandomizeBossNPCTrainerParty(struct Pokemon* party, u16 trainerNum,
        const struct Smogon* preferredTier, u16 preferredTierMonCount,
        const struct Smogon* secondaryTier, u16 secondaryTierMonCount, u8 preferredType,
        u8 badges)
{
    union CompactRandomState seed;
    u16 randomizedSpecies;
    u16 randomized; // just for item, outsource to other function later
    u8 i;

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

        // don't randomize starters for May/Brandon
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
            continue;
        }

        // select randomized species
        // TODO: different distribution for boss battles
        randomizedSpecies = GetRandomizedTrainerMonSpecies(preferredTier, preferredTierMonCount,
                secondaryTier, secondaryTierMonCount, preferredType, party[i].level, &seed);
        
        // create mon
        // TODO: different distribution for boss battles
        if (randomizedSpecies & SECONDARY_TIER_FLAG)
        {
            randomizedSpecies -= SECONDARY_TIER_FLAG;
            CreateMonFromSmogonStats(&(party[i]), randomizedSpecies, secondaryTier, &seed);

            // boss battles always use items
            // TODO: this should be included in CreateBossMon function
            randomized = seed.state % secondaryTier[randomizedSpecies].itemsCount;
            randomized = secondaryTier[randomizedSpecies].items[randomized].item;
            if ((randomized != ITEM_CHOICE_SCARF) && (randomized != ITEM_CHOICE_BAND)
                    && (randomized != ITEM_CHOICE_SPECS))
            {
                SetMonData(&(party[i]), MON_DATA_HELD_ITEM, &randomized);
            }
        }
        else
        {
            CreateMonFromSmogonStats(&(party[i]), randomizedSpecies, preferredTier, &seed);

            // boss battles always use items
            // TODO: this should be included in CreateBossMon function
            randomized = seed.state % preferredTier[randomizedSpecies].itemsCount;
            randomized = preferredTier[randomizedSpecies].items[randomized].item;
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
        u8 badges)
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
            if (i >= 3)
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
        // don't randomize first encounter
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
        else
        {
            preferredTier = gSmogon_gen8uu;
            preferredTierMonCount = SMOGON_GEN8UU_SPECIES_COUNT;
            secondaryTier = gSmogon_gen8zu;
            secondaryTierMonCount = SMOGON_GEN8ZU_SPECIES_COUNT;
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

        if (trainerClass == TRAINER_CLASS_COOLTRAINER)
        {
            RandomizeBossNPCTrainerParty(party, trainerNum, preferredTier, preferredTierMonCount,
                    secondaryTier, secondaryTierMonCount, preferredType, badges);
        }
        else
        {
            RandomizeNormalNPCTrainerParty(party, trainerNum, preferredTier, preferredTierMonCount,
                    secondaryTier, secondaryTierMonCount, preferredType, badges);
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
