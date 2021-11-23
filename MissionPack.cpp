/*
* "MissionPack"
* Represending the entire mission pack folder.
*
* Nov 09 2021
*/

#include "Precompiled.hpp"

import UtlKeyValues;

void MissionPack::CompileFileList(Files_t& rgFiles, const fs::path& hFolder, bool bCheck) noexcept
{
	rgFiles[FILE_OVERVIEW] = hFolder.c_str() + L"\\Overview.vdf"s;
	rgFiles[FILE_THUMBNAIL] = hFolder.c_str() + L"\\Thumbnail.tga"s;
	rgFiles[FILE_EASY] = hFolder.c_str() + L"\\Easy.vdf"s;
	rgFiles[FILE_NORMAL] = hFolder.c_str() + L"\\Normal.vdf"s;
	rgFiles[FILE_HARD] = hFolder.c_str() + L"\\Hard.vdf"s;
	rgFiles[FILE_EXPERT] = hFolder.c_str() + L"\\Expert.vdf"s;
	rgFiles[FILE_BOT_PROFILE] = hFolder.c_str() + L"\\BotProfile.db"s;

	if (bCheck)
	{
		for (const auto& File : rgFiles)
		{
			if (!fs::exists(File))
				std::cout << "Warning: File '" << File << "' is missing from '" << hFolder << "'\n";
		}
	}
}

void MissionPack::CompileFileListOfTurtleRockCounterTerrorist(Files_t& rgFiles, const fs::path& hGameFolder) noexcept
{
	rgFiles[FILE_OVERVIEW] = hGameFolder.c_str() + L"\\MissionPacks\\TurtleRockCounterTerrorist\\Overview.vdf"s;
	rgFiles[FILE_THUMBNAIL] = hGameFolder.c_str() + L"\\MissionPacks\\TurtleRockCounterTerrorist\\Thumbnail.tga"s;
	rgFiles[FILE_EASY] = hGameFolder.c_str() + L"\\CareerGameEasy.vdf"s;
	rgFiles[FILE_NORMAL] = hGameFolder.c_str() + L"\\CareerGameNormal.vdf"s;
	rgFiles[FILE_HARD] = hGameFolder.c_str() + L"\\CareerGameHard.vdf"s;
	rgFiles[FILE_EXPERT] = hGameFolder.c_str() + L"\\CareerGameExpert.vdf"s;
	rgFiles[FILE_BOT_PROFILE] = hGameFolder.c_str() + L"\\BotCampaignProfile.db"s;

	for (const auto& File : rgFiles)
	{
		if (!fs::exists(File))
			std::cout << "Warning: File '" << File << "' is missing from game folder.\n";
	}
}

// Load the folder as if it were a mission pack.
void MissionPack::LoadFolder(const fs::path& hFolder) noexcept
{
	const std::scoped_lock Lock(MissionPack::Mutex, BotProfileMgr::Mutex);

	Name = hFolder.filename().string();
	Folder = hFolder;

	if (!_stricmp(Name.c_str(), "czero"))
		CompileFileListOfTurtleRockCounterTerrorist(Files, hFolder);
	else
		CompileFileList(Files, hFolder);

	for (auto& [iDifficulty, p] : CGKVs)
	{
		if (p)
			p->deleteThis();

		p = nullptr;
	}

	for (auto& [iDifficulty, CG] : CareerGames)
		CG.Reset();

	memset(&Thumbnail, NULL, sizeof(Thumbnail));

	BotProfileMgr::Clear();

	if (fs::exists(Files[FILE_OVERVIEW]))
	{
		// Overfiew file parse #TODO_OVERVIEW_FILE
	}

	if (fs::exists(Files[FILE_THUMBNAIL]))
	{
		ImageLoader::Add(Files[FILE_THUMBNAIL], &Thumbnail);
	}

	if (fs::exists(Files[FILE_BOT_PROFILE]))
	{
		BotProfileMgr::Parse(Files[FILE_BOT_PROFILE]);
	}

	for (auto i = Difficulty_e::EASY; i <= Difficulty_e::EXPERT; ++i)
	{
		if (fs::exists(Files[i + FILE_EASY]))
		{
			CGKVs[i] = new ValveKeyValues(g_rgszDifficultyNames[(size_t)i]);
			CGKVs[i]->LoadFromFile(Files[i + FILE_EASY].string().c_str());
			CareerGames[i].Parse(CGKVs[i]);
		}
	}
}

void MissionPack::Save(const fs::path& hFolder) noexcept
{
	const std::scoped_lock Lock(MissionPack::Mutex, BotProfileMgr::Mutex);

	if (!fs::exists(hFolder))
		fs::create_directories(hFolder);

	Files_t rgFiles;
	CompileFileList(rgFiles, hFolder, false);

	if (!fs::exists(rgFiles[FILE_OVERVIEW]) && fs::exists(Files[FILE_OVERVIEW]))	// Properly output Overview.vdf #TODO_OVERVIEW_FILE
		fs::copy_file(Files[FILE_OVERVIEW], rgFiles[FILE_OVERVIEW]);

	if (!fs::exists(rgFiles[FILE_THUMBNAIL]) && fs::exists(Files[FILE_THUMBNAIL]))
		fs::copy_file(Files[FILE_THUMBNAIL], rgFiles[FILE_THUMBNAIL]);

	for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
	{
		if (CareerGames[i].m_Loci.empty())
		{
			std::cout << "Empty difficulty " << std::quoted(g_rgszDifficultyNames[(size_t)i]) << " will not be save.\n";
			continue;
		}

		if (CGKVs[i])
			CGKVs[i]->deleteThis();

		CGKVs[i] = CareerGames[i].Save();
		CGKVs[i]->SaveToFile(rgFiles[i + FILE_EASY].string().c_str());
	}

	BotProfileMgr::Save(rgFiles[FILE_BOT_PROFILE]);
}

// Is this character enrolled as our potential teammate?
bool MissionPack::IsTeammate(const Name_t& szName) noexcept
{
	return std::find_if(CareerGames[Gui::MissionPack::CurBrowsing].m_rgszCharacters.begin(), CareerGames[Gui::MissionPack::CurBrowsing].m_rgszCharacters.end(),
		[&szName](const Name_t& szCharacter)
		{
			return szName == szCharacter;
		}
	) != CareerGames[Gui::MissionPack::CurBrowsing].m_rgszCharacters.end();
}

// Is this character is enlisted as our enemy in any of our mission locations?
bool MissionPack::IsEnemy(const Name_t& szName) noexcept
{
	for (const auto& Locus : CareerGames[Gui::MissionPack::CurBrowsing].m_Loci)
	{
		bool bFoundHere = std::find_if(Locus.m_rgszBots.begin(), Locus.m_rgszBots.end(),
			[&szName](const Name_t& szCharacter)
			{
				return szName == szCharacter;
			}
		) != Locus.m_rgszBots.end();

		if (bFoundHere)
			return true;
	}

	return false;
}
