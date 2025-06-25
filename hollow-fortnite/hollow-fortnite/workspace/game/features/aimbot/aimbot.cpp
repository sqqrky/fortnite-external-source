#include <workspace/game/features/aimbot/aimbot.hpp>
#include <workspace/unreal/engine/engine.hpp>
#include <workspace/game/entity/enumerations/players.hpp>

static std::mt19937 rng( std::random_device { } ( ) );
static std::uniform_real_distribution< float > dist( -1.0, 1.0 );

double m_class::m_aimbot_query::get_cross_distance(double x, double y)
{
    return std::sqrt( std::pow( ( primitives::screen_width_center - x ), 2 ) + std::pow( ( primitives::screen_height_center - y ), 2 ) );
}

primitives::fvector m_class::m_aimbot_query::prediction_calculations( primitives::fvector target, primitives::fvector velocity, primitives::fvector local_velocity, float projectile_speed_scale, float projectile_gravity_scale, float distance ) {
    float vertical_distance = target.z - local_velocity.z;

    if ( vertical_distance > 0 ) {
        int exceed_count_70 = static_cast< int >( ( distance - 70 ) / 70 );
        int exceed_count_125 = static_cast< int >( ( distance - 125 ) / 125 );
        int exceed_count_150 = static_cast< int >( ( distance - 150 ) / 150 );

        if ( exceed_count_150 > 0 ) {
            projectile_gravity_scale -= 0.3f;
        } else if ( exceed_count_125 > 0 ) {
            projectile_gravity_scale -= 0.4f;
        } else if ( exceed_count_70 > 0 ) {
            projectile_gravity_scale -= 0.5f;
        }
    }

    float horizontal_time = distance / projectile_speed_scale;
    float vertical_time = distance / projectile_speed_scale;

    target.x += velocity.x * horizontal_time;
    target.y += velocity.y * horizontal_time;
    target.z += velocity.z * vertical_time + std::fabs( -980 * projectile_gravity_scale ) * 0.5f * ( vertical_time * vertical_time );

    return target;
}

primitives::frotator m_class::m_aimbot_query::find_look_at_rotation( primitives::fvector & start, primitives::fvector & target ) {
    primitives::fvector direction = target - start;
    direction = direction / std::sqrt( direction.length( ) );

    double yaw = std::atan2( direction.y, direction.x ) * ( 180.0 / std::numbers::pi );
    double pitch = std::atan2( direction.z, std::sqrt( direction.x * direction.x + direction.y * direction.y ) ) * ( 180.0 / std::numbers::pi );

    return primitives::frotator( pitch, yaw, 0.0 );
}

primitives::frotator m_class::m_aimbot_query::clamp_angle( primitives::frotator start_rotation, primitives::frotator end_rotation, float smoothing ) {
    primitives::frotator return_val = end_rotation;

    return_val.pitch -= start_rotation.pitch;
    return_val.yaw -= start_rotation.yaw;

    if ( return_val.yaw < -180.0f )
        return_val.yaw += 360.0f;

    if ( return_val.yaw > 180.0f )
        return_val.yaw -= 360.0f;

    if ( return_val.pitch < -74.0f )
        return_val.pitch = -74.0f;

    if ( return_val.pitch > 74.0f )
        return_val.pitch = 74.0f;

    return_val.pitch = ( return_val.pitch / smoothing ) + start_rotation.pitch;
    return_val.yaw = ( return_val.yaw / smoothing ) + start_rotation.yaw;

    if ( return_val.yaw < -180.0f )
        return_val.yaw += 360.0f;

    if ( return_val.yaw > 180.0f )
        return_val.yaw -= 360.0f;

    if ( return_val.pitch < -74.0f )
        return_val.pitch = -74.0f;

    if ( return_val.pitch > 74.0f )
        return_val.pitch = 74.0f;

    return return_val;
}

void m_class::m_aimbot_query::tick( primitives::fvector aim_location ) {
    auto aim_rotation = find_look_at_rotation( uengine::location, aim_location );

    if ( g_vars->aimbot.smoothing > 1 ) {
        aim_rotation = clamp_angle( uengine::rotation, aim_rotation, g_vars->aimbot.smoothing );
    }

    aim_rotation.pitch = aim_rotation.pitch - uengine::rotation.pitch;
    aim_rotation.yaw = aim_rotation.yaw - uengine::rotation.yaw;

    aim_rotation.normalize( );

    if ( std::abs( aim_rotation.pitch ) > 0.68f ) {
        auto scale_factor_pitch = 0.68f / abs( aim_rotation.pitch );
        aim_rotation.pitch *= scale_factor_pitch;
    }

    if ( std::abs( aim_rotation.yaw ) > 0.68f ) {
        auto scale_factor_yaw = 0.68f / abs( aim_rotation.yaw );
        aim_rotation.yaw *= scale_factor_yaw;
    }

    // write an frotator with your rotation here.
}