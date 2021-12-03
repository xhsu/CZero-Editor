/*
* A simulator of GoldSrc engine file system.
* Nov 08 2021
*/

#include "Precompiled.hpp"

import UtlString;

bool CZFile::Update(void) noexcept
{
	if (!fs::exists(m_HLPath))
	{
		m_HLPath = g_GamePath.parent_path();

		if (!fs::exists(m_HLPath))
			return false;
	}

	m_Directories.clear();

	for (const auto& hEntry : fs::directory_iterator(m_HLPath))
	{
		if (!hEntry.is_directory())
			continue;

		auto szDirName = hEntry.path().filename().string();
		if (!szDirName.starts_with("valve"/*Half Life original game*/) && !szDirName.starts_with("cstrike") && !szDirName.starts_with("czero"))
			continue;

		if (szDirName.starts_with("czeror")/*CZ: Deleted Scenes*/)
			continue;

		m_Directories.emplace_back(hEntry.path());
	}

	m_Directories.sort(
		[](const fs::path& lhs, const fs::path& rhs) -> bool	// Returns true when lhs should be placed before rhs.
		{
			auto szLhsDir = lhs.filename().string();
			auto szRhsDir = rhs.filename().string();

			auto fnDirCompare = [&]<StringLiteral _szDirBaseName>(void) -> bool
			{
				using T = decltype(_szDirBaseName);

				if (szLhsDir.length() != szRhsDir.length())
					return szLhsDir.length() > szRhsDir.length();

				if (_szDirBaseName == szLhsDir)	// Place original dir at the last.
					return false;

				if (szRhsDir == _szDirBaseName)	// Place original dir at the last.
					return true;

				return szLhsDir[T::length + 1] < szRhsDir[T::length + 1];	// And place the localisation dirs before. (+1 for skipping the '_' between mod name and language name.)
			};

			if (szLhsDir.starts_with("czero") && szRhsDir.starts_with("czero"))
				return fnDirCompare.template operator() < "czero" > ();	// #POTENTIAL_BUG	What the fuck, C++20? I think you are supporting templated lambda in a not-so-ugly way!

			else if (szLhsDir.starts_with("valve") && szRhsDir.starts_with("valve"))
				return fnDirCompare.template operator() < "valve" > ();

			else if (szLhsDir.starts_with("cstrike") && szRhsDir.starts_with("cstrike"))
				return fnDirCompare.template operator() < "cstrike" > ();

			// CS and CZ always place before HL.
			else if (szLhsDir[0] == 'c' && szRhsDir[0] == 'v')
				return true;
			else if (szLhsDir[0] == 'c' && szRhsDir[0] == 'v')
				return false;

			// CZ should place before CS.
			else if (szLhsDir[1] == 'z' && szRhsDir[1] == 's')
				return true;
			else
				return false;	// Have to return something afterall.
		}
	);

	return !m_Directories.empty();
}

bool CZFile::Exists(const std::string& szPath) noexcept
{
	for (const auto& Directory : m_Directories)
	{
		if (fs::exists(Directory.string() + '\\' + szPath))
			return true;
	}

	return false;
}

[[nodiscard]]
FILE* CZFile::Open(const char* pszPath, const char* pszMode) noexcept	// #RET_FOPEN
{
	for (const auto& Directory : m_Directories)
	{
		if (!fs::exists(Directory.string() + std::string("\\") + pszPath))
			continue;

		FILE* f = nullptr;
		fopen_s(&f, pszPath, pszMode);
		return f;
	}

	return nullptr;
}

std::string CZFile::GetAbsPath(const std::string& szPath) noexcept
{
	for (const auto& Directory : m_Directories)
	{
		auto sz = Directory.string() + '\\' + szPath;

		if (fs::exists(sz))
			return sz;
	}

	return "";
}

std::string CZFile::GetRelativePath(const std::string& szPath) noexcept
{
	const char* p = szPath.data();
	const char* pEnd = p + szPath.length();

	for (; p && p != pEnd; ++p)
	{
		if (!_strnicmp(p, "valve", 5) || !_strnicmp(p, "cstrike", 7) || !_strnicmp(p, "czero", 5))
			break;
	}

	if (!p || p == pEnd || *p == '\0')
		return "";

	size_t iPos = p - szPath.data();	// This thing must be positive.
	iPos = szPath.find_first_of("\\/", iPos);

	if (iPos == std::string::npos || (iPos + 1) > szPath.length())
		return "";

	auto ret = szPath;
	ret.erase(0, iPos + 1);	// '/' included.

	for (auto& c : ret)
	{
		if (c == '\\')
			c = '/';
	}

	return ret;
}
