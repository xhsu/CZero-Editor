// CZeroEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// C++
#include <array>
#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>

// C
#include <cstddef>
#include <cstdio>

// ImGUI
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

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



#define IMGUI_GREEN	ImVec4(0, 153.0f / 255.0f, 0, 1)
#define IMGUI_RED	ImVec4(204.0f / 255.0f, 0, 0, 1)

#define CAST_TO_CHAR	reinterpret_cast<const char*>
#define CAST_TO_UTF8	reinterpret_cast<const char8_t*>
#define CAST_TO_U8S(x)	*reinterpret_cast<std::u8string*>(&x)
#define CAST_TO_STR(x)	*reinterpret_cast<std::string*>(&x)

#define GL_CLAMP_TO_EDGE 0x812F 


#pragma region Objects
// Dummy
enum class Difficulty_e : unsigned __int8;
enum class Weapon_e : unsigned __int8;
struct BotProfile_t;
struct CareerGame_t;
struct Map_t;
struct Task_t;

using BYTE = unsigned __int8;
using BotProfiles_t = std::list<BotProfile_t>;
using CareerGames_t = std::unordered_map<Difficulty_e, CareerGame_t>;
using ConsoleCmd_t = std::string;
using CostAvailability_t = std::array<int, 5>;
using KeyValueSet_t = std::unordered_map<Difficulty_e, NewKeyValues*>;
using Maps_t = std::list<Map_t>;
using Name_t = std::string;
using Names_t = std::list<Name_t>;
using Tasks_t = std::list<Task_t>;
using Weapons_t = std::vector<Weapon_e>;

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
	"scout",
	"xm1014",
	"mac10",
	"aug",
	"ump45",
	"sg550",
	"galil",
	"famas",
	"awp",
	"mp5",
	"m249",
	"m3",
	"m4a1",
	"tmp",
	"sg552",
	"ak47",
	"p90",
	"shield",
	"weapon",
	"knife",
	"grenade",
	"hegren",
	"pistol",
	"SMG",
	"machinegun",
	"shotgun",
	"rifle",
	"sniper",
	"fn57",
	"elites",
};

const char* g_rgszDifficultyNames[] = { "EASY", "NORMAL", "HARD", "EXPERT" };
const wchar_t* g_rgwcsDifficultyNames[] = { L"Easy", L"Normal", L"Hard", L"Expert" };

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

enum class Weapon_e : BYTE
{
	scout,
	xm1014,
	mac10,
	aug,
	ump45,
	sg550,
	galil,
	famas,
	awp,
	mp5,
	m249,
	m3,
	m4a1,
	tmp,
	sg552,
	ak47,
	p90,
	shield,
	weapon,
	knife,
	grenade,	// (text is singular)
	hegren,		// (text is plural, but the behavior is the same)
	pistol,
	SMG,
	machinegun,
	shotgun,
	rifle,
	sniper,
	fn57,
	elites,

	_LAST
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
	Task_t() {}
	Task_t(const std::string& sz) { Parse(sz); }
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
			else if (m_iWeapon == Weapon_e::_LAST)
			{
				// Identify weapon should be use.
				for (m_iWeapon = (Weapon_e)0; m_iWeapon < _countof(g_rgszWeaponNames); ++m_iWeapon)
				{
					if (!_stricmp(szToken.c_str(), g_rgszWeaponNames[(unsigned)m_iWeapon]))
						break;
				}
			}
		}
	}

	std::string ToString(void) const noexcept
	{
		std::string ret = g_rgszTaskNames[(unsigned)m_iType] + " "s;

		if (m_iCount && (1 << m_iType) & Tasks::REQ_COUNT)
			ret += std::to_string(m_iCount) + ' ';

		if (m_iWeapon != Weapon_e::_LAST && (1 << m_iType) & Tasks::REQ_WEAPON)
			ret += g_rgszWeaponNames[(unsigned)m_iWeapon] + " "s;

		if (m_bSurvive && (1 << m_iType) & Tasks::SURVIVE)
			ret += "survive"s + ' ';

		if (m_bInARow && (1 << m_iType) & Tasks::INAROW)
			ret += "inarow"s + ' ';

		ret.pop_back();	// Remove ' ' at the end.
		return ret;
	}

	void SanityCheck(void) noexcept
	{
		if (!((1 << m_iType) & Tasks::REQ_COUNT))
			m_iCount = 0;
		if (!((1 << m_iType) & Tasks::REQ_WEAPON))
			m_iWeapon = Weapon_e::_LAST;
		if (!((1 << m_iType) & Tasks::SURVIVE))
			m_bSurvive = false;
		if (!((1 << m_iType) & Tasks::INAROW))
			m_bInARow = false;
	}

	TaskType_e m_iType{ TaskType_e::kill };
	int m_iCount{ 0 };
	Weapon_e m_iWeapon{ Weapon_e::_LAST };
	bool m_bSurvive{ false };
	bool m_bInARow{ false };
};

