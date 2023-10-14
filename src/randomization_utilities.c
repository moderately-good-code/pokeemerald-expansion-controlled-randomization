#include "global.h"
#include "randomization_utilities.h"
#include "pokemon.h"
#include "random.h"
#include "constants/abilities.h"
#include "constants/map_groups.h"
#include "constants/pokemon.h"
#include "constants/species.h"

#define NUM_ENCOUNTER_RANDOMIZATION_TRIES       50

#define MIN_ENCOUNTER_LVL_NON_EVOLVERS          22
#define MIN_ENCOUNTER_LVL_FRIENDSHIP_EVOLVERS   28
#define MIN_ENCOUNTER_LVL_TRADE_EVOLVERS        38
#define MIN_ENCOUNTER_LVL_ITEM_EVOLVERS         32

#define MAX_LEVEL_OVER_EVOLUTION_LEVEL          7

extern struct Evolution gEvolutionTable[][EVOS_PER_MON];
extern struct SaveBlock2 *gSaveBlock2Ptr;

enum { // stolen from wild_encounter.c - find better solution than defining it here a 2nd time...
    WILD_AREA_LAND,
    WILD_AREA_WATER,
    WILD_AREA_ROCKS,
    WILD_AREA_FISHING,
};

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

    // also allow bat-like mons
    switch (species)
    {
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
            if (species != SPECIES_JELLICENT)
            {
                match = TRUE;
            }
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