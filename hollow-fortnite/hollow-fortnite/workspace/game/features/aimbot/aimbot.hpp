#pragma once
#include <impl/includes.hpp>
#include <workspace/unreal/core/math/math.hpp>

namespace m_class {
	class m_aimbot_query {
	private: 
		std::mutex m_mutex; 
	public:
		struct projectile_properties {
			int speed;
			double gravity;
		};

		primitives::fvector local_velocity;
		primitives::fvector component_velocity;

		static float projectile_speed;
		static float projectile_gravity_scale;

		static double get_cross_distance (double x, double y );
		static primitives::fvector prediction_calculations( primitives::fvector target, primitives::fvector velocity, primitives::fvector local_velocity, float projectile_speed_scale, float projectile_gravity_scale, float distance );
		static primitives::frotator find_look_at_rotation( primitives::fvector& start, primitives::fvector& target );
		static primitives::frotator clamp_angle( primitives::frotator start_rotation, primitives::frotator end_rotation, float smoothing );
		static void tick( primitives::fvector aim_location );
	};
}