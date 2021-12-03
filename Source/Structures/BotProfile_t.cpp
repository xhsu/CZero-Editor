/*
* "BotProfile_t"
* Exactly as it says.
*
* Nov 09 2021
*/

#include "Precompiled.hpp"


bool BotProfile_t::SaveAttrib(std::ofstream& hFile) noexcept
{
	if (AttribValid<offsetof(BotProfile_t, m_rgszWpnPreference)>())
	{
		for (const auto& szWeapon : m_rgszWpnPreference)
			hFile << "\tWeaponPreference = " << szWeapon << std::endl;
	}

#define SAVE_MEMBER_HELPER_INT(x)	if (AttribValid<offsetof(BotProfile_t, m_i##x)>())	\
										hFile << "\t" #x " = " << m_i##x << std::endl
#define SAVE_MEMBER_HELPER_FLT(x)	if (AttribValid<offsetof(BotProfile_t, m_fl##x)>())	\
										hFile << "\t" #x " = " << m_fl##x << std::endl
#define SAVE_MEMBER_HELPER_STR(x)	if (AttribValid<offsetof(BotProfile_t, m_sz##x)>())	\
										hFile << "\t" #x " = " << m_sz##x << std::endl

	SAVE_MEMBER_HELPER_FLT(AttackDelay);
	SAVE_MEMBER_HELPER_FLT(ReactionTime);

	if (AttribValid<offsetof(BotProfile_t, m_bitsDifficulty)>())
	{
		bool bFirst = true;

		hFile << "\tDifficulty = ";

		for (size_t i = 0; i < _countof(g_rgszDifficultyNames); i++)
		{
			if (m_bitsDifficulty & (1 << i))
			{
				if (bFirst)
					bFirst = false;
				else
					hFile << '+';

				hFile << g_rgszDifficultyNames[i];
			}
		}

		hFile << std::endl;
	}

	SAVE_MEMBER_HELPER_INT(Aggression);
	SAVE_MEMBER_HELPER_INT(Cost);
	SAVE_MEMBER_HELPER_INT(Skill);
	SAVE_MEMBER_HELPER_INT(Teamwork);
	SAVE_MEMBER_HELPER_INT(VoicePitch);

	SAVE_MEMBER_HELPER_STR(Skin);
	SAVE_MEMBER_HELPER_STR(Team);
	SAVE_MEMBER_HELPER_STR(VoiceBank);

	return true;

#undef SAVE_MEMBER_HELPER_INT
#undef SAVE_MEMBER_HELPER_FLT
#undef SAVE_MEMBER_HELPER_STR
}

bool BotProfile_t::IsDefault(void) const noexcept
{
	return this == &BotProfileMgr::m_Default;
}

bool BotProfile_t::IsTemplate(void) const noexcept
{
	return BotProfileMgr::m_Templates.contains(m_szName);
}

const char* BotProfile_t::GetPreferedWpn(void) const noexcept
{
	if (!m_rgszWpnPreference.empty())
		return m_rgszWpnPreference.front().c_str();

	for (auto iter = m_rgszRefTemplates.crbegin(); iter != m_rgszRefTemplates.crend(); ++iter)
	{
		auto& Template = BotProfileMgr::m_Templates[*iter];

		if (!Template.m_rgszWpnPreference.empty())
			return Template.m_rgszWpnPreference.front().c_str();
	}

	if (!BotProfileMgr::m_Default.m_rgszWpnPreference.empty())
		return BotProfileMgr::m_Default.m_rgszWpnPreference.front().c_str();

	return "none";
}

const Weapons_t& BotProfile_t::GetPreferedWpnList(void) const noexcept
{
	if (!m_rgszWpnPreference.empty())
		return m_rgszWpnPreference;

	for (auto iter = m_rgszRefTemplates.crbegin(); iter != m_rgszRefTemplates.crend(); ++iter)
	{
		auto& Template = BotProfileMgr::m_Templates[*iter];

		if (!Template.m_rgszWpnPreference.empty())
			return Template.m_rgszWpnPreference;
	}

	return BotProfileMgr::m_Default.m_rgszWpnPreference;
}

void BotProfile_t::Reset(bool bDeleteReferences) noexcept
{
	m_szName = "Error - No name"s;
	m_rgszWpnPreference.clear();
	m_bPrefersSilencer = false;
	m_flAttackDelay = -1;
	m_flReactionTime = -1;
	m_bitsDifficulty = 0;
	m_iAggression = -1;
	m_iCost = -1;
	m_iSkill = -1;
	m_iTeamwork = -1;
	m_iVoicePitch = -1;

	if (bDeleteReferences)
		m_rgszRefTemplates.clear();

	m_szSkin = "";
	m_szTeam = "";
	m_szVoiceBank = "";
}

