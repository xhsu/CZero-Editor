/*
* Use ImGui to generate a Default profile editor.
* Nov 14 2021
*/

#include "Precompiled.hpp"

import UtlKeyValues;
import UtlString;

static inline BotProfile_t BotCopy;
static inline BotProfile_t* pBotProfOnEditing = nullptr;
static inline WeaponSelMask_t rgbCanWpnEnlist;
static inline bool bHandlingWithTemplates = false;
static inline TemplateNames_t rgszTplNamesOnEditing;



template<size_t iOffset, typename T = Reflection::BotProfile::TypeOf<iOffset>>
static void fnAttrib(const std::pair<T, T>& iSteps = {}) noexcept
{
	constexpr auto		pszAttribName = BotProfile_t::GetAttribName<iOffset>();
	constexpr bool		bIsOptional = BotProfile_t::AttribOptional<iOffset>();
	const BotProfile_t* pSource = bHandlingWithTemplates ? pBotProfOnEditing : nullptr;
	T*					pValue = bHandlingWithTemplates ? pBotProfOnEditing->GetSelf<iOffset>() : pBotProfOnEditing->Get<iOffset>(&pSource);
	const char*			pszSanityRet = pBotProfOnEditing->AttribSanity<iOffset>(bHandlingWithTemplates);	// Nullptr means value makes sense.
	bool				bValueValid = pBotProfOnEditing->AttribValid<iOffset>();
	std::string			szHintText = BotProfile_t::GetAttribIntro<iOffset>();

	if constexpr (bIsOptional)
		szHintText += "\n\nThis attribute is optional. Leave it blank if you don't know what to do.";

	// Red background for insane value.
	if (!bIsOptional && pszSanityRet && bValueValid)
		ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.98f, 0.2f, 1));

	// Gray for invalid/template controlled value.
	if (!bValueValid && !bHandlingWithTemplates)
	{
		ImGui::BeginDisabled();
		if constexpr (std::integral<T>)
			ImGui::InputInt(pszAttribName, pValue, iSteps.first, iSteps.second, ImGuiInputTextFlags_CharsDecimal);
		else if constexpr (std::floating_point<T>)
			ImGui::InputFloat(pszAttribName, pValue, iSteps.first, iSteps.second, "%.2f", ImGuiInputTextFlags_CharsDecimal);
		else if constexpr (std::convertible_to<T, std::string>)
			ImGui::InputText(pszAttribName, pValue, ImGuiInputTextFlags_CharsNoBlank);
		ImGui::EndDisabled();

		if (pSource == &BotProfileMgr::m_Default)
			szHintText += "\n\nYou can't modify because this value was inherited from the Default.\nRight-click to introduce this attribute into character database.";
		else
			szHintText += "\n\nYou can't modify because this value was inherited from template \"" + pSource->m_szName + "\".\nRight-click to introduce this attribute into character database.";
	}
	else
	{
		if constexpr (std::integral<T>)
			ImGui::InputInt(pszAttribName, pValue, iSteps.first, iSteps.second, ImGuiInputTextFlags_CharsDecimal);
		else if constexpr (std::floating_point<T>)
			ImGui::InputFloat(pszAttribName, pValue, iSteps.first, iSteps.second, "%.2f", ImGuiInputTextFlags_CharsDecimal);
		else if constexpr (std::convertible_to<T, std::string>)
			ImGui::InputText(pszAttribName, pValue, ImGuiInputTextFlags_CharsNoBlank);
	}

	if (!bIsOptional && pszSanityRet && bValueValid)
	{
		ImGui::PopStyleColor();
		szHintText += "\n\nField ill-formed.\nReason: "s + pszSanityRet;
	}

	// Do the hint outside the style change.
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		if (!bValueValid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
			ImGui::Text("\"%s\" fields from this profile is invalidated.\nField will not be saved into file.\n\n", pszAttribName);
			ImGui::PopStyleColor();
		}
		ImGui::TextUnformatted(szHintText.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	constexpr auto szAttribName = StringLiteral<char, strlen_c(pszAttribName)>(pszAttribName);
	constexpr auto szPopupHashString = szAttribName + "##AddValueOfThisAttrib"_s;
	constexpr auto szIntroduce = "Introduce \""_s + szAttribName + "\"."_s;
	constexpr auto szSetAsInherit = "Use inherited \""_s + szAttribName + "\" value."_s;
	constexpr auto szInvalidate = "Invalidate field \""_s + szAttribName + "\"."_s;

	// Right-click to introduce or invalidate.
	if (ImGui::BeginPopupContextItem(szPopupHashString))
	{
		if (!bValueValid && ImGui::MenuItem(szIntroduce))
		{
			*pBotProfOnEditing->GetSelf<iOffset>() = *pValue;

			if (pBotProfOnEditing->AttribSanity<iOffset>(true))
				pBotProfOnEditing->Rationalize<iOffset>();
		}

		if (bValueValid && ImGui::MenuItem(bHandlingWithTemplates ? szInvalidate : szSetAsInherit))
			pBotProfOnEditing->Invalidate<iOffset>();

		ImGui::EndPopup();
	}

	if constexpr (iOffset == Reflection::BotProfile::NAME)
	{
		if (!pBotProfOnEditing->m_szName.empty())
			pBotProfOnEditing->m_bPrefersSilencer = static_cast<bool>(pBotProfOnEditing->m_szName[0] % 2);
	}
}

