// CZeroEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// C++
#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <mutex>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

// C
#include <cstddef>
#include <cstdio>

// ImGUI
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"

// OpenGL
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// custom specialization of std::hash can be injected in namespace std.
// Must be done before declaring its ANY usage.
namespace std
{
	template<> struct hash<filesystem::path>
	{
		std::size_t operator()(filesystem::path const& obj) const noexcept
		{
			return std::hash<std::wstring>{}(obj.c_str());
		}
	};
}

import UtlKeyValues;
import UtlString;
import UtlImage;

namespace fs = std::filesystem;

using namespace std::literals::string_literals;
using namespace std::literals;
using namespace std::string_literals;

extern std::list<std::string> BSP_CompileResourceList(const char* pszBspPath);

template<class... Ts>
concept all_the_same = sizeof...(Ts) < 2 || std::conjunction_v<std::is_same<std::tuple_element_t<0, std::tuple<Ts...>>, Ts>...>;




#define IMGUI_GREEN		ImVec4(102.0f / 255.0f, 204.0f / 255.0f, 102.0f / 255.0f, 1)
#define IMGUI_YELLOW	ImVec4(1, 204.0f / 255.0f, 102.0f / 255.0f, 1)
#define IMGUI_RED		ImVec4(204.0f / 255.0f, 0, 0, 1)

#define CAST_TO_CHAR	reinterpret_cast<const char*>
#define CAST_TO_UTF8	reinterpret_cast<const char8_t*>
#define CAST_TO_U8S(x)	*reinterpret_cast<std::u8string*>(&x)
#define CAST_TO_STR(x)	*reinterpret_cast<std::string*>(&x)

#define GL_CLAMP_TO_EDGE 0x812F 


#pragma region Objects
// Dummy
enum class Difficulty_e : unsigned __int8;
struct BotProfile_t;
struct CareerGame_t;
struct Locus_t;
struct Map_t;
struct Task_t;
struct Thumbnail_t;

using BYTE = unsigned __int8;
using BotProfiles_t = std::list<BotProfile_t>;
using CareerGames_t = std::unordered_map<Difficulty_e, CareerGame_t>;
using ConsoleCmd_t = std::string;
using CostAvailability_t = std::array<int, 5>;
using Directories_t = std::list<fs::path>;
using KeyValueSet_t = std::unordered_map<Difficulty_e, NewKeyValues*>;
using Loci_t = std::list<Locus_t>;
using Maps_t = std::unordered_map<std::string, Map_t>;
using Name_t = std::string;
using Names_t = std::list<Name_t>;
using Resources_t = std::list<std::string>;
using SkinThumbnails_t = std::unordered_map<std::string, Thumbnail_t>;
using Tasks_t = std::list<Task_t>;
using Weapons_t = std::vector<std::string>;

const char* g_rgszTaskNames[] =
{
	"defuse",
	"plant",
	"killall",
	"kill",
	"killwith",
	"killblind",
	"killvip",
	"headshot",
	"headshotwith",
	"winfast",
	"rescue",
	"rescueall",
	"injure",
	"injurewith",
	"killdefuser",
	"stoprescue",
	"defendhostages",
	"hostagessurvive",
	"preventdefuse",
};

const char* g_rgszWeaponNames[] =
{
	"pistol",
	"glock",
	"usp",
	"p228",
	"deagle",
	"fn57",
	"elites",

	"shotgun",
	"m3",
	"xm1014",

	"SMG",
	"tmp",
	"mac10",
	"mp5",
	"ump45",
	"p90",

	"rifle",
	"galil",
	"ak47",
	"famas",
	"m4a1",
	"aug",
	"sg552",

	"sniper",
	"scout",
	"sg550",
	"awp",
	"g3sg1",

	"machinegun",
	"m249",
	
	"Equipments",	// Pure lable, no usage.
	"knife",
	"shield",
	"grenade",	// singular
	"hegren",	// plural
	"flashbang",
	"smokegrenade",
};

using WeaponSelMask_t = std::array<bool, _countof(g_rgszWeaponNames)>;

constexpr WeaponSelMask_t g_rgbIsBuyCommand =
{
	false,	// pistol
	true,
	true,
	true,
	true,
	true,
	true,

	false,	// shotgun
	true,
	true,

	false,	// smg
	true,
	true,
	true,
	true,
	true,

	false,	// rifle
	true,
	true,
	true,
	true,
	true,
	true,

	false,	// sniper
	true,
	true,
	true,
	true,

	false,
	true,	// m249

	false,	// "Equipment"
	false,	// knife
	true,	// shield
	false,	// grenade (singular)
	true,	// hegren (plural)
	true,	// FB
	true,	// SG
};

constexpr WeaponSelMask_t g_rgbIsTaskWeapon =
{
	true,	// pistol
	true,
	true,
	true,
	true,
	true,
	true,

	true,	// shotgun
	true,
	true,

	true,	// smg
	true,
	true,
	true,
	true,
	true,

	true,	// rifle
	true,
	true,
	true,
	true,
	true,
	true,

	true,	// sniper
	true,
	true,
	true,
	true,

	true,
	true,	// m249

	false,	// "Equipment"
	true,	// knife
	true,	// shield
	true,	// grenade (singular)
	true,	// hegren (plural)
	false,	// FB
	false,	// SG
};

const char* g_rgszDifficultyNames[] = { "EASY", "NORMAL", "HARD", "EXPERT" };
const wchar_t* g_rgwcsDifficultyNames[] = { L"Easy", L"Normal", L"Hard", L"Expert" };
const char* g_rgszCTSkinName[] = { nullptr, "urban", "gsg9", "sas", "gign", "spetsnaz", "ct_random" };
const char* g_rgszTSkinName[] = { nullptr, "terror", "leet", "arctic", "guerilla", "militia", "t_random" };

extern inline fs::path	g_GamePath;

enum class TaskType_e : BYTE
{
	defuse,
	plant,
	killall,
	kill,
	killwith,
	killblind,
	killvip,
	headshot,
	headshotwith,
	winfast,
	rescue,
	rescueall,
	injure,
	injurewith,
	killdefuser,
	stoprescue,
	defendhostages,
	hostagessurvive,
	preventdefuse,

	_LAST,
};

enum Async_e : int
{
	UNKNOWN = 0,
	RENDERING = 1 << 0,

	//REQ_UPDATE_GAME_INFO		= 1 << 1,
	//UPDATING_GAME_INFO			= 1 << 2,
	//GAME_INFO_READY				= 1 << 3,

	//REQ_UPDATE_MISSION_PACK		= 1 << 4,
	UPDATING_MISSION_PACK_INFO	= 1 << 5,
	//MISSION_PACK_READY			= 1 << 6,

	UPDATING_MAPS_INFO			= 1 << 7,

	_LAST = 1 << 30,
};

enum class Difficulty_e : BYTE
{
	EASY = 0,
	NORMAL,
	HARD,
	EXPERT,

	_LAST
};

enum Weapon_e
{
	pistol,
	glock,
	usp,
	p228,
	deagle,
	fn57,
	elites,

	shotgun,
	m3,
	xm1014,

	SMG,
	tmp,
	mac10,
	mp5,
	ump45,
	p90,

	rifle,
	galil,
	ak47,
	famas,
	m4a1,
	aug,
	sg552,

	sniper,
	scout,
	sg550,
	awp,
	g3sg1,

	machinegun,
	m249,

	knife,
	shield,
	grenade,
	hegren,
	flashbang,
	smokegrenade,

	_WEAPON_COUNT
};

#pragma region ScopedEnumOperator
template<typename E>
concept GoodEnum = requires(E e)
{
	{E::_LAST};
	requires !std::is_convertible_v<E, std::underlying_type_t<E>>;	// replacement of C++23 std::is_scpoed_enum<>
	requires std::is_enum_v<E>;
};

template<GoodEnum Enumerator_t>
constexpr Enumerator_t& operator++(Enumerator_t& i)
{
	int val = (int)i;
	val++;
	
	if (val > (int)Enumerator_t::_LAST)
		val = (int)Enumerator_t::_LAST;	// Marker for passing valid position.

	return i = (Enumerator_t)val;
}

template<GoodEnum Enumerator_t, typename U>
constexpr std::strong_ordering operator<=> (const Enumerator_t& lhs, const U& rhs)
{
	if constexpr (std::is_enum_v<U>)	// std::conditional will evaluate the 'false' branch no matter what. In this case, std::underlying_type will cause a ill-form.
	{
		using T = std::underlying_type_t<U>;
		return static_cast<T>(lhs) <=> static_cast<T>(rhs);
	}
	else
	{
		using T = std::decay_t<U>;
		return static_cast<T>(lhs) <=> rhs;
	}
}

template<GoodEnum Enumerator_t>
constexpr Enumerator_t operator| (const Enumerator_t& lhs, const Enumerator_t& rhs) { return (Enumerator_t)(int(lhs) | int(rhs)); }

template<GoodEnum Enumerator_t>
constexpr Enumerator_t& operator|= (Enumerator_t& lhs, const Enumerator_t& rhs) { return lhs = (Enumerator_t)(int(lhs) | int(rhs)); }

template<GoodEnum Enumerator_t>
constexpr Enumerator_t operator& (const Enumerator_t& lhs, const Enumerator_t& rhs) { return (Enumerator_t)(int(lhs) & int(rhs)); }

template<GoodEnum Enumerator_t>
constexpr Enumerator_t& operator&= (Enumerator_t& lhs, const Enumerator_t& rhs) { return lhs = (Enumerator_t)(int(lhs) & int(rhs)); }

template<GoodEnum Enumerator_t>
constexpr Enumerator_t operator~ (const Enumerator_t& lhs) { return (Enumerator_t)~static_cast<int>(lhs); }

template<GoodEnum Enumerator_t>
constexpr bool operator! (const Enumerator_t& lhs) { return int(lhs) == 0; }

template<GoodEnum Enumerator_t, typename U>
constexpr decltype(auto) operator+ (const Enumerator_t& lhs, const U& rhs)
{
	if constexpr (std::is_enum_v<U>)	// std::conditional will evaluate the 'false' branch no matter what. In this case, std::underlying_type will cause a ill-form.
	{
		using T = std::underlying_type_t<U>;
		return static_cast<T>(lhs) + static_cast<T>(rhs);
	}
	else
	{
		using T = std::decay_t<U>;
		return static_cast<T>(lhs) + rhs;
	}
}

template<GoodEnum Enumerator_t, typename U>
constexpr decltype(auto) operator- (const Enumerator_t& lhs, const U& rhs)
{
	if constexpr (std::is_enum_v<U>)	// std::conditional will evaluate the 'false' branch no matter what. In this case, std::underlying_type will cause a ill-form.
	{
		using T = std::underlying_type_t<U>;
		return static_cast<T>(lhs) - static_cast<T>(rhs);
	}
	else
	{
		using T = std::decay_t<U>;
		return static_cast<T>(lhs) - rhs;
	}
}

template<GoodEnum Enumerator_t, std::integral U>
constexpr decltype(auto) operator<< (const U& lhs, const Enumerator_t& rhs) { return lhs << static_cast<std::decay_t<U>>(rhs); }

#pragma endregion ScopedEnumOperator

namespace Tasks
{
	constexpr auto REQ_COUNT = ~((1 << TaskType_e::killall) | (1 << TaskType_e::rescueall));
	constexpr auto REQ_WEAPON = (1 << TaskType_e::injurewith) | (1 << TaskType_e::killwith) | (1 << TaskType_e::headshotwith);
	constexpr auto SURVIVE = ~((1 << TaskType_e::killall) | (1 << TaskType_e::rescueall) | (1 << TaskType_e::defendhostages) | (1 << TaskType_e::hostagessurvive));
	constexpr auto INAROW = ~((1 << TaskType_e::killall) | (1 << TaskType_e::winfast) | (1 << TaskType_e::rescueall) | (1 << TaskType_e::defendhostages) | (1 << TaskType_e::hostagessurvive));
};

struct Task_t
{
	Task_t() noexcept {}
	Task_t(const std::string& sz) noexcept { Parse(sz); }
	Task_t(const Task_t& rhs) noexcept :
		m_iType(rhs.m_iType),
		m_iCount(rhs.m_iCount),
		m_szWeapon(rhs.m_szWeapon),
		m_bSurvive(rhs.m_bSurvive),
		m_bInARow(rhs.m_bInARow) {}
	Task_t& operator= (const Task_t& rhs) noexcept
	{
		m_iType = rhs.m_iType;
		m_iCount = rhs.m_iCount;
		m_szWeapon = rhs.m_szWeapon;
		m_bSurvive = rhs.m_bSurvive;
		m_bInARow = rhs.m_bInARow;
		return *this;
	}
	Task_t(Task_t&& rhs) noexcept :
		m_iType(rhs.m_iType),
		m_iCount(rhs.m_iCount),
		m_szWeapon(std::move(rhs.m_szWeapon)),
		m_bSurvive(rhs.m_bSurvive),
		m_bInARow(rhs.m_bInARow) {}
	Task_t& operator= (Task_t&& rhs) noexcept
	{
		m_iType = rhs.m_iType;
		m_iCount = rhs.m_iCount;
		m_szWeapon = std::move(rhs.m_szWeapon);
		m_bSurvive = rhs.m_bSurvive;
		m_bInARow = rhs.m_bInARow;
		return *this;
	}
	virtual ~Task_t() {}

