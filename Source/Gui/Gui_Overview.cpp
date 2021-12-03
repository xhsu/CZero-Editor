/*
* Gui module for overview editor.
* Nov 03 2021
*/

#include "Precompiled.hpp"

import UtlColor;

inline ImVec4 ToImVec4(const Color4b& obj) noexcept
{
	return ImVec4(obj[0], obj[1], obj[2], obj[3]);
}

inline ImVec4 ToImVec4(const Color4f& obj) noexcept
{
	return ImVec4(obj[0], obj[1], obj[2], obj[3]);
}

static void Picker(const char* pszTitle, ImVec4& CurColor, const ImVec4& LastColor) noexcept
{
	if (ImGui::BeginPopup(pszTitle))
	{
		ImGui::Text("Pick a color for: %s", pszTitle);
		ImGui::Separator();

		ImGui::ColorPicker3("##picker", (float*)&CurColor, ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview);
		ImGui::SameLine();

		ImGui::BeginGroup(); // Lock X position
		ImGui::Text("Current");
		ImGui::ColorButton("##current", CurColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40));
		ImGui::Text("Previous");
		if (ImGui::ColorButton("##previous", LastColor, ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_AlphaPreviewHalf, ImVec2(60, 40)))
			CurColor = LastColor;
		ImGui::Separator();
		ImGui::Text("Palette");
		for (size_t n = 0; n < Gui::m_rgszPalette.size(); ++n)
		{
			ImVec4 Palette = ToImVec4(Gui::m_rgszPalette[n]);

			ImGui::PushID(n);
			if ((n % 8) != 0)
				ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);

			ImGuiColorEditFlags palette_button_flags = ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_NoPicker | ImGuiColorEditFlags_NoTooltip;
			if (ImGui::ColorButton("##palette", Palette, palette_button_flags, ImVec2(20, 20)))
				CurColor = ImVec4(Palette.x, Palette.y, Palette.z, CurColor.w); // Preserve alpha!

			// Allow user to drop colors into each palette entry. Note that ColorButton() is already a
			// drag source by default, unless specifying the ImGuiColorEditFlags_NoDragDrop flag.
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F); payload != nullptr)
					memcpy((float*)&Palette, payload->Data, sizeof(float) * 3);
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F); payload != nullptr)
					memcpy((float*)&Palette, payload->Data, sizeof(float) * 4);
				ImGui::EndDragDropTarget();
			}

			ImGui::PopID();
		}
		ImGui::EndGroup();
		ImGui::EndPopup();
	}
}

void Gui::Overview::DrawWindow(void) noexcept
{
	using namespace ::Overview;
	// #TODO_ADD_LOCK I am not sure about that..

	static ImVec4 LastColor;
	static constexpr std::array<const char*, 2> rgszTeams = { "T", "CT" };
	static int iTeamIndex = -1;

	if (!g_bShowOverviewWindow)
		return;

	if (ImGui::Begin("Overview", &g_bShowOverviewWindow, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize(ImVec2(400, 460));

		ImGui::InputText("Author", &m_szAuthor);
		ImGui::InputText("Title", &m_szTitle);
		ImGui::InputTextMultiline("Description", &m_szDescription);
		ImGui::InputText("URL", &m_szUrl);
		ImGui::Checkbox("SoloPlay", &m_bSoloPlay); ImGui::SameLine();
		ImGui::Checkbox("CoopPlay", &m_bSoloPlay);

		iTeamIndex = m_pszTeam == "T"s ? 0 : 1;
		if (ImGui::Combo("Team", &iTeamIndex, rgszTeams.data(), rgszTeams.size()))
			m_pszTeam = rgszTeams[iTeamIndex];

		if (ImGui::ColorButton("BGColor1##ColorButton", m_BGColor1))
		{
			ImGui::OpenPopup("BGColor1");
			LastColor = m_BGColor1;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Background Color 1");
		if (ImGui::ColorButton("BGColor2##ColorButton", m_BGColor2))
		{
			ImGui::OpenPopup("BGColor2");
			LastColor = m_BGColor2;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Background Color 2");
		if (ImGui::ColorButton("TextColor##ColorButton", m_TextColor))
		{
			ImGui::OpenPopup("TextColor");
			LastColor = m_TextColor;
		}
		ImGui::SameLine();
		ImGui::TextUnformatted("Text Color");
		Picker("BGColor1", m_BGColor1.Value, LastColor);
		Picker("BGColor2", m_BGColor2.Value, LastColor);
		Picker("TextColor", m_BGColor2.Value, LastColor);

#pragma region Bot profile path
		bool bFileExists = CZFile::Exists(m_BotProfile);
		if (!bFileExists)
			ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.98f, 0.2f, 1));

		ImGui::InputText("BotProfile", &m_BotProfile);	// #UNDONE_LONG_TERM

		if (!bFileExists)
			ImGui::PopStyleColor();
#pragma endregion Bot profile path
	}

	ImGui::End();
}
