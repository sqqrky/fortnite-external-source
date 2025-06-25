#ifndef G_VARS_HPP
#define G_VARS_HPP

#include <locale>

class vars
{
public:
	struct
	{
		bool enabled = true;
		bool prediction = true;
		float fov = 600;
		float smoothing = 1.75;
		bool ignore_invisible = false;
		bool ignore_knocked = false;
		bool ignore_dying = false;
		int hit_bone = 0;
		int key = VK_RBUTTON;
		int aim_type = 0;

		struct
		{
			bool dot = false;
			bool line = false;
			bool fov = false;
			bool crosshair = false;
		} display;

		struct
		{
			bool enabled = true;
			bool always = true;
			bool shotgun = true;
			float ms_delay = 0.0f;
			float max_distance = 20.0f;
			bool has_clicked;
			int key;
			std::chrono::steady_clock::time_point tb_begin;
			std::chrono::steady_clock::time_point tb_end;
			int tb_time_since;
		} triggerbot;

	} aimbot;

	struct
	{
		bool enabled = true;
		bool box = false;
		bool img_box = false;
		bool bounding_box = true;
		bool cornered_box = false;
		bool three_dimensional_box = false;
		int box_style = 0;
		int box_thickness = 1.25;
		bool distance = true;
		bool weapon = true;
		bool ammo = true;
		bool username = true;
		bool skeleton = false;
		bool rank = false;

		struct
		{
			bool filled = true;
			bool outlined = true;
			bool outlined_text = true;
		} other;

		struct
		{
			bool weapon = true;
			bool chest = true;
			bool llama = false;
			bool ammo = true;
			bool vehicle = false;
			bool supply = false;
			bool containers = false;

			struct {
				bool common_loot = true;
				bool uncommon_loot = true;
				bool rare_loot = true;
				bool epic_loot = true;
				bool legendary_loot = true;
				bool mythic_loot = true;
			} rarity;

			float max_distance = 50;
		} floor;

		struct {
			bool enabled = true;
			int style = 1;
			bool distance = true;
			bool outlined_text = true;
			bool rings = false;
			bool gridded = true;
			bool cross = false;
			int alpha = 200;
			int radar_position_x = 25;
			int radar_position_y = 25;
			int radar_size = 150;
		} radar;

		float max_distance = 200.0f;
	} visuals;

	struct
	{
		bool vsync = true;
		bool show_menu = true;
		bool performance_mode = false;

		struct
		{
			bool logo = true;
			bool fps = true;
		} watermark;
	} misc;

}; inline std::unique_ptr<vars> g_vars = std::make_unique<vars>();

#endif