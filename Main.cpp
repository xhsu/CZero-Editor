#include "Precompiled.hpp"


import UtlKeyValues;
import UtlString;


void ListKeyValue(ValveKeyValues* pkv)
{
	if (!pkv)
		return;

	if (!ImGui::TreeNode(pkv->GetName()))
		return;

	ValveKeyValues* pSub = pkv->GetFirstKeyValue();
	while (pSub)
	{
		ImGui::Bullet();
		ImGui::SameLine();
		ImGui::TextWrapped("(Value) %s: %s", pSub->GetName(), pSub->GetValue<const char*>());
		pSub = pSub->GetNextKeyValue();
	}

	pSub = pkv->GetFirstSubkey();
	while (pSub)
	{
		ListKeyValue(pSub);	// New tree included.
		pSub = pSub->GetNextSubkey();
	}

	ImGui::TreePop();
}

void HelpMarker(const char* desc, ...)
{
	va_list argptr{};
	static constexpr size_t BUF_LEN = 2048;
	static char rgsz[BUF_LEN] = "\0";

	va_start(argptr, desc);
	_vsnprintf_s(rgsz, BUF_LEN, desc, argptr);
	va_end(argptr);

	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(rgsz);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void MainMenuBar(void)
{
	bool bSaveAsSelected = false;	// One frame variable.
	static std::string szSaveTo;	// But this shouldn't.

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save", "Ctrl+S") && !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO))
				MissionPack::Save();

			if (ImGui::MenuItem("Save as...", "Ctrl+Alt+S") && !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO))
				bSaveAsSelected = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Config", "", &g_bShowConfigWindow);
			ImGui::Separator();
			ImGui::MenuItem("Campaign", "F1", &g_bShowCampaignWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));
			ImGui::MenuItem("Loci", "F2", &g_bShowLociWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));
			ImGui::MenuItem("Maps", "F3", &g_bShowMapsWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MAPS_INFO));
			ImGui::MenuItem("BOTs", "F4", &g_bShowBotsWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (bSaveAsSelected)
		ImGui::OpenPopup("Save as...");

		// Always center this window when appearing
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// Modal of Inserting character into list.
	if (ImGui::BeginPopupModal("Save as...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Save as... (Full mission pack path)");
		ImGui::InputText("##SaveTo", &szSaveTo);

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			MissionPack::Save(szSaveTo);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void ConfigWindow(void)
{
	static bool bFirstOpen = true;
	static std::unordered_map<fs::path, MissionPack::Files_t> KnownMods;
	static const auto fnOnGamePathChanged = [](void)
	{
		g_bCurGamePathValid = fs::exists(g_szInputGamePath + "\\liblist.gam"s);

		if (g_bCurGamePathValid)
		{
			g_GamePath = g_szInputGamePath;
			CZFile::Update();

			KnownMods.clear();

			// Add official pack first.
			MissionPack::CompileFileListOfTurtleRockCounterTerrorist(
				KnownMods[g_GamePath],
				g_GamePath
			);

			for (const auto& hEntry : fs::directory_iterator(g_szInputGamePath + "\\MissionPacks\\"))
			{
				if (KnownMods.contains(hEntry))
					continue;

				if (hEntry.is_directory() && fs::exists(hEntry.path().c_str() + L"\\Overview.vdf"s) && hEntry.path().filename().c_str() != L"TurtleRockCounterTerrorist"s)
					MissionPack::CompileFileList(KnownMods[hEntry.path()], hEntry.path());
			}

			//std::thread t(
			//	[](void)
			//	{
					// #UNTESTED Try quick selection.

					g_bitsAsyncStatus |= Async_e::UPDATING_MAPS_INFO;
					Maps::Load();	// This has to happen after all potential hlmod folders were enlisted.
					g_bitsAsyncStatus &= ~Async_e::UPDATING_MAPS_INFO;

			//	}
			//);
			//t.detach();

			// Load skin portraits.
			BotProfileMgr::LoadSkinThumbnails();
		}

		//g_bitsAsyncStatus |= Async_e::REQ_UPDATE_GAME_INFO;
	};

	if (!g_bShowConfigWindow)
		return;

	if (ImGui::Begin("Config", &g_bShowConfigWindow))
	{
		if (bFirstOpen)
		{
			g_szInputGamePath = g_Config->GetValue<std::string>("LastGamePath");
			bFirstOpen = false;

			if (g_szInputGamePath.length())
				fnOnGamePathChanged();
		}

		ImGui::Text("CZero game path:");
		if (ImGui::InputText("##Game", &g_szInputGamePath))
			fnOnGamePathChanged();

		ImGui::SameLine();
		ImGui::TextColored(g_bCurGamePathValid ? IMGUI_GREEN : IMGUI_RED, g_bCurGamePathValid ? "Valid" : "Invalid");

		// #TODO add detection for CZCareerFix.amxx

		if (!MissionPack::Name.empty())
		{
			ImGui::BulletText((g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO) ? "Loading: %s" : "Current mission pack: %s", MissionPack::Name.c_str());

			if (!(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO) && MissionPack::Thumbnail.m_iTexId)
				ImGui::Image((void*)(intptr_t)MissionPack::Thumbnail.m_iTexId, MissionPack::Thumbnail.Size());
		}

		if (!KnownMods.empty() && ImGui::CollapsingHeader("Mods", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& [hPath, Mod] : KnownMods)
			{
				if (ImGui::Selectable(hPath.filename().string().c_str()))
				{
					//std::thread t(
						[&hPath](void)
						{
							if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO)
								return;

							g_bitsAsyncStatus |= Async_e::UPDATING_MISSION_PACK_INFO;
							MissionPack::LoadFolder(hPath);
							g_bitsAsyncStatus &= ~Async_e::UPDATING_MISSION_PACK_INFO;
						}
					//);
						();
					//t.detach();
				}
			}
		}
	}

	ImGui::End();	// Path
}

void CampaignWindow(void)
{
	static struct  
	{
		Names_t m_Enrolled{};
		bool m_bShouldEnter{ false };
		Names_t::iterator m_itInsertingPos;

	} InsertionModal;

	static struct  
	{
		Name_t m_szEnrolled{ ""s };
		bool m_bShouldEnter{ false };
		Names_t::iterator m_itReplacingPos;

	} ReplacementModal;

	if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO || !g_bShowCampaignWindow)
		return;

	const std::unique_lock Lock(MissionPack::Mutex, std::try_to_lock);
	if (!Lock.owns_lock())
		return;

	if (ImGui::Begin("Campaign", &g_bShowCampaignWindow))
	{
		if (ImGui::BeginTabBar("TabBar: Campaign", ImGuiTabBarFlags_None))
		{
			for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
			{
				if (!fs::exists(MissionPack::Files[i + MissionPack::FILE_EASY]))
					continue;

				if (!ImGui::BeginTabItem(g_rgszDifficultyNames[(size_t)i]))
					continue;

				MissionPack::CurBrowsing = i;	// Switched to this tab.
				auto& CareerGame = MissionPack::CareerGames[i];

				if (ImGui::CollapsingHeader("Generic", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::InputInt("Initial Points", &CareerGame.m_iInitialPoints, 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Match Wins", &CareerGame.m_iMatchWins, 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Match Win By", &CareerGame.m_iMatchWinBy, 1, 5, ImGuiInputTextFlags_CharsDecimal);
				}

				if (ImGui::CollapsingHeader("Characters", ImGuiTreeNodeFlags_None))
				{
					// Enlist all characters in current difficulty.
					for (auto iter = CareerGame.m_rgszCharacters.begin(); iter != CareerGame.m_rgszCharacters.end(); /* Do nothing */ )
					{
						if (ImGui::Selectable(iter->c_str()))
						{
							g_bShowBotsWindow = true;
							Gui::BotProfile::m_iSetTab = Gui::BotProfile::TAB_CHARACTERS;
							strcpy_s(Gui::BotProfile::Filter.InputBuf, iter->c_str());
							Gui::BotProfile::Filter.Build();
							ImGui::SetWindowFocus("BOTs");
						}

						// Right-click the name.
						if (ImGui::BeginPopupContextItem())
						{
							if ((InsertionModal.m_bShouldEnter = ImGui::Selectable(UTIL_VarArgs("Insert before %s", iter->c_str()))) == true)
								InsertionModal.m_itInsertingPos = iter;

							else if ((InsertionModal.m_bShouldEnter = ImGui::Selectable("Insert at the end")) == true)
								InsertionModal.m_itInsertingPos = CareerGame.m_rgszCharacters.end();

							else if ((ReplacementModal.m_bShouldEnter = ImGui::Selectable(UTIL_VarArgs("Replace %s", iter->c_str()))) == true)
								ReplacementModal.m_itReplacingPos = iter;

							else if (ImGui::Selectable(UTIL_VarArgs("Rule out %s", iter->c_str())))
							{
								iter = CareerGame.m_rgszCharacters.erase(iter);
								goto LAB_SKIP_INCREMENT;
							}

							ImGui::EndPopup();
						}

						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::TextUnformatted("Right-click to edit.\nDraw and draw item to reorder.\n");
							Gui::BotProfile::Summary(BotProfileMgr::Find(*iter));
							ImGui::EndTooltip();
						}

						// Drag and draw.
						if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
						{
							bool bMovingUp = (ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y < 0.f);
							if (iter == CareerGame.m_rgszCharacters.begin() && bMovingUp)	// The first element shouldnot be able to moving up.
								goto LAB_CONTINUE;

							auto iterMovingTo = iter;
							bMovingUp ? iterMovingTo-- : iterMovingTo++;

							if (iterMovingTo == CareerGame.m_rgszCharacters.end())	// You can't moving to ... obliterate his name.
								goto LAB_CONTINUE;

							std::swap(*iter, *iterMovingTo);	// Have to dereference to actually swap.
							ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
						}

					LAB_CONTINUE:;
						++iter;
					LAB_SKIP_INCREMENT:;
					}

#pragma region Character insertion modal
					if (InsertionModal.m_bShouldEnter)
						ImGui::OpenPopup("Inserting character...");

					// Always center this window when appearing
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					// Modal of Inserting character into list.
					if (ImGui::BeginPopupModal("Inserting character...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						InsertionModal.m_bShouldEnter = false;	// Clear the enter flag, but keep the iterator value.

						if (ImGui::BeginListBox("##Add_character_ListBox", ImVec2(-FLT_MIN, 21 * ImGui::GetTextLineHeightWithSpacing())))
						{
							for (const auto& Character : g_BotProfiles)
							{
								auto it = std::find(CareerGame.m_rgszCharacters.begin(), CareerGame.m_rgszCharacters.end(), Character.m_szName);

								bool bAlreadyEnrolled = it != CareerGame.m_rgszCharacters.end();
								if (bAlreadyEnrolled)
									continue;

								it = std::find(InsertionModal.m_Enrolled.begin(), InsertionModal.m_Enrolled.end(), Character.m_szName);

								bAlreadyEnrolled = it != InsertionModal.m_Enrolled.end();

								if (ImGui::Selectable(Character.m_szName.c_str(), bAlreadyEnrolled))
								{
									if (bAlreadyEnrolled)	// Which means deselect.
										InsertionModal.m_Enrolled.erase(it);
									else
										InsertionModal.m_Enrolled.emplace_back(Character.m_szName);
								}
							}

							ImGui::EndListBox();
						}

						if (ImGui::Button("OK", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							CareerGame.m_rgszCharacters.insert(InsertionModal.m_itInsertingPos, InsertionModal.m_Enrolled.begin(), InsertionModal.m_Enrolled.end());
							InsertionModal.m_Enrolled.clear();
						}

						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();

						if (ImGui::Button("Cancel", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							InsertionModal.m_Enrolled.clear();
						}

						ImGui::EndPopup();
					}
#pragma endregion Character insertion modal

#pragma region Character replacement modal
					if (ReplacementModal.m_bShouldEnter)
						ImGui::OpenPopup("Replace character...");

					if (ImGui::BeginPopupModal("Replace character...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ReplacementModal.m_bShouldEnter = false;	// Clear the enter flag, but keep the iterator value.

						if (ImGui::BeginListBox("##Add_character_ListBox", ImVec2(-FLT_MIN, 21 * ImGui::GetTextLineHeightWithSpacing())))
						{
							for (const auto& Character : g_BotProfiles)
							{
								auto it = std::find_if(CareerGame.m_rgszCharacters.begin(), CareerGame.m_rgszCharacters.end(),
									[&Character](const std::string& szName)
									{
										return szName == Character.m_szName;
									}
								);

								bool bAlreadyEnrolled = it != CareerGame.m_rgszCharacters.end();
								if (bAlreadyEnrolled)
									continue;

								bAlreadyEnrolled = Character.m_szName == ReplacementModal.m_szEnrolled;

								if (ImGui::Selectable(Character.m_szName.c_str(), bAlreadyEnrolled))
								{
									if (bAlreadyEnrolled)	// Which means deselect.
										ReplacementModal.m_szEnrolled.clear();
									else
										ReplacementModal.m_szEnrolled = Character.m_szName;
								}
							}

							ImGui::EndListBox();
						}

						if (ImGui::Button("OK", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							*ReplacementModal.m_itReplacingPos = std::move(ReplacementModal.m_szEnrolled);
							ReplacementModal.m_szEnrolled.clear();
						}

						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();

						if (ImGui::Button("Cancel", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							ReplacementModal.m_szEnrolled.clear();
						}

						ImGui::EndPopup();
					}
#pragma endregion Character replacement modal

					// Collapsing header. Nothing to pop.
				}

				if (ImGui::CollapsingHeader("Cost Availability", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::InputInt("Tier 1", &CareerGame.m_rgiCostAvailability[0], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 2", &CareerGame.m_rgiCostAvailability[1], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 3", &CareerGame.m_rgiCostAvailability[2], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 4", &CareerGame.m_rgiCostAvailability[3], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 5", &CareerGame.m_rgiCostAvailability[4], 1, 5, ImGuiInputTextFlags_CharsDecimal);
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void LocusWindow(void)
{
	if (g_bitsAsyncStatus & (Async_e::UPDATING_MISSION_PACK_INFO | Async_e::UPDATING_MAPS_INFO) || !g_bShowLociWindow)	// The drawing is using map thumbnail.
		return;

	const std::unique_lock Lock1(MissionPack::Mutex, std::try_to_lock);
	const std::unique_lock Lock2(Maps::Mutex, std::try_to_lock);
	if (!Lock1.owns_lock() || !Lock2.owns_lock())
		return;

	if (ImGui::Begin("Loci", &g_bShowLociWindow, ImGuiWindowFlags_NoResize))
	{
		constexpr ImGuiTableFlags bitsTableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBody;
		const auto vecSize = ImVec2(128 * 3 + 48, ImGui::GetTextLineHeightWithSpacing() * 3 + 128 * 3);
		const auto vecWindowSize = ImVec2(vecSize.x + 16, vecSize.y + 70);

		ImGui::SetWindowSize(vecWindowSize);

		if (ImGui::BeginTabBar("TabBar: Loci", ImGuiTabBarFlags_None))
		{
			for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
			{
				if (!fs::exists(MissionPack::Files[i + MissionPack::FILE_EASY]))
					continue;

				if (!ImGui::BeginTabItem(g_rgszDifficultyNames[(size_t)i]))
					continue;

				auto& CareerGame = MissionPack::CareerGames[i];
				auto& szCurDifficulty = g_rgszDifficultyNames[(size_t)i];

				// Table of all locations.
				if (ImGui::BeginTable("Table: Loci", 3, bitsTableFlags, vecSize))
				{
					static bool bTableInit[(size_t)Difficulty_e::_LAST] = { false, false, false, false };
					if (!bTableInit[(size_t)i])	// What the fuck, imgui?
					{
						bTableInit[(size_t)i] = true;
						ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
						ImGui::TableSetupColumn("##1", ImGuiTableColumnFlags_None);
						ImGui::TableSetupColumn("##2", ImGuiTableColumnFlags_None);
						ImGui::TableSetupColumn("##3", ImGuiTableColumnFlags_None);
						ImGui::TableHeadersRow();
					}

					for (auto itLocus = CareerGame.m_Loci.begin(); itLocus != CareerGame.m_Loci.end(); itLocus++)
					{
						auto& Locus = *itLocus;
						static int iColumnCount[(size_t)Difficulty_e::_LAST] = { 0, 0, 0 };

						if (iColumnCount[(size_t)i] >= 3)
						{
							ImGui::TableNextRow();
							iColumnCount[(size_t)i] = 0;
						}

						ImGui::TableSetColumnIndex(iColumnCount[(size_t)i]);
						ImGui::Text(Locus.m_szMap.c_str());

						ImGui::PushID((szCurDifficulty + Locus.m_szMap).c_str());
						bool bShouldOpen = ImGui::ImageButton((void*)(intptr_t)g_Maps[Locus.m_szMap].m_Thumbnail.m_iTexId, g_Maps[Locus.m_szMap].m_Thumbnail.Size());
						ImGui::PopID();

						static Locus_t LocusCopy{};	// Make a copy if we needs to enter editor.
						if (bShouldOpen)
						{
							ImGui::OpenPopup(UTIL_VarArgs("%s##%s", Locus.m_szMap.c_str(), szCurDifficulty));
							LocusCopy = Locus;	// Make a copy only once. Or our changes will be kept after 1 frame.
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
								std::swap(*itLocus, *itDraggedLocus);
							}

							ImGui::EndDragDropTarget();
						}

						// Always center this window when appearing
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

						// Location editor.
						if (ImGui::BeginPopupModal(UTIL_VarArgs("%s##%s", Locus.m_szMap.c_str(), szCurDifficulty), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
						{
							// List all potential teammates in the current difficulty.
							// Why? Because they can't be assign as your enemy at the same time.
							Names_t& BotTeammates = MissionPack::CareerGames[i].m_rgszCharacters;

							// Title picture. #TODO make this a button such that we can select map via this?
							ImGui::Image((void*)(intptr_t)g_Maps[Locus.m_szMap].m_WiderPreview.m_iTexId, g_Maps[Locus.m_szMap].m_WiderPreview.Size());

							if (ImGui::CollapsingHeader(UTIL_VarArgs("%d Enem%s selected###EnemySelection", LocusCopy.m_rgszBots.size(), LocusCopy.m_rgszBots.size() < 2 ? "y" : "ies")) &&
								ImGui::BeginListBox("##Enemys_ListBox", ImVec2(-FLT_MIN, 7 * ImGui::GetTextLineHeightWithSpacing())))
							{
								for (const auto& Character : g_BotProfiles)
								{
									if (std::find(BotTeammates.begin(), BotTeammates.end(), Character.m_szName) != BotTeammates.end())
										continue;	// Skip drawing if our candidate is our potential teammate.

									auto it = std::find(LocusCopy.m_rgszBots.begin(), LocusCopy.m_rgszBots.end(), Character.m_szName);
									bool bEnrolled = it != LocusCopy.m_rgszBots.end();

									if (ImGui::Selectable(Character.m_szName.c_str(), bEnrolled))
									{
										if (bEnrolled)	// Select enrolled character -> rule them out.
											LocusCopy.m_rgszBots.erase(it);
										else
											LocusCopy.m_rgszBots.emplace_back(Character.m_szName);
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

							ImGui::InputInt("Min Enemies", &LocusCopy.m_iMinEnemies, 1, 5, ImGuiInputTextFlags_CharsDecimal);
							ImGui::InputInt("Threshold", &LocusCopy.m_iThreshold, 1, 5, ImGuiInputTextFlags_CharsDecimal);
							ImGui::Checkbox("Friendly Fire", &LocusCopy.m_bFriendlyFire);
							ImGui::InputText("Console Command(s)", &LocusCopy.m_szConsoleCommands);

							// Actual tasks.
							if (ImGui::CollapsingHeader("Task(s)", ImGuiTreeNodeFlags_DefaultOpen))
							{
								int iIdentifierIndex = 0;
								for (auto itTask = LocusCopy.m_Tasks.begin(); itTask != LocusCopy.m_Tasks.end(); /* Do nothing */)
								{
									Task_t& Task = *itTask;
									bool bTreeNodeExpanded = ImGui::TreeNode(UTIL_VarArgs("%s###task%d", Task.ToString().c_str(), iIdentifierIndex));

									if (!bTreeNodeExpanded && ImGui::IsItemHovered())
										ImGui::SetTooltip("Right-click to add or delete.\nDrag and draw to reorder.");

									// Right-click menu
									if (!bTreeNodeExpanded && ImGui::BeginPopupContextItem())
									{
										if (ImGui::Selectable("Insert new task"))
											LocusCopy.m_Tasks.push_back(Task_t{});

										ImGui::BeginDisabled(LocusCopy.m_Tasks.size() < 2);
										bool bRemoved = false;
										if (ImGui::Selectable("Delete"))	// Must have at least one task.
										{
											itTask = LocusCopy.m_Tasks.erase(itTask);
											bRemoved = true;
										}
										if (LocusCopy.m_Tasks.size() < 2 && ImGui::IsItemHovered())
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
											auto itTaskDragged = *(decltype(itTask)*)payload->Data;
											std::swap(*itTaskDragged, *itTask);
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
								Locus = std::move(LocusCopy);
								ImGui::CloseCurrentPopup();
							}

							ImGui::SetItemDefaultFocus();
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}

						iColumnCount[(size_t)i]++;
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void MapsWindow(void)
{
	if (g_bitsAsyncStatus & Async_e::UPDATING_MAPS_INFO || !g_bShowMapsWindow)
		return;

	const std::unique_lock Lock(Maps::Mutex, std::try_to_lock);
	if (!Lock.owns_lock())
		return;

	if (ImGui::Begin("Maps## of independent window", &g_bShowMapsWindow, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize(ImVec2(240, 480));

		Maps::Filter.Draw("Search");

		for (const auto& [szName, Map] : g_Maps)
		{
			if (!Maps::Filter.PassFilter(szName.c_str()))
				continue;

			ImGui::Selectable(szName.c_str());

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

	ImGui::End();
}






int main(int argc, char** argv)
{
	// Load config.
	g_Config = new ValveKeyValues("CZeroEditorConfig");
	if (fs::exists("cze_config.ini"))
		g_Config->LoadFromFile("cze_config.ini");

#pragma region Initialize GL and ImGUI
	// Setup window
	glfwSetErrorCallback([](int error, const char* description) { std::fprintf(stderr, "Glfw Error %d: %s\n", error, description); });
	if (!glfwInit())
		return EXIT_FAILURE;

	// Decide GL+GLSL versions
	// GL 3.0 + GLSL 130
	constexpr const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	g_hGLFWWindow = glfwCreateWindow(1280, 720, "Condition Zero: Career Game Editor", NULL, NULL);
	if (g_hGLFWWindow == NULL)
		return EXIT_FAILURE;

	glfwMakeContextCurrent(g_hGLFWWindow);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(g_hGLFWWindow, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Font settings
	ImFontConfig fontConfig; fontConfig.MergeMode = true;
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\SegoeUI.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\MSMincho.ttc", 18.0f, &fontConfig, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\KaiU.ttf", 18.0f, &fontConfig, io.Fonts->GetGlyphRangesChineseFull());
#pragma endregion Initialize GL and ImGUI

	// Main loop
	while (!glfwWindowShouldClose(g_hGLFWWindow))
	{
		// Lock
		g_bitsAsyncStatus |= Async_e::RENDERING;

		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

#pragma region Debug Window
#ifdef _DEBUG
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (ImGui::IsKeyPressed(0x60))
			g_bShowDebugWindow = !g_bShowDebugWindow;

		if (g_bShowDebugWindow)
			ImGui::ShowDemoWindow(&g_bShowDebugWindow);
#endif
#pragma endregion Debug Window

#pragma region Key detection
		if (ImGui::IsKeyPressed(0x122))	// F1
			g_bShowCampaignWindow = !g_bShowCampaignWindow;
		if (ImGui::IsKeyPressed(0x123))	// F2
			g_bShowLociWindow = !g_bShowLociWindow;
		if (ImGui::IsKeyPressed(0x124))	// F3
			g_bShowMapsWindow = !g_bShowMapsWindow;
		if (ImGui::IsKeyPressed(0x125))	// F4
			g_bShowBotsWindow = !g_bShowBotsWindow;
#pragma endregion Key detection

		MainMenuBar();
		ConfigWindow();
		CampaignWindow();
		LocusWindow();
		MapsWindow();
		Gui::BotProfile::DrawWindow();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g_hGLFWWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.45f, 0.55f, 0.60f, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_hGLFWWindow);

		// Unlock
		g_bitsAsyncStatus &= ~Async_e::RENDERING;

		// Read & create glTexture outside of render.
		// Or it would fail and return 0.
		ImageLoader::Execute();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_hGLFWWindow);
	glfwTerminate();

	// Save config.
	if (g_szInputGamePath.length())
	{
		g_Config->SetValue("LastGamePath", g_szInputGamePath);
		g_Config->SaveToFile("cze_config.ini");
	}

	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
