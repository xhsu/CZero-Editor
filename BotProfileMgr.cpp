/*
* "BotProfileMgr"
* Represending the BotProfiles.db file.
*
* Nov 09 2021
*/

#include "Precompiled.hpp"

import UtlString;

void BotProfileMgr::Clear(void) noexcept
{
	m_Profiles.clear();
	m_Default.Reset();
	m_Templates.clear();
	m_Skins.clear();
}

void BotProfileMgr::RemoveNoneInWpnPref(Weapons_t* p) noexcept	// #UNDONE_LONG_TERM move this to .hpp header.
{
	p->erase(
		std::remove_if(
			p->begin(),
			p->end(),
			[](const std::string& szWpn) { return !_stricmp(szWpn.c_str(), "none"); }
		),
		p->end());
}

void BotProfileMgr::GenerateSkinsFromProfiles(void) noexcept
{
	using Reflection::BotProfile::SKIN;

	if (!UTIL_GetStringType(m_Default.m_szSkin.c_str()) && !m_Default.AttribSanity<SKIN>(true))	// Ignore heritage duing the process.
	{
		if (!m_Skins.contains(m_Default.m_szSkin))
			m_Skins[m_Default.m_szSkin] = m_Default.m_szSkin;
	}

	for (auto& [szName, Template] : m_Templates)
	{
		if (!UTIL_GetStringType(Template.m_szSkin.c_str()) && !Template.AttribSanity<SKIN>(true))
		{
			if (!m_Skins.contains(Template.m_szSkin))
				m_Skins[Template.m_szSkin] = Template.m_szSkin;
		}
	}

	for (auto& Character : m_Profiles)
	{
		if (!Character.AttribSanity<SKIN>(true))
		{
			auto& szSkin = *Character.Get<SKIN>();

			if (!UTIL_GetStringType(szSkin.c_str()) && !m_Skins.contains(szSkin))
				m_Skins[szSkin] = szSkin;
		}
	}
}

