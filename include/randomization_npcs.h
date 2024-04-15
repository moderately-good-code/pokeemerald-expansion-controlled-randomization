#ifndef GUARD_RANDOMIZATION_NPCS_H
#define GUARD_RANDOMIZATION_NPCS_H

void RandomizeTrainerParty(struct Pokemon* party, u16 trainerNum, u8 trainerClass);

void SetRandomizedAbility(struct BattlePokemon* battleMon, u16 trainerNum_A, u8 trainerClass_A,
        u16 trainerNum_B, u8 trainerClass_B);

#endif // GUARD_RANDOMIZATION_NPCS_H