	void Parse(const std::string& sz) noexcept
	{
		std::vector<std::string> rgszTokens;
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

	std::string ToString(void) const noexcept
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

	// Modify and give reason for ill-formed.
	const char* SanityCheck(void) noexcept
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

	TaskType_e m_iType{ TaskType_e::kill };
	int m_iCount{ 0 };
	Name_t m_szWeapon{ "" };
	bool m_bSurvive{ false };
	bool m_bInARow{ false };
};

struct Thumbnail_t : public Image_t
{
	constexpr Thumbnail_t(void) noexcept {}
	Thumbnail_t(Thumbnail_t&& rhs) noexcept
	{
		m_iTexId = rhs.m_iTexId;
		m_iWidth = rhs.m_iWidth;
		m_iHeight = rhs.m_iHeight;
	}
	Thumbnail_t& operator=(Thumbnail_t&& rhs) noexcept
	{
		m_iTexId = rhs.m_iTexId;
		m_iWidth = rhs.m_iWidth;
		m_iHeight = rhs.m_iHeight;
		return *this;
	}
	Thumbnail_t(const Thumbnail_t& rhs) noexcept
	{
		m_iTexId = rhs.m_iTexId;
		m_iWidth = rhs.m_iWidth;
		m_iHeight = rhs.m_iHeight;
	}
	Thumbnail_t& operator=(const Thumbnail_t& rhs) noexcept
	{
		m_iTexId = rhs.m_iTexId;
		m_iWidth = rhs.m_iWidth;
		m_iHeight = rhs.m_iHeight;
		return *this;
	}

	inline ImVec2	Size() const noexcept { return ImVec2((float)m_iWidth, (float)m_iHeight); }
};

namespace HalfLifeFileStructure	// None of these function requires "\\" before relative path (to the game dir).
{
	inline fs::path			m_HLPath;
	inline Directories_t	m_Directories;

	bool Update(void) noexcept
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
					return fnDirCompare.template operator() <"czero">();	// #POTENTIAL_BUG	What the fuck, C++20? I think you are supporting templated lambda in a not-so-ugly way!

				else if (szLhsDir.starts_with("valve") && szRhsDir.starts_with("valve"))
					return fnDirCompare.template operator() <"valve">();

				else if (szLhsDir.starts_with("cstrike") && szRhsDir.starts_with("cstrike"))
					return fnDirCompare.template operator() <"cstrike">();

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

	bool Exists(const std::string& szPath) noexcept
	{
		for (const auto& Directory : m_Directories)
		{
			if (fs::exists(Directory.string() + '\\' + szPath))
				return true;
		}

		return false;
	}

	[[nodiscard]]
	FILE* Open(const char* pszPath, const char* pszMode) noexcept	// #RET_FOPEN
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
};

struct Map_t
{
	Map_t() noexcept {}
	virtual ~Map_t() noexcept {}

	void Initialize(const fs::path& hPath) noexcept
	{
		m_Path = hPath;
		m_szName = hPath.filename().string();
		m_szName.erase(m_szName.find('.'));	// Remove the ext name.

		CheckBasicFile();

		m_rgszResources = BSP_CompileResourceList(m_Path.string().c_str());
	}

	void CheckBasicFile(void) noexcept
	{
		m_bBriefFileExists = HalfLifeFileStructure::Exists("maps\\" + m_szName + ".txt"s);
		m_bDetailFileExists = HalfLifeFileStructure::Exists("maps\\" + m_szName + "_detail.txt"s);
		m_bNavFileExists = HalfLifeFileStructure::Exists("maps\\" + m_szName + ".nav"s);
		m_bOverviewFileExists = HalfLifeFileStructure::Exists("overviews\\" + m_szName + ".bmp"s) && HalfLifeFileStructure::Exists("overviews\\" + m_szName + ".txt"s);	// You can't miss either or them!
		m_bThumbnailExists = HalfLifeFileStructure::Exists("gfx\\thumbnails\\maps\\" + m_szName + ".tga"s);
		m_bWiderPreviewExists = HalfLifeFileStructure::Exists("gfx\\thumbnails\\maps_wide\\" + m_szName + ".tga"s);

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

			if (FILE* f = HalfLifeFileStructure::Open(("maps\\" + m_szName + ".txt"s).c_str(), "rb"); f != nullptr)
			{
				fseek(f, 0, SEEK_END);
				auto iSize = ftell(f);

				m_pszBriefString = (char*)malloc(iSize + 1);
				fseek(f, 0, SEEK_SET);
				fread_s(m_pszBriefString, iSize + 1, iSize, 1, f);
				m_pszBriefString[iSize] = '\0';

				fclose(f);
			}
		}
	}

	Name_t			m_szName{ "Error - no name of this .bsp file." };	// .bsp ext name not included.
	Name_t			m_szSky{ "Error - no sky texture." };
	Resources_t		m_rgszResources{};
	Thumbnail_t		m_Thumbnail{};
	Thumbnail_t		m_WiderPreview{};
	bool			m_bBriefFileExists{ false };	// The text to be shown in MOTD screen.
	bool			m_bDetailFileExists{ false };	// texture detail file.
	bool			m_bNavFileExists{ false };	// CZ bot nav mesh
	bool			m_bOverviewFileExists{ false };
	bool			m_bThumbnailExists{ false };	// Thumbnail image when selecting location.
	bool			m_bWiderPreviewExists{ false };	// Preview image in preparation screen. (selecting teammates)
	char*			m_pszBriefString{ nullptr };
	fs::path		m_Path;
};

struct Locus_t
{
	Locus_t() noexcept {}
	Locus_t(NewKeyValues* pkv) noexcept { Parse(pkv); }
	Locus_t(const Locus_t& rhs) noexcept :
		m_szMap(rhs.m_szMap),
		m_rgszBots(rhs.m_rgszBots),
		m_iMinEnemies(rhs.m_iMinEnemies),
		m_iThreshold(rhs.m_iThreshold),
		m_Tasks(rhs.m_Tasks),
		m_bFriendlyFire(rhs.m_bFriendlyFire),
		m_szConsoleCommands(rhs.m_szConsoleCommands)
	{
	}
	Locus_t& operator=(const Locus_t& rhs) noexcept
	{
		m_szMap = rhs.m_szMap;
		m_rgszBots = rhs.m_rgszBots;
		m_iMinEnemies = rhs.m_iMinEnemies;
		m_iThreshold = rhs.m_iThreshold;
		m_Tasks = rhs.m_Tasks;
		m_bFriendlyFire = rhs.m_bFriendlyFire;
		m_szConsoleCommands = rhs.m_szConsoleCommands;
		return *this;
	}
	Locus_t(Locus_t&& rhs) noexcept :
		m_szMap(std::move(rhs.m_szMap)),
		m_rgszBots(std::move(rhs.m_rgszBots)),
		m_iMinEnemies(rhs.m_iMinEnemies),
		m_iThreshold(rhs.m_iThreshold),
		m_Tasks(std::move(rhs.m_Tasks)),
		m_bFriendlyFire(rhs.m_bFriendlyFire),
		m_szConsoleCommands(std::move(rhs.m_szConsoleCommands))
	{
	}
	Locus_t& operator=(Locus_t&& rhs) noexcept
	{
		m_szMap = std::move(rhs.m_szMap);
		m_rgszBots = std::move(rhs.m_rgszBots);
		m_iMinEnemies = rhs.m_iMinEnemies;
		m_iThreshold = rhs.m_iThreshold;
		m_Tasks = std::move(rhs.m_Tasks);
		m_bFriendlyFire = rhs.m_bFriendlyFire;
		m_szConsoleCommands = std::move(rhs.m_szConsoleCommands);
		return *this;
	}
	virtual ~Locus_t() noexcept {}

	void Parse(NewKeyValues* pkv)
	{
		m_rgszBots.clear();
		m_Tasks.clear();

		m_szMap = pkv->GetName();

		NewKeyValues* pSub = pkv->FindEntry("bots");
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
	NewKeyValues* Save(void) const	// #RET_HEAP_MEM
	{
		auto pkv = new NewKeyValues(m_szMap.c_str());

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

	Name_t			m_szMap{ ""s };
	Names_t			m_rgszBots{};
	int				m_iMinEnemies{ 3 };
	int				m_iThreshold{ 2 };
	Tasks_t			m_Tasks{};
	bool			m_bFriendlyFire{ false };
	ConsoleCmd_t	m_szConsoleCommands{ ""s };
};

struct CareerGame_t
{
	void Parse(NewKeyValues* pkv)
	{
		Reset();

		NewKeyValues* pSub = pkv->FindEntry("InitialPoints");
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
				NewKeyValues* p = nullptr;
				(((p = pSub->FindEntry(UTIL_IntToString<I + 1>())) != nullptr ? (m_rgiCostAvailability[I] = p->GetValue<int>()) : (int{})), ...);
			}
			(std::make_index_sequence<5>{});
		}

		if ((pSub = pkv->FindEntry("Maps")) != nullptr)
		{
			NewKeyValues* p = pSub->GetFirstSubkey();

			while (p != nullptr)
			{
				m_Loci.emplace_back(p);
				p = p->GetNextSubkey();
			}
		}
	}

	void Reset(void)
	{
		m_iInitialPoints = 2;
		m_iMatchWins = 3;
		m_iMatchWinBy = 2;
		m_rgszCharacters.clear();
		m_rgiCostAvailability = { 1, 6, 10, 15, 99 };
		m_Loci.clear();
	}

	[[nodiscard]]
	NewKeyValues* Save(void) const	// #RET_HEAP_MEM
	{
		auto pkv = new NewKeyValues("CareerGame");

		pkv->SetValue("InitialPoints", m_iInitialPoints);
		pkv->SetValue("MatchWins", m_iMatchWins);
		pkv->SetValue("MatchWinBy", m_iMatchWinBy);

		Name_t szAllNames;
		for (const auto& szName : m_rgszCharacters)
			szAllNames += szName + ' ';
		szAllNames.pop_back();	// Remove last space.
		pkv->SetValue("Characters", szAllNames);

		auto p = pkv->CreateEntry("CostAvailability");
		for (int i = 0; i < 5; i++)
			p->SetValue(std::to_string(i + 1).c_str(), m_rgiCostAvailability[i]);

		p = pkv->CreateEntry("Maps");
		for (const auto& Locus : m_Loci)
			p->AddEntry(Locus.Save());

		return pkv;
	}

	int m_iInitialPoints{ 2 };
	int m_iMatchWins{ 3 };
	int m_iMatchWinBy{ 2 };
	Names_t m_rgszCharacters{};
	CostAvailability_t m_rgiCostAvailability{ 1, 6, 10, 15, 99 };
	Loci_t m_Loci;
};

namespace BotProfileMgr
{
	extern inline BotProfiles_t m_Profiles;
	extern inline BotProfile_t m_Default;
	inline std::unordered_map<std::string, BotProfile_t> m_Templates{};
	inline std::unordered_map<std::string, std::string> m_Skins{};
};

struct BotProfile_t
{
	BotProfile_t(void) noexcept {}
	BotProfile_t(const BotProfile_t& rhs) noexcept :
		m_szName(rhs.m_szName),
		m_rgszWpnPreference(rhs.m_rgszWpnPreference),
		m_bPrefersSilencer(rhs.m_bPrefersSilencer),
		m_bExplicitlyNoneWpnPref(rhs.m_bExplicitlyNoneWpnPref),
		m_flAttackDelay(rhs.m_flAttackDelay),
		m_flReactionTime(rhs.m_flReactionTime),
		m_bitsDifficulty(rhs.m_bitsDifficulty),
		m_iAggression(rhs.m_iAggression),
		m_iCost(rhs.m_iCost),
		m_iSkill(rhs.m_iSkill),
		m_iTeamwork(rhs.m_iTeamwork),
		m_iVoicePitch(rhs.m_iVoicePitch),
		m_rgszRefTemplates(rhs.m_rgszRefTemplates),
		m_szSkin(rhs.m_szSkin),
		m_szTeam(rhs.m_szTeam),
		m_szVoiceBank(rhs.m_szVoiceBank) {}
	BotProfile_t& operator=(const BotProfile_t& rhs) noexcept
	{
		m_szName = rhs.m_szName;
		m_rgszWpnPreference = rhs.m_rgszWpnPreference;
		m_bPrefersSilencer = rhs.m_bPrefersSilencer;
		m_bExplicitlyNoneWpnPref = rhs.m_bExplicitlyNoneWpnPref;
		m_flAttackDelay = rhs.m_flAttackDelay;
		m_flReactionTime = rhs.m_flReactionTime;
		m_bitsDifficulty = rhs.m_bitsDifficulty;
		m_iAggression = rhs.m_iAggression;
		m_iCost = rhs.m_iCost;
		m_iSkill = rhs.m_iSkill;
		m_iTeamwork = rhs.m_iTeamwork;
		m_iVoicePitch = rhs.m_iVoicePitch;
		m_rgszRefTemplates = rhs.m_rgszRefTemplates;
		m_szSkin = rhs.m_szSkin;
		m_szTeam = rhs.m_szTeam;
		m_szVoiceBank = rhs.m_szVoiceBank;
		return *this;
	}
	BotProfile_t(BotProfile_t&& rhs) noexcept :
		m_szName(std::move(rhs.m_szName)),
		m_rgszWpnPreference(std::move(rhs.m_rgszWpnPreference)),
		m_bPrefersSilencer(rhs.m_bPrefersSilencer),
		m_bExplicitlyNoneWpnPref(rhs.m_bExplicitlyNoneWpnPref),
		m_flAttackDelay(rhs.m_flAttackDelay),
		m_flReactionTime(rhs.m_flReactionTime),
		m_bitsDifficulty(rhs.m_bitsDifficulty),
		m_iAggression(rhs.m_iAggression),
		m_iCost(rhs.m_iCost),
		m_iSkill(rhs.m_iSkill),
		m_iTeamwork(rhs.m_iTeamwork),
		m_iVoicePitch(rhs.m_iVoicePitch),
		m_rgszRefTemplates(std::move(rhs.m_rgszRefTemplates)),
		m_szSkin(std::move(rhs.m_szSkin)),
		m_szTeam(std::move(rhs.m_szTeam)),
		m_szVoiceBank(std::move(rhs.m_szVoiceBank)) {}
	BotProfile_t& operator=(BotProfile_t&& rhs) noexcept
	{
		m_szName = std::move(rhs.m_szName);
		m_rgszWpnPreference = std::move(rhs.m_rgszWpnPreference);
		m_bPrefersSilencer = rhs.m_bPrefersSilencer;
		m_bExplicitlyNoneWpnPref = rhs.m_bExplicitlyNoneWpnPref;
		m_flAttackDelay = rhs.m_flAttackDelay;
		m_flReactionTime = rhs.m_flReactionTime;
		m_bitsDifficulty = rhs.m_bitsDifficulty;
		m_iAggression = rhs.m_iAggression;
		m_iCost = rhs.m_iCost;
		m_iSkill = rhs.m_iSkill;
		m_iTeamwork = rhs.m_iTeamwork;
		m_iVoicePitch = rhs.m_iVoicePitch;
		m_rgszRefTemplates = std::move(rhs.m_rgszRefTemplates);
		m_szSkin = std::move(rhs.m_szSkin);
		m_szTeam = std::move(rhs.m_szTeam);
		m_szVoiceBank = std::move(rhs.m_szVoiceBank);
		return *this;
	}
	virtual ~BotProfile_t() {}