static void fnBitsAttrib(void) noexcept	// Specialize for m_bitsDifficulty
{
	using Reflection::BotProfile::DIFFICULTY;

	const BotProfile_t* pSource = nullptr;
	int*				pValue = pBotProfOnEditing->Get<DIFFICULTY>(&pSource);
	bool				bValueValid = pBotProfOnEditing->AttribValid<DIFFICULTY>();
	std::string			szHintText = BotProfile_t::m_pszDifficultyIntro + "\n\nThis attribute is optional. Leave it blank if you don't know what to do."s;

	// In this case, forget about the silly red box, just make the value sense.
	if (pBotProfOnEditing->AttribSanity<DIFFICULTY>())
		*pValue = 0;

	// No optional since this is absolutely a optional field.

	// Gray for invalid/template controlled value.
	if (!bValueValid && !bHandlingWithTemplates)
	{
		ImGui::BeginDisabled();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[0], pValue, (1 << Difficulty_e::EASY)); ImGui::SameLine();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[1], pValue, (1 << Difficulty_e::NORMAL)); ImGui::SameLine();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[2], pValue, (1 << Difficulty_e::HARD)); ImGui::SameLine();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[3], pValue, (1 << Difficulty_e::EXPERT));
		ImGui::EndDisabled();

		if (pSource == &BotProfileMgr::m_Default)
			szHintText += "\n\nYou can't modify because this value was inherited from the Default.\nRight-click to introduce this attribute into character database.";
		else
			szHintText += "\n\nYou can't modify because this value was inherited from template \"" + pSource->m_szName + "\".\nRight-click to introduce this attribute into character database.";
	}
	else
	{
		ImGui::CheckboxFlags(g_rgszDifficultyNames[0], pValue, (1 << Difficulty_e::EASY)); ImGui::SameLine();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[1], pValue, (1 << Difficulty_e::NORMAL)); ImGui::SameLine();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[2], pValue, (1 << Difficulty_e::HARD)); ImGui::SameLine();
		ImGui::CheckboxFlags(g_rgszDifficultyNames[3], pValue, (1 << Difficulty_e::EXPERT));
	}

	// Do the hint outside the style change.
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		if (!bValueValid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
			ImGui::TextUnformatted("\"Difficulty\" fields from this profile is invalidated.\nField will not be saved into file.\n\n");
			ImGui::PopStyleColor();
		}
		ImGui::TextUnformatted(szHintText.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	// Right-click to introduce.
	if (ImGui::BeginPopupContextItem("Difficulty##AddValueOfThisAttrib"))
	{
		if (!bValueValid && ImGui::MenuItem("Introduce \"Difficulty\"."))
		{
			pBotProfOnEditing->m_bitsDifficulty = *pValue;

			if (pBotProfOnEditing->AttribSanity<DIFFICULTY>(true))
				pBotProfOnEditing->Rationalize<DIFFICULTY>();
		}
		if (bValueValid && ImGui::MenuItem(bHandlingWithTemplates ? "Invalidate field \"Difficulty\"." : "Use inherited \"Difficulty\" value."))
			pBotProfOnEditing->Invalidate<DIFFICULTY>();
		ImGui::EndPopup();
	}
}

static void fnComboAttrib(void) noexcept	// Specialized for m_szTeam.
{
	using Reflection::BotProfile::TEAM;

	const BotProfile_t* pSource = bHandlingWithTemplates ? pBotProfOnEditing : nullptr;
	std::string*		pValue = bHandlingWithTemplates ? pBotProfOnEditing->GetSelf<TEAM>() : pBotProfOnEditing->Get<TEAM>(&pSource);
	bool				bValueValid = pBotProfOnEditing->AttribValid<TEAM>();
	std::string			szHintText = BotProfile_t::m_pszTeamIntro + "\n\nThis attribute is optional. Leave it blank if you don't know what to do."s;
	const char*			pszSanityRet = pBotProfOnEditing->AttribSanity<TEAM>(bHandlingWithTemplates);

	// No optional since this is absolutely a optional field.

	// Gray for invalid/template controlled value.
	if (!bValueValid && !bHandlingWithTemplates)
	{
		ImGui::BeginDisabled();
		ImGui::InputText("Preferred Team", pValue, ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsUppercase);
		ImGui::EndDisabled();

		if (pSource == &BotProfileMgr::m_Default)
			szHintText += "\n\nYou can't modify because this value was inherited from the Default.\nRight-click to introduce this attribute into character database.";
		else
			szHintText += "\n\nYou can't modify because this value was inherited from template \"" + pSource->m_szName + "\".\nRight-click to introduce this attribute into character database.";
	}
	else if (ImGui::BeginCombo("Preferred Team", pValue->c_str(), ImGuiComboFlags_None))
	{
		if (ImGui::Selectable("Terrorist"))
			*pValue = "T";
		if (ImGui::Selectable("Counter-Terrorist"))
			*pValue = "CT";
		if (ImGui::Selectable("Any"))
			*pValue = "ANY";

		ImGui::EndCombo();
	}

	if (pszSanityRet && bValueValid)
		szHintText += "\n\nField ill-formed.\nReason: "s + pszSanityRet;

	// Do the hint outside the style change.
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		if (!bValueValid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
			ImGui::TextUnformatted("\"Team\" fields from this profile is invalidated.\nField will not be saved into file.\n\n");
			ImGui::PopStyleColor();
		}
		ImGui::TextUnformatted(szHintText.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	// Right-click menu.
	if (ImGui::BeginPopupContextItem("Team##AddValueOfThisAttrib"))
	{
		if (!bValueValid && ImGui::MenuItem("Introduce \"Team\"."))
			pBotProfOnEditing->m_szTeam = "ANY";
		if (bValueValid && ImGui::MenuItem(bHandlingWithTemplates ? "Invalidate field \"Team\"." : "Use inherited \"Team\" value."))
			pBotProfOnEditing->Invalidate<Reflection::BotProfile::DIFFICULTY>();

		ImGui::EndPopup();
	}
}

static void fnListAttrib(void) noexcept	// Specialized for m_rgszWpn
{
	using Reflection::BotProfile::WEAPON_PREFERENCE;

	const BotProfile_t* pSource = bHandlingWithTemplates ? pBotProfOnEditing : nullptr;
	Weapons_t*			pValue = bHandlingWithTemplates ? pBotProfOnEditing->GetSelf<WEAPON_PREFERENCE>() : pBotProfOnEditing->Get<WEAPON_PREFERENCE>(&pSource);
	const char*			pszSanityRet = pBotProfOnEditing->AttribSanity<WEAPON_PREFERENCE>(bHandlingWithTemplates);	// Is the value applying on me no matter where did it come from valid?
	const bool			bValueValid = pBotProfOnEditing->AttribValid<WEAPON_PREFERENCE>();	// Is value from myself valid? Or am I using value from a template?
	std::string			szHintText = "Right-click me for more options.\n\n"s + BotProfile_t::m_pszWeaponPreferenceIntro;

	// Generate a mask for current character.
	rgbCanWpnEnlist.fill(true);
	for (int i = 0; i < _countof(g_rgszWeaponNames); ++i)
	{
		for (const auto& szWeapon : *pValue)
		{
			if (szWeapon == g_rgszWeaponNames[i])
			{
				rgbCanWpnEnlist[i] = false;
				break;
			}
		}
	}

	// Add disabled hint.
	if (!bValueValid && !bHandlingWithTemplates)
	{
		if (pSource == &BotProfileMgr::m_Default)
			szHintText += "\n\nYou can't modify because this value was inherited from the Default.\nRight-click to introduce this attribute into character database.";
		else
			szHintText += "\n\nYou can't modify because this value was inherited from template \"" + pSource->m_szName + "\".\nRight-click to introduce this attribute into character database.";
	}

	if (pszSanityRet)
		szHintText += "\n\nField ill-formed.\nReason: "s + pszSanityRet;

	// Print title 'Prefered Weapon' and (?) first.
	ImGui::Text("Preferred Weapons:"); ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::BeginPopupContextItem("HintMenuOfPrefWpns"))
	{
		if (!bValueValid && ImGui::MenuItem("Introduce \"WeaponPreference\""))
		{
			*pBotProfOnEditing->GetSelf<WEAPON_PREFERENCE>() = *pValue;

			if (pBotProfOnEditing->AttribSanity<WEAPON_PREFERENCE>(true))	// If a invalid value is introduced, use the 'default' rational value.
				pBotProfOnEditing->Rationalize<WEAPON_PREFERENCE>();
		}
		else if (bValueValid && ImGui::MenuItem("Drop \"WeaponPreference\" table"))
			pBotProfOnEditing->GetSelf<WEAPON_PREFERENCE>()->clear();

		if (bValueValid && ImGui::BeginMenu("Append at the end..."))	// Valid first, and you will have append menu.
		{
			if (const char* psz = Gui::fnWeaponMenu(g_rgbIsBuyCommand, rgbCanWpnEnlist); psz != nullptr)
			{
				BotProfileMgr::RemoveNoneInWpnPref(pValue);
				pValue->emplace_back(psz);
			}

			ImGui::EndMenu();
		}

		if (bValueValid && ImGui::MenuItem("Set to 'none'"))
		{
			pValue->clear();
			pValue->emplace_back("none");
		}

		ImGui::EndPopup();
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		if (!bValueValid)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
			ImGui::TextUnformatted("\"WeaponPreference\" fields from this profile is invalidated.\nField will not be saved into file.\n\n");
			ImGui::PopStyleColor();
		}
		ImGui::TextUnformatted(szHintText.c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	if (!bValueValid)
		ImGui::BeginDisabled();

	// Draw the actual box.
	auto iLineCount = std::clamp<size_t>(pValue->size(), 1, 7);
	if (ImGui::BeginListBox("##WeaponList", ImVec2(-FLT_MIN, iLineCount * ImGui::GetTextLineHeightWithSpacing() + 4)))
	{
		for (auto itszWeapon = pValue->begin(); itszWeapon != pValue->end(); /* Do nothing */)
		{
			bool bInsaneItem = !_stricmp(itszWeapon->c_str(), "none") && pValue->size() > 1;

			// Red background for insane value.
			if (!bInsaneItem)
				ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImVec4)ImColor::HSV(0.98f, 0.2f, 1));

			ImGui::Selectable(itszWeapon->c_str());

			// The red background works on one item per time.
			if (!bInsaneItem)
				ImGui::PopStyleColor();

			// After red-mask was removed, we can start a tooptip.
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Right click to insert a new weapon here.\nDrag and draw to reorder.");

			if (!bValueValid)	// You can't edit them anyway.
				goto LAB_CONTINUE;

			// Right click on weapon name.
			if (ImGui::BeginPopupContextItem())
			{
				bool bShouldSkipIterInc = false;

				if (ImGui::MenuItem("Delete"))
				{
					itszWeapon = pValue->erase(itszWeapon);
					bShouldSkipIterInc = true;
				}

				if (ImGui::BeginMenu("Insert weapon..."))
				{
					if (const char* psz = Gui::fnWeaponMenu(g_rgbIsBuyCommand, rgbCanWpnEnlist); psz != nullptr)
					{
						BotProfileMgr::RemoveNoneInWpnPref(pValue);
						itszWeapon = pValue->insert(itszWeapon, psz);
						bShouldSkipIterInc = true;
					}

					ImGui::EndMenu();
				}

				ImGui::EndPopup();

				if (bShouldSkipIterInc)
					continue;
			}

			// Reordering
			if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
			{
				bool bMovingUp = (ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y < 0.f);
				if (itszWeapon == pValue->begin() && bMovingUp)
					goto LAB_CONTINUE;

				auto iterMovingTo = itszWeapon;
				bMovingUp ? iterMovingTo-- : iterMovingTo++;

				if (iterMovingTo == pValue->end())
					goto LAB_CONTINUE;

				std::swap(*itszWeapon, *iterMovingTo);	// Have to dereference to actually iterators. Kinda' sucks.
				ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
			}

		LAB_CONTINUE:;
			++itszWeapon;
		}

		// #TODO Handle 'none' tag.
		ImGui::EndListBox();
	}

	if (!bValueValid)
		ImGui::EndDisabled();
}

