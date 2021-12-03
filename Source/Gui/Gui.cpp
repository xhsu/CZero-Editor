/*
* General Gui functions.
* Nov. 24 2021
*/

#include "Precompiled.hpp"

import UtlColor;

const std::array<Color4f, 32> Gui::m_rgszPalette =
{
	Color4f::HSV(0.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(1.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(2.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(3.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(4.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(5.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(6.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(7.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(8.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(9.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(10.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(11.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(12.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(13.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(14.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(15.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(16.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(17.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(18.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(19.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(20.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(21.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(22.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(23.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(24.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(25.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(26.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(27.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(28.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(29.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(30.0 / 31.0 * 360.0, 0.8, 0.8),
	Color4f::HSV(31.0 / 31.0 * 360.0, 0.8, 0.8),
};

void Gui::CheckDifficultySync(void) noexcept
{
	if (
		g_iSetDifficulty == Gui::Locations::CurBrowsing
		&& g_iSetDifficulty == Gui::MissionPack::CurBrowsing
		)
	{
		g_iSetDifficulty = Difficulty_e::_LAST;	// Unset
	}
}

void Gui::About(void) noexcept
{
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (!ImGui::BeginPopupModal("About", nullptr, ImGuiWindowFlags_NoResize))
		return;

	ImGui::TextUnformatted("Author: Hydrogen");
	ImGui::TextUnformatted("Version: Beta 4");
	ImGui::TextUnformatted("https://github.com/xhsu/CZero-Editor");
	ImGui::TextUnformatted("xujiananxm@gmail.com");

	ImGui::NewLine();
	ImGui::TextUnformatted("Dependency project & Thanks to:");
	ImGui::Bullet(); ImGui::SameLine(); ImGui::TextUnformatted("freetype (https://github.com/ubawurinna/freetype-windows-binaries)");
	ImGui::Bullet(); ImGui::SameLine(); ImGui::TextUnformatted("glfw (https://github.com/glfw/glfw)");
	ImGui::Bullet(); ImGui::SameLine(); ImGui::TextUnformatted("ImGui (https://github.com/ocornut/imgui)");
	ImGui::Bullet(); ImGui::SameLine(); ImGui::TextUnformatted("stb (https://github.com/nothings/stb)");
	ImGui::Bullet(); ImGui::SameLine(); ImGui::TextUnformatted("ziplib (https://bitbucket.org/wbenny/ziplib)");

	ImGui::NewLine();

	ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 60);
	if (ImGui::Button("Close", ImVec2(120, 24)))
		ImGui::CloseCurrentPopup();

	ImGui::EndPopup();
}
