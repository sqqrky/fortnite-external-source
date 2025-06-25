#pragma once
#include <impl/includes.hpp>
#include <shared_mutex>
#include <unordered_set>
#include <workspace/unreal/core/sdk.hpp>

namespace m_class {
	class m_tick_query {
	private:
		static inline std::unordered_map< std::uintptr_t, std::string > cached_names;

		static inline std::vector<std::uintptr_t> mesh_addresses;
		static inline std::vector<std::uintptr_t> mesh_pointers;
		static inline std::vector<std::uintptr_t> dying_addresses, downed_addresses;

		static inline std::shared_mutex name_mutex;
		static inline std::mutex entity_mutex;
		static inline std::mutex world_data_mutex;
		static inline std::mutex tick_mutex;
	public:
		static void get_world();
		static void get_actors();
		static void tick() noexcept;
	};
}