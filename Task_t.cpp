/*
* A "Task_t" is a single task involved in a specific location.
* Nov 08 2021
*/

#include "Precompiled.hpp"

import UtlString;

void Task_t::Parse(const std::string& sz) noexcept
{
	std::vector<std::string> rgszTokens(5, ""s);
	UTIL_Split(sz, rgszTokens, " "s);

	auto iArgCount = rgszTokens.size();

	// Identify task.
	for (m_iType = (TaskType_e)0; m_iType < _countof(g_rgszTaskNames); ++m_iType)
	{
		if (!_stricmp(rgszTokens[0].c_str(), g_rgszTaskNames[(unsigned)m_iType]))
			break;
	}

	for (const auto& szToken : rgszTokens)
	{
		if (UTIL_GetStringType(szToken.c_str()) == 1)	// int
			m_iCount = std::stoi(szToken);
		else if (szToken == "survive"s)
			m_bSurvive = true;
		else if (szToken == "inarow"s)
			m_bInARow = true;
		else if (m_szWeapon.empty())
		{
			// Identify weapon should be use.
			for (int i = 0; i < _countof(g_rgszWeaponNames); i++)
			{
				if (!g_rgbIsTaskWeapon[i])
					continue;

				if (!_stricmp(szToken.c_str(), g_rgszWeaponNames[i]))
				{
					m_szWeapon = g_rgszWeaponNames[i];	// Invisible std::tolower()
					break;
				}
			}
		}
	}
}

std::string Task_t::ToString(void) const noexcept
{
	std::string ret = g_rgszTaskNames[(unsigned)m_iType] + " "s;

	if (m_iCount && (1 << m_iType) & Tasks::REQ_COUNT)
		ret += std::to_string(m_iCount) + ' ';

	if (!m_szWeapon.empty() && (1 << m_iType) & Tasks::REQ_WEAPON)
		ret += m_szWeapon + " "s;

	if (m_bSurvive && (1 << m_iType) & Tasks::SURVIVE)
		ret += "survive"s + ' ';

	if (m_bInARow && (1 << m_iType) & Tasks::INAROW)
		ret += "inarow"s + ' ';

	ret.pop_back();	// Remove ' ' at the end.
	return ret;
}

const char* Task_t::SanityCheck(void) noexcept
{
	if (!((1 << m_iType) & Tasks::REQ_COUNT))
		m_iCount = 0;
	if (!((1 << m_iType) & Tasks::REQ_WEAPON))
		m_szWeapon = "";
	if (!((1 << m_iType) & Tasks::SURVIVE))
		m_bSurvive = false;
	if (!((1 << m_iType) & Tasks::INAROW))
		m_bInARow = false;

	if ((1 << m_iType) & Tasks::REQ_COUNT && m_iCount < 1)
		return "Count must greater than 0 for this task!";
	if ((1 << m_iType) & Tasks::REQ_WEAPON && m_szWeapon.empty())
		return "Weapon name must be provided for this task!";

	return nullptr;
}