/*
* Just... make sure that user won't get bored when zipping file.
* Nov 24 2021
*/

#include "Precompiled.hpp"

import UtlString;

void Gui::ZipProgress::Dialog(const char* pszTitle) noexcept
{
	m_iTotal = std::max<size_t>(1, m_iTotal);
	m_flPercentage = std::clamp<double>(double(m_iCur) / double(m_iTotal), 0, 1);

	const bool bDone = m_flPercentage >= 1.0;

	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal(pszTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
	{
		ImGui::TextUnformatted((*m_pszCurFile).c_str());
		ImGui::ProgressBar(m_flPercentage, ImVec2(-FLT_MIN, 0), UTIL_VarArgs("%d of %d", m_iCur.load(), m_iTotal.load()));
		ImGui::NewLine();

		if (!bDone)
			ImGui::BeginDisabled();

		ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2 - 60);
		if (ImGui::Button(bDone ? "Done" : "Please wait...", ImVec2(120, 24)) && bDone)
			ImGui::CloseCurrentPopup();

		if (!bDone)
			ImGui::EndDisabled();

		ImGui::EndPopup();
	}
}
