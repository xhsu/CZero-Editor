/*
* A "Map_t" represents a set of information that extracted from a .bsp file.
* Nov 08 2021
*/

#include "Precompiled.hpp"

extern std::list<std::string> BSP_CompileResourceList(const char* pszBspPath) noexcept;

void Map_t::Initialize(const fs::path & hPath) noexcept
{
	m_Path = hPath;
	m_szName = hPath.filename().string();
	m_szName.erase(m_szName.find('.'));	// Remove the ext name.

	CheckBasicFile();

	m_rgszResources = BSP_CompileResourceList(m_Path.string().c_str());
}

void Map_t::CheckBasicFile(void) noexcept
{
	m_bBriefFileExists = CZFile::Exists("maps\\" + m_szName + ".txt"s);
	m_bDetailFileExists = CZFile::Exists("maps\\" + m_szName + "_detail.txt"s);
	m_bNavFileExists = CZFile::Exists("maps\\" + m_szName + ".nav"s);
	m_bOverviewFileExists = CZFile::Exists("overviews\\" + m_szName + ".bmp"s) && CZFile::Exists("overviews\\" + m_szName + ".txt"s);	// You can't miss either or them!
	m_bThumbnailExists = CZFile::Exists("gfx\\thumbnails\\maps\\" + m_szName + ".tga"s);
	m_bWiderPreviewExists = CZFile::Exists("gfx\\thumbnails\\maps_wide\\" + m_szName + ".tga"s);

	if (m_bThumbnailExists)
		ImageLoader::Add(g_GamePath.string() + "\\gfx\\thumbnails\\maps\\" + m_szName + ".tga"s, &m_Thumbnail);
	if (m_bWiderPreviewExists)
		ImageLoader::Add(g_GamePath.string() + "\\gfx\\thumbnails\\maps_wide\\" + m_szName + ".tga"s, &m_WiderPreview);
	if (m_bBriefFileExists)
	{
		if (m_pszBriefString)
		{
			free(m_pszBriefString);
			m_pszBriefString = nullptr;
		}

		if (FILE* f = CZFile::Open(("maps\\" + m_szName + ".txt"s).c_str(), "rb"); f != nullptr)
		{
			fseek(f, 0, SEEK_END);
			auto iSize = ftell(f);

			m_pszBriefString = (char*)malloc(iSize + 1);
			fseek(f, 0, SEEK_SET);
			fread_s(m_pszBriefString, iSize + 1, iSize, 1, f);
			m_pszBriefString[iSize] = '\0';

			fclose(f);	// #RET_FCLOSE
		}
	}
}
