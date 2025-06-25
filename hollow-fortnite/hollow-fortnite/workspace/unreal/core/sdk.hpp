#pragma once
#include <impl/includes.hpp>
#include <workspace/unreal/engine/engine.hpp>
#include <workspace/unreal/core/math/math.hpp>
#include <workspace/game/entity/enumerations/enumerations.hpp>


// credits to whoever made this, it had a bunch of unnecessary code, so this is it organized
#define current_class reinterpret_cast< std::uint64_t >( this )

#define declare_member( type, name, offset ) type name( ) { return kernel->read< type >( current_class + offset ); } 
#define declare_member_bit( bit, name, offset ) bool name( ) { return bool( kernel->read<char>( current_class + offset ) & ( 1 << bit ) ); }

#define apply_member( type, name, offset ) void name( type val ) { kernel->write<type>( current_class + offset, val ); }
#define apply_member_bit( bit, name, offset ) void name( bool value ) { kernel->write< char >( kernel->read< char >( current_class + offset ) | ( 1 << bit ), value ); }

namespace sdk {
    struct fname {
        std::uint32_t idx;
    };

	struct fbox_sphere_bounds final
	{
	public:
		struct primitives::fvector orgin;
		struct primitives::fvector box_extent;
		double sphere_radius;
	};

	class u_object {
	public:
		declare_member(fname, name_private, 0x8);
	};

    class u_actor_component : public u_object {
    public:
    };

    class u_scene_component : public u_actor_component {
    public:
        declare_member( primitives::fvector, component_velocity, 0x180 );
        declare_member( primitives::fvector, relative_location, 0x138 );
    };

    class u_primitive_component : public u_scene_component {
    public:
        declare_member( float, last_render_time, 0x32c );
    };

    class u_mesh_component : public u_primitive_component {
    public:
    };

    class u_skinned_mesh_component : public u_mesh_component {
    public:
        declare_member( uengine::ftransform, component_to_world, 0x1e0 );
    } inline skinned_mesh;

	class u_skeletal_mesh_component : public u_skinned_mesh_component {
	public:
		auto is_visible( std::uintptr_t g_world ) -> bool {
			auto current_time = kernel->read< double >( g_world + 0x198 );
			auto mesh_render_time = kernel->read< float >( current_class + 0x32c );
			return ( current_time - mesh_render_time ) <= 0.06f;
		}

		declare_member( std::int32_t, is_cached, 0x5f8 );
		declare_member( uengine::tarray< uengine::ftransform >, bone_array, 0x5e8 + ( this->is_cached( ) * 0x60 ) );
		declare_member( D3DMATRIX, component_to_world, 0x1e0 );
		declare_member( fbox_sphere_bounds, get_bounds, 0xf8 );

		// for the love of god, use the defines above
		primitives::fvector get_bone_id( int index )
		{
			std::uintptr_t bones = kernel->read<std::uintptr_t>( current_class + 0x5e8 );
			if ( !bones) {
				bones = kernel->read< std::uintptr_t >( current_class + 0x5e8 + 0x10 );
			}
			auto bones_transform = kernel->read< uengine::ftransform >( bones + ( index * 0x60 ) );
			uengine::ftransform component_to_world = kernel->read< uengine::ftransform >( current_class + 0x1e0 );
			D3DMATRIX matrix = uengine::to_multiplication(bones_transform.to_matrix(), component_to_world.to_matrix());
			return primitives::fvector( matrix._41, matrix._42, matrix._43 );
		}
    };

    class a_actor : public u_object {
    public:
        declare_member( u_scene_component*, root_component, 0x1b0 );
        declare_member( std::uintptr_t, f_data, 0x20 );
        declare_member( std::int32_t, f_length, 0x28 );
    };

    class f_text : public a_actor {
    public:
        declare_member( a_actor*, name, 0x40 );
        declare_member( std::uintptr_t, data, 0x20 );
        declare_member( std::int32_t, length, 0x28 );
    };

    struct a_controller : public a_actor {

    public:
    };

