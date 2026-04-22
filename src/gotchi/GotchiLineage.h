#pragma once
#include "GotchiDNA.h"

struct GotchiAncestor {
    GotchiID id;
    uint16_t days_lived;
    uint8_t  cause_of_death;   // 0=hambre, 1=salud, 2=otro
    uint8_t  avg_mood_pct;
    uint8_t  avg_health_pct;
};

struct GotchiHeritage {
    float    bonus_mood;      // 0–10 pts adicionales base
    float    bonus_health;    // 0–10 pts adicionales base
    float    hatch_speedup;   // 0.0–0.2 (0–20% más rápido)
    bool     beauty_bonus;    // forma adulta mejorada
};

class GotchiLineage {
public:
    void save(const GotchiID& id, const GotchiAncestor ancestors[5], const GotchiHeritage& heritage);
    void load(GotchiID& id, GotchiAncestor ancestors[5], GotchiHeritage& heritage);

    static GotchiHeritage computeHeritage(const GotchiAncestor& dying, const GotchiAncestor ancestors[5]);
    static void shiftAncestors(GotchiAncestor ancestors[5], const GotchiAncestor& new_ancestor);
};
