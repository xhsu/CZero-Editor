/*
* Use ImGui to generate a Location window.
* Nov 18 2021
*/

#include "Precompiled.hpp"

import UtlString;

static inline Locus_t LocusCopy;



Gui::EditorResult_e Gui::Locations::EditingDialog(const char* pszTitle, Locus_t* pLocus) noexcept
{
	Gui::EditorResult_e iRet = Gui::EditorResult_e::UNTOUCHED;

	// Always center this window when appearing
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// Location editor.
	if (ImGui::BeginPopupModal(pszTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		// List all potential teammates in the current difficulty.
		// Why? Because they can't be assign as your enemy at the same time.
		Names_t& BotTeammates = ::MissionPack::CareerGames[Gui::Locations::CurBrowsing].m_rgszCharacters;

		// Title picture.
		if (ImGui::ImageButton((void*)(intptr_t)g_Maps[pLocus->m_szMap].m_WiderPreview.m_iTexId, g_Maps[pLocus->m_szMap].m_WiderPreview.Size()))
			ImGui::OpenPopup("Map Selection");

#pragma region Map Selector
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal("Map Selection"))
		{
			ImGui::SetWindowSize(ImVec2(240, 480));
			Gui::Maps::SelectionInterface(nullptr, &pLocus->m_szMap);	// #TODO_PREVENT_SAME_MAP_DUP

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() / 2.0f - 60);

			ImGui::NewLine();
			if (ImGui::Button("Close", ImVec2(120, 24)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
#pragma endregion Map Selector

		if (ImGui::CollapsingHeader(UTIL_VarArgs("%d Enem%s selected###EnemySelection", pLocus->m_rgszBots.size(), pLocus->m_rgszBots.size() < 2 ? "y" : "ies")) &&
			ImGui::BeginListBox("##Enemys_ListBox", ImVec2(-FLT_MIN, 7 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (const auto& Character : g_BotProfiles)
			{
				if (std::find(BotTeammates.begin(), BotTeammates.end(), Character.m_szName) != BotTeammates.end())
					continue;	// Skip drawing if our candidate is our potential teammate.

				auto it = std::find(pLocus->m_rgszBots.begin(), pLocus->m_rgszBots.end(), Character.m_szName);
				bool bEnrolled = it != pLocus->m_rgszBots.end();

				if (ImGui::Selectable(Character.m_szName.c_str(), bEnrolled))
				{
					if (bEnrolled)	// Select enrolled character -> rule them out.
						pLocus->m_rgszBots.erase(it);
					else
						pLocus->m_rgszBots.emplace_back(Character.m_szName);
				}

				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					Gui::BotProfile::Summary(&Character);
					ImGui::EndTooltip();
				}
			}

			ImGui::EndListBox();
		}

		if (ImGui::CollapsingHeader("Options", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputInt("Min Enemies", &pLocus->m_iMinEnemies, 1, 5, ImGuiInputTextFlags_CharsDecimal);
			ImGui::InputInt("Threshold", &pLocus->m_iThreshold, 1, 5, ImGuiInputTextFlags_CharsDecimal);
			ImGui::Checkbox("Friendly Fire", &pLocus->m_bFriendlyFire);
			ImGui::InputText("Console Command(s)", &pLocus->m_szConsoleCommands);
		}

		// Actual tasks.
		if (ImGui::CollapsingHeader("Task(s)", ImGuiTreeNodeFlags_DefaultOpen))
		{
			int iIdentifierIndex = 0;
			for (auto itTask = pLocus->m_Tasks.begin(); itTask != pLocus->m_Tasks.end(); /* Do nothing */)
			{
				Task_t& Task = *itTask;
				bool bTreeNodeExpanded = ImGui::TreeNode(UTIL_VarArgs("%s###task%d", Task.ToString().c_str(), iIdentifierIndex));

				if (!bTreeNodeExpanded && ImGui::IsItemHovered())
					ImGui::SetTooltip("Right-click to add or delete.\nDrag and draw to reorder.");

				// Right-click menu
				if (!bTreeNodeExpanded && ImGui::BeginPopupContextItem())
				{
					if (ImGui::Selectable("Insert new task"))
						pLocus->m_Tasks.push_back(Task_t{});

					ImGui::BeginDisabled(pLocus->m_Tasks.size() < 2);
					bool bRemoved = false;
					if (ImGui::Selectable("Delete"))	// Must have at least one task.
					{
						itTask = pLocus->m_Tasks.erase(itTask);
						bRemoved = true;
					}
					if (pLocus->m_Tasks.size() < 2 && ImGui::IsItemHovered())
						ImGui::SetTooltip("You must have at least one task.");	// #TODO how to show this tooltip while I was disabled?
					ImGui::EndDisabled();

					ImGui::EndPopup();

					if (bRemoved)
						continue;	// Skip the it++;
				}

				// Reordering.
				if (!bTreeNodeExpanded && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
				{
					ImGui::SetDragDropPayload("itTask", &itTask, sizeof(itTask));

					// Display preview
					ImGui::Text("Reordering task: %s", g_rgszTaskNames[(size_t)Task.m_iType]);
					ImGui::EndDragDropSource();
				}
				if (!bTreeNodeExpanded && ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("itTask"))
					{
						IM_ASSERT(payload->DataSize == sizeof(itTask));
						pLocus->m_Tasks.splice(itTask, pLocus->m_Tasks, *(decltype(itTask)*)payload->Data);
					}

					ImGui::EndDragDropTarget();
				}

				// Task editor.
				if (bTreeNodeExpanded)
				{
					int iTaskType = (int)Task.m_iType;
					ImGui::Combo("Task Type", &iTaskType, g_rgszTaskNames, IM_ARRAYSIZE(g_rgszTaskNames));
					Task.m_iType = (TaskType_e)iTaskType;
					Task.SanityCheck();	// For example, you reassign this task from 'kill' to 'killall', in which you can't have any parameter at all.

					if (Task.m_iType == TaskType_e::winfast)
						ImGui::InputInt("In 'X' seconds", &Task.m_iCount, 1, 15, ImGuiInputTextFlags_CharsDecimal);
					else if ((1 << iTaskType) & Tasks::REQ_COUNT)
						ImGui::InputInt("Do 'X' times", &Task.m_iCount, 1, 5, ImGuiInputTextFlags_CharsDecimal);

					if ((1 << iTaskType) & Tasks::REQ_WEAPON)
					{
						if (ImGui::BeginCombo("Required Weapon", Task.m_szWeapon.c_str(), ImGuiComboFlags_None))
						{
							for (int i = 0; i < _countof(g_rgszWeaponNames); i++)
							{
								if (!g_rgbIsTaskWeapon[i])
									continue;

								const bool bIsSelected = Task.m_szWeapon == g_rgszWeaponNames[i];
								if (ImGui::Selectable(g_rgszWeaponNames[i], bIsSelected))
									Task.m_szWeapon = g_rgszWeaponNames[i];

								if (bIsSelected)
									ImGui::SetItemDefaultFocus();
							}

							ImGui::EndCombo();
						}
					}

					if ((1 << iTaskType) & Tasks::SURVIVE)
						ImGui::Checkbox("Survive the round", &Task.m_bSurvive);

					if ((1 << iTaskType) & Tasks::INAROW)
						ImGui::Checkbox("Finish tasks in a row", &Task.m_bInARow);

					ImGui::TreePop();
				}

				++iIdentifierIndex;
				++itTask;
			}
		}

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			iRet = EditorResult_e::SAVED;
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(120, 0)))
		{
			iRet = EditorResult_e::DISCARD;
			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}

	return iRet;
}

void Gui::Locations::DrawWindow(void) noexcept
{
	if (g_bitsAsyncStatus & (Async_e::UPDATING_MISSION_PACK_INFO | Async_e::UPDATING_MAPS_INFO) || !g_bShowLociWindow)	// The drawing is using map thumbnail.
		return;

	const std::unique_lock Lock1(::MissionPack::Mutex, std::try_to_lock);
	const std::unique_lock Lock2(::Maps::Mutex, std::try_to_lock);
	if (!Lock1.owns_lock() || !Lock2.owns_lock())
		return;

	if (ImGui::Begin("Loci", &g_bShowLociWindow, ImGuiWindowFlags_NoResize))
	{
		constexpr ImGuiTableFlags bitsTableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBody;
		const auto vecSize = ImVec2(128 * 3 + 48, ImGui::GetTextLineHeightWithSpacing() * 5 + 128 * 3);
		const auto vecWindowSize = ImVec2(vecSize.x + 16, vecSize.y + 70);

		ImGui::SetWindowSize(vecWindowSize);

		if (ImGui::BeginTabBar("TabBar: Loci", ImGuiTabBarFlags_None))
		{
			for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
			{
				if (!fs::exists(::MissionPack::Files[i + ::MissionPack::FILE_EASY]))
					continue;

				if (!ImGui::BeginTabItem(g_rgszDifficultyNames[(size_t)i], nullptr, (g_iSetDifficulty == i) ? ImGuiTabItemFlags_SetSelected : ImGuiTabItemFlags_None))
					continue;

				// #POTENTIAL_BUG Recursing once.
				Gui::Locations::LastBrowsing = Gui::Locations::CurBrowsing;
				Gui::Locations::CurBrowsing = i;
				Gui::CheckDifficultySync();

				// Tab switched!
				// #POTENTIAL_BUG recursing once.
				if (Gui::Locations::LastBrowsing != Gui::Locations::CurBrowsing)
					g_iSetDifficulty = Gui::Locations::CurBrowsing;

				auto& CareerGame = ::MissionPack::CareerGames[i];
				auto& szCurDifficulty = g_rgszDifficultyNames[(size_t)i];

				ImGui::BeginChild("LocationsTable", vecSize, true, ImGuiWindowFlags_HorizontalScrollbar);

				const auto yInit = ImGui::GetCursorPosY();
				float x = ImGui::GetCursorPosX(), y = yInit;
				for (auto itLocus = CareerGame.m_Loci.begin(); itLocus != CareerGame.m_Loci.end(); itLocus++)
				{
					auto& Locus = *itLocus;

					if (y > ImGui::GetWindowContentRegionMax().y - ImGui::GetWindowContentRegionMin().y)
					{
						x += 128 + 24;	// Space out for a little will feel better.
						y = yInit;
					}

					ImGui::SetCursorPos(ImVec2(x, y));
					ImGui::TextUnformatted(Locus.m_szMap.c_str());

					y += ImGui::GetTextLineHeight();
					ImGui::SetCursorPos(ImVec2(x, y));

					ImGui::PushID((szCurDifficulty + Locus.m_szMap).c_str());
					bool bShouldOpen = ImGui::ImageButton((void*)(intptr_t)g_Maps[Locus.m_szMap].m_Thumbnail.m_iTexId, g_Maps[Locus.m_szMap].m_Thumbnail.Size());
					ImGui::PopID();

					bool bShouldInsert = false, bShouldDel = false;
					if (ImGui::BeginPopupContextItem(("Right-click menu: "s + szCurDifficulty + Locus.m_szMap).c_str()))
					{
						bShouldInsert = ImGui::MenuItem("Insert...");
						bShouldDel = ImGui::MenuItem(("Delete location '" + Locus.m_szMap + '\'').c_str());
						ImGui::EndPopup();
					}

					if (bShouldInsert)
					{
						ImGui::OpenPopup(("Add a location...##" + Locus.m_szMap + szCurDifficulty).c_str());
						LocusCopy.Reset();
						LocusCopy.m_Tasks.emplace_back("kill 1");	// Add at least one task, or it would cause error.
					}
					else if (bShouldDel)
					{
						itLocus = CareerGame.m_Loci.erase(itLocus);
						break;	// Skip this frame, reset the drawing process.
					}
					else if (bShouldOpen)
					{
						ImGui::OpenPopup(UTIL_VarArgs("%s##%s", Locus.m_szMap.c_str(), szCurDifficulty));
						LocusCopy = Locus;	// Make a copy only once. Or our changes will not be kept after 1 frame.
					}

					// Our buttons are both drag sources and drag targets here!
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
					{
						// Set payload to carry the index of our item (could be anything)
						ImGui::SetDragDropPayload("LocusIterator", &itLocus, sizeof(itLocus));

						// Preview popup.
						ImGui::Text(Locus.m_szMap.c_str());
						ImGui::Image((void*)(intptr_t)g_Maps[Locus.m_szMap].m_Thumbnail.m_iTexId, g_Maps[Locus.m_szMap].m_Thumbnail.Size());

						ImGui::EndDragDropSource();
					}
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload("LocusIterator"))
						{
							IM_ASSERT(Payload->DataSize == sizeof(decltype(itLocus)));

							auto itDraggedLocus = *(decltype(itLocus)*)Payload->Data;
							CareerGame.m_Loci.splice(itLocus, CareerGame.m_Loci, itDraggedLocus);
							break;	// Reset the drawing process and start over. Although Splice() doesn't affect the iterator, but it potentially removes the locations we drawn or removes the future.
						}

						ImGui::EndDragDropTarget();
					}

					if (EditingDialog(UTIL_VarArgs("%s##%s", Locus.m_szMap.c_str(), szCurDifficulty), &LocusCopy) == EditorResult_e::SAVED)
						Locus = std::move(LocusCopy);

					if (EditingDialog(("Add a location...##" + Locus.m_szMap + szCurDifficulty).c_str(), &LocusCopy) == EditorResult_e::SAVED)
						CareerGame.m_Loci.insert(itLocus, std::move(LocusCopy));

					y += 128 + 8;	// Assume image sized 128*128.
				}

				ImGui::EndChild();

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}