struct Thumbnail_t : public Image_t
{
	constexpr Thumbnail_t(void) {}
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

struct Map_t
{
	Map_t() noexcept {}
	Map_t(NewKeyValues* pkv) noexcept { Parse(pkv); }
	Map_t(const Map_t& rhs) noexcept :
		m_szMapName(rhs.m_szMapName),
		m_rgszBots(rhs.m_rgszBots),
		m_iMinEnemies(rhs.m_iMinEnemies),
		m_iThreshold(rhs.m_iThreshold),
		m_Tasks(rhs.m_Tasks),
		m_bFriendlyFire(rhs.m_bFriendlyFire),
		m_szConsoleCommands(rhs.m_szConsoleCommands),
		m_Thumbnail(rhs.m_Thumbnail),
		m_WiderPreview(rhs.m_WiderPreview)
	{
	}
	Map_t& operator=(const Map_t& rhs) noexcept
	{
		m_szMapName = rhs.m_szMapName;
		m_rgszBots = rhs.m_rgszBots;
		m_iMinEnemies = rhs.m_iMinEnemies;
		m_iThreshold = rhs.m_iThreshold;
		m_Tasks = rhs.m_Tasks;
		m_bFriendlyFire = rhs.m_bFriendlyFire;
		m_szConsoleCommands = rhs.m_szConsoleCommands;
		m_Thumbnail = rhs.m_Thumbnail;
		m_WiderPreview = rhs.m_WiderPreview;	// #FORGET_HOW_TO_TEST_THE_CHANGE	orignally should be a memcpy() call.
		return *this;
	}
	Map_t(Map_t&& rhs) noexcept :
		m_szMapName(std::move(rhs.m_szMapName)),
		m_rgszBots(std::move(rhs.m_rgszBots)),
		m_iMinEnemies(rhs.m_iMinEnemies),
		m_iThreshold(rhs.m_iThreshold),
		m_Tasks(std::move(rhs.m_Tasks)),
		m_bFriendlyFire(rhs.m_bFriendlyFire),
		m_szConsoleCommands(std::move(rhs.m_szConsoleCommands)),
		m_Thumbnail(std::move(rhs.m_Thumbnail)),
		m_WiderPreview(std::move(rhs.m_WiderPreview))
	{
	}
	Map_t& operator=(Map_t&& rhs) noexcept
	{
		m_szMapName = std::move(rhs.m_szMapName);
		m_rgszBots = std::move(rhs.m_rgszBots);
		m_iMinEnemies = rhs.m_iMinEnemies;
		m_iThreshold = rhs.m_iThreshold;
		m_Tasks = std::move(rhs.m_Tasks);
		m_bFriendlyFire = rhs.m_bFriendlyFire;
		m_szConsoleCommands = std::move(rhs.m_szConsoleCommands);
		//m_Thumbnail = std::move(m_Thumbnail);
		//m_WiderPreview = std::move(m_WiderPreview);
		memcpy(&m_Thumbnail, &rhs.m_Thumbnail, sizeof(m_Thumbnail));
		memcpy(&m_WiderPreview, &rhs.m_WiderPreview, sizeof(m_WiderPreview));
		return *this;
	}
	virtual ~Map_t() noexcept {}

