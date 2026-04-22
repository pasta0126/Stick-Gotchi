#include "GotchiLineage.h"
#include <Preferences.h>

void GotchiLineage::save(const GotchiID& id, const GotchiAncestor ancestors[5], const GotchiHeritage& heritage) {
    Preferences prefs;
    prefs.begin("lineage", false);

    prefs.putBytes("id", (const uint8_t*)&id, sizeof(GotchiID));

    for (int i = 0; i < 5; i++) {
        char key[8];
        snprintf(key, sizeof(key), "anc_%d", i);
        prefs.putBytes(key, (const uint8_t*)&ancestors[i], sizeof(GotchiAncestor));
    }

    prefs.putBytes("heritage", (const uint8_t*)&heritage, sizeof(GotchiHeritage));

    prefs.end();
}

void GotchiLineage::load(GotchiID& id, GotchiAncestor ancestors[5], GotchiHeritage& heritage) {
    Preferences prefs;
    prefs.begin("lineage", true);

    if (!prefs.isKey("id")) {
        prefs.end();
        return;
    }

    prefs.getBytes("id", (uint8_t*)&id, sizeof(GotchiID));

    for (int i = 0; i < 5; i++) {
        char key[8];
        snprintf(key, sizeof(key), "anc_%d", i);
        if (prefs.isKey(key)) {
            prefs.getBytes(key, (uint8_t*)&ancestors[i], sizeof(GotchiAncestor));
        } else {
            memset(&ancestors[i], 0, sizeof(GotchiAncestor));
        }
    }

    if (prefs.isKey("heritage")) {
        prefs.getBytes("heritage", (uint8_t*)&heritage, sizeof(GotchiHeritage));
    } else {
        memset(&heritage, 0, sizeof(GotchiHeritage));
    }

    prefs.end();
}

GotchiHeritage GotchiLineage::computeHeritage(const GotchiAncestor& dying, const GotchiAncestor ancestors[5]) {
    GotchiHeritage h = {};

    if (dying.days_lived > 14) {
        h.hatch_speedup = 0.2f;
    }

    if (dying.avg_mood_pct > 75) {
        h.bonus_mood = 10.0f;
    }

    if (dying.avg_health_pct > 75) {
        h.bonus_health = 10.0f;
    }

    if (dying.avg_mood_pct > 60 && dying.avg_health_pct > 60) {
        h.beauty_bonus = true;
    }

    return h;
}

void GotchiLineage::shiftAncestors(GotchiAncestor ancestors[5], const GotchiAncestor& new_ancestor) {
    for (int i = 4; i > 0; i--) {
        ancestors[i] = ancestors[i - 1];
    }
    ancestors[0] = new_ancestor;
}