    struct u_fort_item_definition : public f_text {
        declare_member( enumerations::e_fort_rarity, tier, 0xa2 );
        declare_member( enumerations::e_fort_rarity, type, 0x70 );
        declare_member( f_text*, display_name, 0x40 );
    };

    struct u_fort_world_item_definition : public u_fort_item_definition {
    public:
        declare_member( f_text*, weapon_display_name, 0x40 );
    };

    struct u_fort_weapon_item_defintion : public u_fort_world_item_definition {
    public:
    };

    struct u_fort_weapon_anim_set {
    public:
    };

    struct f_fort_ranged_weapon_recoil {
    public:
    };

    struct u_fort_weapon_mod_runtime_data : public u_object {
    public:
    };

    struct a_fort_weapon : public a_actor {
    public:
        declare_member( std::int32_t, ammo_count, 0x11e4 );
        declare_member( u_fort_weapon_item_defintion*, weapon_data, 0x6b8 );
        declare_member( bool, reloading, 0x4dc );
    };

    struct f_fort_item_entry {
        declare_member( u_fort_item_definition*, item_def, 0x18 );
    };

    class a_fort_pickup : public a_actor {
    public:
    };

    class a_pawn : public a_actor {
    private:
    public:
    };

    class a_character : public a_pawn {
    public:
        declare_member( u_skeletal_mesh_component*, mesh, 0x328 );
    };

    struct afgf_character : public a_character {
    public:
    };

	struct a_fort_pawn : public afgf_character {
	public:
		declare_member( f_text*, pickup_entry, 0x370 + 0x10 );
		declare_member( a_fort_weapon*, current_weapon, 0xaf0 );

		std::string retrieve_player_weapon() {
			auto current_weapon = this->current_weapon( );
			if (!current_weapon) return encrypt( "" );

			auto weapon_data = current_weapon->weapon_data( );
			if (!weapon_data) return encrypt( "" );

			auto display_name = weapon_data->weapon_display_name( );
			if (!display_name) return encrypt( "" );

			auto weapon_length = display_name->length( );
			if (weapon_length <= 0 || weapon_length > 128) return encrypt( "Building Plan" );

			auto data_ptr = display_name->data( );
			if ( !data_ptr ) return encrypt( "Building Plan" );

			wchar_t* weapon_name = nullptr;
			try {
				weapon_name = new wchar_t[ weapon_length + 1 ]( );
			}
			catch ( std::exception e ) {
				return encrypt( "caught exception in retrieve_player_weapon, what:", e );
			}
			catch ( ... ) {
				return encrypt( "Building Plan" );
			}

			const bool read_success = kernel->read_memory( data_ptr, weapon_name, weapon_length * sizeof( wchar_t ));
			if ( !read_success ) {
				delete[ ] weapon_name;
				return encrypt( "Building Plan" );
			}

			weapon_name[ weapon_length ] = L'\0';

			std::wstring wname( weapon_name );
			delete[ ] weapon_name;

			return std::string( wname.begin( ), wname.end( ) );
		}
	};

	struct f_replicated_athena_vehicle_attributes
	{
	public:
	};

	struct a_fort_athena_vehicle : public a_pawn
	{
	public:

	};

	struct a_fort_player_pawn : public a_fort_pawn
	{
	public:
		int get_item_index() const {
			return kernel->read<int>(current_class + 0x8);
		}
	};

	struct a_player_controller : public a_controller
	{
	public:

		declare_member(a_fort_player_pawn*, acknowledged_pawn, 0x350);
	};


	struct a_fort_player_pawn_athena : public a_fort_player_pawn
	{
	public:

		declare_member(a_player_controller*, controller, 0x2b0);
	};

	class a_player_state : public u_object
	{
	public:
		declare_member(a_fort_player_pawn_athena*, pawn_private, 0x320);

	public:

	};

	class a_fort_player_state : public a_player_state
	{
	public:
	};


	class a_fort_player_state_zone : public a_fort_player_state
	{
	public:

	};

