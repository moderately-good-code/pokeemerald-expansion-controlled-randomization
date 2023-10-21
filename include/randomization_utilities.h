#ifndef GUARD_RANDOMIZATION_UTILITIES_H
#define GUARD_RANDOMIZATION_UTILITIES_H

/*
    Improved RNG taken from https://github.com/tertu-m/pokeemerald/blob/random/include/random.h
*/
#if MODERN
#define RANDOM_IMPL_NONCONST extern inline __attribute__((gnu_inline))
#define RANDOM_IMPL_CONST extern inline __attribute__((const,gnu_inline))
#define RANDOM_NONCONST extern inline __attribute__((gnu_inline))
#else
#define RANDOM_IMPL_NONCONST extern inline
#define RANDOM_IMPL_CONST extern inline __attribute__((const))
#define RANDOM_NONCONST extern inline
#endif

union CompactRandomState {
    u16 state;
    s16 state_signed;
};

// A 16-bit state version of Wyrand.
RANDOM_NONCONST u16 CompactRandom(union CompactRandomState *s)
{
    u32 hash;
    u16 temp;
    temp = s->state;
    temp += 0xFC15;
    hash = (u32)temp * 0x2AB;
    s->state = temp;
    return (u16)((hash >> 16) ^ hash);
}

u16 GetRandomizedSpecies(u16 seedSpecies, u8 level, u8 areaType);

void RandomizeTrainerParty(struct Pokemon* party, u16 trainerNum);

#endif