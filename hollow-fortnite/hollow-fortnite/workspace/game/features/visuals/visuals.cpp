#include <workspace/game/features/visuals/visuals.hpp>
#include <workspace/game/features/aimbot/aimbot.hpp>
#include <workspace/unreal/engine/engine.hpp>
#include <dependencies/framework/framework.hpp>
#include <workspace/game/entity/enumerations/players.hpp>

// static inside of visuals.hpp, duh
std::mutex m_class::m_visuals_query::m_mutex;

// these are static inside of aimbot.hpp
float m_class::m_aimbot_query::projectile_speed = 0.0f;
float m_class::m_aimbot_query::projectile_gravity_scale = 0.0f;

void m_class::m_visuals_query::tick( ) {
    auto draw_list = ImGui::GetBackgroundDrawList( );
    if ( !draw_list )
        return;

    if ( entities_cache.empty( ) )
        return;

    std::vector< players::player > players_cache;
    {
        std::lock_guard< std::mutex > lock( m_mutex );
        if ( entities_cache.empty( ) )
            return;
        players_cache = entities_cache;
    }

    uengine::update_matrix( );

    std::size_t player_count = players_cache.size( );
    if ( player_count == 0 )
        return;

    float closest_distance = FLT_MAX;
    const players::player* closest_player = nullptr;

    for ( std::size_t j = 0; j < player_count; ++j ) {
        const auto& entity = players_cache[ j ];

        auto m_head = entity.mesh->get_bone_id( enumerations::head );
        auto m_root = entity.mesh->get_bone_id( enumerations::root );

        auto m_distance = uengine::location.distance( m_root ) / 100.0f;
        if ( m_distance >= g_vars->visuals.max_distance )
            continue;

        const auto is_in_lobby = entity.game_state->server_world_time( ) == 0.0f;

        auto head_screen = uengine::world_to_screen( { m_head.x, m_head.y, m_head.z } );
        auto root_screen = uengine::world_to_screen( { m_root.x, m_root.y, m_root.z } );

        auto head_box = uengine::world_to_screen({ m_head.x, m_head.y, m_head.z + 15 });

        float height = std::abs(head_box.y - root_screen.y);
        float width = height * 0.5f;

        auto box_color = framework->get_color( 255, 255, 255 );

        if ( g_vars->visuals.bounding_box )
            framework->rect(
             draw_list,
             head_box.x - width * 0.5f,
             head_box.y,
             width,
             height,
             box_color,
             g_vars->visuals.box_thickness
        );

        if ( g_vars->visuals.distance ) {
            auto distance = std::format( "{:.0f}m", m_distance );
            ImVec2 size = ImGui::CalcTextSize( distance.c_str( ) );

            float text_x = head_box.x - ( size.x * 0.5f );
            float text_y = root_screen.y - ( size.y * 0.5f ) + 7.0f; // it was inside of the boxes line

            framework->text(
                draw_list,
                static_cast< int >( text_x ),
                static_cast< int >( text_y ),
                primitives::flinearcolor( 255, 255, 255, 255 ),
                distance.c_str( ),
                true
            );
        }

        if ( g_vars->visuals.username ) {
            auto size = ImGui::CalcTextSize( entity.user_name.c_str( ) );
            framework->text(
                draw_list, 
                head_box.x - size.x * 0.5f, 
                head_box.y - 15.f,
                framework->get_color( 255, 255, 255 ),
                entity.user_name.c_str( ),
                g_vars->visuals.other.outlined_text 
            );
        }

        float dx = head_screen.x - primitives::screen_width_center;
        float dy = head_screen.y - primitives::screen_height_center;
        float dist_to_center = std::sqrt( dx * dx + dy * dy );

        if ( dist_to_center < g_vars->aimbot.fov && dist_to_center < closest_distance ) {
            closest_distance = dist_to_center;
            closest_player = &entity;

            primitives::fvector target_bone;
            if ( closest_player ) {
                switch ( g_vars->aimbot.hit_bone ) {
                    case 1: target_bone = closest_player->mesh->get_bone_id( enumerations::neck_02 ); break;
                    case 2: target_bone = closest_player->mesh->get_bone_id( enumerations::spine_05 ); break;
                    default: target_bone = m_head; break;
                }

                primitives::fvector position = target_bone;

                if ( g_vars->aimbot.prediction ) {
                    auto current_weapon = cache->acknowledged_pawn->current_weapon( );
                    if ( current_weapon ) {
                        std::uintptr_t weapon_address = reinterpret_cast< std::uintptr_t >( current_weapon );

                        std::vector< std::uintptr_t > weapon_vec = {
                            weapon_address + 0x2214,
                            weapon_address + 0x2218
                        };

                        std::vector< float > weapon_vals( 2 );
                        kernel->batch_read( weapon_vec, weapon_vals );

                        m_class::m_aimbot_query::projectile_speed = weapon_vals[ 0 ];
                        m_class::m_aimbot_query::projectile_gravity_scale = weapon_vals[ 1 ];

                        if ( m_class::m_aimbot_query::projectile_gravity_scale > 0.0f ) {
                            auto root_private = kernel->read< std::uintptr_t >( closest_player->pawn_private + 0x1b0 );
                            auto local_velocity = kernel->read< primitives::fvector >( cache->acknowledged_pawn + 0x180 );
                            auto comp_velocity = kernel->read< primitives::fvector >( root_private + 0x180 );
                            float head_distance = uengine::location.distance( target_bone );

                            position = m_class::m_aimbot_query::prediction_calculations(
                                target_bone,
                                comp_velocity,
                                local_velocity,
                                m_class::m_aimbot_query::projectile_speed,
                                m_class::m_aimbot_query::projectile_gravity_scale,
                                head_distance
                            );
                        }
                    }
                }

                auto target_screen = uengine::world_to_screen( position );

                if ( ( GetAsyncKeyState ( g_vars->aimbot.key ) & 0x8000 ) &&
                     g_vars->aimbot.enabled &&
                     !target_screen.is_zero( ) &&
                     m_class::m_aimbot_query::get_cross_distance( target_screen.x, target_screen.y ) <= g_vars->aimbot.fov ) {
                    m_class::m_aimbot_query::tick( position );
                }
            }
        }
    }
}