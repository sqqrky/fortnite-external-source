#include <algorithm>
#include <workspace/unreal/engine/engine.hpp>
#include <workspace/game/entity/enumerations/players.hpp>

sdk::u_scene_view_state* get_view_state( ) {
    auto view_matrix = cache->local_player->view_state( );
    return view_matrix.get( 1 );
}

void uengine::update_matrix ( ) {
    if ( ! get_view_state( ) )
          return;

     auto projection = kernel->read< uengine::fmatrix > ( get_view_state ( ) + 0x930 );

     rotation.pitch = std::asin ( projection.z_plane.w ) * 180.0f / std::numbers::pi;
     rotation.yaw = std::atan2 ( projection.y_plane.w, projection.x_plane.w ) * 180.0f / std::numbers::pi;
     rotation.roll = 0.0f;

     uengine::location.x = projection.m[ 3 ][ 0 ];
     uengine::location.y = projection.m[ 3][ 1 ];
     uengine::location.z = projection.m[ 3 ][ 2 ];

     auto radians = 2.0f * std::atanf( 1.0f / static_cast< float > ( kernel->read< double >( get_view_state( ) + 0x730 )));

     field_of_view = radians * 180.0f / std::numbers::pi;
}

primitives::fvector2d uengine::world_to_screen(primitives::fvector world_location) {
    D3DMATRIX rotation_matrix = uengine::to_rotation_matrix(uengine::rotation);

    auto& axis_x = rotation_matrix.m[0];
    auto& axis_y = rotation_matrix.m[1];
    auto& axis_z = rotation_matrix.m[2];

    primitives::fvector delta = world_location - uengine::location;

    auto transformed_x = delta.dot(primitives::fvector(axis_y[0], axis_y[1], axis_y[2]));
    auto transformed_y = delta.dot(primitives::fvector(axis_z[0], axis_z[1], axis_z[2]));
    auto transformed_z = max(delta.dot(primitives::fvector(axis_x[0], axis_x[1], axis_x[2])), 1.0f);

    float fov_rad = uengine::field_of_view * std::numbers::pi / 180.0f;
    float fov_tan = tanf(fov_rad / 2.0f);

    float screen_x = primitives::screen_width_center + (transformed_x / fov_tan) * primitives::screen_width_center / transformed_z;
    float screen_y = primitives::screen_height_center - (transformed_y / fov_tan) * primitives::screen_width_center / transformed_z;

    return primitives::fvector2d(screen_x, screen_y);
}