	bool SaveAttrib(std::ofstream& hFile) const
	{
		if (m_bExplicitlyNoneWpnPref)
			hFile << "\tWeaponPreference = none" << std::endl;
		else
		{
			for (const auto& szWeapon : m_rgszWpnPreference)
			{
				if (!m_rgszRefTemplates.empty())
				{
					BotProfile_t& Ref = BotProfileMgr::m_Templates[m_rgszRefTemplates.back()];	// #PENDING_ON_REVIEW this should never happen. If there is a weapon template, one should never customize his weapon preference.

					for (const auto& szRefWpn : Ref.m_rgszWpnPreference)
					{
						if (szRefWpn == szWeapon)
							goto LAB_SKIP_SAVING;
					}
				}

				hFile << "\tWeaponPreference = " << szWeapon << std::endl;

			LAB_SKIP_SAVING:;
			}
		}

#define SAVE_MEMBER_HELPER_INT(x)	if (m_i##x >= 0 && !IsMemberInherited<int>(offsetof(BotProfile_t, m_i##x)))	\
										hFile << "\t" #x " = " << m_i##x << std::endl
#define SAVE_MEMBER_HELPER_FLT(x)	if (m_fl##x >= 0 && !IsMemberInherited<float>(offsetof(BotProfile_t, m_fl##x)))	\
										hFile << "\t" #x " = " << m_fl##x << std::endl
#define SAVE_MEMBER_HELPER_STR(x)	if (!m_sz##x.empty() && !m_sz##x.starts_with("Error - "s) && !IsMemberInherited<std::string>(offsetof(BotProfile_t, m_sz##x)))	\
										hFile << "\t" #x " = " << m_sz##x << std::endl

		SAVE_MEMBER_HELPER_FLT(AttackDelay);
		SAVE_MEMBER_HELPER_FLT(ReactionTime);

		if (m_bitsDifficulty > 0 && !IsMemberInherited(offsetof(BotProfile_t, m_bitsDifficulty)))
		{
			bool bFirst = true;

			hFile << "\tDifficulty = ";

			for (size_t i = 0; i < _countof(g_rgszDifficultyNames); i++)
			{
				if (m_bitsDifficulty & (1 << i))
				{
					if (bFirst)
						bFirst = false;
					else
						hFile << '+';

					hFile << g_rgszDifficultyNames[i];
				}
			}

			hFile << std::endl;
		}

		if (m_szName == "Cole")
		{
			if (IsMemberInherited<int>(offsetof(BotProfile_t, m_iTeamwork)))
			{
				std::roundf(1);
			}
		}

		SAVE_MEMBER_HELPER_INT(Aggression);
		SAVE_MEMBER_HELPER_INT(Cost);
		SAVE_MEMBER_HELPER_INT(Skill);
		SAVE_MEMBER_HELPER_INT(Teamwork);
		SAVE_MEMBER_HELPER_INT(VoicePitch);

		SAVE_MEMBER_HELPER_STR(Skin);
		SAVE_MEMBER_HELPER_STR(Team);
		SAVE_MEMBER_HELPER_STR(VoiceBank);

		return true;

#undef SAVE_MEMBER_HELPER_INT
#undef SAVE_MEMBER_HELPER_FLT
#undef SAVE_MEMBER_HELPER_STR
	}

	template<typename T = int>
	bool IsMemberInherited(size_t iOffset) const noexcept	// #HACKHACK undefined behavior involved.
	{
		if (this == &BotProfileMgr::m_Default)
			return false;

		bool bTemplateFound = false;
		T* pMyValue = reinterpret_cast<T*>(size_t(this) + iOffset);
		T* pRefValue = nullptr;

		for (auto iter = m_rgszRefTemplates.rbegin(); iter != m_rgszRefTemplates.rend(); iter++)
		{
			BotProfile_t* pRef = &BotProfileMgr::m_Templates[*iter];
			pRefValue = reinterpret_cast<T*>(size_t(pRef) + iOffset);

			if constexpr (std::is_same_v<T, std::string>)	// Does my reference has no value for themself?
			{
				if (pRefValue->empty() || pRefValue->starts_with("Error - "s))
					continue;
				else
				{
					bTemplateFound = true;
					break;	// Check the latest used template. If it is a valid, skip all the others including m_Default.
				}
			}
			else
			{
				if (*pRefValue < 0 || (!*pRefValue && iOffset == offsetof(BotProfile_t, m_bitsDifficulty)))
					continue;
				else
				{
					bTemplateFound = true;
					break;
				}
			}
		}

		if (bTemplateFound)
			return (*pMyValue == *pRefValue);	// If there is a valid template, skip m_Default. It must be overwritten.

		// Don't forget 'Default' template.
		T* pDefValue = reinterpret_cast<T*>(size_t(&BotProfileMgr::m_Default) + iOffset);
		return (*pDefValue == *pMyValue);
	}

	template<typename T = int>
	bool IsMemberDefault(size_t iOffset) const noexcept	// #HACKHACK undefined behavior involved.
	{
		if (this == &BotProfileMgr::m_Default)
			return true;

		T* pMyValue = reinterpret_cast<T*>(size_t(this) + iOffset);
		T* pDefValue = reinterpret_cast<T*>(size_t(&BotProfileMgr::m_Default) + iOffset);
		return (*pDefValue == *pMyValue);
	}

	template<typename T = int>
	bool IsMemberTemplated(size_t iOffset) const noexcept	// #HACKHACK undefined behavior involved.
	{
		bool bTemplateFound = false;
		T* pMyValue = reinterpret_cast<T*>(size_t(this) + iOffset);
		T* pRefValue = nullptr;

		for (auto iter = m_rgszRefTemplates.rbegin(); iter != m_rgszRefTemplates.rend(); iter++)
		{
			BotProfile_t* pRef = &BotProfileMgr::m_Templates[*iter];
			pRefValue = reinterpret_cast<T*>(size_t(pRef) + iOffset);

			if constexpr (std::is_same_v<T, std::string>)	// Does my reference has no value for themself?
			{
				if (pRefValue->empty() || pRefValue->starts_with("Error - "s))
					continue;
				else
				{
					bTemplateFound = true;
					break;	// Check the latest used template. If it is a valid, skip all the others.
				}
			}
			else
			{
				if (*pRefValue < 0 || (!*pRefValue && iOffset == offsetof(BotProfile_t, m_bitsDifficulty)))
					continue;
				else
				{
					bTemplateFound = true;
					break;
				}
			}
		}

		if (bTemplateFound)
			return (*pMyValue == *pRefValue);

		return false;	// If none of template value can be located, sure it is write by the user.
	}

	void Reset(bool bDeleteReferences = true)
	{
		m_szName = "Error - No name"s;
		m_rgszWpnPreference.clear();
		m_bPrefersSilencer = false;
		m_bExplicitlyNoneWpnPref = false;
		m_flAttackDelay = -1;
		m_flReactionTime = -1;
		m_bitsDifficulty = -1;
		m_iAggression = -1;
		m_iCost = -1;
		m_iSkill = -1;
		m_iTeamwork = -1;
		m_iVoicePitch = -1;

		if (bDeleteReferences)
			m_rgszRefTemplates.clear();

		m_szSkin = "";
		m_szTeam = "";
		m_szVoiceBank = "";
	}

	// Different from Task_t::SanityCheck(), this one doesnot modify original value.
	const char* SanityCheck(void) const noexcept
	{
		if (m_szName.empty())
			return "Empty bot name.";
		if (m_szName.find_first_of(" \t\n\v\f\r") != Name_t::npos)	// basic_string::find will try to match the entire param.
			return "Name string contains space(s).";
		if (m_flAttackDelay < 0)
			return "Attack delay must be a non-negative value.";
		if (m_flReactionTime < 0)
			return "Reaction time must be a non-negative value.";
		if (m_iAggression < 0 || m_iAggression > 100)
			return "Aggression must be a value between 0 and 100.";
		if (m_iCost < 1 || m_iCost > 5)
			return "Cost of a teammate must be a value between 1 and 5.";
		if (m_iSkill < 0 || m_iSkill > 100)
			return "Skill must be a value between 0 and 100.";
		if (m_iTeamwork < 0 || m_iTeamwork > 100)
			return "Teamwork must be a value between 0 and 100.";
		if (m_iVoicePitch < 1)
			return "Voice pitch must be a value greater than 0.";
		if (m_rgszRefTemplates.empty())
			return "You must using at least one template for your character.";

		for (const auto& szWpn : m_rgszWpnPreference)
		{
			bool bValidBuyCommand = false;

			for (int i = 0; i < _countof(g_rgszWeaponNames); i++)
			{
				if (!g_rgbIsBuyCommand[i])
					continue;

				if (szWpn == g_rgszWeaponNames[i])
				{
					bValidBuyCommand = true;
					break;
				}
			}

			if (!bValidBuyCommand)
				return "Invalid buy command found in WeaponPreference field!";
		}

		return nullptr;	// Nullptr is good news!
	}

	void Inherit(const BotProfile_t& Parent) noexcept
	{
		if (!Parent.m_szName.starts_with("Error"))
			m_szName = Parent.m_szName;

		if (!Parent.m_rgszWpnPreference.empty())
		{
			m_rgszWpnPreference = Parent.m_rgszWpnPreference;
			m_bExplicitlyNoneWpnPref = false;
		}

		if (Parent.m_flAttackDelay >= 0)
			m_flAttackDelay = Parent.m_flAttackDelay;

		if (Parent.m_flReactionTime >= 0)
			m_flReactionTime = Parent.m_flReactionTime;

		if (Parent.m_bitsDifficulty > 0)
			m_bitsDifficulty = Parent.m_bitsDifficulty;

		if (Parent.m_iAggression >= 0)
			m_iAggression = Parent.m_iAggression;

		if (Parent.m_iCost > 0)
			m_iCost = Parent.m_iCost;

		if (Parent.m_iSkill >= 0)
			m_iSkill = Parent.m_iSkill;

		if (Parent.m_iTeamwork >= 0)
			m_iTeamwork = Parent.m_iTeamwork;

		if (Parent.m_iVoicePitch > 0)
			m_iVoicePitch = Parent.m_iVoicePitch;

		if (!Parent.m_szSkin.empty() && !Parent.m_szSkin.starts_with("Error"))
			m_szSkin = Parent.m_szSkin;

		if (!Parent.m_szTeam.empty() && !Parent.m_szTeam.starts_with("Error"))
			m_szTeam = Parent.m_szTeam;

		if (!Parent.m_szVoiceBank.empty() && !Parent.m_szVoiceBank.starts_with("Error"))
			m_szVoiceBank = Parent.m_szVoiceBank;
	}

	void Inherit(bool bShouldResetFirst = true, bool bCopyFromDefault = true) noexcept
	{
		if (m_rgszRefTemplates.empty())
			return;

		if (bShouldResetFirst)
			Reset(false);	// Will can inherit a shit if we have RefTemplate field cleared.

		if (bCopyFromDefault)	// Copy from default first.
		{
			Inherit(BotProfileMgr::m_Default);
			m_bExplicitlyNoneWpnPref = BotProfileMgr::m_Default.m_bExplicitlyNoneWpnPref;	// This should be copied standalone.
		}

		for (const auto& szInherit : m_rgszRefTemplates)
		{
			// Inherit code.
			if (auto iter = BotProfileMgr::m_Templates.find(szInherit); iter != BotProfileMgr::m_Templates.end())
			{
				Inherit(iter->second);
			}
			else
			{
				std::cout << "Error while " << std::quoted(m_szName) << " attempting to inherit from " << std::quoted(iter->first) << " - invalid template reference " << szInherit << '\n';
			}
		}
	}