static void fnDisplayTemplateWithOverrideInfo(const TemplateNames_t& rgszTemplateList, const BotProfile_t& Tpl) noexcept
{
	using namespace Reflection::BotProfile;

	if (Tpl.m_rgszWpnPreference.size() == 1 && !_stricmp(Tpl.m_rgszWpnPreference.front().c_str(), "none"))
	{
		auto pCur = BotProfileMgr::Deduce<WEAPON_PREFERENCE>(rgszTemplateList);

		if (pCur->size() == 1 && !_stricmp(pCur->front().c_str(), "none"))
			ImGui::BulletText("WeaponPreference: none");
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) WeaponPreference: none");
		}
	}
	else if (!Tpl.m_rgszWpnPreference.empty())
	{
		for (const auto& szWpn : Tpl.m_rgszWpnPreference)
			ImGui::BulletText("WeaponPreference: %s", szWpn.c_str());
	}
	if (!Tpl.AttribSanity<ATTACK_DELAY>(true))
	{
		if (Tpl.m_flAttackDelay == *BotProfileMgr::Deduce<ATTACK_DELAY>(rgszTemplateList))
			ImGui::BulletText("Attack Delay: %.2f", Tpl.m_flAttackDelay);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Attack Delay: %.2f", Tpl.m_flAttackDelay);
		}
	}
	if (!Tpl.AttribSanity<REACTION_TIME>(true))
	{
		if (Tpl.m_flReactionTime == *BotProfileMgr::Deduce<REACTION_TIME>(rgszTemplateList))
			ImGui::BulletText("Reaction Time: %.2f", Tpl.m_flReactionTime);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Reaction Time: %.2f", Tpl.m_flReactionTime);
		}
	}
	if (!Tpl.AttribSanity<DIFFICULTY>(true))
	{
		for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
		{
			if (Tpl.m_bitsDifficulty & (1 << i))
			{
				if (Tpl.m_bitsDifficulty == *BotProfileMgr::Deduce<DIFFICULTY>(rgszTemplateList))
					ImGui::BulletText("Difficulty: %s", g_rgszDifficultyNames[(size_t)i]);
				else
				{
					ImGui::Bullet();
					ImGui::TextColored(IMGUI_RED, "(Overrided) Difficulty: %s", g_rgszDifficultyNames[(size_t)i]);
				}
			}
		}
	}
	if (!Tpl.AttribSanity<AGGRESSION>(true))
	{
		if (Tpl.m_iAggression == *BotProfileMgr::Deduce<AGGRESSION>(rgszTemplateList))
			ImGui::BulletText("Aggression: %d", Tpl.m_iAggression);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Aggression: %d", Tpl.m_iAggression);
		}
	}
	if (!Tpl.AttribSanity<COST>(true))
	{
		if (Tpl.m_iCost == *BotProfileMgr::Deduce<COST>(rgszTemplateList))
			ImGui::BulletText("Cost: %d", Tpl.m_iCost);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Cost: %d", Tpl.m_iCost);
		}
	}
	if (!Tpl.AttribSanity<SKILL>(true))
	{
		if (Tpl.m_iSkill == *BotProfileMgr::Deduce<SKILL>(rgszTemplateList))
			ImGui::BulletText("Skill: %d", Tpl.m_iSkill);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Skill: %d", Tpl.m_iSkill);
		}
	}
	if (!Tpl.AttribSanity<TEAMWORK>(true))
	{
		if (Tpl.m_iTeamwork == *BotProfileMgr::Deduce<TEAMWORK>(rgszTemplateList))
			ImGui::BulletText("Teamwork: %d", Tpl.m_iTeamwork);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Teamwork: %d", Tpl.m_iTeamwork);
		}
	}
	if (!Tpl.AttribSanity<VOICEPITCH>(true))
	{
		if (Tpl.m_iVoicePitch == *BotProfileMgr::Deduce<VOICEPITCH>(rgszTemplateList))
			ImGui::BulletText("Voice Pitch: %d", Tpl.m_iVoicePitch);
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Voice Pitch: %d", Tpl.m_iVoicePitch);
		}
	}
	if (!Tpl.AttribSanity<SKIN>(true))
	{
		if (Tpl.m_szSkin == *BotProfileMgr::Deduce<SKIN>(rgszTemplateList))
			ImGui::BulletText("Skin: %s", Tpl.m_szSkin.c_str());
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Skin: %s", Tpl.m_szSkin.c_str());
		}
	}
	if (!Tpl.AttribSanity<TEAM>(true))
	{
		if (Tpl.m_szTeam == *BotProfileMgr::Deduce<TEAM>(rgszTemplateList))
			ImGui::BulletText("Team: %s", Tpl.m_szTeam.c_str());
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Team: %s", Tpl.m_szTeam.c_str());
		}
	}
	if (!Tpl.AttribSanity<VOICEBANK>(true))
	{
		if (Tpl.m_szVoiceBank == *BotProfileMgr::Deduce<VOICEBANK>(rgszTemplateList))
			ImGui::BulletText("Voicebank: %s", Tpl.m_szVoiceBank.c_str());
		else
		{
			ImGui::Bullet();
			ImGui::TextColored(IMGUI_RED, "(Overrided) Voicebank: %s", Tpl.m_szVoiceBank.c_str());
		}
	}
}

