/*
* This is the file describing the missionpack author.
* Nov 24 2021
*/

#include "Precompiled.hpp"

import UtlColor;
import UtlKeyValues;

inline ImColor ToImColor(const Color4b& c) noexcept
{
	return ImColor(c[0], c[1], c[2], c[3]);
}

inline ImColor ToImColor(const Color4f& c) noexcept
{
	return ImColor(c[0], c[1], c[2], c[3]);
}

static Color4b ToColor4b(const ImColor& c) noexcept
{
	Color4b obj;
	obj.SetColor(c.Value.x, c.Value.y, c.Value.z, 0.0);	// ImColor have different u32 color code with us.
	return obj;
}

void Overview::Parse(const fs::path& hPath) noexcept
{
	if (m_pKeyValue)
		delete m_pKeyValue;

	m_pKeyValue = new ValveKeyValues("");
	m_pKeyValue->LoadFromFile(hPath.string().c_str());

	m_szAuthor = m_pKeyValue->GetValue<std::string>("Author");
	m_szTitle = m_pKeyValue->GetValue<std::string>("Title");
	m_szDescription = m_pKeyValue->GetValue<std::string>("Description");
	m_szUrl = m_pKeyValue->GetValue<std::string>("URL");
	m_bSoloPlay = m_pKeyValue->GetValue<bool>("SoloPlay");
	m_bCoopPlay = m_pKeyValue->GetValue<bool>("CoopPlay");
	m_pszTeam = m_pKeyValue->GetValue<std::string>("Team") == "CT" ? "CT" : "T";
	m_BGColor1 = ToImColor(m_pKeyValue->GetValue<Color4b>("BGColor1"));
	m_BGColor2 = ToImColor(m_pKeyValue->GetValue<Color4b>("BGColor2"));
	m_TextColor = ToImColor(m_pKeyValue->GetValue<Color4b>("TextColor"));
	m_BotProfile = m_pKeyValue->GetValue<std::string>("BotProfile");

	// Update the profile location.
	MissionPack::Files[MissionPack::FILE_BOT_PROFILE] = CZFile::GetAbsPath(m_BotProfile);

	if (!CZFile::Exists(m_BotProfile))
		std::cout << "Warning: File '" << m_BotProfile << "' is missing from game folder.\n";
}

void Overview::Clear(void) noexcept
{
	m_szAuthor = "";
	m_szTitle = "";
	m_szDescription = "";
	m_szUrl = "";
	m_bSoloPlay = true;	// Normally should be solo... I guess?
	m_bCoopPlay = false;
	m_pszTeam = "CT";
	m_BGColor1 = ImColor(47, 62, 90);
	m_BGColor2 = ImColor(0, 0, 0);
	m_TextColor = ImColor(255, 255, 255);
	m_BotProfile = "";
}

void Overview::Save(const fs::path& hPath) noexcept
{
	if (!m_pKeyValue)
		m_pKeyValue = new ValveKeyValues("MissionPack");

	m_pKeyValue->SetValue("Author", m_szAuthor);
	m_pKeyValue->SetValue("Title", m_szTitle);
	m_pKeyValue->SetValue("Description", m_szDescription);
	m_pKeyValue->SetValue("URL", m_szUrl);
	m_pKeyValue->SetValue("SoloPlay", m_bSoloPlay);
	m_pKeyValue->SetValue("CoopPlay", m_bCoopPlay);
	m_pKeyValue->SetValue("Team", m_pszTeam);
	m_pKeyValue->SetValue("BGColor1", ToColor4b(m_BGColor1));
	m_pKeyValue->SetValue("BGColor2", ToColor4b(m_BGColor2));
	m_pKeyValue->SetValue("TextColor", ToColor4b(m_TextColor));
	m_pKeyValue->SetValue("BotProfile", m_BotProfile);

	m_pKeyValue->SaveToFile(hPath.string().c_str());
}