	Name_t m_szName{ "Error - No name"s };	// Not in attrib
	Weapons_t m_rgszWpnPreference{};
	bool m_bPrefersSilencer{ false };	// Not editable.
	bool m_bExplicitlyNoneWpnPref{ false };	// Share with WeaponPreference, no inheritance
	float m_flAttackDelay{ -1 };
	float m_flReactionTime{ -1 };
	int m_bitsDifficulty{ 0 };
	int m_iAggression{ -1 };
	int m_iCost{ -1 };
	int m_iSkill{ -1 };
	int m_iTeamwork{ -1 };
	int m_iVoicePitch{ -1 };
	std::list<std::string> m_rgszRefTemplates{};	// Not in attrib
	std::string m_szSkin{ "" };
	std::string m_szTeam{ "" };
	std::string m_szVoiceBank{ "" };
};


inline BotProfiles_t&		g_BotProfiles = BotProfileMgr::m_Profiles;
inline GLFWwindow*			g_hGLFWWindow = nullptr;
inline Maps_t				g_Maps;
inline bool					g_bShowDebugWindow = false, g_bCurGamePathValid = false, g_bShowConfigWindow = true, g_bShowLociWindow = false, g_bShowCampaignWindow = false, g_bShowMapsWindow = false, g_bShowBotsWindow = true;
inline fs::path				g_GamePath;
inline std::atomic<int>		g_bitsAsyncStatus = Async_e::UNKNOWN;
inline std::string			g_szInputGamePath;
inline NewKeyValues*		g_Config = nullptr;


namespace BotProfileMgr
{
	// Real Declration
	inline BotProfiles_t	m_Profiles{};
	inline BotProfile_t		m_Default{};
	inline SkinThumbnails_t	m_Thumbnails{};
	inline std::mutex		Mutex;

	// File headers.
	inline const char m_szHeader[] =
		"// BotProfile.db\n"
		"// Author: Michael S. Booth, Turtle Rock Studios (www.turtlerockstudios.com)\n"
		"//\n"
		"// This database defines bot \"personalities\".\n"
		"// Feel free to edit it and define your own bots.\n";
	inline const char m_szSeparator[] =
		"//----------------------------------------------------------------------------\n";
	inline const char m_szDefaultTitle[] =
		"// All profiles begin with this data and overwrite their own\n";
	inline const char m_szSkinTitle[] =
		"// Skins\n"
		"// Reference to https://steamcommunity.com/sharedfiles/filedetails/?id=154853395 for more details.\n";
	inline const char m_szTemplateTitle[] =
		"// These templates inherit from Default and override with their values\n"
		"// The name of the template defines a type that is used by individual bot profiles\n";
	inline const char m_szCharacterTitle[] =
		"// These are the individual bot profiles, which inherit first from \n"
		"// Default and then the specified Template(s), in order\n";

	void Clear(void)
	{
		m_Profiles.clear();
		m_Default.Reset();
		m_Templates.clear();
		m_Skins.clear();
	}

	bool Parse(const fs::path& hPath)	// Assuming this file IS ACTUALLY exists.
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
						pProfile->m_bExplicitlyNoneWpnPref = true;
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
							pProfile->m_rgszWpnPreference.emplace_back(g_rgszWeaponNames[i]);	// Invisible std::tolower().
							pProfile->m_bExplicitlyNoneWpnPref = false;
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

	bool Save(const fs::path& hPath)
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
		hFile << m_szSeparator << m_szSkinTitle << m_szSeparator << std::endl;
		for (const auto& Skin : m_Skins)
		{
			hFile << "Skin " << Skin.first << std::endl;
			hFile << "\tModel = " << Skin.second << std::endl;
			hFile << "End\n" << std::endl;
		}

		// Templates
		hFile << m_szSeparator << m_szTemplateTitle << m_szSeparator << std::endl;
		for (const auto& Template : m_Templates)
		{
			hFile << "Template " << Template.first << std::endl;
			Template.second.SaveAttrib(hFile);
			hFile << "End\n" << std::endl;
		}

		// Characters
		hFile << m_szSeparator << m_szCharacterTitle << m_szSeparator << std::endl;
		for (const auto& Character : m_Profiles)
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

	void LoadSkinThumbnails(void)
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

	void ModifyDefault(BotProfile_t& NewDefault) noexcept
	{
		for (auto& Character : m_Profiles)
		{
			// Why check the overall heritage status first, and then check heritage from Default again specifically?
			// Consider this:
			// Default: Teamwork = 75
			// Template1: Teamwork = 50
			// Character to whom Template1 was applied: Teamwork = 75
			// In this case, the use explicitly state that this BOT has a Teamwork valued 75, better than other Characters Template1 applied to.
			// There were no connection between the 75 from Character table and the 75 from Default table.
			// Therefore, it makes no sense to make the Teamwork value from Character table change with the Default table.

#define CHECK_AND_SYNC(m)	if (Character.IsMemberInherited<decltype(BotProfile_t::m)>(offsetof(BotProfile_t, m)) && Character.IsMemberDefault<decltype(BotProfile_t::m)>(offsetof(BotProfile_t, m)))	\
								Character.m = NewDefault.m

			// Should never sync with name.

			CHECK_AND_SYNC(m_flAttackDelay);
			CHECK_AND_SYNC(m_flReactionTime);
			CHECK_AND_SYNC(m_bitsDifficulty);
			CHECK_AND_SYNC(m_iAggression);
			CHECK_AND_SYNC(m_iCost);
			CHECK_AND_SYNC(m_iSkill);
			CHECK_AND_SYNC(m_iTeamwork);
			CHECK_AND_SYNC(m_iVoicePitch);
			CHECK_AND_SYNC(m_szSkin);
			CHECK_AND_SYNC(m_szTeam);
			CHECK_AND_SYNC(m_szVoiceBank);

			if (Character.m_rgszWpnPreference == m_Default.m_rgszWpnPreference && Character.m_bExplicitlyNoneWpnPref == m_Default.m_bExplicitlyNoneWpnPref)
			{
				Character.m_rgszWpnPreference = NewDefault.m_rgszWpnPreference;
				Character.m_bExplicitlyNoneWpnPref = NewDefault.m_bExplicitlyNoneWpnPref;
			}
#undef CHECK_AND_SYNC
		}

		m_Default = std::move(NewDefault);
	}

	void ModifyTemplate(const Name_t& szTemplateName, BotProfile_t& NewTemplate) noexcept
	{
		assert(m_Templates.contains(szTemplateName));

		for (auto& Character : m_Profiles)
		{
			bool bUsingThisTemplate = false;
			for (const auto& szTemplate : Character.m_rgszRefTemplates)
			{
				if (szTemplate == szTemplateName)
				{
					bUsingThisTemplate = true;
					break;
				}
			}

			if (!bUsingThisTemplate)
				continue;

			// Why check the overall heritage status first, and then check heritage from Default again specifically?
			// Consider this:
			// Default: Teamwork = 75
			// Template1: Teamwork = 50
			// Character to whom Template1 was applied: Teamwork = 75
			// In this case, the use explicitly state that this BOT has a Teamwork valued 75, better than other Characters Template1 applied to.
			// There were no connection between the 75 from Character table and the 75 from Default table.
			// Therefore, it makes no sense to make the Teamwork value from Character table change with the Default table.

#define CHECK_AND_SYNC(m)	if (Character.IsMemberInherited<decltype(BotProfile_t::m)>(offsetof(BotProfile_t, m)) && Character.IsMemberTemplated<decltype(BotProfile_t::m)>(offsetof(BotProfile_t, m)))	\
								Character.m = NewTemplate.m

			// Should never sync with name.

			CHECK_AND_SYNC(m_flAttackDelay);
			CHECK_AND_SYNC(m_flReactionTime);
			CHECK_AND_SYNC(m_bitsDifficulty);
			CHECK_AND_SYNC(m_iAggression);
			CHECK_AND_SYNC(m_iCost);
			CHECK_AND_SYNC(m_iSkill);
			CHECK_AND_SYNC(m_iTeamwork);
			CHECK_AND_SYNC(m_iVoicePitch);
			CHECK_AND_SYNC(m_szSkin);
			CHECK_AND_SYNC(m_szTeam);
			CHECK_AND_SYNC(m_szVoiceBank);

			//if (Character.m_rgszWpnPreference == m_Default.m_rgszWpnPreference && Character.m_bExplicitlyNoneWpnPref == m_Default.m_bExplicitlyNoneWpnPref)
			//{
			//	Character.m_rgszWpnPreference = NewDefault.m_rgszWpnPreference;
			//	Character.m_bExplicitlyNoneWpnPref = NewDefault.m_bExplicitlyNoneWpnPref;
			//}
#undef CHECK_AND_SYNC
		}

		//m_Default = std::move(NewDefault);
	}
};

namespace MissionPack
{
	enum : size_t
	{
		FILE_OVERVIEW = 0U,
		FILE_THUMBNAIL,
		FILE_EASY,
		FILE_NORMAL,
		FILE_HARD,
		FILE_EXPERT,
		FILE_BOT_PROFILE,

		FILES_COUNT
	};

	using Files_t = std::array<fs::path, FILES_COUNT>;
	using Folder_t = fs::path;

	inline Name_t			Name;
	inline Folder_t			Folder;
	inline Files_t			Files;
	inline Thumbnail_t		Thumbnail;
	inline CareerGames_t	CareerGames;
	inline KeyValueSet_t	CGKVs;
	inline Difficulty_e		CurBrowsing{ Difficulty_e::EASY };
	inline std::mutex		Mutex;

	void CompileFileList(Files_t& rgFiles, const fs::path& hFolder, bool bCheck = true)
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

