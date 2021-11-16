/*
* A "CareerGame_t" is a single playthrough in one difficulty.
* This also represents a .vdf file.
* 
* Nov 08 2021
*/

#include "Precompiled.hpp"

import UtlKeyValues;
import UtlString;

void CareerGame_t::Parse(ValveKeyValues* pkv) noexcept
{
	Reset();

	ValveKeyValues* pSub = pkv->FindEntry("InitialPoints");
	if (pSub)
		m_iInitialPoints = pSub->GetValue<int>();

	if ((pSub = pkv->FindEntry("MatchWins")) != nullptr)
		m_iMatchWins = pSub->GetValue<int>();

	if ((pSub = pkv->FindEntry("MatchWinBy")) != nullptr)
		m_iMatchWinBy = pSub->GetValue<int>();

	if ((pSub = pkv->FindEntry("Characters")) != nullptr)
	{
		m_rgszCharacters.clear();
		UTIL_Split(pSub->GetValue<Name_t>(), m_rgszCharacters, " "s);
	}

	if ((pSub = pkv->FindEntry("CostAvailability")) != nullptr)
	{
		[&] <size_t... I>(std::index_sequence<I...>)
		{
			ValveKeyValues* p = nullptr;
			(((p = pSub->FindEntry(UTIL_IntToString<I + 1>())) != nullptr ? (m_rgiCostAvailability[I] = p->GetValue<int>()) : (int{})), ...);
		}
		(std::make_index_sequence<5>{});
	}

	if ((pSub = pkv->FindEntry("Maps")) != nullptr)
	{
		ValveKeyValues* p = pSub->GetFirstSubkey();

		while (p != nullptr)
		{
			m_Loci.emplace_back(p);
			p = p->GetNextSubkey();
		}
	}
}

void CareerGame_t::Reset(void) noexcept
{
	m_iInitialPoints = 2;
	m_iMatchWins = 3;
	m_iMatchWinBy = 2;
	m_rgszCharacters.clear();
	m_rgiCostAvailability = { 1, 6, 10, 15, 99 };
	m_Loci.clear();
}

[[nodiscard]]
ValveKeyValues* CareerGame_t::Save(void) const noexcept	// #RET_HEAP_MEM
{
	auto pkv = new ValveKeyValues("CareerGame");

	pkv->SetValue("InitialPoints", m_iInitialPoints);
	pkv->SetValue("MatchWins", m_iMatchWins);
	pkv->SetValue("MatchWinBy", m_iMatchWinBy);

	Name_t szAllNames;
	for (const auto& szName : m_rgszCharacters)
		szAllNames += szName + ' ';
	szAllNames.pop_back();	// Remove last space.
	pkv->SetValue("Characters", szAllNames);

	auto p = pkv->CreateEntry("CostAvailability");
	[&] <size_t... I>(std::index_sequence<I...>)
	{
		(p->SetValue(UTIL_IntToString<I + 1>(), m_rgiCostAvailability[I]), ...);
	}
	(std::make_index_sequence<5>{});	// Static for.

	p = pkv->CreateEntry("Maps");
	for (const auto& Locus : m_Loci)
		p->AddEntry(Locus.Save());

	return pkv;
}
