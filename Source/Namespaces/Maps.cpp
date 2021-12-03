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

std::string Maps::ListResources(const Map_t& Map) noexcept
{
	std::stringstream ss;

	ss << '[' << (Map.m_bBriefFileExists ? "Found" : "No found") << "] " << "maps\\" + Map.m_szName + ".txt" << std::endl;
	ss << '[' << (Map.m_bDetailFileExists ? "Found" : "No found") << "] " << "maps\\" + Map.m_szName + "_detail.txt"s << std::endl;
	ss << '[' << (Map.m_bNavFileExists ? "Found" : "No found") << "] " << "maps\\" + Map.m_szName + ".nav" << std::endl;
	ss << '[' << (CZFile::Exists("overviews\\" + Map.m_szName + ".bmp") ? "Found" : "No found") << "] " << "overviews\\" + Map.m_szName + ".bmp" << std::endl;
	ss << '[' << (CZFile::Exists("overviews\\" + Map.m_szName + ".txt") ? "Found" : "No found") << "] " << "overviews\\" + Map.m_szName + ".txt" << std::endl;
	ss << '[' << (Map.m_bThumbnailExists ? "Found" : "No found") << "] " << "gfx\\thumbnails\\maps\\" + Map.m_szName + ".tga" << std::endl;
	ss << '[' << (Map.m_bWiderPreviewExists ? "Found" : "No found") << "] " << "gfx\\thumbnails\\maps_wide\\" + Map.m_szName + ".tga" << std::endl;

	for (const auto& szResource : Map.m_rgszResources)
		ss << '[' << (CZFile::Exists(szResource) ? "Found" : "No found") << "] " << szResource << std::endl;

	return ss.str();
}

std::array<std::string, 7> Maps::GetUnlistedResources(const std::string& szMapName) noexcept
{
	return {
		"maps\\" + szMapName + ".txt",
		"maps\\" + szMapName + "_detail.txt",
		"maps\\" + szMapName + ".nav",
		"overviews\\" + szMapName + ".bmp",
		"overviews\\" + szMapName + ".txt",
		"gfx\\thumbnails\\maps\\" + szMapName + ".tga",
		"gfx\\thumbnails\\maps_wide\\" + szMapName + ".tga",
	};
}