	void CompileFileListOfTurtleRockCounterTerrorist(Files_t& rgFiles, const fs::path& hGameFolder)
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
	void LoadFolder(const fs::path& hFolder)
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
			// Overfiew file parse #TODO
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
				CGKVs[i] = new NewKeyValues(g_rgszDifficultyNames[(size_t)i]);
				CGKVs[i]->LoadFromFile(Files[i + FILE_EASY].string().c_str());
				CareerGames[i].Parse(CGKVs[i]);
			}
		}
	}

	void Save(const fs::path& hFolder = Folder)
	{
		const std::scoped_lock Lock(MissionPack::Mutex, BotProfileMgr::Mutex);

		if (!fs::exists(hFolder))
			fs::create_directories(hFolder);

		Files_t rgFiles;
		CompileFileList(rgFiles, hFolder, false);

		if (!fs::exists(rgFiles[FILE_OVERVIEW]) && fs::exists(Files[FILE_OVERVIEW]))	// Properly output Overview.vdf #TODO
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
	bool IsTeammate(const Name_t& szName) noexcept
	{
		return std::find_if(CareerGames[CurBrowsing].m_rgszCharacters.begin(), CareerGames[CurBrowsing].m_rgszCharacters.end(),
			[&szName](const Name_t& szCharacter)
			{
				return szName == szCharacter;
			}
		) != CareerGames[CurBrowsing].m_rgszCharacters.end();
	}

	// Is this character is enlisted as our enemy in any of our mission locations?
	bool IsEnemy(const Name_t& szName) noexcept
	{
		for (const auto& Locus : CareerGames[CurBrowsing].m_Loci)
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
};

namespace Maps	// This is the game map instead of career quest 'Locus_t'!
{
	std::mutex Mutex;
	ImGuiTextFilter Filter;	// Used in MapsWindow(). Call Build() after manually edits it.

	void Load(void)
	{
		std::scoped_lock Lock(Mutex);

		g_Maps.clear();

		for (const auto& Directory : HalfLifeFileStructure::m_Directories)
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
};

#pragma endregion Objects

#pragma region GUI
void ListKeyValue(NewKeyValues* pkv)
{
	if (!pkv)
		return;

	if (!ImGui::TreeNode(pkv->GetName()))
		return;

	NewKeyValues* pSub = pkv->GetFirstKeyValue();
	while (pSub)
	{
		ImGui::Bullet();
		ImGui::SameLine();
		ImGui::TextWrapped("(Value) %s: %s", pSub->GetName(), pSub->GetValue<const char*>());
		pSub = pSub->GetNextKeyValue();
	}

	pSub = pkv->GetFirstSubkey();
	while (pSub)
	{
		ListKeyValue(pSub);	// New tree included.
		pSub = pSub->GetNextSubkey();
	}

	ImGui::TreePop();
}

const auto fnWeaponMenu = []<typename... Tys> requires all_the_same<Tys...>(const Tys&... rgbMask) -> const char*
{
	const char* pszResult = nullptr;
	bool bEmptyCategory = true;
	constexpr std::pair<size_t, size_t> rgPairStartEnds[] =
	{
		{pistol, shotgun},
		{shotgun, SMG},
		{SMG, rifle},
		{rifle, sniper},
		{sniper, machinegun},
		{machinegun, knife},
		{knife, _WEAPON_COUNT}
	};

	for (const auto& [iStart, iEnd] : rgPairStartEnds)
	{
		bEmptyCategory = true;

		if (ImGui::BeginMenu(g_rgszWeaponNames[iStart]))
		{
			for (size_t i = iStart; i < iEnd; i++)
			{
				if ((!rgbMask[i] || ...))
					continue;

				bEmptyCategory = false;

				if (ImGui::MenuItem(g_rgszWeaponNames[i]))
				{
					pszResult = g_rgszWeaponNames[i];
					break;
				}
			}

			if (bEmptyCategory)
				ImGui::TextDisabled("<EMPTY>");

			ImGui::EndMenu();
		}
	}

	return pszResult;
};

void MainMenuBar(void)
{
	bool bSaveAsSelected = false;	// One frame variable.
	static std::string szSaveTo;	// But this shouldn't.

	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Save", "Ctrl+S") && !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO))
				MissionPack::Save();

			if (ImGui::MenuItem("Save as...", "Ctrl+Alt+S") && !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO))
				bSaveAsSelected = true;

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Window"))
		{
			ImGui::MenuItem("Config", "", &g_bShowConfigWindow);
			ImGui::Separator();
			ImGui::MenuItem("Campaign", "F1", &g_bShowCampaignWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));
			ImGui::MenuItem("Loci", "F2", &g_bShowLociWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));
			ImGui::MenuItem("Maps", "F3", &g_bShowMapsWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MAPS_INFO));	// #TODO shotcut does not implement yet.
			ImGui::MenuItem("BOTs", "F4", &g_bShowBotsWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));	// #TODO shotcut does not implement yet.
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}

	if (bSaveAsSelected)
		ImGui::OpenPopup("Save as...");

		// Always center this window when appearing
	ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	// Modal of Inserting character into list.
	if (ImGui::BeginPopupModal("Save as...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::Text("Save as... (Full mission pack path)");
		ImGui::InputText("##SaveTo", &szSaveTo);

		if (ImGui::Button("OK", ImVec2(120, 0)))
		{
			MissionPack::Save(szSaveTo);
			ImGui::CloseCurrentPopup();
		}

		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
}

void ConfigWindow(void)
{
	static bool bFirstOpen = true;
	static std::unordered_map<fs::path, MissionPack::Files_t> KnownMods;
	static const auto fnOnGamePathChanged = [](void)
	{
		g_bCurGamePathValid = fs::exists(g_szInputGamePath + "\\liblist.gam"s);

		if (g_bCurGamePathValid)
		{
			g_GamePath = g_szInputGamePath;
			HalfLifeFileStructure::Update();

			KnownMods.clear();

			// Add official pack first.
			MissionPack::CompileFileListOfTurtleRockCounterTerrorist(
				KnownMods[g_GamePath],
				g_GamePath
			);

			for (const auto& hEntry : fs::directory_iterator(g_szInputGamePath + "\\MissionPacks\\"))
			{
				if (KnownMods.contains(hEntry))
					continue;

				if (hEntry.is_directory() && fs::exists(hEntry.path().c_str() + L"\\Overview.vdf"s) && hEntry.path().filename().c_str() != L"TurtleRockCounterTerrorist"s)
					MissionPack::CompileFileList(KnownMods[hEntry.path()], hEntry.path());
			}

			std::thread t(
				[](void)
				{
					if (g_bitsAsyncStatus & Async_e::UPDATING_MAPS_INFO)
						std::this_thread::sleep_for(2s);	// #UNTESTED_BUT_SHOULD_WORK

					g_bitsAsyncStatus |= Async_e::UPDATING_MAPS_INFO;
					Maps::Load();	// This has to happen after all potential hlmod folders were enlisted.
					g_bitsAsyncStatus &= ~Async_e::UPDATING_MAPS_INFO;

				}
			);
			t.detach();

			// Load skin portraits.
			BotProfileMgr::LoadSkinThumbnails();
		}

		//g_bitsAsyncStatus |= Async_e::REQ_UPDATE_GAME_INFO;
	};

	if (!g_bShowConfigWindow)
		return;

	if (ImGui::Begin("Config", &g_bShowConfigWindow))
	{
		if (bFirstOpen)
		{
			g_szInputGamePath = g_Config->GetValue<std::string>("LastGamePath");
			bFirstOpen = false;

			if (g_szInputGamePath.length())
				fnOnGamePathChanged();
		}

		ImGui::Text("CZero game path:");
		if (ImGui::InputText("##Game", &g_szInputGamePath))
			fnOnGamePathChanged();

		ImGui::SameLine();
		ImGui::TextColored(g_bCurGamePathValid ? IMGUI_GREEN : IMGUI_RED, g_bCurGamePathValid ? "Valid" : "Invalid");

		// #TODO add detection for CZCareerFix.amxx

		if (!MissionPack::Name.empty())
		{
			ImGui::BulletText((g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO) ? "Loading: %s" : "Current mission pack: %s", MissionPack::Name.c_str());

			if (!(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO) && MissionPack::Thumbnail.m_iTexId)
				ImGui::Image((void*)(intptr_t)MissionPack::Thumbnail.m_iTexId, MissionPack::Thumbnail.Size());
		}

		if (!KnownMods.empty() && ImGui::CollapsingHeader("Mods", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (const auto& [hPath, Mod] : KnownMods)
			{
				if (ImGui::Selectable(hPath.filename().string().c_str()))
				{
					std::thread t(
						[&hPath](void)
						{
							if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO)
								return;

							g_bitsAsyncStatus |= Async_e::UPDATING_MISSION_PACK_INFO;
							MissionPack::LoadFolder(hPath);
							g_bitsAsyncStatus &= ~Async_e::UPDATING_MISSION_PACK_INFO;
						}
					);

					t.detach();
				}
			}
		}
	}

	ImGui::End();	// Path
}

void CampaignWindow(void)
{
	static struct  
	{
		Names_t m_Enrolled{};
		bool m_bShouldEnter{ false };
		Names_t::iterator m_itInsertingPos;

	} InsertionModal;

	static struct  
	{
		Name_t m_szEnrolled{ ""s };
		bool m_bShouldEnter{ false };
		Names_t::iterator m_itReplacingPos;

	} ReplacementModal;

	if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO || !g_bShowCampaignWindow)
		return;

	const std::unique_lock Lock(MissionPack::Mutex, std::try_to_lock);
	if (!Lock.owns_lock())
		return;

	if (ImGui::Begin("Campaign", &g_bShowCampaignWindow))
	{
		if (ImGui::BeginTabBar("TabBar: Campaign", ImGuiTabBarFlags_None))
		{
			for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
			{
				if (!fs::exists(MissionPack::Files[i + MissionPack::FILE_EASY]))
					continue;

				if (!ImGui::BeginTabItem(g_rgszDifficultyNames[(size_t)i]))
					continue;

				MissionPack::CurBrowsing = i;	// Switched to this tab.
				auto& CareerGame = MissionPack::CareerGames[i];

				if (ImGui::CollapsingHeader("Generic", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::InputInt("Initial Points", &CareerGame.m_iInitialPoints, 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Match Wins", &CareerGame.m_iMatchWins, 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Match Win By", &CareerGame.m_iMatchWinBy, 1, 5, ImGuiInputTextFlags_CharsDecimal);
				}

				if (ImGui::CollapsingHeader("Characters", ImGuiTreeNodeFlags_None))
				{
					// Enlist all characters in current difficulty.
					for (auto iter = CareerGame.m_rgszCharacters.begin(); iter != CareerGame.m_rgszCharacters.end(); ++iter)
					{
						ImGui::Selectable(iter->c_str());	// #TODO jump tp BOT editing screen.

						// Right-click the name.
						if (ImGui::BeginPopupContextItem())
						{
							if ((InsertionModal.m_bShouldEnter = ImGui::Selectable(UTIL_VarArgs("Insert before %s", iter->c_str()))) == true)
								InsertionModal.m_itInsertingPos = iter;

							else if ((InsertionModal.m_bShouldEnter = ImGui::Selectable("Insert at the end")) == true)
								InsertionModal.m_itInsertingPos = CareerGame.m_rgszCharacters.end();

							else if ((ReplacementModal.m_bShouldEnter = ImGui::Selectable(UTIL_VarArgs("Replace %s", iter->c_str()))) == true)
								ReplacementModal.m_itReplacingPos = iter;

							else if (ImGui::Selectable(UTIL_VarArgs("Rule out %s", iter->c_str())))
							{
								iter = CareerGame.m_rgszCharacters.erase(iter);
								iter--;	// So we can iter++ later in the for loop.
							}

							ImGui::EndPopup();
						}

						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Right-click to edit.\nDraw and draw item to reorder.");

						// Drag and draw.
						if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
						{
							bool bMovingUp = (ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y < 0.f);
							if (iter == CareerGame.m_rgszCharacters.begin() && bMovingUp)	// The first element shouldnot be able to moving up.
								continue;

							auto iterMovingTo = iter;
							bMovingUp ? iterMovingTo-- : iterMovingTo++;

							if (iterMovingTo == CareerGame.m_rgszCharacters.end())	// You can't moving to ... obliterate his name.
								continue;

							std::swap(*iter, *iterMovingTo);	// Have to dereference to actually swap.
							ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
						}
					}

#pragma region Character insertion modal
					if (InsertionModal.m_bShouldEnter)
						ImGui::OpenPopup("Inserting character...");

					// Always center this window when appearing
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					// Modal of Inserting character into list.
					if (ImGui::BeginPopupModal("Inserting character...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						InsertionModal.m_bShouldEnter = false;	// Clear the enter flag, but keep the iterator value.

						if (ImGui::BeginListBox("##Add_character_ListBox", ImVec2(-FLT_MIN, 21 * ImGui::GetTextLineHeightWithSpacing())))
						{
							for (const auto& Character : g_BotProfiles)
							{
								auto it = std::find_if(CareerGame.m_rgszCharacters.begin(), CareerGame.m_rgszCharacters.end(),
									[&Character](const std::string& szName)
									{
										return szName == Character.m_szName;
									}
								);

								bool bAlreadyEnrolled = it != CareerGame.m_rgszCharacters.end();
								if (bAlreadyEnrolled)
									continue;

								it = std::find_if(InsertionModal.m_Enrolled.begin(), InsertionModal.m_Enrolled.end(),
									[&Character](const std::string& szName)
									{
										return szName == Character.m_szName;
									}
								);

								bAlreadyEnrolled = it != InsertionModal.m_Enrolled.end();

								if (ImGui::Selectable(Character.m_szName.c_str(), bAlreadyEnrolled))
								{
									if (bAlreadyEnrolled)	// Which means deselect.
										InsertionModal.m_Enrolled.erase(it);
									else
										InsertionModal.m_Enrolled.emplace_back(Character.m_szName);
								}
							}

							ImGui::EndListBox();
						}

						if (ImGui::Button("OK", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							CareerGame.m_rgszCharacters.insert(InsertionModal.m_itInsertingPos, InsertionModal.m_Enrolled.begin(), InsertionModal.m_Enrolled.end());
							InsertionModal.m_Enrolled.clear();
						}

						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();

						if (ImGui::Button("Cancel", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							InsertionModal.m_Enrolled.clear();
						}

						ImGui::EndPopup();
					}
#pragma endregion Character insertion modal

#pragma region Character replacement modal
					if (ReplacementModal.m_bShouldEnter)
						ImGui::OpenPopup("Replace character...");

					if (ImGui::BeginPopupModal("Replace character...", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ReplacementModal.m_bShouldEnter = false;	// Clear the enter flag, but keep the iterator value.

						if (ImGui::BeginListBox("##Add_character_ListBox", ImVec2(-FLT_MIN, 21 * ImGui::GetTextLineHeightWithSpacing())))
						{
							for (const auto& Character : g_BotProfiles)
							{
								auto it = std::find_if(CareerGame.m_rgszCharacters.begin(), CareerGame.m_rgszCharacters.end(),
									[&Character](const std::string& szName)
									{
										return szName == Character.m_szName;
									}
								);

								bool bAlreadyEnrolled = it != CareerGame.m_rgszCharacters.end();
								if (bAlreadyEnrolled)
									continue;

								bAlreadyEnrolled = Character.m_szName == ReplacementModal.m_szEnrolled;

								if (ImGui::Selectable(Character.m_szName.c_str(), bAlreadyEnrolled))
								{
									if (bAlreadyEnrolled)	// Which means deselect.
										ReplacementModal.m_szEnrolled.clear();
									else
										ReplacementModal.m_szEnrolled = Character.m_szName;
								}
							}

							ImGui::EndListBox();
						}

						if (ImGui::Button("OK", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							*ReplacementModal.m_itReplacingPos = std::move(ReplacementModal.m_szEnrolled);
							ReplacementModal.m_szEnrolled.clear();
						}

						ImGui::SetItemDefaultFocus();
						ImGui::SameLine();

						if (ImGui::Button("Cancel", ImVec2(120, 0)))
						{
							ImGui::CloseCurrentPopup();
							ReplacementModal.m_szEnrolled.clear();
						}

						ImGui::EndPopup();
					}
#pragma endregion Character replacement modal

					// Collapsing header. Nothing to pop.
				}

				if (ImGui::CollapsingHeader("Cost Availability", ImGuiTreeNodeFlags_DefaultOpen))
				{
					ImGui::InputInt("Tier 1", &CareerGame.m_rgiCostAvailability[0], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 2", &CareerGame.m_rgiCostAvailability[1], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 3", &CareerGame.m_rgiCostAvailability[2], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 4", &CareerGame.m_rgiCostAvailability[3], 1, 5, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Tier 5", &CareerGame.m_rgiCostAvailability[4], 1, 5, ImGuiInputTextFlags_CharsDecimal);
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void LocusWindow(void)
{
	if (g_bitsAsyncStatus & (Async_e::UPDATING_MISSION_PACK_INFO | Async_e::UPDATING_MAPS_INFO) || !g_bShowLociWindow)	// The drawing is using map thumbnail.
		return;

	const std::unique_lock Lock1(MissionPack::Mutex, std::try_to_lock);
	const std::unique_lock Lock2(Maps::Mutex, std::try_to_lock);
	if (!Lock1.owns_lock() || !Lock2.owns_lock())
		return;

	if (ImGui::Begin("Loci", &g_bShowLociWindow, ImGuiWindowFlags_NoResize))
	{
		constexpr ImGuiTableFlags bitsTableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBody;
		const auto vecSize = ImVec2(128 * 3 + 48, ImGui::GetTextLineHeightWithSpacing() * 3 + 128 * 3);
		const auto vecWindowSize = ImVec2(vecSize.x + 16, vecSize.y + 70);

		ImGui::SetWindowSize(vecWindowSize);

		if (ImGui::BeginTabBar("TabBar: Loci", ImGuiTabBarFlags_None))
		{
			for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
			{
				if (!fs::exists(MissionPack::Files[i + MissionPack::FILE_EASY]))
					continue;

				if (!ImGui::BeginTabItem(g_rgszDifficultyNames[(size_t)i]))
					continue;

				auto& CareerGame = MissionPack::CareerGames[i];
				auto& szCurDifficulty = g_rgszDifficultyNames[(size_t)i];

				// Table of all locations.
				if (ImGui::BeginTable("Table: Loci", 3, bitsTableFlags, vecSize))
				{
					static bool bTableInit[(size_t)Difficulty_e::_LAST] = { false, false, false, false };
					if (!bTableInit[(size_t)i])	// What the fuck, imgui?
					{
						bTableInit[(size_t)i] = true;
						ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
						ImGui::TableSetupColumn("##1", ImGuiTableColumnFlags_None);
						ImGui::TableSetupColumn("##2", ImGuiTableColumnFlags_None);
						ImGui::TableSetupColumn("##3", ImGuiTableColumnFlags_None);
						ImGui::TableHeadersRow();
					}

					for (auto itLocus = CareerGame.m_Loci.begin(); itLocus != CareerGame.m_Loci.end(); itLocus++)
					{
						auto& Locus = *itLocus;
						static int iColumnCount[(size_t)Difficulty_e::_LAST] = { 0, 0, 0 };

						if (iColumnCount[(size_t)i] >= 3)
						{
							ImGui::TableNextRow();
							iColumnCount[(size_t)i] = 0;
						}

						ImGui::TableSetColumnIndex(iColumnCount[(size_t)i]);
						ImGui::Text(Locus.m_szMap.c_str());

						ImGui::PushID((szCurDifficulty + Locus.m_szMap).c_str());
						bool bShouldOpen = ImGui::ImageButton((void*)(intptr_t)g_Maps[Locus.m_szMap].m_Thumbnail.m_iTexId, g_Maps[Locus.m_szMap].m_Thumbnail.Size());
						ImGui::PopID();

						static Locus_t LocusCopy{};	// Make a copy if we needs to enter editor.
						if (bShouldOpen)
						{
							ImGui::OpenPopup(UTIL_VarArgs("%s##%s", Locus.m_szMap.c_str(), szCurDifficulty));
							LocusCopy = Locus;	// Make a copy only once. Or our changes will be kept after 1 frame.
						}

						// Our buttons are both drag sources and drag targets here!
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							// Set payload to carry the index of our item (could be anything)
							ImGui::SetDragDropPayload("LocusIterator", &itLocus, sizeof(itLocus));

							// Preview popup.
							ImGui::Text(Locus.m_szMap.c_str());
							ImGui::Image((void*)(intptr_t)g_Maps[Locus.m_szMap].m_Thumbnail.m_iTexId, g_Maps[Locus.m_szMap].m_Thumbnail.Size());

							ImGui::EndDragDropSource();
						}

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload("LocusIterator"))
							{
								IM_ASSERT(Payload->DataSize == sizeof(decltype(itLocus)));

								auto itDraggedLocus = *(decltype(itLocus)*)Payload->Data;
								std::swap(*itLocus, *itDraggedLocus);
							}

							ImGui::EndDragDropTarget();
						}

						// Always center this window when appearing
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

						// Location editor.
						if (ImGui::BeginPopupModal(UTIL_VarArgs("%s##%s", Locus.m_szMap.c_str(), szCurDifficulty), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
						{
							// List all potential teammates in the current difficulty.
							// Why? Because they can't be assign as your enemy at the same time.
							Names_t& BotTeammates = MissionPack::CareerGames[i].m_rgszCharacters;

							// Title picture. #TODO make this a button such that we can select map via this?
							ImGui::Image((void*)(intptr_t)g_Maps[Locus.m_szMap].m_WiderPreview.m_iTexId, g_Maps[Locus.m_szMap].m_WiderPreview.Size());

							if (ImGui::CollapsingHeader(UTIL_VarArgs("%d Enem%s selected###EnemySelection", LocusCopy.m_rgszBots.size(), LocusCopy.m_rgszBots.size() < 2 ? "y" : "ies")) &&
								ImGui::BeginListBox("##Enemys_ListBox", ImVec2(-FLT_MIN, 7 * ImGui::GetTextLineHeightWithSpacing())))
							{
								for (const auto& Character : g_BotProfiles)
								{
									if (std::find_if(BotTeammates.begin(), BotTeammates.end(),
										[&Character](const Name_t& szOther)
										{
											return szOther == Character.m_szName;
										}
									) != BotTeammates.end())
									{
										continue;	// Skip drawing if our candidate is our potential teammate.
									}

									auto it = std::find_if(LocusCopy.m_rgszBots.begin(), LocusCopy.m_rgszBots.end(),
										[&Character](const Name_t& szOther)
										{
											return szOther == Character.m_szName;
										}
									);

									bool bEnrolled = it != LocusCopy.m_rgszBots.end();

									if (ImGui::Selectable(Character.m_szName.c_str(), bEnrolled))
									{
										if (bEnrolled)	// Select enrolled character -> rule them out.
											LocusCopy.m_rgszBots.erase(it);
										else
											LocusCopy.m_rgszBots.emplace_back(Character.m_szName);
									}
								}

								ImGui::EndListBox();
							}

							ImGui::InputInt("Min Enemies", &LocusCopy.m_iMinEnemies, 1, 5, ImGuiInputTextFlags_CharsDecimal);
							ImGui::InputInt("Threshold", &LocusCopy.m_iThreshold, 1, 5, ImGuiInputTextFlags_CharsDecimal);
							ImGui::Checkbox("Friendly Fire", &LocusCopy.m_bFriendlyFire);
							ImGui::InputText("Console Command(s)", &LocusCopy.m_szConsoleCommands);

							// Actual tasks.
							if (ImGui::CollapsingHeader("Task(s)", ImGuiTreeNodeFlags_DefaultOpen))
							{
								int iIdentifierIndex = 0;
								for (auto itTask = LocusCopy.m_Tasks.begin(); itTask != LocusCopy.m_Tasks.end(); /* Do nothing */)
								{
									Task_t& Task = *itTask;
									bool bTreeNodeExpanded = ImGui::TreeNode(UTIL_VarArgs("%s###task%d", Task.ToString().c_str(), iIdentifierIndex));

									if (!bTreeNodeExpanded && ImGui::IsItemHovered())
										ImGui::SetTooltip("Right-click to add or delete.\nDrag and draw to reorder.");

									// Right-click menu
									if (!bTreeNodeExpanded && ImGui::BeginPopupContextItem())
									{
										if (ImGui::Selectable("Insert new task"))
											LocusCopy.m_Tasks.push_back(Task_t{});

										ImGui::BeginDisabled(LocusCopy.m_Tasks.size() < 2);
										bool bRemoved = false;
										if (ImGui::Selectable("Delete"))	// Must have at least one task.
										{
											itTask = LocusCopy.m_Tasks.erase(itTask);
											bRemoved = true;
										}
										if (LocusCopy.m_Tasks.size() < 2 && ImGui::IsItemHovered())
											ImGui::SetTooltip("You must have at least one task.");	// #TODO how to show this tooltip while I was disabled?
										ImGui::EndDisabled();

										ImGui::EndPopup();

										if (bRemoved)
											continue;	// Skip the it++;
									}

									// Reordering.
									if (!bTreeNodeExpanded && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
									{
										ImGui::SetDragDropPayload("itTask", &itTask, sizeof(itTask));

										// Display preview
										ImGui::Text("Reordering task: %s", g_rgszTaskNames[(size_t)Task.m_iType]);
										ImGui::EndDragDropSource();
									}
									if (!bTreeNodeExpanded && ImGui::BeginDragDropTarget())
									{
										if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("itTask"))
										{
											IM_ASSERT(payload->DataSize == sizeof(itTask));
											auto itTaskDragged = *(decltype(itTask)*)payload->Data;
											std::swap(*itTaskDragged, *itTask);
										}

										ImGui::EndDragDropTarget();
									}

									// Task editor.
									if (bTreeNodeExpanded)
									{
										int iTaskType = (int)Task.m_iType;
										ImGui::Combo("Task Type", &iTaskType, g_rgszTaskNames, IM_ARRAYSIZE(g_rgszTaskNames));
										Task.m_iType = (TaskType_e)iTaskType;
										Task.SanityCheck();	// For example, you reassign this task from 'kill' to 'killall', in which you can't have any parameter at all.

										if (Task.m_iType == TaskType_e::winfast)
											ImGui::InputInt("In 'X' seconds", &Task.m_iCount, 1, 15, ImGuiInputTextFlags_CharsDecimal);
										else if ((1 << iTaskType) & Tasks::REQ_COUNT)
											ImGui::InputInt("Do 'X' times", &Task.m_iCount, 1, 5, ImGuiInputTextFlags_CharsDecimal);

										if ((1 << iTaskType) & Tasks::REQ_WEAPON)
										{
											if (ImGui::BeginCombo("Required Weapon", Task.m_szWeapon.c_str(), ImGuiComboFlags_None))
											{
												for (int i = 0; i < _countof(g_rgszWeaponNames); i++)
												{
													if (!g_rgbIsTaskWeapon[i])
														continue;

													const bool bIsSelected = Task.m_szWeapon == g_rgszWeaponNames[i];
													if (ImGui::Selectable(g_rgszWeaponNames[i], bIsSelected))
														Task.m_szWeapon = g_rgszWeaponNames[i];

													if (bIsSelected)
														ImGui::SetItemDefaultFocus();
												}

												ImGui::EndCombo();
											}
										}

										if ((1 << iTaskType) & Tasks::SURVIVE)
											ImGui::Checkbox("Survive the round", &Task.m_bSurvive);

										if ((1 << iTaskType) & Tasks::INAROW)
											ImGui::Checkbox("Finish tasks in a row", &Task.m_bInARow);

										ImGui::TreePop();
									}

									++iIdentifierIndex;
									++itTask;
								}
							}

							if (ImGui::Button("OK", ImVec2(120, 0)))
							{
								Locus = std::move(LocusCopy);
								ImGui::CloseCurrentPopup();
							}

							ImGui::SetItemDefaultFocus();
							ImGui::SameLine();
							if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
							ImGui::EndPopup();
						}

						iColumnCount[(size_t)i]++;
					}

					ImGui::EndTable();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}
	}

	ImGui::End();
}

void MapsWindow(void)
{
	if (g_bitsAsyncStatus & Async_e::UPDATING_MAPS_INFO || !g_bShowMapsWindow)
		return;

	const std::unique_lock Lock(Maps::Mutex, std::try_to_lock);
	if (!Lock.owns_lock())
		return;

	if (ImGui::Begin("Maps## of independent window", &g_bShowMapsWindow, ImGuiWindowFlags_NoResize))
	{
		ImGui::SetWindowSize(ImVec2(240, 480));

		Maps::Filter.Draw("Search");

		for (const auto& [szName, Map] : g_Maps)
		{
			if (!Maps::Filter.PassFilter(szName.c_str()))
				continue;

			ImGui::Selectable(szName.c_str());

			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();

				ImGui::TextWrapped(Map.m_Path.string().c_str());

				if (Map.m_bThumbnailExists)
				{
					ImGui::Image((void*)(intptr_t)Map.m_Thumbnail.m_iTexId, Map.m_Thumbnail.Size());
					ImGui::SameLine();
				}
				else
				{
					ImGui::Bullet(); ImGui::SameLine();
					ImGui::TextColored(IMGUI_YELLOW, "Thumbnail no found.");
				}

				if (Map.m_bWiderPreviewExists)
				{
					ImGui::Image((void*)(intptr_t)Map.m_WiderPreview.m_iTexId, Map.m_WiderPreview.Size());
				}
				else
				{
					if (!Map.m_bThumbnailExists)	// If a map has only Thumbnail, do not draw our text along with it.
					{
						ImGui::Bullet();
						ImGui::SameLine();
					}

					ImGui::TextColored(IMGUI_YELLOW, "Preview image no found.");
				}

				ImGui::Bullet(); ImGui::SameLine();
				ImGui::TextColored(Map.m_bBriefFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "Brief intro document %s.", Map.m_bBriefFileExists ? "found" : "no found");

				ImGui::Bullet(); ImGui::SameLine();
				ImGui::TextColored(Map.m_bDetailFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "HD texture definition %s.", Map.m_bDetailFileExists ? "found" : "no found");

				ImGui::Bullet(); ImGui::SameLine();
				ImGui::TextColored(Map.m_bNavFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "BOT navigation file %s.", Map.m_bDetailFileExists ? "found" : "no found");

				ImGui::Bullet(); ImGui::SameLine();
				ImGui::TextColored(Map.m_bOverviewFileExists ? IMGUI_GREEN : IMGUI_YELLOW, "Overview-related files %s.", Map.m_bDetailFileExists ? "found" : "no found");

				ImGui::TextWrapped("\nRequired resources for this map:");

				for (const auto& Res : Map.m_rgszResources)
				{
					bool b = HalfLifeFileStructure::Exists(Res);

					ImGui::Bullet(); ImGui::SameLine();
					ImGui::TextColored(b ? IMGUI_GREEN : IMGUI_RED, Res.c_str());
				}

				ImGui::EndTooltip();
			}
		}
	}

	ImGui::End();
}

void BotsWindow(void)
{
	if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO || !g_bShowBotsWindow)
		return;

	const std::unique_lock Lock1(MissionPack::Mutex, std::try_to_lock);
	const std::unique_lock Lock2(Maps::Mutex, std::try_to_lock);
	if (!Lock1.owns_lock() || !Lock2.owns_lock())
		return;

	static BotProfile_t BotCopy{};
	static WeaponSelMask_t rgbCanWpnEnlist;

	if (ImGui::Begin("BOTs", &g_bShowBotsWindow) && !g_BotProfiles.empty())
	{
		//if (ImGui::BeginTabBar("TabBar_BOT", ImGuiTabBarFlags_None))
		//{
		//	if (ImGui::BeginTabItem("Default"))
		//	{
		//		ImGui::EndTabItem();
		//	}
		//	if (ImGui::BeginTabItem("Templates"))
		//	{
		//		ImGui::EndTabItem();
		//	}

		//	ImGui::EndTabBar();
		//}

		constexpr ImGuiTableFlags bitsTableFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders| ImGuiTableFlags_Hideable;

		if (ImGui::BeginTable("TableOfBots", 1, bitsTableFlags))
		{
			for (auto itChar = g_BotProfiles.begin(); itChar != g_BotProfiles.end(); itChar++)
			{
				auto& Character = *itChar;

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);

				Thumbnail_t* pPortrait = nullptr;

				// atoi() is better than stoi() since atoi would return 0 of it is not a number. #POTENTIAL_BUG assert that ctskin[] is as long as tskin[].
				if (int iSkinIndex = std::atoi(Character.m_szSkin.c_str()); UTIL_GetStringType(Character.m_szSkin.c_str()) == 1 && iSkinIndex >= 0 && iSkinIndex < _countof(g_rgszCTSkinName))
				{
					if (iSkinIndex == 0)
						iSkinIndex = 6;	// 'Random' pic.

					pPortrait = &BotProfileMgr::m_Thumbnails[g_rgszTSkinName[iSkinIndex]];
					if (MissionPack::IsTeammate(Character.m_szName))
						pPortrait = &BotProfileMgr::m_Thumbnails[g_rgszCTSkinName[iSkinIndex]];
				}
				else if (BotProfileMgr::m_Skins.contains(Character.m_szSkin) && BotProfileMgr::m_Thumbnails.contains(BotProfileMgr::m_Skins[Character.m_szSkin]))
				{
					pPortrait = &BotProfileMgr::m_Thumbnails[BotProfileMgr::m_Skins[Character.m_szSkin]];
				}

				bool bShouldEnterCharacterEditor = false;
				ImGui::PushID((Character.m_szName + "##ImageButton").c_str());
				if (pPortrait)
					bShouldEnterCharacterEditor = ImGui::ImageButton((void*)(intptr_t)pPortrait->m_iTexId, pPortrait->Size());
				else
					bShouldEnterCharacterEditor = ImGui::Button(UTIL_VarArgs("No Portrait##%s", Character.m_szName.c_str()), ImVec2(128, 128));
				ImGui::PopID();
				ImGui::SameLine();

#pragma region CharacterEditingModal
				if (bShouldEnterCharacterEditor)
				{
					ImGui::OpenPopup(UTIL_VarArgs("%s##BotEditor", itChar->m_szName.c_str()));
					BotCopy = *itChar;
				}

				// Always center this window when appearing
				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				// Have to use the name stored in iterator. Otherwise things will get wired if user edits its name.
				if (ImGui::BeginPopupModal(UTIL_VarArgs("%s##BotEditor", itChar->m_szName.c_str()), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					ImGui::InputText("Character Name", &BotCopy.m_szName);
					ImGui::CheckboxFlags(g_rgszDifficultyNames[0], &BotCopy.m_bitsDifficulty, (1 << Difficulty_e::EASY)); ImGui::SameLine();
					ImGui::CheckboxFlags(g_rgszDifficultyNames[1], &BotCopy.m_bitsDifficulty, (1 << Difficulty_e::NORMAL)); ImGui::SameLine();
					ImGui::CheckboxFlags(g_rgszDifficultyNames[2], &BotCopy.m_bitsDifficulty, (1 << Difficulty_e::HARD)); ImGui::SameLine();
					ImGui::CheckboxFlags(g_rgszDifficultyNames[3], &BotCopy.m_bitsDifficulty, (1 << Difficulty_e::EXPERT));
#pragma region Weapon preference selection

					// Generate a mask for current character.
					rgbCanWpnEnlist.fill(true);
					for (int i = 0; i < _countof(g_rgszWeaponNames); ++i)
					{
						for (const auto& szWeapon : BotCopy.m_rgszWpnPreference)
						{
							if (szWeapon == g_rgszWeaponNames[i])
							{
								rgbCanWpnEnlist[i] = false;
								break;
							}
						}
					}

					ImGui::Text("Prefered Weapons:");
					if (ImGui::BeginPopupContextItem("Add Weapon##popup%s"))
					{
						if (const char* psz = fnWeaponMenu(g_rgbIsBuyCommand, rgbCanWpnEnlist); psz != nullptr)
							BotCopy.m_rgszWpnPreference.emplace_back(psz);

						ImGui::EndPopup();
					}
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Right click to append new preference at the end.\nDrag and draw in the following box to reorder.");

					auto iLineCount = std::clamp(BotCopy.m_rgszWpnPreference.size(), 1U, 7U);
					if (ImGui::BeginListBox("##WeaponList", ImVec2(-FLT_MIN, iLineCount * ImGui::GetTextLineHeightWithSpacing())))
					{
						for (auto itszWeapon = BotCopy.m_rgszWpnPreference.begin(); itszWeapon != BotCopy.m_rgszWpnPreference.end(); /* Do nothing */)
						{
							ImGui::Selectable(itszWeapon->c_str());

							if (ImGui::BeginPopupContextItem())
							{
								bool bShouldSkipIterInc = false;

								if (ImGui::MenuItem("Delete"))
								{
									itszWeapon = BotCopy.m_rgszWpnPreference.erase(itszWeapon);
									bShouldSkipIterInc = true;
								}

								if (ImGui::BeginMenu("Insert weapon..."))
								{
									if (const char* psz = fnWeaponMenu(g_rgbIsBuyCommand, rgbCanWpnEnlist); psz != nullptr)
									{
										itszWeapon = BotCopy.m_rgszWpnPreference.insert(itszWeapon, psz);
										bShouldSkipIterInc = true;
									}

									ImGui::EndMenu();
								}

								ImGui::EndPopup();

								if (bShouldSkipIterInc)
									continue;
							}

							if (ImGui::IsItemActive() && !ImGui::IsItemHovered())
							{
								bool bMovingUp = (ImGui::GetMouseDragDelta(ImGuiMouseButton_Left).y < 0.f);
								if (itszWeapon == BotCopy.m_rgszWpnPreference.begin() && bMovingUp)
									goto LAB_CONTINUE;

								auto iterMovingTo = itszWeapon;
								bMovingUp ? iterMovingTo-- : iterMovingTo++;

								if (iterMovingTo == BotCopy.m_rgszWpnPreference.end())
									goto LAB_CONTINUE;

								std::swap(*itszWeapon, *iterMovingTo);	// Have to dereference to actually iterators. Kinda' sucks.
								ImGui::ResetMouseDragDelta(ImGuiMouseButton_Left);
							}

						LAB_CONTINUE:;
							++itszWeapon;
						}

						// #TODO Handle 'none' tag.
						ImGui::EndListBox();
					}
#pragma endregion Weapon preference selection
					ImGui::InputFloat("Attack Delay", &BotCopy.m_flAttackDelay, 0.05f, 0.25f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputFloat("Reaction Time", &BotCopy.m_flReactionTime, 0.05f, 0.25f, "%.2f", ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Aggression", &BotCopy.m_iAggression, 5, 20, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Cost", &BotCopy.m_iCost, 1, 1, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Skill", &BotCopy.m_iSkill, 5, 20, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Teamwork", &BotCopy.m_iTeamwork, 5, 20, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputInt("Voice Pitch", &BotCopy.m_iVoicePitch, 5, 20, ImGuiInputTextFlags_CharsDecimal);
					ImGui::InputText("Skin Reference", &BotCopy.m_szSkin);
					if (ImGui::BeginCombo("Preferred Team", BotCopy.m_szTeam.c_str(), ImGuiComboFlags_None))
					{
						if (ImGui::Selectable("<EMPTY>"))
							BotCopy.m_szTeam = "";
						if (ImGui::Selectable("Terrorist"))
							BotCopy.m_szTeam = "T";
						if (ImGui::Selectable("Counter-Terrorist"))
							BotCopy.m_szTeam = "CT";
						if (ImGui::Selectable("Any"))
							BotCopy.m_szTeam = "ANY";

						ImGui::EndCombo();
					}
					ImGui::InputText("Voicebank", &BotCopy.m_szVoiceBank);	// #UNDONE_LONG_TERM file selection?

					auto pszErrorMessage = BotCopy.SanityCheck();	// A sanity a day keeps CTDs away.

					if (ImGui::Button("OK", ImVec2(120, 0)))
					{
						if (pszErrorMessage != nullptr)
						{
							ImGui::OpenPopup("Error");	// Can't leave error modal coding here. Or the window will only be draw when the button 'OK' is clicked.
						}
						else
						{
							*itChar = std::move(BotCopy);
							ImGui::CloseCurrentPopup();
						}
					}

#pragma region Modal of Error
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					// Modal of error have to be draw before the end of the character editing dialog.
					if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::TextWrapped(pszErrorMessage);	// It can't be nullptr or how do you get here?

						float flWidth = ImGui::GetWindowWidth();
						ImGui::SetCursorPosX((flWidth - 120.0f) / 2.0f);
						if (ImGui::Button("OK", ImVec2(120, 0)))
							ImGui::CloseCurrentPopup();

						ImGui::EndPopup();
					}
#pragma endregion Modal of Error

					ImGui::SetItemDefaultFocus();
					ImGui::SameLine();
					if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
					ImGui::EndPopup();

				}
#pragma endregion CharacterEditingModal

				auto vecPos = ImGui::GetCursorPos();
				ImGui::Text(Character.m_szName.c_str());

				ImGui::SameLine();
				bool bFirst = true;
				std::string szRefs = "(";
				for (const auto& szRef : Character.m_rgszRefTemplates)
				{
					if (bFirst)
						bFirst = false;
					else
						szRefs += '+';

					szRefs += szRef;
				}
				szRefs += ')';
				ImGui::Text(szRefs.c_str());

				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Right-click to reselect template.");

#pragma region CharacterTemplateModal
				bool bShouldEnterTemplateSelection = false;
				if (ImGui::BeginPopupContextItem(UTIL_VarArgs("TemplateEditingFor%s", Character.m_szName.c_str())))
				{
					if (ImGui::MenuItem("Reselect template..."))
					{
						BotCopy = *itChar;
						BotCopy.Inherit();	// Consider values from Default and Templates only.
						bShouldEnterTemplateSelection = true;
					}

					ImGui::EndPopup();
				}

				if (bShouldEnterTemplateSelection)
					ImGui::OpenPopup(UTIL_VarArgs("%s: Templates", Character.m_szName.c_str()));

				ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

				if (ImGui::BeginPopupModal(UTIL_VarArgs("%s: Templates", Character.m_szName.c_str()), nullptr, ImGuiWindowFlags_NoResize))
				{
					ImGui::SetWindowSize(ImVec2(240, 480));
					auto iModalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

					ImGui::SetCursorPosX(240 / 2 - 60);
					if (ImGui::Button("Add", ImVec2(120, 0)))
						ImGui::OpenPopup("Append template");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Append a new template at the end.");

					if (ImGui::BeginPopupContextItem("Append template"))
					{
						for (const auto& [szName, Template] : BotProfileMgr::m_Templates)
						{
							bool bAlreadyIn = false;
							for (const auto& szTemplate : BotCopy.m_rgszRefTemplates)
							{
								if (szName == szTemplate)
								{
									bAlreadyIn = true;
									break;
								}
							}

							if (bAlreadyIn)
								continue;

							if (ImGui::MenuItem(szName.c_str()))
							{
								BotCopy.m_rgszRefTemplates.emplace_back(szName);
								BotCopy.Inherit();	// Update heritage data.
							}
						}

						ImGui::EndPopup();
					}

					ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
					ImGui::BeginChild("Template hierarchy view", ImVec2(iModalContentWidth, 340), true, ImGuiWindowFlags_None);

					// Code reusage... so ugly...
					auto fnDisplayTemplateWithOverrideInfo = [](const BotProfile_t& Tpl)
					{
						if (!Tpl.m_rgszWpnPreference.empty())
						{
							for (const auto& szWpn : Tpl.m_rgszWpnPreference)
								ImGui::BulletText("WeaponPreference: %s", szWpn.c_str());
						}
						else if (Tpl.m_bExplicitlyNoneWpnPref)
						{
							if (BotCopy.m_bExplicitlyNoneWpnPref)
								ImGui::BulletText("WeaponPreference: none");
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) WeaponPreference: none");
							}
						}
						if (Tpl.m_flAttackDelay >= 0)
						{
							if (Tpl.m_flAttackDelay == BotCopy.m_flAttackDelay)
								ImGui::BulletText("Attack Delay: %.2f", Tpl.m_flAttackDelay);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Attack Delay: %.2f", Tpl.m_flAttackDelay);
							}
						}
						if (Tpl.m_flReactionTime >= 0)
						{
							if (Tpl.m_flReactionTime == BotCopy.m_flReactionTime)
								ImGui::BulletText("Reaction Time: %.2f", Tpl.m_flReactionTime);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Reaction Time: %.2f", Tpl.m_flReactionTime);
							}
						}
						if (Tpl.m_bitsDifficulty > 0)
						{
							for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
							{
								if (Tpl.m_bitsDifficulty & (1 << i))
								{
									if (Tpl.m_bitsDifficulty == BotCopy.m_bitsDifficulty)
										ImGui::BulletText("Difficulty: %s", g_rgszDifficultyNames[(size_t)i]);
									else
									{
										ImGui::Bullet();
										ImGui::TextColored(IMGUI_RED, "(Overrided) Difficulty: %s", g_rgszDifficultyNames[(size_t)i]);
									}
								}
							}
						}
						if (Tpl.m_iAggression >= 0 && Tpl.m_iAggression <= 100)
						{
							if (Tpl.m_iAggression == BotCopy.m_iAggression)
								ImGui::BulletText("Aggression: %d", Tpl.m_iAggression);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Aggression: %d", Tpl.m_iAggression);
							}
						}
						if (Tpl.m_iCost >= 1 && Tpl.m_iCost <= 5)
						{
							if (Tpl.m_iCost == BotCopy.m_iCost)
								ImGui::BulletText("Cost: %d", Tpl.m_iCost);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Cost: %d", Tpl.m_iCost);
							}
						}
						if (Tpl.m_iSkill >= 0 && Tpl.m_iSkill <= 100)
						{
							if (Tpl.m_iSkill == BotCopy.m_iSkill)
								ImGui::BulletText("Skill: %d", Tpl.m_iSkill);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Skill: %d", Tpl.m_iSkill);
							}
						}
						if (Tpl.m_iTeamwork >= 0 && Tpl.m_iTeamwork <= 100)
						{
							if (Tpl.m_iTeamwork == BotCopy.m_iTeamwork)
								ImGui::BulletText("Teamwork: %d", Tpl.m_iTeamwork);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Teamwork: %d", Tpl.m_iTeamwork);
							}
						}
						if (Tpl.m_iVoicePitch > 0)
						{
							if (Tpl.m_iVoicePitch == BotCopy.m_iVoicePitch)
								ImGui::BulletText("Voice Pitch: %d", Tpl.m_iVoicePitch);
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Voice Pitch: %d", Tpl.m_iVoicePitch);
							}
						}
						if (!Tpl.m_szSkin.empty())
						{
							if (Tpl.m_szSkin == BotCopy.m_szSkin)
								ImGui::BulletText("Skin: %s", Tpl.m_szSkin.c_str());
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Skin: %s", Tpl.m_szSkin.c_str());
							}
						}
						if (!Tpl.m_szTeam.empty())
						{
							if (Tpl.m_szTeam == BotCopy.m_szTeam)
								ImGui::BulletText("Team: %s", Tpl.m_szTeam.c_str());
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Team: %s", Tpl.m_szTeam.c_str());
							}
						}
						if (!Tpl.m_szVoiceBank.empty())
						{
							if (Tpl.m_szVoiceBank == BotCopy.m_szVoiceBank)
								ImGui::BulletText("Voicebank: %s", Tpl.m_szVoiceBank.c_str());
							else
							{
								ImGui::Bullet();
								ImGui::TextColored(IMGUI_RED, "(Overrided) Voicebank: %s", Tpl.m_szVoiceBank.c_str());
							}
						}
					};

					// Show the situation of Default.
					bool bTreeNodeExpanded = ImGui::TreeNode("Default##TemplateInfo");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("All characters have to inherit from Default.\nYou cannot reorder, delete or edit this one.");
					if (bTreeNodeExpanded)
					{
						fnDisplayTemplateWithOverrideInfo(BotProfileMgr::m_Default);
						ImGui::TreePop();
					}

					// List all template.
					for (auto itszTemplate = BotCopy.m_rgszRefTemplates.begin(); itszTemplate != BotCopy.m_rgszRefTemplates.end(); /* Do nothing */)
					{
						assert(BotProfileMgr::m_Templates.contains(*itszTemplate));
						auto& Template = BotProfileMgr::m_Templates[*itszTemplate];
						bTreeNodeExpanded = ImGui::TreeNode(itszTemplate->c_str());

						if (!bTreeNodeExpanded && ImGui::IsItemHovered())
							ImGui::SetTooltip("Right-click to delete.\nDrag and draw to reorder.");

						// Right-click to delete.
						if (!bTreeNodeExpanded && ImGui::BeginPopupContextItem(itszTemplate->c_str()))
						{
							if (ImGui::MenuItem("Delete"))
							{
								itszTemplate = BotCopy.m_rgszRefTemplates.erase(itszTemplate);
								BotCopy.Inherit();	// Update heritage data.

								ImGui::EndPopup();
								continue;
							}

							ImGui::EndPopup();
						}

						// Reordering.
						if (!bTreeNodeExpanded && ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							ImGui::SetDragDropPayload("itszTemplate", &itszTemplate, sizeof(itszTemplate));

							// Display preview
							ImGui::Text("Reordering: %s", itszTemplate->c_str());
							ImGui::EndDragDropSource();
						}
						if (!bTreeNodeExpanded && ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("itszTemplate"))
							{
								IM_ASSERT(payload->DataSize == sizeof(itszTemplate));
								auto itszTemplateDragged = *(decltype(itszTemplate)*)payload->Data;
								std::swap(*itszTemplateDragged, *itszTemplate);
								BotCopy.Inherit();	// Update heritage data.
							}

							ImGui::EndDragDropTarget();
						}

						// Template value preview.
						if (bTreeNodeExpanded)
						{
							fnDisplayTemplateWithOverrideInfo(Template);
							ImGui::TreePop();
						}

						++itszTemplate;
					}

					ImGui::EndChild();
					ImGui::PopStyleVar();

					ImGui::SetCursorPos(ImVec2(240 / 2 - 60, ImGui::GetWindowContentRegionMax().y - 60));
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 199, 206));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.98f, 0.35f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.98f, 0.45f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_Text, (ImVec4)ImColor(156, 0, 6));
					bTreeNodeExpanded = ImGui::Button("Reconstitute", ImVec2(120, 24));
					if (ImGui::IsItemHovered(ImGuiHoveredFlags_None))
						ImGui::SetTooltip("This will reconstruct the character: all non-template data will be lost.\nARE YOU SURE?");
					ImGui::PopStyleColor(4);
					if (bTreeNodeExpanded)
					{
						if (BotCopy.m_rgszRefTemplates.empty())
							ImGui::OpenPopup("Error - Template Selection");
						else
						{
							*itChar = std::move(BotCopy);
							ImGui::CloseCurrentPopup();
						}
					}

					// Error on save.
					ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
					if (ImGui::BeginPopupModal("Error - Template Selection", nullptr, ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoResize))
					{
						auto iModalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

						ImGui::Text("Every character have to derive from at least one template!");

						ImGui::SetCursorPosX(iModalContentWidth / 2 - 60);
						if (ImGui::Button("OK", ImVec2(120, 24)))
							ImGui::CloseCurrentPopup();

						ImGui::EndPopup();
					}

					ImGui::SetCursorPos(ImVec2(240 / 2 - 60, ImGui::GetWindowContentRegionMax().y - 24));
					if (ImGui::Button("Cancel", ImVec2(120, 24)))
						ImGui::CloseCurrentPopup();

					ImGui::EndPopup();
				}
