/*
* The graphical interface with Maps.
* Nov. 23 2021
*/

#include "Precompiled.hpp"

import UtlString;


static void RightclickMenu(const Map_t& Map) noexcept
{
	bool bOpenSuccessDialog = false, bStartPackingMap = false;

	if (ImGui::BeginPopupContextItem())
	{
		if (ImGui::MenuItem("Generate map resource report"))
		{
			bOpenSuccessDialog = true;

			std::ofstream hFile(Map.m_szName + ".res");	// #FOPEN
			hFile << ::Maps::ListResources(Map);
			hFile.flush();
			hFile.close();	// #FCLOSE
		}

		if (ImGui::MenuItem("Pack this map"))
		{
			bStartPackingMap = true;
			Gui::ZipProgress::m_iCur = 0;
			Gui::ZipProgress::m_iTotal = 7;
			*Gui::ZipProgress::m_pszCurFile = "";

			std::thread t([&Map](void)
				{
					auto szZipFile = Map.m_szName + ".zip";
					std::remove(szZipFile.c_str());

					auto rgszUnlistedRes = ::Maps::GetUnlistedResources(Map.m_szName);
					for (const auto& szResource : rgszUnlistedRes)
					{
						++Gui::ZipProgress::m_iCur;
						*Gui::ZipProgress::m_pszCurFile = "[Soft dependency] " + szResource;

						if (auto szAbsPath = CZFile::GetAbsPath(szResource); fs::exists(szAbsPath))
							ZipFile::AddFile(szZipFile, szAbsPath, szResource, LzmaMethod::Create());
					}

					Gui::ZipProgress::m_iCur = 0;
					Gui::ZipProgress::m_iTotal = Map.m_rgszResources.size();
					*Gui::ZipProgress::m_pszCurFile = "";

					for (const auto& szResource : Map.m_rgszResources)
					{
						++Gui::ZipProgress::m_iCur;
						*Gui::ZipProgress::m_pszCurFile = "[Hard dependency] " + szResource;

						if (auto szAbsPath = CZFile::GetAbsPath(szResource); fs::exists(szAbsPath))
							ZipFile::AddFile(szZipFile, szAbsPath, szResource, LzmaMethod::Create());
					}

					Gui::ZipProgress::m_iCur = 0;
					Gui::ZipProgress::m_iTotal = 2;
					*Gui::ZipProgress::m_pszCurFile = "Packing BSP file...";

					ZipFile::AddFile(szZipFile, Map.m_Path.string(), "maps/" + Map.m_Path.filename().string(), LzmaMethod::Create());

					Gui::ZipProgress::m_iCur = 1;
					*Gui::ZipProgress::m_pszCurFile = "Writing analyzing report as comment...";

					auto pZipFile = ZipFile::Open(szZipFile);
					pZipFile->SetComment(::Maps::ListResources(Map));
					ZipFile::SaveAndClose(pZipFile, szZipFile);

					Gui::ZipProgress::m_iCur = 2;
					*Gui::ZipProgress::m_pszCurFile = "All done!";
				}
			);

			t.detach();
		}

		ImGui::EndPopup();
	}

	if (bStartPackingMap)
		ImGui::OpenPopup(("Packing files for " + Map.m_szName + "...").c_str());

	Gui::ZipProgress::Dialog(("Packing files for " + Map.m_szName + "...").c_str());

	if (bOpenSuccessDialog)
		ImGui::OpenPopup(("Success##" + Map.m_szName).c_str());

	Gui::fnErrorDialog<true>(	// The address of these rvalues cannot outlive rendering process.
		("Success##" + Map.m_szName).c_str(),
		("File saved as: " + Map.m_szName + ".res").c_str()
		);
}

void Gui::Maps::DrawWindow(void) noexcept

{
	if (g_bitsAsyncStatus & Async_e::UPDATING_MAPS_INFO || !g_bShowMapsWindow)
		return;

	const std::unique_lock Lock(::Maps::Mutex, std::try_to_lock);
	if (!Lock.owns_lock())
		return;

	if (ImGui::Begin("Maps## of independent window", &g_bShowMapsWindow, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize(ImVec2(240, 480));
		SelectionInterface(&RightclickMenu);
	}

	ImGui::End();
}

void Gui::Maps::SelectionInterface(void (*pfnPostAddingItem)(const Map_t& CurMap), std::string* pszSelectedMap) noexcept
{
	Maps::Filter.Draw("Search");

	for (const auto& [szName, Map] : g_Maps)
	{
		if (!Maps::Filter.PassFilter(szName.c_str()))
			continue;

		if (ImGui::Selectable(szName.c_str(), pszSelectedMap && *pszSelectedMap == szName) && pszSelectedMap)
			*pszSelectedMap = szName;

		if (pfnPostAddingItem)
			pfnPostAddingItem(Map);

		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();

			ImGui::TextWrapped(Map.m_Path.string().c_str());

			if (Map.m_bThumbnailExists)
			{
				ImGui::Image((void*)(intptr_t)Map.m_Thumbnail.m_iTexId, Map.m_Thumbnail.Size());
				ImGui::SameLine();
			}
			else
			{
				ImGui::Bullet(); ImGui::SameLine();
				ImGui::TextColored(IMGUI_YELLOW, "Thumbnail no found.");
			}

			if (Map.m_bWiderPreviewExists)
			{
				ImGui::Image((void*)(intptr_t)Map.m_WiderPreview.m_iTexId, Map.m_WiderPreview.Size());
			}
			else
			{
				if (!Map.m_bThumbnailExists)	// If a map has only Thumbnail, do not draw our text along with it.
				{
					ImGui::Bullet();
					ImGui::SameLine();
				}

				ImGui::TextColored(IMGUI_YELLOW, "Preview image no found.");
			}

			ImGui::Bullet(); ImGui::SameLine();
			ImGui::TextColored(Map.m_bBriefFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "Brief intro document %s.", Map.m_bBriefFileExists ? "found" : "no found");

			ImGui::Bullet(); ImGui::SameLine();
			ImGui::TextColored(Map.m_bDetailFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "HD texture definition %s.", Map.m_bDetailFileExists ? "found" : "no found");

			ImGui::Bullet(); ImGui::SameLine();
			ImGui::TextColored(Map.m_bNavFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "BOT navigation file %s.", Map.m_bDetailFileExists ? "found" : "no found");

			ImGui::Bullet(); ImGui::SameLine();
			ImGui::TextColored(Map.m_bOverviewFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "Overview-related files %s.", Map.m_bDetailFileExists ? "found" : "no found");

			ImGui::TextWrapped("\nRequired resources for this map:");

			for (const auto& Res : Map.m_rgszResources)
			{
				bool b = CZFile::Exists(Res);

				ImGui::Bullet(); ImGui::SameLine();
				ImGui::TextColored(b ? IMGUI_GREEN : IMGUI_RED, Res.c_str());
			}

			ImGui::EndTooltip();
		}
	}
}