bool BotProfileMgr::ParseWithInheritance(const fs::path& hPath) noexcept	// Assuming this file IS ACTUALLY exists.
{
	Clear();

	FILE* f = nullptr;
	fopen_s(&f, hPath.string().c_str(), "rb");
	fseek(f, 0, SEEK_END);

	auto flen = ftell(f);
	auto buf = (BYTE*)malloc(flen + 1);	// #MEM_ALLOC
	fseek(f, 0, SEEK_SET);
	fread_s(buf, flen + 1, flen, 1, f);
	buf[flen] = 0;

	std::list<std::string> Tokens;
	char* c = (char*)&buf[0];
	while (c != (char*)&buf[flen + 1] && *c != '\0')
	{
	LAB_SKIP_COMMENT:;
		if (c[0] == '/' && c[1] == '/')
		{
			char* p = c;
			while (true)
			{
				p++;

				if (*p == '\n')
					break;
			}

			p++;	// Skip '\n' symbol.

			memmove(c, p, strlen(p) + 1);	// including '\0' character.
			goto LAB_SKIP_COMMENT;
		}

		char* p = c;
		while (true)
		{
			switch (*p)
			{
			case ' ':
			case '\f':
			case '\n':
			case '\r':
			case '\t':
			case '\v':
			case '\0':	// EOF
				*p = '\0';

				if (c != p)
					Tokens.emplace_back(c);

				c = ++p;
				goto LAB_BACK_TO_MAIN_LOOP;

			default:
				p++;
				break;
			}

			if (p == (char*)&buf[flen + 1])
			{
				std::cout << "Not a '\\0' terminated buffer.\n";
				return false;
			}
		}

	LAB_BACK_TO_MAIN_LOOP:;
	}

	fclose(f);
	free(buf);	// #MEM_FREED
	buf = nullptr;
	c = nullptr;

	for (auto iter = Tokens.begin(); iter != Tokens.end();)
	{
		bool bIsDefault, bIsTemplate, bIsCustomSkin;
		BotProfile_t* pProfile = nullptr;
		std::string szTempToken;

		if (iter->starts_with("//"))
			goto LAB_ERASE_AND_NEXT;

		bIsDefault = !_stricmp(iter->c_str(), "Default");
		bIsTemplate = !_stricmp(iter->c_str(), "Template");
		bIsCustomSkin = !_stricmp(iter->c_str(), "Skin");

		if (bIsCustomSkin)
		{
			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath << " - expected skin name\n";
				return false;
			}

			// Save Skin name
			szTempToken = *iter;

			iter++;
			if (iter == Tokens.end() || *iter != "Model"s)
			{
				std::cout << "Error parsing " << hPath << " - expected 'Model'\n";
				return false;
			}

			iter++;
			if (iter == Tokens.end() || *iter != "="s)
			{
				std::cout << "Error parsing " << hPath << " - expected '='\n";
				return false;
			}

			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath << " - expected a model name\n";
				return false;
			}

			// Save skin path
			m_Skins[szTempToken] = *iter;

			iter++;
			if (iter == Tokens.end() || *iter != "End"s)
			{
				std::cout << "Error parsing " << hPath << " - expected a model name\n";
				return false;
			}

			goto LAB_REGULAR_CONTINUE;
		}

		if (bIsDefault)
			pProfile = &m_Default;

		// Get template name here.
		else if (bIsTemplate)
		{
			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath << " - expected name of template but 'EOF' met.\n";
				return false;
			}

			pProfile = &m_Templates[*iter];
			goto LAB_READ_ATTRIB;
		}
		else
		{
			m_Profiles.push_back(m_Default);	// Is that a ... copy?
			pProfile = &m_Profiles.back();
		}

		// do inheritance in order of appearance
		if (!bIsTemplate && !bIsDefault)
		{
			UTIL_Split(*iter, pProfile->m_rgszRefTemplates, "+"s);
			pProfile->Inherit(false, false);	// We already initialized.
		}

		// get name of this profile
		if (!bIsDefault)
		{
			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath << " - expected name of profile but 'EOF' met.\n";
				return false;
			}

			pProfile->m_szName = *iter;
			pProfile->m_bPrefersSilencer = static_cast<bool>(pProfile->m_szName[0] % 2);
		}

	LAB_READ_ATTRIB:;
		// read attributes for this profile
		while (true)
		{
			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath << " - expected 'End' but 'EOF' met.\n";
				return false;
			}

			szTempToken = *iter;

			// check for End delimiter
			if (szTempToken == "End"s)
				break;

			iter++;
			if (iter == Tokens.end() || *iter != "="s)
			{
				std::cout << "Error parsing " << hPath << " - expected '='\n";
				return false;
			}

			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath << " - expected attribute value of " << std::quoted(szTempToken) << '\n';
				return false;
			}

			if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Skill"))
				pProfile->m_iSkill = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Skin"))
				pProfile->m_szSkin = *iter;

			else if (!_stricmp(szTempToken.c_str(), "Teamwork"))
				pProfile->m_iTeamwork = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Cost"))
				pProfile->m_iCost = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "VoicePitch"))
				pProfile->m_iVoicePitch = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "VoiceBank"))
				pProfile->m_szVoiceBank = *iter;

			else if (!_stricmp(szTempToken.c_str(), "WeaponPreference"))
			{
				if (!_stricmp(iter->c_str(), "none"))
				{
					pProfile->m_rgszWpnPreference.clear();
					pProfile->m_rgszWpnPreference.emplace_back("none");
					continue;
				}

				bool bWeaponNameParsed = false;
				for (size_t i = 0; i < _countof(g_rgszWeaponNames); i++)
				{
					if (!g_rgbIsBuyCommand[i])	// Skip those non-buycommand text.
						continue;

					if (!_stricmp(iter->c_str(), g_rgszWeaponNames[i]))
					{
						pProfile->m_rgszWpnPreference.erase(
							std::remove_if(pProfile->m_rgszWpnPreference.begin(),
								pProfile->m_rgszWpnPreference.end(),
								[](const std::string& szWpn) { return !_stricmp(szWpn.c_str(), "none"); }
							),
							pProfile->m_rgszWpnPreference.end());

						bWeaponNameParsed = true;
						pProfile->m_rgszWpnPreference.emplace_back(g_rgszWeaponNames[i]);	// Invisible std::tolower().
						break;
					}
				}

				if (!bWeaponNameParsed)
					std::cout << "Unknow weapon name " << std::quoted(*iter) << " met during parsing " << hPath << std::endl;

				// #UNDONE_LONG_TERM Ideally when template weapon and its explictly prefered weapon confilx, one should pick its own prefered weapon. But this is not the case in original CZ.
				// Or we can implement that by itInsertpoint = g_rgszWeaponNames.insert(itInsertpoint, weaponId);
			}

			else if (!_stricmp(szTempToken.c_str(), "ReactionTime"))
				pProfile->m_flReactionTime = std::stof(*iter);

			else if (!_stricmp(szTempToken.c_str(), "AttackDelay"))
				pProfile->m_flAttackDelay = std::stof(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Difficulty"))
			{
				std::list<std::string> DifficultyList;
				UTIL_Split(*iter, DifficultyList, "+"s);

				for (auto& Difficulty : DifficultyList)
				{
					for (size_t i = 0; i < _countof(g_rgszDifficultyNames); i++)
					{
						if (!_stricmp(Difficulty.c_str(), g_rgszDifficultyNames[i]))
						{
							pProfile->m_bitsDifficulty |= (1 << i);
							break;
						}
					}
				}
			}

			else if (!_stricmp(szTempToken.c_str(), "Team"))
				pProfile->m_szTeam = *iter;

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else
				std::cout << "Error parsing " << hPath << " - unknown attribute " << std::quoted(szTempToken) << " with value " << std::quoted(*iter) << '\n';
		}

	LAB_REGULAR_CONTINUE:;
		iter++;
		continue;

	LAB_ERASE_AND_NEXT:;
		iter = Tokens.erase(iter);
	}

	return true;
}

