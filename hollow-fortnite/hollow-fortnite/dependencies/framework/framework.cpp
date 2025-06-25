#include <dependencies/framework/framework.hpp>

auto m_class::m_framework_query::get_color(float r, float g, float b, float a) -> primitives::flinearcolor {
	return primitives::flinearcolor{
		r,
		g,
		b,
		a
	};
}

auto m_class::m_framework_query::fov() -> void
{
	float aim_radius = (g_vars->aimbot.fov * primitives::screen_width_center / uengine::field_of_view) / 2;

	if (g_vars->visuals.other.outlined)
	{
		this->circle(
			primitives::screen_width_center, primitives::screen_height_center,
			aim_radius + 1.f,
			this->get_color(0, 0, 0, 255),
			1.f
		);
	}

	this->circle(
		primitives::screen_width_center, primitives::screen_height_center,
		aim_radius,
		this->get_color(255, 255, 255, 255),
		1.f
	);
}
void m_class::m_framework_query::text(
	ImDrawList* drawlist,
	int x,
	int y,
	primitives::flinearcolor color,
	const char* text,
	bool outlined) {

	if (outlined)
	{
		ImVec2 offsets[] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

		for (const auto& offset : offsets)
		{
			drawlist->AddText(ImVec2(x + offset.x, y + offset.y), ImColor(0, 0, 0, 255), text);
		}
	}
	ImU32 im_color = IM_COL32(color.r, color.g, color.b, color.a);
	drawlist->AddText(ImVec2(x, y), im_color, text);
}

void m_class::m_framework_query::line(float x1, float y1, float x2, float y2, primitives::flinearcolor color, float stroke) {
	ImU32 col = IM_COL32(color.r, color.g, color.b, color.a);

	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();
	draw_list->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), col, stroke);
}

void m_class::m_framework_query::rect(
	ImDrawList* draw_list,
	float x,
	float y,
	float width,
	float height,
	primitives::flinearcolor color,
	int thickness) {

	if (g_vars->visuals.other.filled) {
		ImU32 im_color = IM_COL32(color.r, color.g, color.b, color.a);
		draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x + width, y + height), ImColor(0, 0, 0, 85), 0, 0);
	}

	if (g_vars->visuals.other.outlined) {
		ImU32 im_color = IM_COL32(color.r, color.g, color.b, color.a);
		draw_list->AddRect(ImVec2(x - 1, y - 1), ImVec2(x + width + 1, y + height + 1), ImColor(0, 0, 0, 255), 0, 0, thickness);
	}

	ImU32 im_color = IM_COL32(color.r, color.g, color.b, color.a);
	draw_list->AddRect(ImVec2(x, y), ImVec2(x + width, y + height), im_color, 0, 0, thickness);
}

void m_class::m_framework_query::circle(float x, float y, float radius, primitives::flinearcolor color, float stroke, bool filled) {
	const auto draw_list = ImGui::GetBackgroundDrawList();

	ImU32 im_color = IM_COL32(color.r, color.g, color.b, color.a);

	if (filled) {
		draw_list->AddCircleFilled(ImVec2(x, y), radius, im_color, 64);
	}
	else {
		draw_list->AddCircle(ImVec2(x, y), radius, im_color, 64, stroke);
	}
}