	struct FRankedProgressReplicatedData {
		std::uintptr_t base;

		int32_t progress() const
		{
			return kernel->read<int32_t>(current_class + 0x10);
		}
	};


	struct HabaneroComponent {
		std::uintptr_t base;

		FRankedProgressReplicatedData ranked_progress() const {
			return FRankedProgressReplicatedData{ current_class + 0xd0 };
		}
	};

	class a_fort_player_state_athena : public a_fort_player_state_zone
	{
	public:
		declare_member(std::uintptr_t, habanero_component, 0xa60 )
		declare_member(std::int32_t, team_index, 0x1291 );
		declare_member(std::uintptr_t, platform_info, 0x430 );
		declare_member_bit(3, b_is_a_bot, 0x2b2 );

		declare_member_bit(4, b_is_dying, 0x728 );
		declare_member_bit(4, b_is_dbno, 0x9a1 );


		std::string get_platform()
		{
			auto test_platform = platform_info();

			wchar_t platform[64] = { 0 };
			kernel->read_memory((test_platform), reinterpret_cast<uint8_t*>(platform), sizeof(platform));

			std::wstring platform_wstr(platform);
			std::string platform_str(platform_wstr.begin(), platform_wstr.end());

			return platform_str;
		}
	};


	struct u_scene_view_state : public u_object
	{
	public:

	};


	struct u_localplayer : public u_object
	{
	public:

		declare_member(a_player_controller*, controller, 0x30);
		declare_member(uengine::tarray<u_scene_view_state*>, view_state, 0xd0);

	};

	struct u_level : public u_object
	{
	public:
		declare_member(uengine::tarray<a_fort_player_pawn*>, actors, 0x208);

	};

	struct u_game_instance : public u_object
	{
	public:
		declare_member(uengine::tarray<u_localplayer*>, localplayers, 0x38);

		auto get_localplayer() -> u_localplayer* {
			auto vec = localplayers().get_iteration();
			if (vec.empty()) return nullptr;
			return vec[0];
		}
	};

	struct a_game_state_base : public u_object
	{
	public:
		declare_member(float, server_world_time, 0x2e0);
		declare_member(uengine::tarray<a_fort_player_state_athena*>, player_array, 0x2c0);

		// decrypting the players widestr username -> str
		auto get_user_name(a_fort_player_state_athena* state) -> std::string {
			__int64 f_text;
			int length;

			if (this->server_world_time() == 0.0f) {
				length = kernel->read< int >(state + 0x340 + 0x8);
				f_text = kernel->read< __int64 >(state + 0x340);
			}
			else {
				auto f_string = kernel->read< __int64 >(state + 0xb40);
				length = kernel->read< int >(f_string + 16);
				f_text = kernel->read< __int64 >(f_string + 8);
			}

			wchar_t* buffer = new wchar_t[length];
			kernel->read_memory(f_text, buffer, length * sizeof(wchar_t));

			std::int64_t v6 = length;
			if (!v6) {
				delete[] buffer;
				return std::string(encrypt("AI"));
			}

			char v21 = v6 - 1;
			if (!(std::uint32_t)v6) v21 = 0;
			int v22 = 0;
			std::uint16_t* v23 = (std::uint16_t*)buffer;

			for (int i = v21 & 3; ; *v23++ += i & 7) {
				int v25 = v6 - 1;
				if (!(std::uint32_t)v6) v25 = 0;
				if (v22 >= v25) break;
				i += 3;
				++v22;
			}

			std::wstring username(buffer);
			delete[] buffer;

			return std::string(username.begin(), username.end());
		}
	};

	struct u_world : public u_object
	{
	public:

		declare_member(uengine::tarray<u_level*>, item_levels, 0x1e8)

		declare_member(a_game_state_base*, game_state, 0x1d0 );

		declare_member(u_game_instance*, game_instance, 0x248 );

		static inline auto declare_member_uworld( ) -> u_world* {
			return kernel->read<u_world*>(kernel->m_base_address + 0x185565c8);
		}
	};
}