#include "global.h"
#include "randomization_wild_encounters.h"
#include "randomization_utilities.h"
#include "constants/abilities.h"

#define NUM_ENCOUNTER_RANDOMIZATION_TRIES       200

#define WILD_MON_LEVEL_INCREASE_TO_BADGE_1      100
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_2      110
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_3      120
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_4      130
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_5      140
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_6      150
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_7      160
#define WILD_MON_LEVEL_INCREASE_TO_BADGE_8      170
#define WILD_MON_LEVEL_INCREASE_ALL_BADGES      180

// minimum level that early wild encounters need evolutions at
#define MIN_ENCOUNTER_EVO_LEVEL_BEFORE_BADGE_1  30

extern struct SaveBlock2 *gSaveBlock2Ptr;
extern struct Evolution gEvolutionTable[][EVOS_PER_MON];

enum { // stolen from wild_encounter.c - find better solution than defining it here a 2nd time...
    WILD_AREA_LAND,
    WILD_AREA_WATER,
    WILD_AREA_ROCKS,
    WILD_AREA_FISHING,
};

static u8 GetMinEvolutionLevel(u16 species)
{
    u8 i;

    for (i = 0; i < EVOS_PER_MON; i++)
    {
        switch (gEvolutionTable[species][i].method)
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
            return gEvolutionTable[species][i].param;
        }
    }

    // no evolution or evolution is level-independent
    return 1;
}

static bool8 IsSpeciesFossil(u16 species)
{
    switch (species)
    {
    case SPECIES_OMANYTE:
    case SPECIES_OMASTAR:
    case SPECIES_KABUTO:
    case SPECIES_KABUTOPS:
    case SPECIES_AERODACTYL:
    case SPECIES_LILEEP:
    case SPECIES_CRADILY:
    case SPECIES_ANORITH:
    case SPECIES_ARMALDO:
    case SPECIES_CRANIDOS:
    case SPECIES_RAMPARDOS:
    case SPECIES_SHIELDON:
    case SPECIES_BASTIODON:
    case SPECIES_TIRTOUGA:
    case SPECIES_CARRACOSTA:
    case SPECIES_ARCHEN:
    case SPECIES_ARCHEOPS:
    case SPECIES_GENESECT:
    case SPECIES_TYRUNT:
    case SPECIES_TYRANTRUM:
    case SPECIES_AMAURA:
    case SPECIES_AURORUS:
    case SPECIES_DRACOZOLT:
    case SPECIES_ARCTOZOLT:
    case SPECIES_DRACOVISH:
    case SPECIES_ARCTOVISH:
        return TRUE;
    }

    return FALSE;
}

static bool8 DoesSpeciesMatchLandGeneralNature(u16 species)
{
    u8 i;
    bool8 match;

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

    match = FALSE;

    for (i=0; i<2; i++)
    {
        switch (gSpeciesInfo[species].eggGroups[i])
        {
        case EGG_GROUP_NONE:
        case EGG_GROUP_WATER_1:
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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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

    if ((gSpeciesInfo[species].types[0] == TYPE_ICE)
            || gSpeciesInfo[species].types[1] == TYPE_ICE)
    {
        return FALSE;
    }

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
    // early areas (before gym 1) should have only encounters that evolve early
    switch (currentMapId)
    {
    case MAP_ROUTE101:
    case MAP_ROUTE102:
    case MAP_ROUTE103:
    case MAP_ROUTE104:
    case MAP_PETALBURG_WOODS:
    case MAP_ROUTE116:
        if (GetMinEvolutionLevel(species) > MIN_ENCOUNTER_EVO_LEVEL_BEFORE_BADGE_1)
        {
            return FALSE;
        }
    }

    // encounters should match their area type
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

u8 GetWildMonLevelIncrease(u8 level)
{
    u8 badges = GetNumOwnedBadges();
    switch (badges)
    {
    case 0:
        return level;
        // return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_1) / 100;
    case 1:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_2) / 100;
    case 2:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_3) / 100;
    case 3:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_4) / 100;
    case 4:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_5) / 100;
    case 5:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_6) / 100;
    case 6:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_7) / 100;
    case 7:
        return (level * WILD_MON_LEVEL_INCREASE_TO_BADGE_8) / 100;
    case 8:
        return (level * WILD_MON_LEVEL_INCREASE_ALL_BADGES) / 100;
    }
    return level;
}

u16 GetRandomizedEncounterSpecies(u16 seedSpecies, u8 level, u8 areaType)
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
                && (!IsSpeciesFossil(randomizedSpecies))
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
