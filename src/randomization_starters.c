#include "global.h"
#include "event_data.h"
#include "randomization_starters.h"
#include "randomization_utilities.h"

// duplicate definition, also in starter_choose.c:
#define STARTER_MON_COUNT   3

#define RANDOM_STARTER_CANDIDATES_PER_CLASS   13

static const u16 sPossibleStarters[STARTER_MON_COUNT][RANDOM_STARTER_CANDIDATES_PER_CLASS] = {
    // grass and bug types
    {
        // standard grass starters
        SPECIES_BULBASAUR,
        SPECIES_CHIKORITA,
        SPECIES_TREECKO,
        SPECIES_TURTWIG,
        SPECIES_SNIVY,
        SPECIES_CHESPIN,
        SPECIES_ROWLET,
        SPECIES_GROOKEY,

        // other grass types
        SPECIES_SEWADDLE,

        // bug types
        SPECIES_VENIPEDE,
        SPECIES_GRUBBIN,
        SPECIES_BLIPBUG,

        // special surprise
        SPECIES_RALTS,
    },

    // water and electric types
    {
        // standard water starters
        SPECIES_SQUIRTLE,
        SPECIES_TOTODILE,
        SPECIES_MUDKIP,
        SPECIES_PIPLUP,
        SPECIES_OSHAWOTT,
        SPECIES_FROAKIE,
        SPECIES_POPPLIO,
        SPECIES_SOBBLE,

        // other water types
        SPECIES_POLIWAG,
        SPECIES_TYMPOLE,

        // electric types
        SPECIES_MAREEP,
        SPECIES_SHINX,

        // special surprise
        SPECIES_EEVEE,
    },

    // fire and flying types
    {
        // standard fire starters
        SPECIES_CHARMANDER,
        SPECIES_CYNDAQUIL,
        SPECIES_TORCHIC,
        SPECIES_CHIMCHAR,
        SPECIES_TEPIG,
        SPECIES_FENNEKIN,
        SPECIES_LITTEN,
        SPECIES_SCORBUNNY,

        // other fire types (or evolving into fire)
        SPECIES_FLETCHLING,
        SPECIES_ROLYCOLY,

        // flying types
        SPECIES_PIDGEY,
        // SPECIES_ZUBAT, // commenting this one out to have equal amounts in each class
        SPECIES_STARLY,
        SPECIES_ROOKIDEE,
    },
};

u16 GetRandomizedStarterPokemon(u16 chosenStarterId)
{
    union CompactRandomState seed;
    u16 randomizedSpeciesIndex;

    // make sure starter ID is valid
    if (chosenStarterId > STARTER_MON_COUNT)
        chosenStarterId = 0;

    // create randomized index
    seed.state = chosenStarterId
            + (((u16) gSaveBlock2Ptr->playerTrainerId[0]) << 8)
            + (((u16) gSaveBlock2Ptr->playerTrainerId[1])     )
            + (((u16) gSaveBlock2Ptr->playerTrainerId[2]) << 8)
            + (((u16) gSaveBlock2Ptr->playerTrainerId[3])     );
    randomizedSpeciesIndex = CompactRandom(&seed) % RANDOM_STARTER_CANDIDATES_PER_CLASS;

    // return species ID of selected randomized starter
    return sPossibleStarters[chosenStarterId][randomizedSpeciesIndex];
}

u16 GetRivalStarterSpecies(void)
{
    u16 species = VarGet(VAR_STARTER_MON);

    // choose the mon to the left of the player's choice
    if (species == 0)
    {
        species = 2;
    }
    else
    {
        species = species - 1;
    }

    return GetRandomizedStarterPokemon(species);
}
