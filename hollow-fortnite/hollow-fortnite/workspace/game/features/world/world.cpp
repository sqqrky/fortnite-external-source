#include <workspace/game/features/world/world.hpp>
#include <workspace/game/entity/enumerations/players.hpp>
#include <dependencies/framework/framework.hpp>

void m_class::m_world_query::tick( ) {
    for ( ;; ) {
        std::vector< item > new_loot_cache;

        if ( cache->game_state->server_world_time( ) == 0.0f ) {
            std::cout << "[world tick] player is in lobby\n";
            m_e_world_cache.clear( );
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            continue;
        }

        auto item_levels = cache->g_world->item_levels();

        if (!item_levels.count) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        for ( int i = 0; i < item_levels.count; ++i ) {
            auto level = item_levels.get( i );
            if ( !level )
                continue;

            auto actors = level->actors ();
            if ( !actors.count )
                continue;

            for  (int j = 0; j < actors.count; ++j ) {
                auto actor = actors.get( j );
                if ( !actor )
                    continue;

                int index = actor->get_item_index( );
                if ( !index )
                    continue;
                
                std::string name = m_class::m_world_query::f_name::to_string( index );
                if ( name.empty( ) || !m_class::m_world_query::cache_names( name ) )
                    continue;

                if ( !m_class::m_world_query::cache_names( name ) ) {
                    continue;
                }

                auto loc = m_class::m_world_query::get_location( actor );
                if ( loc.is_zero( ) ) {
                    continue;
                }

                float dist = uengine::location.distance( loc ) / 100.0f;
                if ( dist > 100.0f ) {
                    continue;
                }

                item m_item;
                m_item.m_pawn = actor;
                m_item.m_name = std::move( name );
                m_item.m_distance = dist;

                new_loot_cache.push_back( std::move( m_item ) );
            }
        }

        {
            std::lock_guard< std::mutex > lock( m_w_mutex );
            m_e_world_cache = std::move( new_loot_cache );
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
    }
}

void m_class::m_world_query::render( ) {
    auto draw_list = ImGui::GetBackgroundDrawList( );
    if ( !draw_list )
        return;

    std::lock_guard< std::mutex > lock( m_w_mutex );

    for ( const auto & entity : m_e_world_cache ) {
        if ( entity.m_distance > 100.0f )
            continue;

        auto root = m_class::m_world_query::get_root( entity.m_pawn );
        if ( !root )
            continue;

        primitives::fvector pos = m_class::m_world_query::get_location( entity.m_pawn );
        if ( pos.is_zero( ) )
            continue;

        primitives::fvector2d screen = uengine::world_to_screen( pos );

        float dist_m = uengine::location.distance( pos ) / 100.0f;

        loot_type entity_type = m_class::m_world_query::get_entity_type( entity.m_name );

        bool should_render = false;
        std::string display_name;
        primitives::flinearcolor text_color;

        switch ( entity_type ) {
            case entity_chest:
                should_render = g_vars->visuals.floor.chest;
                display_name = encrypt( "Chest" );
                break;

            case entity_ammo:
                should_render = g_vars->visuals.floor.ammo;
                display_name = encrypt( "Ammo Box" );
                break;

            case entity_supply_drop:
                should_render = g_vars->visuals.floor.supply;
                display_name = encrypt( "Supply Drop" );
                break;

            case entity_llama:
                should_render = g_vars->visuals.floor.llama;
                display_name = encrypt( "Llama" );
                break;

            case entity_container:
                should_render = g_vars->visuals.floor.containers;
                display_name = encrypt( "Container" );
                break;

            case entity_loot:
            default: {
                std::string loot_name = m_class::m_world_query::get_pickup_name( entity.m_pawn );
                if ( loot_name.empty( ) || loot_name == encrypt( "Invalid" ) || loot_name == encrypt( "No Pickup" ) )
                    continue;

                enumerations::e_fort_rarity rarity = m_class::m_world_query::get_pickup_rarity( entity.m_pawn );

                switch ( rarity ) {
                case enumerations::e_fort_rarity::common:
                    should_render = g_vars->visuals.floor.rarity.common_loot;
                    text_color = framework->get_color( 128, 128, 128, 255 );
                    break;

                case enumerations::e_fort_rarity::uncommon:
                    should_render = g_vars->visuals.floor.rarity.uncommon_loot;
                    text_color = framework->get_color( 0, 255, 0, 255 );
                    break;

                case enumerations::e_fort_rarity::rare:
                    should_render = g_vars->visuals.floor.rarity.rare_loot;
                    text_color = framework->get_color( 0, 112, 221, 255 );
                    break;

                case enumerations::e_fort_rarity::epic:
                    should_render = g_vars->visuals.floor.rarity.epic_loot;
                    text_color = framework->get_color( 163, 53, 238, 255 ); 
                    break;

                case enumerations::e_fort_rarity::legendary:
                    should_render = g_vars->visuals.floor.rarity.legendary_loot;
                    text_color = framework->get_color( 255, 165, 0, 255 ); 
                    break;

                case enumerations::e_fort_rarity::mythic:
                    should_render = g_vars->visuals.floor.rarity.mythic_loot;
                    text_color = framework->get_color( 0, 255, 255, 255) ;
                    break;
                }


                display_name = loot_name;
                break;
            }
        }

        if ( !should_render )
            continue;

        char buffer[ 64 ];
        sprintf_s( buffer, encrypt( "%.1fm" ), dist_m );

        std::string full_text = display_name + encrypt( " (" ) + buffer + encrypt( ")" );
        ImVec2 text_size = ImGui::CalcTextSize( full_text.c_str( ) );
        ImVec2 pos_text( screen.x - text_size.x / 2, screen.y - 18 );

        framework->text( 
            draw_list, 
            pos_text.x, 
            pos_text.y, 
            text_color, 
            full_text.c_str( ), 
            1 );
    }
}
