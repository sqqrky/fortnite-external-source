#include <workspace/game/entity/entity.hpp>
#include <workspace/unreal/core/sdk.hpp>
#include <workspace/game/entity/enumerations/players.hpp>

std::unique_ptr<players::player> cache = std::make_unique<players::player>();
std::vector<players::player> entities_cache;

void m_class::m_tick_query::get_world( ) {
    std::scoped_lock lock( world_mutex );
    
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

std::unordered_map< std::uintptr_t, std::string > m_class::m_tick_query::username_cache;
std::mutex m_class::m_tick_query::username_cache_mutex;

// credits to whoever decided code should look like this, your a fucking goat.

void m_class::m_tick_query::get_actors( ) {
    if ( !cache->game_state )
        return;

    const auto player_array = cache->game_state->player_array( ).get_iteration( );
    if ( player_array.empty( ) )
        return;

    std::vector< players::player > local_entities;
    local_entities.reserve( player_array.size( ) );

    for ( auto raw : player_array ) {
        auto* player_state = reinterpret_cast< sdk::a_fort_player_state_athena* >( raw );
        if ( !player_state )
            continue;

        auto* pawn = player_state->pawn_private( );
        if ( !pawn || ( cache->acknowledged_pawn && pawn == cache->acknowledged_pawn ) )
            continue;

        auto* mesh = pawn->mesh( );
        bool visible = mesh && cache->g_world
            ? mesh->is_visible( reinterpret_cast< std::uintptr_t >( cache->g_world ) )
            : false;

        std::uintptr_t state_key = reinterpret_cast< std::uintptr_t >( player_state );
        std::string user_name;

        {
            std::scoped_lock lock( username_cache_mutex );
            auto it = username_cache.find( state_key );
            if ( it != username_cache.end( ) ) {
                user_name = it->second;
            } else {
                user_name = cache->game_state->get_user_name( player_state );
                if ( !user_name.empty( ) ) {
                    username_cache[ state_key ] = user_name; 
                }
            }
        }

        players::player entity{ };
        entity.player_state = state_key;
        entity.pawn_private = reinterpret_cast< std::uintptr_t >( pawn );
        entity.mesh = mesh;
        entity.is_visible = visible;
        entity.is_dying = player_state->b_is_dying( );
        entity.is_dbno = player_state->b_is_dbno( );
        entity.user_name = std::move( user_name );
        entity.read_username = !entity.user_name.empty( );

        local_entities.emplace_back( std::move( entity ) );
    }

    {
        std::scoped_lock lock( entity_mutex );
        entities_cache = std::move( local_entities );
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