	void Parse(NewKeyValues* pkv)
	{
		m_rgszBots.clear();
		m_Tasks.clear();

		m_szMapName = pkv->GetName();
		ImageLoader::Add(g_GamePath.string() + "\\gfx\\thumbnails\\maps\\"s + m_szMapName + ".tga"s, &m_Thumbnail);
		ImageLoader::Add(g_GamePath.string() + "\\gfx\\thumbnails\\maps_wide\\"s + m_szMapName + ".tga"s, &m_WiderPreview);

		NewKeyValues* pSub = pkv->FindKey("bots");
		if (pSub)
			UTIL_Split(Name_t(pSub->GetString()), m_rgszBots, " "s);

		if ((pSub = pkv->FindKey("minEnemies")) != nullptr)
			m_iMinEnemies = pSub->GetInt();

		if ((pSub = pkv->FindKey("threshold")) != nullptr)
			m_iThreshold = pSub->GetInt();

		if ((pSub = pkv->FindKey("tasks")) != nullptr)
		{
			std::vector<std::string> rgszTasks;
			UTIL_Split(std::string(pSub->GetString()), rgszTasks, "'"s);

			for (auto& szTask : rgszTasks)
			{
				UTIL_Trim(szTask);

				if (szTask.empty())
					continue;

				m_Tasks.emplace_back(szTask);
			}
		}

		if ((pSub = pkv->FindKey("FriendlyFire")) != nullptr)
			m_bFriendlyFire = static_cast<bool>(pSub->GetInt());

		if ((pSub = pkv->FindKey("commands")) != nullptr)
			m_szConsoleCommands = pSub->GetString();
	}

	[[nodiscard]]
	NewKeyValues* Save(void) const	// #RET_HEAP_MEM
	{
		auto pkv = new NewKeyValues(m_szMapName.c_str());

		auto p = new NewKeyValues("bots");
		std::string szBotNames;
		for (const auto& szBotName : m_rgszBots)
			szBotNames += szBotName + ' ';
		szBotNames.pop_back();	// Remove ' ' at the end.
		p->SetString(nullptr, szBotNames.c_str());
		pkv->AddSubKey(p);

		p = new NewKeyValues("minEnemies");
		p->SetString(nullptr, std::to_string(m_iMinEnemies).c_str());
		pkv->AddSubKey(p);

		p = new NewKeyValues("threshold");
		p->SetString(nullptr, std::to_string(m_iThreshold).c_str());
		pkv->AddSubKey(p);

		p = new NewKeyValues("tasks");
		std::string szTasks;
		for (const auto& Task : m_Tasks)
			szTasks += '\'' + Task.ToString() + "' ";
		szTasks.pop_back();	// Remove ' ' at the end.
		p->SetString(nullptr, szTasks.c_str());
		pkv->AddSubKey(p);

		if (m_bFriendlyFire)
		{
			p = new NewKeyValues("FriendlyFire");
			p->SetString(nullptr, "1");
			pkv->AddSubKey(p);
		}

		if (!m_szConsoleCommands.empty())
		{
			p = new NewKeyValues("commands");
			p->SetString(nullptr, m_szConsoleCommands.c_str());
			pkv->AddSubKey(p);
		}

		return pkv;
	}