bool BotProfileMgr::Parse(const fs::path& hPath) noexcept
{
	Clear();

	FILE* f = nullptr;
	fopen_s(&f, hPath.string().c_str(), "rb");
	fseek(f, 0, SEEK_END);

	auto flen = ftell(f);
	auto buf = (BYTE*)malloc(flen + 1);	// #MEM_ALLOC
	fseek(f, 0, SEEK_SET);
	fread_s(buf, flen + 1, flen, 1, f);
	buf[flen] = 0;

	std::list<std::string> Tokens;
	char* c = (char*)&buf[0];
	while (c != (char*)&buf[flen + 1] && *c != '\0')
	{
	LAB_SKIP_COMMENT:;
		if (c[0] == '/' && c[1] == '/')
		{
			char* p = c;
			while (true)
			{
				p++;

				if (*p == '\n')
					break;
			}

			p++;	// Skip '\n' symbol.

			memmove(c, p, strlen(p) + 1);	// including '\0' character.
			goto LAB_SKIP_COMMENT;
		}

		char* p = c;
		while (true)
		{
			switch (*p)
			{
			case ' ':
			case '\f':
			case '\n':
			case '\r':
			case '\t':
			case '\v':
			case '\0':	// EOF
				*p = '\0';

				if (c != p)
					Tokens.emplace_back(c);

				c = ++p;
				goto LAB_BACK_TO_MAIN_LOOP;

			default:
				p++;
				break;
			}

			if (p == (char*)&buf[flen + 1])
			{
				std::cout << "Not a '\\0' terminated buffer.\n";
				return false;
			}
		}

	LAB_BACK_TO_MAIN_LOOP:;
	}

	fclose(f);
	free(buf);	// #MEM_FREED
	buf = nullptr;
	c = nullptr;

	for (auto iter = Tokens.begin(); iter != Tokens.end(); /* Do nothing */)
	{
		bool bIsDefault, bIsTemplate, bIsCustomSkin;
		BotProfile_t* pProfile = nullptr;
		std::string szTempToken;

		if (iter->starts_with("//"))
			goto LAB_ERASE_AND_NEXT;

		bIsDefault = !_stricmp(iter->c_str(), "Default");
		bIsTemplate = !_stricmp(iter->c_str(), "Template");
		bIsCustomSkin = !_stricmp(iter->c_str(), "Skin");

		if (bIsCustomSkin)
		{
			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected skin name\n";
				return false;
			}

			// Save Skin name
			szTempToken = *iter;

			iter++;
			if (iter == Tokens.end() || *iter != "Model"s)
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected 'Model'\n";
				return false;
			}

			iter++;
			if (iter == Tokens.end() || *iter != "="s)
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected '='\n";
				return false;
			}

			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected a model name\n";
				return false;
			}

			// Save skin path
			m_Skins[szTempToken] = *iter;

			iter++;
			if (iter == Tokens.end() || *iter != "End"s)
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected a model name\n";
				return false;
			}

			goto LAB_REGULAR_CONTINUE;
		}

		// Initialize default.
		if (bIsDefault)
		{
			pProfile = &m_Default;
			pProfile->m_szName = "Default"s;
		}
		// Initialize template
		else if (bIsTemplate)
		{
			++iter;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected name of template but 'EOF' met.\n";
				return false;
			}

			pProfile = &m_Templates[*iter];
			pProfile->m_szName = *iter;
			goto LAB_READ_ATTRIB;
		}
		// Initialize normal character.
		else
		{
			m_Profiles.push_back(BotProfile_t{});
			pProfile = &m_Profiles.back();
		}

		// The start of a character - templates. We only read what template it use, no inherit involved.
		if (!bIsTemplate && !bIsDefault)
			UTIL_Split(*iter, pProfile->m_rgszRefTemplates, "+"s);

		// get name of this profile
		if (!bIsDefault)
		{
			++iter;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected name of character but 'EOF' met.\n";
				return false;
			}

			pProfile->m_szName = *iter;
			pProfile->m_bPrefersSilencer = static_cast<bool>(pProfile->m_szName[0] % 2);
		}

	LAB_READ_ATTRIB:;
		// read attributes for this profile
		while (true)
		{
			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected 'End' but 'EOF' met.\n";
				return false;
			}

			szTempToken = *iter;

			// check for End delimiter
			if (szTempToken == "End"s)
				break;

			iter++;
			if (iter == Tokens.end() || *iter != "="s)
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected '='\n";
				return false;
			}

			iter++;
			if (iter == Tokens.end())
			{
				std::cout << "Error parsing " << hPath.string().c_str() << " - expected attribute value of " << std::quoted(szTempToken) << '\n';
				return false;
			}

			if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Skill"))
				pProfile->m_iSkill = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Skin"))
				pProfile->m_szSkin = *iter;

			else if (!_stricmp(szTempToken.c_str(), "Teamwork"))
				pProfile->m_iTeamwork = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Cost"))
				pProfile->m_iCost = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "VoicePitch"))
				pProfile->m_iVoicePitch = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "VoiceBank"))
				pProfile->m_szVoiceBank = *iter;

			else if (!_stricmp(szTempToken.c_str(), "WeaponPreference"))
			{
				if (!_stricmp(iter->c_str(), "none"))
				{
					pProfile->m_rgszWpnPreference.clear();
					pProfile->m_rgszWpnPreference.emplace_back("none");
					continue;
				}

				bool bWeaponNameParsed = false;
				for (size_t i = 0; i < _countof(g_rgszWeaponNames); i++)
				{
					if (!g_rgbIsBuyCommand[i])	// Skip those non-buycommand text.
						continue;

					if (!_stricmp(iter->c_str(), g_rgszWeaponNames[i]))
					{
						bWeaponNameParsed = true;
						BotProfileMgr::RemoveNoneInWpnPref(&pProfile->m_rgszWpnPreference);
						pProfile->m_rgszWpnPreference.emplace_back(g_rgszWeaponNames[i]);	// Invisible std::tolower().
						break;
					}
				}

				if (!bWeaponNameParsed)
					std::cout << "Unknow weapon name " << std::quoted(*iter) << " met during parsing " << hPath << std::endl;

				// #UNDONE_LONG_TERM Ideally when template weapon and its explictly prefered weapon confilx, one should pick its own prefered weapon. But this is not the case in original CZ.
				// Or we can implement that by itInsertpoint = g_rgszWeaponNames.insert(itInsertpoint, weaponId);
			}

			else if (!_stricmp(szTempToken.c_str(), "ReactionTime"))
				pProfile->m_flReactionTime = std::stof(*iter);

			else if (!_stricmp(szTempToken.c_str(), "AttackDelay"))
				pProfile->m_flAttackDelay = std::stof(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Difficulty"))
			{
				std::list<std::string> DifficultyList;
				UTIL_Split(*iter, DifficultyList, "+"s);

				for (auto& Difficulty : DifficultyList)
				{
					for (size_t i = 0; i < _countof(g_rgszDifficultyNames); i++)
					{
						if (!_stricmp(Difficulty.c_str(), g_rgszDifficultyNames[i]))
						{
							pProfile->m_bitsDifficulty |= (1 << i);
							break;
						}
					}
				}
			}

			else if (!_stricmp(szTempToken.c_str(), "Team"))
				pProfile->m_szTeam = *iter;

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else if (!_stricmp(szTempToken.c_str(), "Aggression"))
				pProfile->m_iAggression = std::stoi(*iter);

			else
				std::cout << "Error parsing " << hPath.string().c_str() << " - unknown attribute " << std::quoted(szTempToken) << " with value " << std::quoted(*iter) << '\n';
		}

	LAB_REGULAR_CONTINUE:;
		++iter;
		continue;

	LAB_ERASE_AND_NEXT:;
		iter = Tokens.erase(iter);
	}

	return true;
}

