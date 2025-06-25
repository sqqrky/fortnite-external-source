#pragma once
#include <impl/includes.hpp>
#include <workspace/unreal/engine/engine.hpp>
#include <workspace/unreal/core/sdk.hpp>

namespace players {
	struct player {
		// world

		sdk::u_world* g_world = 0;
		sdk::a_game_state_base* game_state = 0;
		sdk::u_game_instance* owning_game_instance = 0;
		sdk::u_localplayer* local_player = 0;
		sdk::a_fort_player_pawn* acknowledged_pawn = 0;
		sdk::a_player_controller* player_controller = 0;

		// player
		std::uintptr_t player_array = 0;
		std::uintptr_t player_state = 0;
		std::uintptr_t pawn_private = 0;
		sdk::u_skeletal_mesh_component* mesh = 0;

		std::string user_name;

		bool read_username;
		bool is_visible;

		char is_dying = 0;
		char is_dbno = 0;
	};
} extern std::unique_ptr<players::player> cache;
extern std::vector<players::player> entities_cache;