const char* BotProfile_t::SanityCheck(void) const noexcept
{
	using namespace std;
	using namespace Reflection::BotProfile;

	// #TODO_ADD_DUP_NAME_CHECK

	if (Get<NAME>()->empty())
		return "Empty bot name.";
	if (Get<NAME>()->find_first_of(" \t\n\v\f\r") != Name_t::npos)	// basic_string::find will try to match the entire param.
		return "Name string contains space(s).";
	if (*Get<ATTACK_DELAY>() < 0)
		return "Attack delay must be a non-negative value.";
	if (*Get<REACTION_TIME>() < 0)
		return "Reaction time must be a non-negative value.";
	if (*Get<AGGRESSION>() < 0 || *Get<AGGRESSION>() > 100)
		return "Aggression must be a value between 0 and 100.";
	if (*Get<COST>() < 1 || *Get<COST>() > 5)
		return "Cost of a teammate must be a value between 1 and 5.";
	if (*Get<SKILL>() < 0 || *Get<SKILL>() > 100)
		return "Skill must be a value between 0 and 100.";
	if (*Get<TEAMWORK>() < 0 || *Get<TEAMWORK>() > 100)
		return "Teamwork must be a value between 0 and 100.";
	if (*Get<VOICEPITCH>() < 1)
		return "Voice pitch must be a value greater than 0.";
	if (GetPreferedWpnList().empty())	// This must use member from self.
		return "You must using at least one template for your character.";

	for (const auto& szWpn : GetPreferedWpnList())
	{
		bool bValidBuyCommand = false;

		for (int i = 0; i < _countof(g_rgszWeaponNames); i++)
		{
			if (!g_rgbIsBuyCommand[i])
				continue;

			if (szWpn == g_rgszWeaponNames[i])
			{
				bValidBuyCommand = true;
				break;
			}
		}

		if (!bValidBuyCommand)
			return "Invalid buy command found in WeaponPreference field!";
	}

	return nullptr;	// Nullptr is good news!
}

void BotProfile_t::Inherit(const BotProfile_t& Parent) noexcept
{
	if (!Parent.m_szName.starts_with("Error"))
		m_szName = Parent.m_szName;

	if (!Parent.m_rgszWpnPreference.empty())
		m_rgszWpnPreference = Parent.m_rgszWpnPreference;

	if (Parent.m_flAttackDelay >= 0)
		m_flAttackDelay = Parent.m_flAttackDelay;

	if (Parent.m_flReactionTime >= 0)
		m_flReactionTime = Parent.m_flReactionTime;

	if (Parent.m_bitsDifficulty > 0)
		m_bitsDifficulty = Parent.m_bitsDifficulty;

	if (Parent.m_iAggression >= 0)
		m_iAggression = Parent.m_iAggression;

	if (Parent.m_iCost > 0)
		m_iCost = Parent.m_iCost;

	if (Parent.m_iSkill >= 0)
		m_iSkill = Parent.m_iSkill;

	if (Parent.m_iTeamwork >= 0)
		m_iTeamwork = Parent.m_iTeamwork;

	if (Parent.m_iVoicePitch > 0)
		m_iVoicePitch = Parent.m_iVoicePitch;

	if (!Parent.m_szSkin.empty() && !Parent.m_szSkin.starts_with("Error"))
		m_szSkin = Parent.m_szSkin;

	if (!Parent.m_szTeam.empty() && !Parent.m_szTeam.starts_with("Error"))
		m_szTeam = Parent.m_szTeam;

	if (!Parent.m_szVoiceBank.empty() && !Parent.m_szVoiceBank.starts_with("Error"))
		m_szVoiceBank = Parent.m_szVoiceBank;
}

void BotProfile_t::Inherit(bool bShouldResetFirst, bool bCopyFromDefault) noexcept
{
	if (m_rgszRefTemplates.empty())
		return;

	if (bShouldResetFirst)
		Reset(false);	// Will can inherit a shit if we have RefTemplate field cleared.

	if (bCopyFromDefault)	// Copy from default first.
		Inherit(BotProfileMgr::m_Default);

	for (const auto& szInherit : m_rgszRefTemplates)
	{
		// Inherit code.
		if (auto iter = BotProfileMgr::m_Templates.find(szInherit); iter != BotProfileMgr::m_Templates.end())
		{
			Inherit(iter->second);
		}
		else
		{
			std::cout << "Error while " << std::quoted(m_szName) << " attempting to inherit from " << std::quoted(iter->first) << " - invalid template reference " << szInherit << '\n';
		}
	}
}