	Name_t			m_szMapName{ ""s };
	Names_t			m_rgszBots{};
	int				m_iMinEnemies{ 3 };
	int				m_iThreshold{ 2 };
	Tasks_t			m_Tasks{};
	bool			m_bFriendlyFire{ false };
	ConsoleCmd_t	m_szConsoleCommands{ ""s };
	Thumbnail_t		m_Thumbnail{};
	Thumbnail_t		m_WiderPreview{};
};

struct CareerGame_t
{
	void Parse(NewKeyValues* pkv)
	{
		Reset();

		NewKeyValues* pSub = pkv->FindKey("InitialPoints");
		if (pSub)
			m_iInitialPoints = pSub->GetInt();

		if ((pSub = pkv->FindKey("MatchWins")) != nullptr)
			m_iMatchWins = pSub->GetInt();

		if ((pSub = pkv->FindKey("MatchWinBy")) != nullptr)
			m_iMatchWinBy = pSub->GetInt();

		if ((pSub = pkv->FindKey("Characters")) != nullptr)
		{
			m_rgszCharacters.clear();
			UTIL_Split<Names_t, Name_t>(pSub->GetString(), m_rgszCharacters, " "s);
		}

		if ((pSub = pkv->FindKey("CostAvailability")) != nullptr)
		{
			[&] <size_t... I>(std::index_sequence<I...>)
			{
				NewKeyValues* p = nullptr;
				(((p = pSub->FindKey(UTIL_IntToString<I + 1>())) != nullptr ? (m_rgiCostAvailability[I] = p->GetInt()) : (int{})), ...);
			}
			(std::make_index_sequence<5>{});
		}

		if ((pSub = pkv->FindKey("Maps")) != nullptr)
		{
			NewKeyValues* p = pSub->GetFirstSubKey();

			while (p != nullptr)
			{
				m_Maps.emplace_back(p);
				p = p->GetNextSubKey();
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
		m_Maps.clear();
	}

	[[nodiscard]]
	NewKeyValues* Save(void) const	// #RET_HEAP_MEM
	{
		auto pkv = new NewKeyValues("CareerGame");

		auto p = new NewKeyValues("InitialPoints");
		p->SetString(nullptr, std::to_string(m_iInitialPoints).c_str());
		pkv->AddSubKey(p);

		p = new NewKeyValues("MatchWins");
		p->SetString(nullptr, std::to_string(m_iMatchWins).c_str());
		pkv->AddSubKey(p);

		p = new NewKeyValues("MatchWinBy");
		p->SetString(nullptr, std::to_string(m_iMatchWinBy).c_str());
		pkv->AddSubKey(p);

		Name_t szAllNames;
		for (const auto& szName : m_rgszCharacters)
			szAllNames += szName + ' ';
		szAllNames.pop_back();	// Remove last space.
		p = new NewKeyValues("Characters");
		p->SetString(nullptr, szAllNames.c_str());
		pkv->AddSubKey(p);

		p = new NewKeyValues("CostAvailability");
		for (int i = 0; i < 5; i++)
		{
			auto p2 = new NewKeyValues(std::to_string(i + 1).c_str());	// From 1 to 5.
			p2->SetString(nullptr, std::to_string(m_rgiCostAvailability[i]).c_str());
			p->AddSubKey(p2);
		}
		pkv->AddSubKey(p);

		p = new NewKeyValues("Maps");
		for (const auto& Map : m_Maps)
			p->AddSubKey(Map.Save());
		pkv->AddSubKey(p);

		return pkv;
	}

	int m_iInitialPoints{ 2 };
	int m_iMatchWins{ 3 };
	int m_iMatchWinBy{ 2 };
	Names_t m_rgszCharacters{};
	CostAvailability_t m_rgiCostAvailability{ 1, 6, 10, 15, 99 };
	Maps_t m_Maps;
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
	virtual ~BotProfile_t() {}

	bool SaveAttrib(std::ofstream& hFile) const
	{
		if (m_bExplicitlyNoneWpnPref)
			hFile << "\tWeaponPreference = none" << std::endl;
		else
		{
			for (const auto& iWeapon : m_rgWeaponPreference)
			{
				if (!m_ReferencedTemplates.empty())
				{
					BotProfile_t& Ref = BotProfileMgr::m_Templates[m_ReferencedTemplates.back()];

					for (const auto& iRefWpn : Ref.m_rgWeaponPreference)
					{
						if (iRefWpn == iWeapon)
							goto LAB_SKIP_SAVING;
					}
				}

				hFile << "\tWeaponPreference = " << g_rgszWeaponNames[(size_t)iWeapon] << std::endl;

			LAB_SKIP_SAVING:;
			}
		}

#define SAVE_MEMBER_HELPER_INT(x)	if (m_i##x >= 0 && !IsMemberInherited<int>(offsetof(BotProfile_t, m_i##x)))	\
										hFile << "\t" #x " = " << m_i##x << std::endl
#define SAVE_MEMBER_HELPER_FLT(x)	if (m_fl##x >= 0 && !IsMemberInherited<float>(offsetof(BotProfile_t, m_fl##x)))	\
										hFile << "\t" #x " = " << m_fl##x << std::endl
#define SAVE_MEMBER_HELPER_STR(x)	if (!m_sz##x.starts_with("Error - "s) && !IsMemberInherited<std::string>(offsetof(BotProfile_t, m_sz##x)))	\
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

		for (auto iter = m_ReferencedTemplates.rbegin(); iter != m_ReferencedTemplates.rend(); iter++)
		{
			BotProfile_t* pRef = &BotProfileMgr::m_Templates[*iter];
			pRefValue = reinterpret_cast<T*>(size_t(pRef) + iOffset);

			if constexpr (std::is_same_v<T, std::string>)	// Does my reference has no value for themself?
			{
				if (pRefValue->starts_with("Error - "s))
					continue;
				else
				{
					bTemplateFound = true;
					break;	// Check the latest used template. If it is a valid, skip all the others including m_Default.
				}
			}
			else
			{
				if (*pRefValue <= 0)
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

		for (const auto& Name : m_ReferencedTemplates)
		{
			BotProfile_t* pRef = &BotProfileMgr::m_Templates[Name];
			T* pRefValue = reinterpret_cast<T*>(size_t(pRef) + iOffset);

			if (*pRefValue == *pMyValue)
				return true;
		}

		// Don't forget 'Default' template.
		T* pDefValue = reinterpret_cast<T*>(size_t(&BotProfileMgr::m_Default) + iOffset);
		return (*pDefValue == *pMyValue);
	}

	void Reset(void)
	{
		m_szName = "Error - No name"s;
		m_rgWeaponPreference.clear();
		m_bPrefersSilencer = false;
		m_bExplicitlyNoneWpnPref = false;
		m_flAttackDelay = -1;
		m_flReactionTime = -1;
		m_bitsDifficulty = 0;
		m_iAggression = -1;
		m_iCost = -1;
		m_iSkill = -1;
		m_iTeamwork = -1;
		m_iVoicePitch = -1;
		m_ReferencedTemplates.clear();
		m_szSkin = "Error - No skin assigned"s;
		m_szTeam = "Error - No team preference"s;
		m_szVoiceBank = "Error - No voicebank assigned"s;
	}

	Name_t m_szName{ "Error - No name"s };	// Not in attrib
	Weapons_t m_rgWeaponPreference{};
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
	std::list<std::string> m_ReferencedTemplates{};	// Not in attrib
	std::string m_szSkin{ "Error - No skin assigned"s };
	std::string m_szTeam{ "Error - No team preference"s };
	std::string m_szVoiceBank{ "Error - No voicebank assigned"s };
};


inline BotProfiles_t&		g_BotProfiles = BotProfileMgr::m_Profiles;
inline GLFWwindow*			g_hGLFWWindow = nullptr;
inline bool					g_bShowDebugWindow = false, g_bCurGamePathValid = false, g_bShowConfigWindow = true, g_bShowMapsWindow = false, g_bShowCampaignWindow = false;
inline fs::path				g_GamePath;
inline std::atomic<int>		g_bitsAsyncStatus = Async_e::UNKNOWN;
inline std::string			g_szInputGamePath;


namespace BotProfileMgr
{
	// Real Declration
	inline BotProfiles_t m_Profiles{};
	inline BotProfile_t m_Default{};

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
				pProfile->m_bExplicitlyNoneWpnPref = false;	// Or every character won't have any weapon preference.
			}

			// do inheritance in order of appearance
			if (!bIsTemplate && !bIsDefault)
			{
				UTIL_Split(*iter, pProfile->m_ReferencedTemplates, "+"s);

				for (const auto& szInherit : pProfile->m_ReferencedTemplates)
				{
					// Inherit code.
					if (auto iter2 = m_Templates.find(szInherit); iter2 != m_Templates.end())
					{
						BotProfile_t& Inherit = iter2->second;

						if (!Inherit.m_szName.starts_with("Error"))
							pProfile->m_szName = Inherit.m_szName;

						if (!Inherit.m_rgWeaponPreference.empty())
							pProfile->m_rgWeaponPreference = Inherit.m_rgWeaponPreference;

						if (Inherit.m_flAttackDelay > 0)
							pProfile->m_flAttackDelay = Inherit.m_flAttackDelay;

						if (Inherit.m_flReactionTime > 0)
							pProfile->m_flReactionTime = Inherit.m_flReactionTime;

						if (Inherit.m_bitsDifficulty > 0)
							pProfile->m_bitsDifficulty = Inherit.m_bitsDifficulty;

						if (Inherit.m_iAggression > 0)
							pProfile->m_iAggression = Inherit.m_iAggression;

						if (Inherit.m_iCost > 0)
							pProfile->m_iCost = Inherit.m_iCost;

						if (Inherit.m_iSkill > 0)
							pProfile->m_iSkill = Inherit.m_iSkill;

						if (Inherit.m_iTeamwork > 0)
							pProfile->m_iTeamwork = Inherit.m_iTeamwork;

						if (Inherit.m_iVoicePitch > 0)
							pProfile->m_iVoicePitch = Inherit.m_iVoicePitch;

						if (!Inherit.m_szSkin.starts_with("Error"))
							pProfile->m_szSkin = Inherit.m_szSkin;

						if (!Inherit.m_szTeam.starts_with("Error"))
							pProfile->m_szTeam = Inherit.m_szTeam;

						if (!Inherit.m_szVoiceBank.starts_with("Error"))
							pProfile->m_szVoiceBank = Inherit.m_szVoiceBank;
					}
					else
					{
						std::cout << "Error parsing " << hPath << " - invalid template reference " << szInherit << '\n';
						return false;
					}
				}
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
						pProfile->m_rgWeaponPreference.clear();
						pProfile->m_bExplicitlyNoneWpnPref = true;
						continue;
					}

					for (size_t i = 0; i < _countof(g_rgszWeaponNames); i++)
					{
						if (!_stricmp(iter->c_str(), g_rgszWeaponNames[i]))
							pProfile->m_rgWeaponPreference.push_back((Weapon_e)i);
					}
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
			for (const auto& szRef : Character.m_ReferencedTemplates)
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

	void LoadFolder(const fs::path& hFolder)
	{
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
			if (CareerGames[i].m_Maps.empty())
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
};

#pragma endregion Objects

#pragma region GUI
void ListKeyValue(NewKeyValues* pkv)
{
	if (!pkv)
		return;

	if (!ImGui::TreeNode(pkv->GetName()))
		return;

	NewKeyValues* pSub = pkv->GetFirstValue();
	while (pSub)
	{
		ImGui::Bullet();
		ImGui::SameLine();
		ImGui::TextWrapped("(Value) %s: %s", pSub->GetName(), pSub->GetString());
		pSub = pSub->GetNextValue();
	}

	pSub = pkv->GetFirstSubKey();
	while (pSub)
	{
		ListKeyValue(pSub);	// New tree included.
		pSub = pSub->GetNextSubKey();
	}

	ImGui::TreePop();
}

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
			ImGui::MenuItem("Maps", "F2", &g_bShowMapsWindow, !(g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO));
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
	static std::unordered_map<fs::path, MissionPack::Files_t> KnownMods;
	static const auto fnOnGamePathChanged = [](void)
	{
		g_bCurGamePathValid = fs::exists(g_szInputGamePath + "\\liblist.gam"s);

		if (g_bCurGamePathValid)
		{
			g_GamePath = g_szInputGamePath;

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
		}

		//g_bitsAsyncStatus |= Async_e::REQ_UPDATE_GAME_INFO;
	};

	if (!g_bShowConfigWindow)
		return;

	if (ImGui::Begin("Config", &g_bShowConfigWindow))
	{

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
					std::thread t([&hPath](void)
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
					for (auto iter = CareerGame.m_rgszCharacters.begin(); iter != CareerGame.m_rgszCharacters.end(); iter++)
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

void MapsWindow(void)
{
	if (g_bitsAsyncStatus & Async_e::UPDATING_MISSION_PACK_INFO || !g_bShowMapsWindow)
		return;

	if (ImGui::Begin("Maps", &g_bShowMapsWindow))
	{
		constexpr ImGuiTableFlags bitsTableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_Hideable | ImGuiTableFlags_NoBordersInBody;
		const auto vecSize = ImVec2(128 * 3 + 48, ImGui::GetTextLineHeightWithSpacing() * 3 + 128 * 3);
		const auto vecWindowSize = ImVec2(vecSize.x + 16, vecSize.y + 70);

		ImGui::SetWindowSize(vecWindowSize);

		if (ImGui::BeginTabBar("TabBar: Maps", ImGuiTabBarFlags_None))
		{
			for (auto i = Difficulty_e::EASY; i < Difficulty_e::_LAST; ++i)
			{
				if (!fs::exists(MissionPack::Files[i + MissionPack::FILE_EASY]))
					continue;

				if (!ImGui::BeginTabItem(g_rgszDifficultyNames[(size_t)i]))
					continue;

				auto& CareerGame = MissionPack::CareerGames[i];
				auto& szCurDifficulty = g_rgszDifficultyNames[(size_t)i];

				if (ImGui::BeginTable("Table: Maps", 3, bitsTableFlags, vecSize))
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

					//for (auto& Map : CareerGame.m_Maps)
					for (auto itMap = CareerGame.m_Maps.begin(); itMap != CareerGame.m_Maps.end(); itMap++)
					{
						auto& Map = *itMap;
						static int iColumnCount[(size_t)Difficulty_e::_LAST] = { 0, 0, 0 };

						if (iColumnCount[(size_t)i] >= 3)
						{
							ImGui::TableNextRow();
							iColumnCount[(size_t)i] = 0;
						}

						ImGui::TableSetColumnIndex(iColumnCount[(size_t)i]);
						ImGui::Text(Map.m_szMapName.c_str());

						static Map_t MapCopy{};	// Make a copy if we needs to enter editor.
						if (ImGui::ImageButton((void*)(intptr_t)Map.m_Thumbnail.m_iTexId, Map.m_Thumbnail.Size()))
						{
							ImGui::OpenPopup(UTIL_VarArgs("%s##%s", Map.m_szMapName.c_str(), szCurDifficulty));
							MapCopy = Map;	// Make a copy only once. Or our changes will be kept after 1 frame.
						}

						// Our buttons are both drag sources and drag targets here!
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
						{
							// Set payload to carry the index of our item (could be anything)
							ImGui::SetDragDropPayload("MapIterator", &itMap, sizeof(itMap));

							// Preview popup.
							ImGui::Image((void*)(intptr_t)Map.m_Thumbnail.m_iTexId, Map.m_Thumbnail.Size());

							ImGui::EndDragDropSource();
						}

						if (ImGui::BeginDragDropTarget())
						{
							if (const ImGuiPayload* Payload = ImGui::AcceptDragDropPayload("MapIterator"))
							{
								IM_ASSERT(Payload->DataSize == sizeof(decltype(itMap)));

								auto itDraggedMap = *(decltype(itMap)*)Payload->Data;
								std::swap(*itMap, *itDraggedMap);
							}

							ImGui::EndDragDropTarget();
						}

						// Always center this window when appearing
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

						// Map task editor.
						if (ImGui::BeginPopupModal(UTIL_VarArgs("%s##%s", Map.m_szMapName.c_str(), szCurDifficulty), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
						{
							// List all potential teammates in the current difficulty.
							// Why? Because they can't be assign as your enemy at the same time.
							Names_t& BotTeammates = MissionPack::CareerGames[i].m_rgszCharacters;

							// Title picture.
							ImGui::Image((void*)(intptr_t)MapCopy.m_WiderPreview.m_iTexId, MapCopy.m_WiderPreview.Size());

							if (ImGui::CollapsingHeader(UTIL_VarArgs("%d Enem%s selected###EnemySelection", MapCopy.m_rgszBots.size(), MapCopy.m_rgszBots.size() < 2 ? "y" : "ies")) &&
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

									auto it = std::find_if(MapCopy.m_rgszBots.begin(), MapCopy.m_rgszBots.end(),
										[&Character](const Name_t& szOther)
										{
											return szOther == Character.m_szName;
										}
									);

									bool bEnrolled = it != MapCopy.m_rgszBots.end();

									if (ImGui::Selectable(Character.m_szName.c_str(), bEnrolled))
									{
										if (bEnrolled)	// Select enrolled character -> rule them out.
											MapCopy.m_rgszBots.erase(it);
										else
											MapCopy.m_rgszBots.emplace_back(Character.m_szName);
									}
								}

								ImGui::EndListBox();
							}

							ImGui::InputInt("Min Enemies", &MapCopy.m_iMinEnemies, 1, 5, ImGuiInputTextFlags_CharsDecimal);
							ImGui::InputInt("Threshold", &MapCopy.m_iThreshold, 1, 5, ImGuiInputTextFlags_CharsDecimal);
							ImGui::Checkbox("Friendly Fire", &MapCopy.m_bFriendlyFire);
							ImGui::InputText("Console Command(s)", &MapCopy.m_szConsoleCommands);

							// Actual tasks.
							if (ImGui::CollapsingHeader("Task(s)", ImGuiTreeNodeFlags_DefaultOpen))
							{
								int iIdentifierIndex = 0;
								//for (auto& Task : MapCopy.m_Tasks)
								for (auto itTask = MapCopy.m_Tasks.begin(); itTask != MapCopy.m_Tasks.end(); /* Do nothing */)
								{
									Task_t& Task = *itTask;

									if (ImGui::TreeNode(UTIL_VarArgs("%s###task%d", Task.ToString().c_str(), iIdentifierIndex)))
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
											int iWeaponType = (int)Task.m_iWeapon;
											ImGui::Combo("Weapon Type", &iWeaponType, g_rgszWeaponNames, IM_ARRAYSIZE(g_rgszWeaponNames));
											Task.m_iWeapon = (Weapon_e)iWeaponType;
										}

										if ((1 << iTaskType) & Tasks::SURVIVE)
											ImGui::Checkbox("Survive the round", &Task.m_bSurvive);

										if ((1 << iTaskType) & Tasks::INAROW)
											ImGui::Checkbox("Finish tasks in a row", &Task.m_bInARow);

										ImGui::TreePop();
									}

									iIdentifierIndex++;

									if (ImGui::BeginPopupContextItem())
									{
										if (ImGui::Selectable("Add task"))
											MapCopy.m_Tasks.push_back(Task_t{});

										bool bRemoved = false;
										if (MapCopy.m_Tasks.size() > 1 && ImGui::Selectable("Remove this"))	// Must have at least one task.
										{
											itTask = MapCopy.m_Tasks.erase(itTask);
											bRemoved = true;
										}

										ImGui::EndPopup();

										if (bRemoved)
											continue;	// Skip the it++;
									}

									itTask++;
								}
							}

							if (ImGui::Button("OK", ImVec2(120, 0)))
							{
								Map = std::move(MapCopy);
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
#pragma endregion GUI





int main(int argc, char** argv)
{
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
	ImGui::StyleColorsDark();
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
			g_bShowMapsWindow = !g_bShowMapsWindow;
#pragma endregion Key detection

		MainMenuBar();
		ConfigWindow();
		CampaignWindow();
		MapsWindow();

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