#pragma endregion CharacterTemplateModal

				ImGui::SameLine();
				ImGui::TextDisabled("Cost %d", Character.m_iCost);

				float flY = vecPos.y + 22;	// Save the line 1 Y coord of this table.

				vecPos.y += 22; ImGui::SetCursorPos(vecPos);
				ImGui::TextColored(Character.m_iSkill > 66 ? IMGUI_GREEN : Character.m_iSkill > 33 ? IMGUI_YELLOW : IMGUI_RED, "Skill: %d", Character.m_iSkill);
				vecPos.y += 22; ImGui::SetCursorPos(vecPos);
				ImGui::TextColored(Character.m_iTeamwork > 66 ? IMGUI_GREEN : Character.m_iTeamwork > 33 ? IMGUI_YELLOW : IMGUI_RED, "Co-op: %d", Character.m_iTeamwork);
				vecPos.y += 22; ImGui::SetCursorPos(vecPos);
				ImGui::TextColored(Character.m_iAggression > 66 ? IMGUI_GREEN : Character.m_iAggression > 33 ? IMGUI_YELLOW : IMGUI_RED, "Bravery: %d", Character.m_iAggression);

				vecPos = ImVec2((pPortrait ? pPortrait->m_iWidth : 128) + 120, flY); ImGui::SetCursorPos(vecPos);
				ImGui::Text("Silencer User: %s", Character.m_bPrefersSilencer ? "Yes" : "No");

				if (!Character.m_rgszWpnPreference.empty())
				{
					vecPos.y += 22; ImGui::SetCursorPos(vecPos);
					ImGui::Text("Preference: %s", Character.m_rgszWpnPreference.front().c_str());
				}
			}

			ImGui::EndTable();
		}
	}

	ImGui::End();
}
#pragma endregion GUI





