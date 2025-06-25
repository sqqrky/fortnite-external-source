#include <workspace/game/entity/entity.hpp>
#include <workspace/unreal/core/sdk.hpp>
#include <workspace/game/entity/enumerations/players.hpp>
//#include <workspace/game/features/aimbot/aimbot.hpp>

std::unique_ptr<players::player> cache = std::make_unique<players::player>();
std::vector<players::player> entities_cache;

void m_class::m_tick_query::get_world( ) {
    cache->g_world = sdk::u_world::declare_member_uworld( );
    if ( !cache->g_world )
        return;

    cache->game_state = cache->g_world->game_state( );
    if ( !cache->game_state )
        return;

    cache->owning_game_instance = cache->g_world->game_instance( );
    if ( !cache->owning_game_instance )
        return;

    cache->local_player = cache->owning_game_instance->get_localplayer( );
    if ( !cache->local_player )
        return;

    cache->player_controller = cache->local_player->controller( );
    if ( !cache->player_controller )
        return;

    cache->acknowledged_pawn = cache->player_controller->acknowledged_pawn( );
}

void m_class::m_tick_query::get_actors( ) {
    if ( !cache->game_state )
        return;

    auto player_array = cache->game_state->player_array( ).get_iteration( );
    if ( player_array.empty( ) )
        return;

    entities_cache.clear( );
    entities_cache.reserve( player_array.size( ) );

    std::vector< sdk::a_fort_player_state_athena* > filtered_player_states;
    std::vector< sdk::a_fort_player_pawn* > filtered_pawns;

    filtered_player_states.reserve( player_array.size( ) );
    filtered_pawns.reserve( player_array.size( ) );

    for ( auto ptr : player_array ) {
        if ( !ptr )
            continue;

        auto player_state = reinterpret_cast< sdk::a_fort_player_state_athena* >( ptr );
        if ( !player_state )
            continue;

        auto pawn = player_state->pawn_private( );
        if ( !pawn )
            continue;

        if ( cache->acknowledged_pawn && pawn == cache->acknowledged_pawn )
            continue;

        filtered_player_states.push_back( player_state );
        filtered_pawns.push_back( pawn );
    }

    std::vector< sdk::u_skeletal_mesh_component* > meshes;
    meshes.reserve( filtered_pawns.size( ) );

    for ( auto pawn : filtered_pawns ) {
        meshes.push_back( pawn ? pawn->mesh( ) : nullptr );
    }

    std::vector< bool > visibility_flags;
    visibility_flags.reserve( meshes.size( ) );

    for ( auto mesh : meshes ) {
        visibility_flags.push_back( mesh && cache->g_world ? mesh->is_visible( reinterpret_cast< std::uintptr_t >( cache->g_world ) ) : false );
    }

    std::vector< bool > is_dying_flags;
    std::vector< bool > is_dbno_flags;
    is_dying_flags.reserve( filtered_player_states.size( ) );
    is_dbno_flags.reserve( filtered_player_states.size( ) );

    for ( auto player_state : filtered_player_states ) {
        is_dying_flags.push_back( player_state ? player_state->b_is_dying( ) : false );
        is_dbno_flags.push_back( player_state ? player_state->b_is_dbno( ) : false );
    }

    for ( std::size_t i = 0; i < filtered_player_states.size( ); ++i ) {
        auto player_state = filtered_player_states[ i ];
        if ( !player_state )
            continue;

        players::player entity{ };
        entity.player_state = reinterpret_cast< std::uintptr_t >( player_state );
        entity.pawn_private = reinterpret_cast< std::uintptr_t >( filtered_pawns[ i ] );
        entity.mesh = meshes[ i ];
        entity.is_visible = visibility_flags[ i ];
        entity.is_dying = is_dying_flags[ i ];
        entity.is_dbno = is_dbno_flags[ i ];
        entity.user_name = cache->game_state->get_user_name( player_state );
        entity.read_username = !entity.user_name.empty( );

        entities_cache.push_back( std::move( entity ) );
    }
}

auto m_class::m_tick_query::tick( ) noexcept -> void {
    SetThreadPriority( GetCurrentThread( ), THREAD_PRIORITY_HIGHEST );

    using clock = std::chrono::steady_clock;
    constexpr auto target_interval = std::chrono::milliseconds( 5 );
    float smoothed_delta = 5.0f;
    auto last_time = clock::now( );

    while ( true ) {
        const auto now = clock::now( );
        const auto delta = now - last_time;
        float delta_ms = std::chrono::duration< float, std::milli >( delta ).count( );
        smoothed_delta += 0.05f * ( delta_ms - smoothed_delta );

        if ( delta >= target_interval ) {
            auto processing_start = clock::now( );

            try {
                std::unique_lock< std::mutex > lock( tick_mutex, std::try_to_lock );
                if ( lock.owns_lock( ) ) {
                    m_class::m_tick_query::get_world( );
                    m_class::m_tick_query::get_actors( );
                    last_time = now;
                } else {
                    std::this_thread::sleep_for( std::chrono::milliseconds( 1 ) );
                }
            } catch ( const std::exception & e ) {
                printf( "%s%s\n", encrypt( "caught exception in tick, what: " ), e.what( ) );
                std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            } catch ( ... ) {
                printf( "%s\n", encrypt( "caught an unknown exception in tick, no what!!1" ) );
                std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
            }

            auto processing_end = clock::now( );
            auto processing_time = processing_end - processing_start;

            auto sleep_duration = target_interval > processing_time ? target_interval - processing_time : std::chrono::milliseconds( 0 );
            if ( sleep_duration.count( ) > 0 )
                std::this_thread::sleep_for( sleep_duration );
            else
                std::this_thread::yield( );
        } else {
            std::this_thread::sleep_for( target_interval - delta );
        }
    }
}