void Gui::BotProfile::DrawWindow(void) noexcept
{
	if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO || !g_bShowBotsWindow)
		return;

	const std::unique_lock Lock1(MissionPack::Mutex, std::try_to_lock);
	const std::unique_lock Lock2(Maps::Mutex, std::try_to_lock);
	if (!Lock1.owns_lock() || !Lock2.owns_lock())
		return;

	if (ImGui::Begin("BOTs", &g_bShowBotsWindow) && !g_BotProfiles.empty())
	{
		if (ImGui::BeginTabBar("TabBar_BOT", ImGuiTabBarFlags_None))
		{
			if (ImGui::BeginTabItem("Default", nullptr, m_iSetTab == TAB_DEFAULT ? ImGuiTabItemFlags_SetSelected : 0))
			{
				using namespace Reflection::BotProfile;
				using namespace std;

				pBotProfOnEditing = &BotProfileMgr::m_Default;
				bHandlingWithTemplates = true;

				fnBitsAttrib();	// m_bitsDifficulty
				fnListAttrib();	// m_rgszWpnPreference
				fnAttrib<ATTACK_DELAY>(make_pair(0.05f, 0.25f));
				fnAttrib<REACTION_TIME>(make_pair(0.05f, 0.25f));
				fnAttrib<AGGRESSION>(make_pair(5, 20));
				fnAttrib<COST>(make_pair(1, 1));
				fnAttrib<SKILL>(make_pair(5, 20));
				fnAttrib<TEAMWORK>(make_pair(5, 20));
				fnAttrib<VOICEPITCH>(make_pair(5, 20));
				fnAttrib<SKIN>();
				fnComboAttrib();	// m_szTeam
				fnAttrib<VOICEBANK>();	// #UNDONE_LONG_TERM file selection?

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Templates", nullptr, m_iSetTab == TAB_TEMPLATES ? ImGuiTabItemFlags_SetSelected : 0))
			{
				pBotProfOnEditing = &BotCopy;
				bHandlingWithTemplates = true;

				bool bClickAdd = ImGui::Button("+", ImVec2(24, 24));
				ImGui::SameLine();
				Filter.Draw("Search");

				if (bClickAdd)
				{
					ImGui::OpenPopup("Add a template...");
					BotCopy.Reset();
				}

				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				
				if (ImGui::BeginPopupModal("Add a template..."))
				{
					using namespace Reflection::BotProfile;
					using namespace std;

					fnAttrib<NAME>();
					fnBitsAttrib();	// m_bitsDifficulty
					fnListAttrib();	// m_rgszWpnPreference
					fnAttrib<ATTACK_DELAY>(make_pair(0.05f, 0.25f));
					fnAttrib<REACTION_TIME>(make_pair(0.05f, 0.25f));
					fnAttrib<AGGRESSION>(make_pair(5, 20));
					fnAttrib<COST>(make_pair(1, 1));
					fnAttrib<SKILL>(make_pair(5, 20));
					fnAttrib<TEAMWORK>(make_pair(5, 20));
					fnAttrib<VOICEPITCH>(make_pair(5, 20));
					fnAttrib<SKIN>();
					fnComboAttrib();	// m_szTeam
					fnAttrib<VOICEBANK>();	// #UNDONE_LONG_TERM file selection?

					ImGui::NewLine();

					const auto iWindowWidth = ImGui::GetWindowContentRegionMax().x;
					const auto iCursorPoxY = ImGui::GetCursorPosY();
					ImGui::SetCursorPos(ImVec2(iWindowWidth / 2.0f - 120 - 15, iCursorPoxY));

					if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						if (BotProfileMgr::TemplateNameCount(BotCopy.m_szName) > 1)
							ImGui::OpenPopup("Error");
						else
						{
							BotProfileMgr::m_Templates.try_emplace(BotCopy.m_szName, std::move(BotCopy));
							ImGui::CloseCurrentPopup();
						}
					}

					ImGui::SetCursorPos(ImVec2(iWindowWidth / 2.0f + 15, iCursorPoxY));
					if (ImGui::Button("Cancel", ImVec2(120, 0)))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				for (auto itTplPair = BotProfileMgr::m_Templates.begin(); itTplPair != BotProfileMgr::m_Templates.end(); /* Do nothing */ )
				{
					auto& szName = itTplPair->first;
					auto& Template = itTplPair->second;
					bool bOpenErrorWindow = false;

					if (!Filter.PassFilter(szName.c_str()))
						goto LAB_TEMPLATE_MAIN_LOOP_CONTINUE;

					if (ImGui::Selectable(szName.c_str()))
					{
						ImGui::OpenPopup(UTIL_VarArgs("Editing: %s", szName.c_str()));
						BotCopy = Template;
					}

					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						Summary(&Template);
						ImGui::EndTooltip();
					}

					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::MenuItem(UTIL_VarArgs("Delete template \"%s\"", szName.c_str())))
						{
							if (BotProfileMgr::TemplateOccupied(Template))
								bOpenErrorWindow = true;
							else
							{
								itTplPair = BotProfileMgr::m_Templates.erase(itTplPair);
								ImGui::EndPopup();
								continue;
							}
						}

						ImGui::EndPopup();
					}

					if (bOpenErrorWindow)	// Fuck ImGui. Why the id stack is even a thing? This is so ugly.
						ImGui::OpenPopup(UTIL_VarArgs("Error##%s", szName.c_str()));

					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					// Modal: Cannot delete
					if (ImGui::BeginPopupModal(UTIL_VarArgs("Error##%s", szName.c_str()), nullptr, ImGuiWindowFlags_NoResize))
					{
						const auto iModalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

						ImGui::TextUnformatted("You may not delete the template because the following Characters are refering it:\n");
						
						for (const auto& Character : g_BotProfiles)
						{
							if (std::find(Character.m_rgszRefTemplates.begin(), Character.m_rgszRefTemplates.end(), szName) != Character.m_rgszRefTemplates.end())
								ImGui::BulletText(Character.m_szName.c_str());
						}

						ImGui::NewLine();

						ImGui::SetCursorPosX(iModalContentWidth / 2 - 60);
						if (ImGui::Button("OK", ImVec2(120, 24)))
							ImGui::CloseCurrentPopup();

						ImGui::EndPopup();
					}

					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					// Template editing modal.
					if (ImGui::BeginPopupModal(UTIL_VarArgs("Editing: %s", szName.c_str())))
					{
						using namespace Reflection::BotProfile;
						using namespace std;

						fnAttrib<NAME>();
						fnBitsAttrib();	// m_bitsDifficulty
						fnListAttrib();	// m_rgszWpnPreference
						fnAttrib<ATTACK_DELAY>(make_pair(0.05f, 0.25f));
						fnAttrib<REACTION_TIME>(make_pair(0.05f, 0.25f));
						fnAttrib<AGGRESSION>(make_pair(5, 20));
						fnAttrib<COST>(make_pair(1, 1));
						fnAttrib<SKILL>(make_pair(5, 20));
						fnAttrib<TEAMWORK>(make_pair(5, 20));
						fnAttrib<VOICEPITCH>(make_pair(5, 20));
						fnAttrib<SKIN>();
						fnComboAttrib();	// m_szTeam
						fnAttrib<VOICEBANK>();	// #UNDONE_LONG_TERM file selection?

						ImGui::NewLine();

						const auto iWindowWidth = ImGui::GetWindowContentRegionMax().x;
						const auto iCursorPoxY = ImGui::GetCursorPosY();
						ImGui::SetCursorPos(ImVec2(iWindowWidth / 2.0f - 120 - 15, iCursorPoxY));

						if (ImGui::Button("OK", ImVec2(120, 0)))
						{
							if (BotProfileMgr::TemplateNameCount(BotCopy.m_szName) > 1)
								ImGui::OpenPopup("Error");
							else
							{
								Template = std::move(BotCopy);
								ImGui::CloseCurrentPopup();
							}
						}

						ImGui::SetCursorPos(ImVec2(iWindowWidth / 2.0f + 15, iCursorPoxY));
						if (ImGui::Button("Cancel", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

				LAB_TEMPLATE_MAIN_LOOP_CONTINUE:;
					++itTplPair;
				}

				// (Modal) Error on naming.
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
				if (ImGui::BeginPopupModal("Error"))
				{
					ImGui::TextUnformatted("Duplicated name is not allowed.");

					const auto iWindowWidth = ImGui::GetWindowWidth();
					ImGui::SetCursorPosX(iWindowWidth / 2.0f - 60);

					if (ImGui::Button("OK", ImVec2(120, 0)))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}

				// Match the name with entry.
				for (auto iter = BotProfileMgr::m_Templates.begin(); iter != BotProfileMgr::m_Templates.end(); /* Do nothing */)
				{
					if (iter->first == iter->second.m_szName)
					{
						++iter;
						continue;
					}

					// Modity all profiles first.
					for (auto& Character : BotProfileMgr::m_Profiles)
					{
						for (auto& szTpl : Character.m_rgszRefTemplates)
						{
							if (szTpl == iter->first)
							{
								szTpl = iter->second.m_szName;
								break;	// A template can only be refered once. Exit the loop once we found one.
							}
						}
					}

					BotProfileMgr::m_Templates.try_emplace(iter->second.m_szName, std::move(iter->second));	// #UNTESTED #POTENTIAL_BUG Will this work? Or should I priveded a copy for the name?
					BotProfileMgr::m_Templates.erase(iter);

					iter = BotProfileMgr::m_Templates.begin();	// Start all over.
				}

				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Characters", nullptr, m_iSetTab == TAB_CHARACTERS ? ImGuiTabItemFlags_SetSelected : 0))
			{
				bHandlingWithTemplates = false;
				pBotProfOnEditing = &BotCopy;

				bool bClickAdd = ImGui::Button("+", ImVec2(24, 24));
				bool bNewCharacter = false;

				if (BotProfileMgr::m_Templates.empty() && ImGui::IsItemHovered())
					ImGui::SetTooltip("You have to add a template before adding any character!");

				ImGui::SameLine();
				Filter.Draw("Search");

#pragma region Add Character
				if (bClickAdd && !BotProfileMgr::m_Templates.empty())
				{
					ImGui::OpenPopup("Add a character...");
					rgszTplNamesOnEditing.clear();
				}

				if (ImGui::BeginPopupModal("Add a character...", nullptr, ImGuiWindowFlags_NoResize))
				{
					ImGui::SetWindowSize(ImVec2(240, 480));
					auto iModalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					if (ImGui::BeginChild("Template hierarchy view", ImVec2(iModalContentWidth, 372), true, ImGuiWindowFlags_None))
					{
						// Place default on the top.
						ImGui::Selectable("Default", true);
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();

							ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
							ImGui::TextUnformatted("The \"Default\" template can never be deselected.\n\n");
							ImGui::PopStyleColor();

							fnDisplayTemplateWithOverrideInfo(rgszTplNamesOnEditing, BotProfileMgr::m_Default);

							ImGui::EndTooltip();
						}

						// Selected items.
						for (auto iter = rgszTplNamesOnEditing.begin(); iter != rgszTplNamesOnEditing.end(); /* Do nothing */)
						{
							// Delete from selected list.
							if (ImGui::Selectable(iter->c_str(), true, ImGuiSelectableFlags_AllowDoubleClick) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								iter = rgszTplNamesOnEditing.erase(iter);
								continue;
							}

							// Hover & Info
							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								fnDisplayTemplateWithOverrideInfo(rgszTplNamesOnEditing, BotProfileMgr::m_Templates[*iter]);
								ImGui::EndTooltip();
							}

							// Drag and draw.
							if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
							{
								bool bMovingUp = (ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y < 0.f);
								if (iter == rgszTplNamesOnEditing.begin() && bMovingUp)	// The first element shouldnot be able to moving up.
									goto LAB_CONTINUE;

								auto iterMovingTo = iter;
								bMovingUp ? iterMovingTo-- : iterMovingTo++;

								if (iterMovingTo == rgszTplNamesOnEditing.end())	// You can't moving to ... obliterate his name.
									goto LAB_CONTINUE;

								std::swap(*iter, *iterMovingTo);	// Have to dereference to actually swap.
								ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
							}

						LAB_CONTINUE:;
							++iter;
						}

						// Draw the rest.
						for (const auto& [szName, Template] : BotProfileMgr::m_Templates)
						{
							if (std::find(rgszTplNamesOnEditing.begin(), rgszTplNamesOnEditing.end(), szName) != rgszTplNamesOnEditing.end())
								continue;	// The name is selected. Skip it.

							// Add template into generation.
							if (ImGui::Selectable(szName.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick) && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
							{
								rgszTplNamesOnEditing.emplace_back(szName);
								continue;
							}

							// Hover & Info
							if (ImGui::IsItemHovered())
							{
								ImGui::BeginTooltip();
								Summary(&Template);
								ImGui::EndTooltip();
							}
						}
					}

					ImGui::EndChild();	// This one is special. Have to call ImGui::EndChild() either way.
					ImGui::PopStyleVar();

					// 'Yes"
					ImGui::SetCursorPos(ImVec2(240 / 2 - 60, ImGui::GetWindowContentRegionMax().y - 60));
					if (ImGui::Button("OK", ImVec2(120, 24)))
					{
						if (rgszTplNamesOnEditing.empty())
							ImGui::OpenPopup("Error - Adding character");
						else
						{
							BotProfileMgr::m_Profiles.emplace_back();
							BotProfileMgr::m_Profiles.back().m_rgszRefTemplates = std::move(rgszTplNamesOnEditing);
							ImGui::CloseCurrentPopup();

							bNewCharacter = true;
						}
					}

					// Error on save.
					Gui::fnErrorDialog(
						"Error - Adding character",
						"You have to select another template besides \"Default\"!");

					ImGui::SetCursorPos(ImVec2(240 / 2 - 60, ImGui::GetWindowContentRegionMax().y - 24));
					if (ImGui::Button("Cancel", ImVec2(120, 24)))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
#pragma endregion Add Character

				constexpr ImGuiTableFlags bitsTableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable;

				if (ImGui::BeginTable("TableOfBots", 1, bitsTableFlags))
				{
					for (auto itChar = g_BotProfiles.begin(); itChar != g_BotProfiles.end(); /* Do nothing */)
					{
						using namespace Reflection::BotProfile;

						auto& Character = *itChar;

						if (!Filter.PassFilter(Character.Get<NAME>()->c_str()))
						{
							++itChar;
							continue;
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);

						Thumbnail_t* pPortrait = nullptr;

						// There is no need to update the image library since all .tga images under /skin had been loaded.
						// atoi() is better than stoi() since atoi would return 0 of it is not a number. #POTENTIAL_BUG assert that ctskin[] is as long as tskin[].
						if (int iSkinIndex = std::atoi(Character.Get<SKIN>()->c_str()); UTIL_GetStringType(Character.Get<SKIN>()->c_str()) == 1 && iSkinIndex >= 0 && iSkinIndex < _countof(g_rgszCTSkinName))
						{
							if (iSkinIndex == 0)
								iSkinIndex = 6;	// 'Random' pic.

							pPortrait = &BotProfileMgr::m_Thumbnails[g_rgszTSkinName[iSkinIndex]];
							if (MissionPack::IsTeammate(Character.m_szName))
								pPortrait = &BotProfileMgr::m_Thumbnails[g_rgszCTSkinName[iSkinIndex]];
						}
						else if (BotProfileMgr::m_Skins.contains(*Character.Get<SKIN>()) && BotProfileMgr::m_Thumbnails.contains(BotProfileMgr::m_Skins[Character.m_szSkin]))
						{
							pPortrait = &BotProfileMgr::m_Thumbnails[BotProfileMgr::m_Skins[*Character.Get<SKIN>()]];
						}

						bool bShouldEnterCharacterEditor = false;
						ImGui::PushID((Character.m_szName + "##ImageButton").c_str());
						if (pPortrait)
							bShouldEnterCharacterEditor = ImGui::ImageButton((void*)(intptr_t)pPortrait->m_iTexId, pPortrait->Size());
						else
							bShouldEnterCharacterEditor = ImGui::Button(UTIL_VarArgs("No Portrait##%s", Character.m_szName.c_str()), ImVec2(96, 96));
						ImGui::PopID();
						ImGui::SameLine();

#pragma region Deletion
						if (ImGui::BeginPopupContextItem(UTIL_VarArgs("Delete%s", Character.m_szName.c_str())))
						{
							if (ImGui::MenuItem(UTIL_VarArgs("Delete character \"%s\"", Character.m_szName.c_str())))
							{
								itChar = g_BotProfiles.erase(itChar);

								// Delete the name in the campaign config as well.
								for (auto& [iDifficulty, CareerGame] : MissionPack::CareerGames)
								{
									if (auto iter = std::find(CareerGame.m_rgszCharacters.begin(), CareerGame.m_rgszCharacters.end(), itChar->m_szName);
										iter != CareerGame.m_rgszCharacters.end())
									{
										CareerGame.m_rgszCharacters.erase(iter);
									}

									for (auto& Locus : CareerGame.m_Loci)
									{
										if (auto iter = std::find(Locus.m_rgszBots.begin(), Locus.m_rgszBots.end(), itChar->m_szName);
											iter != Locus.m_rgszBots.end())
										{
											Locus.m_rgszBots.erase(iter);
										}
									}
								}

								ImGui::EndPopup();
								continue;
							}

							ImGui::EndPopup();
						}
#pragma endregion Deletion

#pragma region CharacterEditingModal
						if (bShouldEnterCharacterEditor)
						{
							ImGui::OpenPopup(UTIL_VarArgs("%s##BotEditor", itChar->m_szName.c_str()));
							BotCopy = *itChar;
						}

						// Always center this window when appearing
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

						// Have to use the name stored in iterator. Otherwise things will get wired if user edits its name.
						if (ImGui::BeginPopupModal(UTIL_VarArgs("%s##BotEditor", itChar->m_szName.c_str()), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
						{
							using namespace Reflection::BotProfile;

							fnAttrib<NAME>();
							fnBitsAttrib();	// m_bitsDifficulty
							fnListAttrib();	// m_rgszWpnPreference
							fnAttrib<ATTACK_DELAY>(std::make_pair(0.05f, 0.25f));
							fnAttrib<REACTION_TIME>(std::make_pair(0.05f, 0.25f));
							fnAttrib<AGGRESSION>(std::make_pair(5, 20));
							fnAttrib<COST>(std::make_pair(1, 1));
							fnAttrib<SKILL>(std::make_pair(5, 20));
							fnAttrib<TEAMWORK>(std::make_pair(5, 20));
							fnAttrib<VOICEPITCH>(std::make_pair(5, 20));
							fnAttrib<SKIN>();
							fnComboAttrib();	// m_szTeam
							fnAttrib<VOICEBANK>();	// #UNDONE_LONG_TERM file selection?

							auto pszErrorMessage = BotCopy.SanityCheck();	// A sanity a day keeps CTDs away.

							if (!pszErrorMessage && BotProfileMgr::ProfileNameCount(BotCopy.m_szName) > 1)	// Test2: is it has a dup name?
								pszErrorMessage = "Duplicated name is not allowed.";

							if (ImGui::Button("OK", ImVec2(120, 0)))
							{
								if (pszErrorMessage != nullptr)
								{
									ImGui::OpenPopup("Error");	// Can't leave error modal coding here. Or the window will only be draw when the button 'OK' is clicked.
								}
								else
								{
									*itChar = std::move(BotCopy);
									ImGui::CloseCurrentPopup();
								}
							}

							Gui::fnErrorDialog("Error", pszErrorMessage);

							ImGui::SetItemDefaultFocus();
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}
#pragma endregion CharacterEditingModal

						auto vecPos = ImGui::GetCursorPos();
						ImGui::Text(Character.m_szName.c_str());

						ImGui::SameLine();
						bool bFirst = true;
						std::string szRefs = "(";
						for (const auto& szRef : Character.m_rgszRefTemplates)
						{
							if (bFirst)
								bFirst = false;
							else
								szRefs += '+';

							szRefs += szRef;
						}
						szRefs += ')';
						ImGui::Text(szRefs.c_str());

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Right-click to reselect template.");

#pragma region CharacterTemplateModal
						bool bShouldEnterTemplateSelection = false;
						if (ImGui::BeginPopupContextItem(UTIL_VarArgs("TemplateEditingFor%s", Character.m_szName.c_str())))
						{
							if (ImGui::MenuItem("Reselect template..."))
							{
								BotCopy = *itChar;
								BotCopy.Inherit();	// Consider values from Default and Templates only.
								bShouldEnterTemplateSelection = true;
							}

							ImGui::EndPopup();
						}

						if (bShouldEnterTemplateSelection)
							ImGui::OpenPopup(UTIL_VarArgs("%s: Templates", Character.m_szName.c_str()));

						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

						if (ImGui::BeginPopupModal(UTIL_VarArgs("%s: Templates", Character.m_szName.c_str()), nullptr, ImGuiWindowFlags_NoResize))
						{
							ImGui::SetWindowSize(ImVec2(240, 480));
							auto iModalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

							ImGui::SetCursorPosX(240 / 2 - 60);
							if (ImGui::Button("Add", ImVec2(120, 0)))
								ImGui::OpenPopup("Append template");
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("Append a new template at the end.");

							if (ImGui::BeginPopupContextItem("Append template"))
							{
								for (const auto& [szName, Template] : BotProfileMgr::m_Templates)
								{
									bool bAlreadyIn = false;
									for (const auto& szTemplate : BotCopy.m_rgszRefTemplates)
									{
										if (szName == szTemplate)
										{
											bAlreadyIn = true;
											break;
										}
									}

									if (bAlreadyIn)
										continue;

									if (ImGui::MenuItem(szName.c_str()))
									{
										BotCopy.m_rgszRefTemplates.emplace_back(szName);
										BotCopy.Inherit();	// Update heritage data.
									}

									if (ImGui::IsItemHovered())
									{
										ImGui::BeginTooltip();
										Summary(&Template);
										ImGui::EndTooltip();
									}
								}

								ImGui::EndPopup();
							}

							ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
							ImGui::BeginChild("Template hierarchy view", ImVec2(iModalContentWidth, 340), true, ImGuiWindowFlags_None);

							// Show the situation of Default.
							bool bTreeNodeExpanded = ImGui::TreeNode("Default##TemplateInfo");
							if (ImGui::IsItemHovered())
								ImGui::SetTooltip("All characters have to inherit from Default.\nYou cannot reorder, delete or edit this one.");
							if (bTreeNodeExpanded)
							{
								fnDisplayTemplateWithOverrideInfo(BotCopy.m_rgszRefTemplates, BotProfileMgr::m_Default);
								ImGui::TreePop();
							}

							// List all template.
							for (auto itszTemplate = BotCopy.m_rgszRefTemplates.begin(); itszTemplate != BotCopy.m_rgszRefTemplates.end(); /* Do nothing */)
							{
								assert(BotProfileMgr::m_Templates.contains(*itszTemplate));
								auto& Template = BotProfileMgr::m_Templates[*itszTemplate];
								bTreeNodeExpanded = ImGui::TreeNode(itszTemplate->c_str());

								if (!bTreeNodeExpanded && ImGui::IsItemHovered())
									ImGui::SetTooltip("Right-click to delete.\nDrag and draw to reorder.");

								// Right-click to delete.
								if (!bTreeNodeExpanded && ImGui::BeginPopupContextItem(itszTemplate->c_str()))
								{
									if (ImGui::MenuItem("Delete"))
									{
										itszTemplate = BotCopy.m_rgszRefTemplates.erase(itszTemplate);
										BotCopy.Inherit();	// Update heritage data.

										ImGui::EndPopup();
										continue;
									}

									ImGui::EndPopup();
								}

								// Reordering.
								if (!bTreeNodeExpanded && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
								{
									ImGui::SetDragDropPayload("itszTemplate", &itszTemplate, sizeof(itszTemplate));

									// Display preview
									ImGui::Text("Reordering: %s", itszTemplate->c_str());
									ImGui::EndDragDropSource();
								}
								if (!bTreeNodeExpanded && ImGui::BeginDragDropTarget())
								{
									if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("itszTemplate"))
									{
										IM_ASSERT(payload->DataSize == sizeof(itszTemplate));
										auto itszTemplateDragged = *(decltype(itszTemplate)*)payload->Data;
										std::swap(*itszTemplateDragged, *itszTemplate);
										BotCopy.Inherit();	// Update heritage data.
									}

									ImGui::EndDragDropTarget();
								}

								// Template value preview.
								if (bTreeNodeExpanded)
								{
									fnDisplayTemplateWithOverrideInfo(BotCopy.m_rgszRefTemplates, Template);
									ImGui::TreePop();
								}

								++itszTemplate;
							}

							ImGui::EndChild();
							ImGui::PopStyleVar();

							// 'Yes' button.
							ImGui::SetCursorPos(ImVec2(240 / 2 - 60, ImGui::GetWindowContentRegionMax().y - 60));
							ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 199, 206));
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.98f, 0.35f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.98f, 0.45f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
							bTreeNodeExpanded = ImGui::Button("Reconstitute", ImVec2(120, 24));	// Recycled variable. Ignore its name.
							//if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
							//	ImGui::SetTooltip("This will reconstruct the character: all non-template data will be lost.\nARE YOU SURE?");
							ImGui::PopStyleColor(4);
							if (bTreeNodeExpanded)	// Recycled variable. Ignore its name.
							{
								if (BotCopy.m_rgszRefTemplates.empty())
									ImGui::OpenPopup("Error - Template Selection");
								else
								{
									Character.m_rgszRefTemplates = BotCopy.m_rgszRefTemplates;
									ImGui::CloseCurrentPopup();
								}
							}

							// Error on save.
							Gui::fnErrorDialog(
								"Error - Template Selection",
								"Every character have to derive from at least one template!");

							ImGui::SetCursorPos(ImVec2(240 / 2 - 60, ImGui::GetWindowContentRegionMax().y - 24));
							if (ImGui::Button("Cancel", ImVec2(120, 24)))
								ImGui::CloseCurrentPopup();

							ImGui::EndPopup();
						}
#pragma endregion CharacterTemplateModal

						ImGui::SameLine();
						ImGui::TextDisabled("Cost %d", *Character.Get<COST>());

						float flY = vecPos.y + 22;	// Save the line 1 Y coord of this table.

						int iSkill = *Character.Get<SKILL>();
						int iTeamwork = *Character.Get<TEAMWORK>();
						int iAggression = *Character.Get<AGGRESSION>();

						vecPos.y += 22; ImGui::SetCursorPos(vecPos);
						ImGui::TextColored(iSkill > 66 ? IMGUI_GREEN : iSkill > 33 ? IMGUI_YELLOW : IMGUI_RED, "Skill: %d", iSkill);
						vecPos.y += 22; ImGui::SetCursorPos(vecPos);
						ImGui::TextColored(iTeamwork > 66 ? IMGUI_GREEN : iTeamwork > 33 ? IMGUI_YELLOW : IMGUI_RED, "Co-op: %d", iTeamwork);
						vecPos.y += 22; ImGui::SetCursorPos(vecPos);
						ImGui::TextColored(iAggression > 66 ? IMGUI_GREEN : iAggression > 33 ? IMGUI_YELLOW : IMGUI_RED, "Bravery: %d", iAggression);

						vecPos = ImVec2((pPortrait ? pPortrait->m_iWidth : 128) + 120, flY); ImGui::SetCursorPos(vecPos);
						ImGui::Text("Silencer User: %s", Character.m_bPrefersSilencer ? "Yes" : "No");	// Since all character has their own name, so..

						vecPos.y += 22; ImGui::SetCursorPos(vecPos);
						ImGui::Text("Preference: %s", Character.GetPreferedWpn());

						++itChar;
					}

					if (bNewCharacter)
						ImGui::SetScrollHereY(1.0f);

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			m_iSetTab = TAB_UNSET;
			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void Gui::BotProfile::Summary(const BotProfile_t* pProfile) noexcept
{
	if (!pProfile)
		return;

	using namespace Reflection::BotProfile;

	//if (pProfile->AttribValid<NAME>())
	//	ImGui::BulletText("%s: %s", BotProfile_t::GetAttribName<NAME>(), pProfile->m_szName.c_str());

	if (pProfile->AttribValid<WEAPON_PREFERENCE>())
	{
		for (const auto& szWeapon : pProfile->m_rgszWpnPreference)
			ImGui::BulletText("%s: %s", BotProfile_t::GetAttribName<WEAPON_PREFERENCE>(), szWeapon.c_str());
	}

	if (pProfile->AttribValid<ATTACK_DELAY>())
		ImGui::BulletText("%s: %.2f", BotProfile_t::GetAttribName<ATTACK_DELAY>(), pProfile->m_flAttackDelay);

	if (pProfile->AttribValid<REACTION_TIME>())
		ImGui::BulletText("%s: %.2f", BotProfile_t::GetAttribName<REACTION_TIME>(), pProfile->m_flReactionTime);

	if (pProfile->AttribValid<DIFFICULTY>())
	{
		for (size_t i = 0; i < Difficulty_e::_LAST; ++i)
		{
			if (pProfile->m_bitsDifficulty & (1 << i))
				ImGui::BulletText("%s: %s", BotProfile_t::GetAttribName<DIFFICULTY>(), g_rgszDifficultyNames[i]);
		}
	}

	if (pProfile->AttribValid<AGGRESSION>())
		ImGui::BulletText("%s: %d", BotProfile_t::GetAttribName<AGGRESSION>(), pProfile->m_iAggression);

	if (pProfile->AttribValid<COST>())
		ImGui::BulletText("%s: %d", BotProfile_t::GetAttribName<COST>(), pProfile->m_iCost);

	if (pProfile->AttribValid<SKILL>())
		ImGui::BulletText("%s: %d", BotProfile_t::GetAttribName<SKILL>(), pProfile->m_iSkill);

	if (pProfile->AttribValid<TEAMWORK>())
		ImGui::BulletText("%s: %d", BotProfile_t::GetAttribName<TEAMWORK>(), pProfile->m_iTeamwork);
	
	if (pProfile->AttribValid<VOICEPITCH>())
		ImGui::BulletText("%s: %d", BotProfile_t::GetAttribName<VOICEPITCH>(), pProfile->m_iVoicePitch);

	if (pProfile->AttribValid<SKIN>())
	{
		if (UTIL_GetStringType(pProfile->m_szSkin.c_str()) == 1)	// int
		{
			const auto iSkin = std::atoi(pProfile->m_szSkin.c_str());
			ImGui::BulletText("%s: %s (%d)", BotProfile_t::GetAttribName<SKIN>(), MissionPack::IsTeammate(pProfile->m_szName) ? g_rgszCTSkinName[iSkin] : g_rgszTSkinName[iSkin], iSkin);
		}
		else
			ImGui::BulletText("%s: %s", BotProfile_t::GetAttribName<SKIN>(), pProfile->m_szSkin.c_str());
	}

	if (pProfile->AttribValid<TEAM>())
		ImGui::BulletText("%s: %s", BotProfile_t::GetAttribName<TEAM>(), pProfile->m_szTeam.c_str());

	if (pProfile->AttribValid<VOICEBANK>())
		ImGui::BulletText("%s: %s", BotProfile_t::GetAttribName<VOICEBANK>(), pProfile->m_szVoiceBank.c_str());
}
