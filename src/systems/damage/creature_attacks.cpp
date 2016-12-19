#include "creature_attacks.hpp"
#include "../../messages/messages.hpp"
#include "../tasks/civ_dislike.hpp"
#include "../../components/game_stats.hpp"
#include "../weapons_helpers.hpp"
#include "../../components/item.hpp"
#include "../../components/species.hpp"
#include "../../raws/creatures.hpp"

using namespace rltk;

void creature_attacks_system::configure() {
    system_name = "Creature Attack System";
    subscribe_mbox<creature_attack_message>();
}

void creature_attacks_system::creature_attacks() {
    std::queue<creature_attack_message> * cattack = mbox<creature_attack_message>();
    while (!cattack->empty()) {
        creature_attack_message msg = cattack->front();
        cattack->pop();

        // Start by determining the attack
        auto attacker = entity(msg.attacker);
        if (!attacker) break;
        auto attack_species = attacker->component<species_t>();
        auto creaturefinder = get_creature_def(attack_species->tag);
        if (!creaturefinder) {
            std::cout << "Unable to find creature: " << attack_species->tag << "\n";
            break;
        }
        auto creature = creaturefinder.get();

        auto defender = entity(msg.victim);
        if (!defender) break;
        auto defender_stats = defender->component<game_stats_t>();

        for (const creature_attack_t &weapon : creature.attacks) {
            LOG ss;
            ss.other_name(msg.attacker)->text(" attacks ")->other_name(msg.victim)->text(" with its ")->col(rltk::colors::YELLOW)->text(weapon.type)->col(rltk::colors::WHITE)->text(". ");
            const int hit_roll = rng.roll_dice(1,20) + weapon.hit_bonus;
            const int target = calculate_armor_class(*defender) + stat_modifier(defender_stats->dexterity);
            if (hit_roll < target) {
                ss.text("The attack misses.");
            } else {
                const int damage = std::max(1,rng.roll_dice(weapon.damage_n_dice, weapon.damage_dice) + weapon.damage_mod);
                ss.text(std::string("The attack hits, for ")+std::to_string(damage)+std::string(" points of damage."));
                emit(inflict_damage_message{ msg.victim, damage, weapon.type });
            }
            emit_deferred(log_message{ss.chars});
        }
    }
}

void creature_attacks_system::update(const double ms) {
    creature_attacks();
}