bool BotProfileMgr::Save(const fs::path& hPath) noexcept
{
	std::ofstream hFile(hPath, std::ios::out);
	if (!hFile)
	{
		std::cout << "File \"" << hPath << "\" cannot be created.\n";
		return false;
	}

	hFile << m_szSeparator << m_szHeader << m_szSeparator << std::endl;

	// Default.
	hFile << m_szSeparator << m_szDefaultTitle << m_szSeparator << std::endl;
	hFile << "Default" << std::endl;
	m_Default.SaveAttrib(hFile);
	hFile << "End\n" << std::endl;

	// Skins
	GenerateSkinsFromProfiles();
	hFile << m_szSeparator << m_szSkinTitle << m_szSeparator << std::endl;
	for (const auto& Skin : m_Skins)
	{
		hFile << "Skin " << Skin.first << std::endl;
		hFile << "\tModel = " << Skin.second << std::endl;
		hFile << "End\n" << std::endl;
	}

	// Templates
	hFile << m_szSeparator << m_szTemplateTitle << m_szSeparator << std::endl;
	for (auto& Template : m_Templates)
	{
		hFile << "Template " << Template.first << std::endl;
		Template.second.SaveAttrib(hFile);
		hFile << "End\n" << std::endl;
	}

	// Characters
	hFile << m_szSeparator << m_szCharacterTitle << m_szSeparator << std::endl;
	for (auto& Character : m_Profiles)
	{
		bool bFirst = true;
		for (const auto& szRef : Character.m_rgszRefTemplates)
		{
			if (bFirst)
				bFirst = false;
			else
				hFile << '+';

			hFile << szRef;
		}

		hFile << ' ' << Character.m_szName << std::endl;
		Character.SaveAttrib(hFile);
		hFile << "End\n" << std::endl;
	}

	hFile.close();
	return true;
}

void BotProfileMgr::LoadSkinThumbnails(void) noexcept
{
	fs::path hPath = g_GamePath.c_str() + L"\\gfx\\thumbnails\\skins"s;

	for (const auto& hEntry : fs::directory_iterator(hPath))
	{
		if (hEntry.is_directory())
			continue;

		Name_t szName = hEntry.path().filename().string();
		if (!szName.ends_with(".tga"))
			continue;

		szName.erase(szName.find(".tga"));
		ImageLoader::Add(hEntry.path(), &m_Thumbnails[szName]);
	}
}

size_t BotProfileMgr::TemplateNameCount(const std::string& szName) noexcept
{
	size_t iRet = 0;

	for (const auto& [sz, Template] : m_Templates)
	{
		if (!_stricmp(szName.c_str(), Template.m_szName.c_str()))
			++iRet;
	}

	return iRet;
}

size_t BotProfileMgr::ProfileNameCount(const std::string& szName) noexcept
{
	size_t iRet = 0;

	for (const auto& Character : m_Profiles)
	{
		if (!_stricmp(szName.c_str(), Character.Get<Reflection::BotProfile::NAME>()->c_str()))
			++iRet;
	}

	return iRet;
}
