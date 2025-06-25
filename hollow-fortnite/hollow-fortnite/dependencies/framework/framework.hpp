#pragma once
#include <impl/includes.hpp>
#include <workspace/unreal/engine/engine.hpp>

using request = enum {
	m_endframe,
	m_rectangle,
	m_fill_rect,
	m_line,
	m_circle,
	m_text,
	m_boundingbox,
	m_boundingcorner,
	m_icon
};

using vec2 = struct {
	float x, y;
};

using rect_t = struct {
	float x, y, height, width;
};

using rectangle_t = struct {

	rect_t object;
	primitives::flinearcolor color;
	vec2 radius;
	bool rounded;

};

using bounding_t = struct {
	float left, right, top, bottom;
	primitives::flinearcolor color;
	bool rounded;
};

using line_t = struct {
	vec2 point1, point2;
	primitives::flinearcolor color;
	float stroke;
};

using icon_t = struct {
	vec2 point;
	primitives::flinearcolor color;
	float font_size;
	int icon;
};

using circle_t = struct {
	vec2 point;
	float radius;
	primitives::flinearcolor color;
	float stroke;
	bool filled;
};

using text_t = struct {

	std::uint32_t text_length;
	std::string text;
	vec2 point;
	primitives::flinearcolor color;
	bool center;
	bool menu;
	int weight;
};

namespace m_class {
	class m_framework_query {
	public:
		[[nodiscard]] primitives::flinearcolor get_color(float r, float g, float b, float a = 255);

		void text(ImDrawList* drawlist, int x, int y, primitives::flinearcolor color, const char* text, bool outlined);
		void rect(ImDrawList* draw_list, float x, float y, float width, float height, primitives::flinearcolor color, int thickness);
		void line(float x1, float y1, float x2, float y2, primitives::flinearcolor color, float stroke = 1.0f);
		void circle(float x, float y, float radius, primitives::flinearcolor color, float stroke, bool filled = false);
		void fov();

	};
} inline auto framework = std::make_unique<m_class::m_framework_query>();