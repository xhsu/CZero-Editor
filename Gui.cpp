/*
* General Gui functions.
* Nov. 24 2021
*/

#include "Precompiled.hpp"

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