int main(int argc, char** argv)
{
	// Load config.
	g_Config = new NewKeyValues("CZeroEditorConfig");
	if (fs::exists("cze_config.ini"))
		g_Config->LoadFromFile("cze_config.ini");

#pragma region Initialize GL and ImGUI
	// Setup window
	glfwSetErrorCallback([](int error, const char* description) { std::fprintf(stderr, "Glfw Error %d: %s\n", error, description); });
	if (!glfwInit())
		return EXIT_FAILURE;

	// Decide GL+GLSL versions
	// GL 3.0 + GLSL 130
	constexpr const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	// Create window with graphics context
	g_hGLFWWindow = glfwCreateWindow(1280, 720, "Condition Zero: Career Game Editor", NULL, NULL);
	if (g_hGLFWWindow == NULL)
		return EXIT_FAILURE;

	glfwMakeContextCurrent(g_hGLFWWindow);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	//ImGui::StyleColorsDark();
	ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(g_hGLFWWindow, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Font settings
	ImFontConfig fontConfig; fontConfig.MergeMode = true;
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\SegoeUI.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\MSMincho.ttc", 18.0f, &fontConfig, io.Fonts->GetGlyphRangesJapanese());
	io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\KaiU.ttf", 18.0f, &fontConfig, io.Fonts->GetGlyphRangesChineseFull());
#pragma endregion Initialize GL and ImGUI

	// Main loop
	while (!glfwWindowShouldClose(g_hGLFWWindow))
	{
		// Lock
		g_bitsAsyncStatus |= Async_e::RENDERING;

		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

#pragma region Debug Window
#ifdef _DEBUG
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (ImGui::IsKeyPressed(0x60))
			g_bShowDebugWindow = !g_bShowDebugWindow;

		if (g_bShowDebugWindow)
			ImGui::ShowDemoWindow(&g_bShowDebugWindow);
#endif
#pragma endregion Debug Window

#pragma region Key detection
		if (ImGui::IsKeyPressed(0x122))
			g_bShowCampaignWindow = !g_bShowCampaignWindow;
		if (ImGui::IsKeyPressed(0x123))
			g_bShowLociWindow = !g_bShowLociWindow;
#pragma endregion Key detection

		MainMenuBar();
		ConfigWindow();
		CampaignWindow();
		LocusWindow();
		MapsWindow();
		BotsWindow();

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g_hGLFWWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.45f, 0.55f, 0.60f, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_hGLFWWindow);

		// Unlock
		g_bitsAsyncStatus &= ~Async_e::RENDERING;

		// Read & create glTexture outside of render.
		// Or it would fail and return 0.
		ImageLoader::Execute();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(g_hGLFWWindow);
	glfwTerminate();

	// Save config.
	if (g_szInputGamePath.length())
	{
		g_Config->SetValue("LastGamePath", g_szInputGamePath);
		g_Config->SaveToFile("cze_config.ini");
	}

	return EXIT_SUCCESS;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
