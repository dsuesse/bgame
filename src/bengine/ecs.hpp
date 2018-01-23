#pragma once

#include "cereal_include.hpp"
#include "ecs_impl.hpp"

namespace bengine {

    /* Public interface to allow existing calls to continue to work */

    extern ecs default_ecs;

    inline entity_t * entity(ecs &ECS, const std::size_t id) noexcept {
        return ECS.entity(id);
    }

    inline entity_t * entity(const std::size_t id) noexcept {
        return entity(default_ecs, id);
    }

    inline entity_t * create_entity(ecs &ECS) {
        return ECS.create_entity();
    }

    inline entity_t * create_entity() {
        return create_entity(default_ecs);
    }

    inline entity_t * create_entity(ecs &ECS, const std::size_t new_id) {
        return ECS.create_entity(new_id);
    }

    inline entity_t * create_entity(const std::size_t new_id) {
        return create_entity(default_ecs, new_id);
    }

    inline void delete_entity(ecs &ECS, const std::size_t id) noexcept {
        ECS.delete_entity(id);
    }

    inline void delete_entity(const std::size_t id) noexcept {
        delete_entity(default_ecs, id);
    }

    inline void delete_entity(ecs &ECS, entity_t &e) noexcept {
        ECS.delete_entity(e);
    }

    inline void delete_entity(entity_t &e) noexcept {
        delete_entity(default_ecs, e);
    }

    inline void delete_all_entities(ecs &ECS) noexcept {
        ECS.delete_all_entities();
    }

    inline void delete_all_entities() noexcept {
        delete_all_entities(default_ecs);
    }

    template<class C>
    inline void delete_component(ecs &ECS, const std::size_t entity_id, bool delete_entity_if_empty=false) noexcept {
        ECS.delete_component<C>(entity_id, delete_entity_if_empty);
    }

    template<class C>
    inline void delete_component(const std::size_t entity_id, bool delete_entity_if_empty=false) noexcept {
        delete_component<C>(default_ecs, entity_id, delete_entity_if_empty);
    }

    template<class C>
    inline std::vector<entity_t *> entities_with_component(ecs &ECS) {
        return ECS.entities_with_component<C>();
    }

    template<class C>
    inline std::vector<entity_t *> entities_with_component() {
        return entities_with_component<C>(default_ecs);
    }

    template <class C>
    inline void all_components(ecs &ECS, typename std::function<void(entity_t &, C &)> func) {
        ECS.all_components<C>(func);
    }

    template <class C>
    inline void all_components(typename std::function<void(entity_t &, C &)> func) {
        all_components<C>(default_ecs, func);
    }

    template <typename... Cs, typename F>
    inline void each(ecs &ECS, F callback) {
        ECS.each<Cs...>(callback);
    }

    template <typename... Cs, typename F>
    inline void each(F callback) {
        each<Cs...>(default_ecs, callback);
    }

	template <typename EXCLUDE, typename... Cs, typename F>
	inline void each_without(ecs &ECS, F callback) {
		ECS.each_without<EXCLUDE, Cs...>(callback);
	}

	template <typename EXCLUDE, typename... Cs, typename F>
	inline void each_without(F callback) {
		each_without<EXCLUDE, Cs...>(default_ecs, callback);
	}

    template <typename... Cs, typename P, typename F>
    inline void each_if(ecs &ECS, P&& predicate, F callback) {
        ECS.each_if<Cs...>(predicate, callback);
    }

    template <typename... Cs, typename P, typename F>
    inline void each_if(P&& predicate, F callback) {
        each_if<Cs...>(default_ecs, predicate, callback);
    }

    inline void ecs_garbage_collect(ecs &ECS) {
        ECS.ecs_garbage_collect();
    }

    inline void ecs_garbage_collect() {
        ecs_garbage_collect(default_ecs);
    }

    inline void ecs_save(ecs &ECS, std::unique_ptr<std::ofstream> &lbfile) {
        ECS.ecs_save(lbfile);
    }

    inline void ecs_save(std::unique_ptr<std::ofstream> &lbfile) {
        ecs_save(default_ecs, lbfile);
    }

    inline void ecs_load(ecs &ECS, std::unique_ptr<std::ifstream> &lbfile) {
        ECS.ecs_load(lbfile);
    }

    inline void ecs_load(std::unique_ptr<std::ifstream> &lbfile) {
        ecs_load(default_ecs, lbfile);
    }

    inline std::string ecs_profile_dump(ecs &ECS) {
        return ECS.ecs_profile_dump();
    }

    inline std::string ecs_profile_dump() {
        return ecs_profile_dump(default_ecs);
    }
}
