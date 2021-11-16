/*
* A "Locus_t" is a map with a set of tasks i.e. a mission.
* This is the base unit of a mission pack.
* 
* Nov 08 2021
*/

#include "Precompiled.hpp"

import UtlKeyValues;
import UtlString;

void Locus_t::Parse(ValveKeyValues* pkv) noexcept
{
	m_rgszBots.clear();
	m_Tasks.clear();

	m_szMap = pkv->GetName();

	ValveKeyValues* pSub = pkv->FindEntry("bots");
	if (pSub)
		UTIL_Split(pSub->GetValue<Name_t>(), m_rgszBots, " "s);

	if ((pSub = pkv->FindEntry("minEnemies")) != nullptr)
		m_iMinEnemies = pSub->GetValue<int>();

	if ((pSub = pkv->FindEntry("threshold")) != nullptr)
		m_iThreshold = pSub->GetValue<int>();

	if ((pSub = pkv->FindEntry("tasks")) != nullptr)
	{
		std::vector<std::string> rgszTasks;
		UTIL_Split(pSub->GetValue<std::string>(), rgszTasks, "'"s);

		for (auto& szTask : rgszTasks)
		{
			UTIL_Trim(szTask);

			if (szTask.empty())
				continue;

			m_Tasks.emplace_back(szTask);
		}
	}

	if ((pSub = pkv->FindEntry("FriendlyFire")) != nullptr)
		m_bFriendlyFire = pSub->GetValue<bool>();

	if ((pSub = pkv->FindEntry("commands")) != nullptr)
		m_szConsoleCommands = pSub->GetValue<std::string>();
}

[[nodiscard]]
ValveKeyValues* Locus_t::Save(void) const	noexcept // #RET_HEAP_MEM
{
	auto pkv = new ValveKeyValues(m_szMap.c_str());

	std::string szBotNames;
	for (const auto& szBotName : m_rgszBots)
		szBotNames += szBotName + ' ';
	szBotNames.pop_back();	// Remove ' ' at the end.
	pkv->SetValue("bots", szBotNames);

	pkv->SetValue("minEnemies", m_iMinEnemies);
	pkv->SetValue("threshold", m_iThreshold);

	std::string szTasks;
	for (const auto& Task : m_Tasks)
		szTasks += '\'' + Task.ToString() + "' ";
	szTasks.pop_back();	// Remove ' ' at the end.
	pkv->SetValue("tasks", szTasks);

	if (m_bFriendlyFire)
	{
		pkv->SetValue("FriendlyFire", true);
	}

	if (!m_szConsoleCommands.empty())
	{
		pkv->SetValue("commands", m_szConsoleCommands);
	}

	return pkv;
}
