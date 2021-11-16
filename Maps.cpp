/*
* "Maps"
* Represending all maps that Condition Zero have access to.
*
* Nov 09 2021
*/

#include "Precompiled.hpp"

import UtlString;

void Maps::Load(void) noexcept
{
	std::scoped_lock Lock(Mutex);

	g_Maps.clear();

	for (const auto& Directory : CZFile::m_Directories)
	{
		if (Directory.string().find("valve") != std::string::npos)	// Skip all HL maps. They shouldn't appears in CZ.
			continue;

		fs::path hMapsFolder = Directory.c_str() + L"\\maps"s;
		if (!fs::exists(hMapsFolder))
			continue;

		for (const auto& File : fs::directory_iterator(hMapsFolder))
		{
			std::string szFile = File.path().filename().string();
			if (!stristr(szFile.c_str(), ".bsp"))
				continue;

			szFile.erase(szFile.find(".bsp"));	// Remove this part. Out Locus_t accesses g_Maps without ext name.
			if (g_Maps.find(szFile) != g_Maps.end())	// Already added by other mod.
				continue;

			g_Maps[szFile].Initialize(File.path());
		}
	}
}
