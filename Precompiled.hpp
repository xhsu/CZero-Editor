#pragma once

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
#include <type_traits>
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

#pragma region Namespaces
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

namespace fs = std::filesystem;

using namespace std::literals::string_literals;
using namespace std::literals;
using namespace std::string_literals;

template<typename... Ts>
concept all_the_same = sizeof...(Ts) < 2 || std::conjunction_v<std::is_same<std::tuple_element_t<0, std::tuple<Ts...>>, Ts>...>;

template<typename T, typename... Ts>
concept all_same_as = std::conjunction_v<std::is_same<T, Ts>...>;
#pragma endregion Namespaces

// Helper object file.
#include "UtlImage.hpp"

#pragma region Macros
#define IMGUI_GREEN		ImVec4(102.0f / 255.0f, 204.0f / 255.0f, 102.0f / 255.0f, 1)
#define IMGUI_YELLOW	ImVec4(1, 204.0f / 255.0f, 102.0f / 255.0f, 1)
#define IMGUI_RED		ImVec4(204.0f / 255.0f, 0, 0, 1)

#define CAST_TO_CHAR	reinterpret_cast<const char*>
#define CAST_TO_UTF8	reinterpret_cast<const char8_t*>
#define CAST_TO_U8S(x)	*reinterpret_cast<std::u8string*>(&x)
#define CAST_TO_STR(x)	*reinterpret_cast<std::string*>(&x)

#define GL_CLAMP_TO_EDGE 0x812F
#pragma endregion Marcos


#pragma region Dummy
struct BotProfile_t;
struct CareerGame_t;
struct Locus_t;
struct Map_t;
struct ValveKeyValues;
struct Task_t;
struct Thumbnail_t;
#pragma endregion Dummy


#pragma region Enumerators
using BYTE = unsigned __int8;

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
	UNKNOWN						= 0,
	RENDERING					= 1 << 0,

	//REQ_UPDATE_GAME_INFO		= 1 << 1,
	//UPDATING_GAME_INFO		= 1 << 2,
	//GAME_INFO_READY			= 1 << 3,

	//REQ_UPDATE_MISSION_PACK	= 1 << 4,
	UPDATING_MISSION_PACK_INFO	= 1 << 5,
	//MISSION_PACK_READY		= 1 << 6,

	UPDATING_MAPS_INFO			= 1 << 7,
};

enum class Difficulty_e : BYTE
{
	EASY = 0,
	NORMAL,
	HARD,
	EXPERT,

	_LAST
};

enum Weapon_e : short
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
#pragma endregion Enumerators


#pragma region Type Alias
using BotProfiles_t = std::list<BotProfile_t>;
using CareerGames_t = std::unordered_map<Difficulty_e, CareerGame_t>;
using ConsoleCmd_t = std::string;
using CostAvailability_t = std::array<int, 5>;
using Directories_t = std::list<fs::path>;
using KeyValueSet_t = std::unordered_map<Difficulty_e, ValveKeyValues*>;
using Loci_t = std::list<Locus_t>;
using Maps_t = std::unordered_map<std::string, Map_t>;
using Name_t = std::string;
using Names_t = std::list<Name_t>;
using Resources_t = std::list<std::string>;
using SkinThumbnails_t = std::unordered_map<std::string, Thumbnail_t>;
using Tasks_t = std::list<Task_t>;
using TemplateNames_t = std::list<std::string>;
using Weapons_t = std::vector<std::string>;
#pragma endregion Type Alias


#include "ScopedEnumOperator.hpp"


#pragma region Global Constants
inline const char* g_rgszTaskNames[] =
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

inline const char* g_rgszWeaponNames[] =
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

inline const char* g_rgszDifficultyNames[] = { "EASY", "NORMAL", "HARD", "EXPERT" };
inline const wchar_t* g_rgwcsDifficultyNames[] = { L"Easy", L"Normal", L"Hard", L"Expert" };
inline const char* g_rgszCTSkinName[] = { nullptr, "urban", "gsg9", "sas", "gign", "spetsnaz", "ct_random" };
inline const char* g_rgszTSkinName[] = { nullptr, "terror", "leet", "arctic", "guerilla", "militia", "t_random" };

// Sadly this have to place after ScopedEnumOperator.hpp
// Or I could have put it in the enum declaration.
namespace Tasks
{
	constexpr auto REQ_COUNT = ~((1 << TaskType_e::killall) | (1 << TaskType_e::rescueall));
	constexpr auto REQ_WEAPON = (1 << TaskType_e::injurewith) | (1 << TaskType_e::killwith) | (1 << TaskType_e::headshotwith);
	constexpr auto SURVIVE = ~((1 << TaskType_e::killall) | (1 << TaskType_e::rescueall) | (1 << TaskType_e::defendhostages) | (1 << TaskType_e::hostagessurvive));
	constexpr auto INAROW = ~((1 << TaskType_e::killall) | (1 << TaskType_e::winfast) | (1 << TaskType_e::rescueall) | (1 << TaskType_e::defendhostages) | (1 << TaskType_e::hostagessurvive));
};
#pragma endregion Global Constants


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
	virtual ~Task_t() noexcept {}

	void Parse(const std::string& sz) noexcept;

	std::string ToString(void) const noexcept;

	// Modify and give reason for ill-formed.
	const char* SanityCheck(void) noexcept;

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

namespace CZFile	// None of these function requires "\\" before relative path (to the game dir).
{
	inline fs::path			m_HLPath;
	inline Directories_t	m_Directories;

	extern bool Update(void) noexcept;

	extern bool Exists(const std::string& szPath) noexcept;

	[[nodiscard]]
	extern FILE* Open(const char* pszPath, const char* pszMode) noexcept;
};

struct Map_t
{
	Map_t() noexcept {}
	virtual ~Map_t() noexcept {}

	void Initialize(const fs::path& hPath) noexcept;

	void CheckBasicFile(void) noexcept;

	Name_t			m_szName{ "Error - no name of this .bsp file." };	// .bsp ext name not included.
	Name_t			m_szSky{ "Error - no sky texture." };
	Resources_t		m_rgszResources{};
	Thumbnail_t		m_Thumbnail{};
	Thumbnail_t		m_WiderPreview{};
	bool			m_bBriefFileExists		: 1	{ false };	// The text to be shown in MOTD screen.
	bool			m_bDetailFileExists		: 1	{ false };	// texture detail file.
	bool			m_bNavFileExists		: 1	{ false };	// CZ bot nav mesh
	bool			m_bOverviewFileExists	: 1	{ false };
	bool			m_bThumbnailExists		: 1	{ false };	// Thumbnail image when selecting location.
	bool			m_bWiderPreviewExists	: 1	{ false };	// Preview image in preparation screen. (selecting teammates)
	char*			m_pszBriefString{ nullptr };
	fs::path		m_Path;
};

struct Locus_t
{
	Locus_t() noexcept {}
	Locus_t(ValveKeyValues* pkv) noexcept { Parse(pkv); }
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

	void Parse(ValveKeyValues* pkv) noexcept;

	[[nodiscard]]
	ValveKeyValues* Save(void) const noexcept;

	void Reset(void) noexcept;

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
	void Parse(ValveKeyValues* pkv) noexcept;

	void Reset(void) noexcept;

	[[nodiscard]]
	ValveKeyValues* Save(void) const noexcept;

	int m_iInitialPoints{ 2 };
	int m_iMatchWins{ 3 };
	int m_iMatchWinBy{ 2 };
	Names_t m_rgszCharacters{};
	CostAvailability_t m_rgiCostAvailability{ 1, 6, 10, 15, 99 };
	Loci_t m_Loci;
};

struct BotProfile_t
{
	BotProfile_t(void) noexcept {}
	BotProfile_t(const BotProfile_t& rhs) noexcept :
		m_szName(rhs.m_szName),
		m_rgszWpnPreference(rhs.m_rgszWpnPreference),
		m_bPrefersSilencer(rhs.m_bPrefersSilencer),
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
	virtual ~BotProfile_t() noexcept {}

	bool SaveAttrib(std::ofstream& hFile) noexcept;

	template<size_t iOffset>
	bool IsMemberInherited(void) const noexcept;

	template<size_t iOffset>
	bool IsMemberDefault(void) const noexcept;

	template<size_t iOffset>
	bool IsMemberTemplated(void) const noexcept;

	bool IsDefault(void) const noexcept;

	bool IsTemplate(void) const noexcept;

	// If this value is not the initialized value, then it is 'Valid'. But it still could be 'Insane'.
	template<size_t iOffset>
	bool AttribValid(void) const noexcept;

	// This function checks whether an attribute makes sense.
	// Return nullptr means value makes sense.
	template<size_t iOffset>
	const char* AttribSanity(bool bSelfOnly = false) const noexcept;

	template<size_t iOffset>
	decltype(auto) Get(const BotProfile_t** ppSource = nullptr) const noexcept;

	template<size_t iOffset>
	decltype(auto) GetSelf(void) const noexcept;

	const char* GetPreferedWpn(void) const noexcept;

	const Weapons_t& GetPreferedWpnList(void) const noexcept;

	template<size_t iOffset>
	static consteval const char* GetAttribIntro(void) noexcept;

	template<size_t iOffset>
	static consteval const char* GetAttribName(void) noexcept;

	template<size_t iOffset>
	static consteval bool AttribOptional(void) noexcept;

	// Deactivate a field.
	template<size_t iOffset>
	void Invalidate(void) noexcept;

	// Activate a field and give it a sane value.
	template<size_t iOffset>
	void Rationalize(void) noexcept;

	void Reset(bool bDeleteReferences = true) noexcept;

	// Different from Task_t::SanityCheck(), this one doesnot modify original value.
	// This is just a overall evaluation. The optional attributes will not sound the alarm.
	// This function is lack of name dup check.
	const char* SanityCheck(void) const noexcept;

	void Inherit(const BotProfile_t& Parent) noexcept;

	void Inherit(bool bShouldResetFirst = true, bool bCopyFromDefault = true) noexcept;

	Name_t m_szName{ "Error - No name"s };	// Not in attrib
	Weapons_t m_rgszWpnPreference{};
	bool m_bPrefersSilencer{ false };	// Not editable, not an attribute.
	float m_flAttackDelay{ -1 };
	float m_flReactionTime{ -1 };
	int m_bitsDifficulty{ 0 };
	int m_iAggression{ -1 };
	int m_iCost{ -1 };
	int m_iSkill{ -1 };
	int m_iTeamwork{ -1 };
	int m_iVoicePitch{ -1 };
	TemplateNames_t m_rgszRefTemplates{};	// Not in attrib
	std::string m_szSkin{ "" };
	std::string m_szTeam{ "" };
	std::string m_szVoiceBank{ "" };

	static inline constexpr const char* m_pszNameIntro = "The name of this character.\nYou may not include any non-ASCI character or spaces in here.";
	static inline constexpr const char* m_pszWeaponPreferenceIntro = "Value: \"none\" or a buy alias such as \"m4a1\"\nDescription: Defines the bot's weapon preference.A bot can have many WeaponPreference\ndefinitions in a row, specifying a prioritized list(earlier ones are favorite over later ones) of\nweapons the bot will try to buy or pick up from the ground.A preference of “none” will cause\nthe bot to buy a random weapon.";
	static inline constexpr const char* m_pszAttackDelayIntro = "After a bot has become aware of an enemy,\nthis duration must also elapse before it will begin firing upon its victim.";
	static inline constexpr const char* m_pszReactionTimeIntro = "Determines the reaction time of a bot. A bot's \"reaction time\" is the delay between when a visual\nor audio event occurs and the bot becomes \"aware\" of it, and can begin to act upon it.\nThis simulates the time it takes a human to process incoming stimuli and become\"conscious\" of it.";
	static inline constexpr const char* m_pszDifficultyIntro = "Defines the difficulty categories where this bot is used.";
	static inline constexpr const char* m_pszAggressionIntro = "Determines how aggressively a bot behaves. High aggression bots pay less attention to \"danger\"\n(ie: where teammates have died previously), are more likely to rush, and less likely to retreat.\nLow aggression bots are just the opposite.";
	static inline constexpr const char* m_pszCostIntro = "The point you need to spend in order to hire it.";
	static inline constexpr const char* m_pszSkillIntro = "Defines the overall \"skill\" of the bot. Low skill bots have terrible aim and don’t look around very\nmuch, whereas high skill bots can have extremely good aim, try to check as many corners\nand hiding spots as they can, and know subtle things like using the knife to run faster, switching\nto the pistol when out of ammo, and so on.";
	static inline constexpr const char* m_pszTeamworkIntro = "Defines how cooperative and \"team oriented\" the bot is. High teamwork bots are more likely\nto obey radio commands and stay with their teammates. Low teamwork bots tend to \"go rogue\"\nand do their own thing.";
	static inline constexpr const char* m_pszVoicePitchIntro = "Defines the pitch shift value this bot will use for its \"chatter\". Lower values create lower pitched voices.";
	static inline constexpr const char* m_pszSkinIntro = "Value: 0-5 or <skin name>\nDescription: Defines which “skin” to select when the bot joins the game.Values 1 through 5 map\nto the associated skins on the player menu in game.A value of 0 selects a skin at random.If a\n<skin name> is given, the skin must have been previously defined using a Skin data block.\nSkin data blocks can refer to custom skins other than the default CZ skins.Custom skins should be\nplaced in czero\\models\\player\\<skin name>\\<skin name>.mdl.";
	static inline constexpr const char* m_pszTeamIntro = "Value: [CT] [T] [ANY]\nDescription: The team to which this character prefer to join.";
	static inline constexpr const char* m_pszVoicebankIntro = "Use a customized voicebank instead of default \"BotChatter.db\".";
};

namespace Reflection::BotProfile
{
	inline constexpr auto NAME = offsetof(BotProfile_t, m_szName);
	inline constexpr auto WEAPON_PREFERENCE = offsetof(BotProfile_t, m_rgszWpnPreference);
	inline constexpr auto ATTACK_DELAY = offsetof(BotProfile_t, m_flAttackDelay);
	inline constexpr auto REACTION_TIME = offsetof(BotProfile_t, m_flReactionTime);
	inline constexpr auto DIFFICULTY = offsetof(BotProfile_t, m_bitsDifficulty);
	inline constexpr auto AGGRESSION = offsetof(BotProfile_t, m_iAggression);
	inline constexpr auto COST = offsetof(BotProfile_t, m_iCost);
	inline constexpr auto SKILL = offsetof(BotProfile_t, m_iSkill);
	inline constexpr auto TEAMWORK = offsetof(BotProfile_t, m_iTeamwork);
	inline constexpr auto VOICEPITCH = offsetof(BotProfile_t, m_iVoicePitch);
	inline constexpr auto SKIN = offsetof(BotProfile_t, m_szSkin);
	inline constexpr auto TEAM = offsetof(BotProfile_t, m_szTeam);
	inline constexpr auto VOICEBANK = offsetof(BotProfile_t, m_szVoiceBank);

	template<size_t iOffset>
	struct _impl_TypeOf
	{
		using type =
			std::conditional_t<iOffset == NAME, decltype(BotProfile_t::m_szName),
			std::conditional_t<iOffset == WEAPON_PREFERENCE, decltype(BotProfile_t::m_rgszWpnPreference),
			std::conditional_t<iOffset == ATTACK_DELAY, decltype(BotProfile_t::m_flAttackDelay),
			std::conditional_t<iOffset == REACTION_TIME, decltype(BotProfile_t::m_flReactionTime),
			std::conditional_t<iOffset == DIFFICULTY, decltype(BotProfile_t::m_bitsDifficulty),
			std::conditional_t<iOffset == AGGRESSION, decltype(BotProfile_t::m_iAggression),
			std::conditional_t<iOffset == COST, decltype(BotProfile_t::m_iCost),
			std::conditional_t<iOffset == SKILL, decltype(BotProfile_t::m_iSkill),
			std::conditional_t<iOffset == TEAMWORK, decltype(BotProfile_t::m_iTeamwork),
			std::conditional_t<iOffset == VOICEPITCH, decltype(BotProfile_t::m_iVoicePitch),
			std::conditional_t<iOffset == SKIN, decltype(BotProfile_t::m_szSkin),
			std::conditional_t<iOffset == TEAM, decltype(BotProfile_t::m_szTeam),
			std::conditional_t<iOffset == VOICEBANK, decltype(BotProfile_t::m_szVoiceBank),

			void>>>>>>>>>>>>>;
	};

	template<size_t iOffset>
	using TypeOf = _impl_TypeOf<iOffset>::type;
};

namespace BotProfileMgr
{
	// Real Declration
	inline BotProfiles_t m_Profiles{};
	inline BotProfile_t m_Default{};	// Have to be extern here. The class isn't exist yet.
	inline std::unordered_map<std::string, BotProfile_t> m_Templates{};
	inline std::unordered_map<std::string, std::string> m_Skins{};
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

	extern void Clear(void) noexcept;

	extern void RemoveNoneInWpnPref(Weapons_t* p) noexcept;

	extern void GenerateSkinsFromProfiles(void) noexcept;

	extern bool ParseWithInheritance(const fs::path& hPath) noexcept;	// Assuming this file IS ACTUALLY exists.

	extern bool Parse(const fs::path& hPath) noexcept;

	extern bool Save(const fs::path& hPath) noexcept;

	extern void LoadSkinThumbnails(void) noexcept;

	extern size_t TemplateNameCount(const std::string& szName) noexcept;	// Compare with actual name instead of entry name.

	extern size_t ProfileNameCount(const std::string& szName) noexcept;

	// Deduce the result after a set of succession.
	template<size_t iOffset>
	extern decltype(auto) Deduce(const TemplateNames_t& rgszTemplates, bool bIncludeDefault = true, const BotProfile_t** ppSource = nullptr) noexcept;

	// Find a profile.
	extern BotProfile_t* Find(const std::string& szName) noexcept;

	extern bool TemplateOccupied(const BotProfile_t& Tpl) noexcept;
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
	inline std::mutex		Mutex;

	extern void CompileFileList(Files_t& rgFiles, const fs::path& hFolder, bool bCheck = true) noexcept;

	extern void CompileFileListOfTurtleRockCounterTerrorist(Files_t& rgFiles, const fs::path& hGameFolder) noexcept;

	// Load the folder as if it were a mission pack.
	extern void LoadFolder(const fs::path& hFolder) noexcept;

	extern void Save(const fs::path& hFolder = Folder) noexcept;

	// Is this character enrolled as our potential teammate?
	extern bool IsTeammate(const Name_t& szName) noexcept;

	// Is this character is enlisted as our enemy in any of our mission locations?
	extern bool IsEnemy(const Name_t& szName) noexcept;
};

namespace Maps	// This is the game map instead of career quest 'Locus_t'!
{
	inline std::mutex Mutex;

	extern void Load(void) noexcept;
	extern std::string ListResources(const Map_t& Map) noexcept;
};

namespace Gui
{
	enum EditorResult_e : short
	{
		UNTOUCHED = 0,
		DISCARD,
		SAVED
	};

	template<typename... Tys>
	requires all_same_as<WeaponSelMask_t, Tys...>
	const char* fnWeaponMenu(const Tys&... rgbMask) noexcept
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
	}

	template<bool bDynamic = false>
	inline void fnErrorDialog(const char* pszTitle, const char* pszContent) noexcept
	{
		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (ImGui::BeginPopupModal(pszTitle, nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize))
		{
			const auto iModalContentWidth = ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x;

			if constexpr (bDynamic)
				ImGui::Text(pszContent);
			else
				ImGui::TextUnformatted(pszContent);

			ImGui::NewLine();

			ImGui::SetCursorPosX(iModalContentWidth / 2 - 60);
			if (ImGui::Button("OK", ImVec2(120, 24)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}

	extern void CheckDifficultySync(void) noexcept;
};

namespace Gui::BotProfile
{
	inline enum : short
	{
		TAB_UNSET = -1,
		TAB_DEFAULT,
		TAB_TEMPLATES,
		TAB_CHARACTERS
	}
	m_iSetTab;

	inline ImGuiTextFilter Filter;	// Call Build() after manually edits it.

	extern void DrawWindow(void) noexcept;
	extern void Summary(const BotProfile_t* pProfile) noexcept;	// Only value from self will be included.
};

namespace Gui::Locations
{
	inline ::Difficulty_e	LastBrowsing{ Difficulty_e::EASY };
	inline ::Difficulty_e	CurBrowsing{ Difficulty_e::EASY };

	extern EditorResult_e EditingDialog(const char* pszTitle, Locus_t* pLocus) noexcept;
	extern void DrawWindow(void) noexcept;
};

namespace Gui::MissionPack
{
	inline ::Difficulty_e	LastBrowsing{ Difficulty_e::EASY };
	inline ::Difficulty_e	CurBrowsing{ Difficulty_e::EASY };
};

namespace Gui::Maps
{
	inline ImGuiTextFilter Filter;	// Used in MapsWindow(). Call Build() after manually edits it.

	extern void DrawWindow(void) noexcept;
	extern void SelectionInterface(void (*pfnPostAddingItem)(const Map_t& CurMap) = nullptr, std::string* pszSelectedMap = nullptr) noexcept;
};

#pragma region Global Variables
inline BotProfiles_t&		g_BotProfiles = BotProfileMgr::m_Profiles;
inline GLFWwindow*			g_hGLFWWindow = nullptr;
inline Maps_t				g_Maps;
inline bool					g_bShowDebugWindow = false, g_bCurGamePathValid = false, g_bShowConfigWindow = true, g_bShowLociWindow = false, g_bShowCampaignWindow = false, g_bShowMapsWindow = false, g_bShowBotsWindow = false;
inline fs::path				g_GamePath;
inline std::atomic<int>		g_bitsAsyncStatus = Async_e::UNKNOWN;
inline std::string			g_szInputGamePath;
inline ValveKeyValues*		g_Config = nullptr;
inline Difficulty_e			g_iSetDifficulty = Difficulty_e::_LAST;
#pragma endregion Global Variables

#pragma region Templated Functions
template<size_t iOffset>
inline bool BotProfile_t::IsMemberInherited(void) const noexcept	// #HACKHACK undefined behavior involved.
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;

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
			if (*pRefValue < 0 || (!*pRefValue && iOffset == Reflection::BotProfile::DIFFICULTY))
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

template<size_t iOffset>
inline bool BotProfile_t::IsMemberDefault(void) const noexcept	// #HACKHACK undefined behavior involved.
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;

	if (this == &BotProfileMgr::m_Default)
		return true;

	T* pMyValue = reinterpret_cast<T*>(size_t(this) + iOffset);
	T* pDefValue = reinterpret_cast<T*>(size_t(&BotProfileMgr::m_Default) + iOffset);
	return (*pDefValue == *pMyValue);
}

template<size_t iOffset>
inline bool BotProfile_t::IsMemberTemplated(void) const noexcept	// #HACKHACK undefined behavior involved.
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;

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
			if (*pRefValue < 0 || (!*pRefValue && iOffset == Reflection::BotProfile::DIFFICULTY))
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

	return false;	// If none of template value can be located, sure it is wrote either by user or Default.
}

template<size_t iOffset>
inline bool BotProfile_t::AttribValid(void) const noexcept
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;

	const T* pValue = reinterpret_cast<const T*>(size_t(this) + iOffset);

	if constexpr (std::is_same_v<T, Weapons_t>)
	{
		return !pValue->empty();
	}
	else if constexpr (std::is_same_v<T, std::string>)	// Does my reference has no value for themself?
	{
		return !pValue->empty() && !pValue->starts_with("Error - "s);
	}
	else
	{
		if (iOffset == Reflection::BotProfile::DIFFICULTY)
			return *pValue > 0;

		return *pValue >= 0;
	}
}

template<size_t iOffset>
inline const char* BotProfile_t::AttribSanity(bool bSelfOnly) const noexcept
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;

	const BotProfile_t* pSource = nullptr;
	const T* pValue = bSelfOnly ? GetSelf<iOffset>() : Get<iOffset>(&pSource);

	switch (iOffset)
	{
	case offsetof(BotProfile_t, m_szName):
		if constexpr (std::is_same_v<T, Name_t>)
		{
			// Do not compare address with 'this' pointer.
			// While we are editing a copy of some profile, doing such thing could be a problem.

			unsigned iCount = 0;

			if (IsTemplate())
			{
				for (const auto& [szName, Template] : BotProfileMgr::m_Templates)
				{
					if (Template.m_szName == *pValue)
						iCount++;

					if (iCount > 1)
						return "Duplicated name.";
				}
			}
			else if (!IsTemplate() && !IsDefault())
			{
				for (const auto& Character : BotProfileMgr::m_Profiles)
				{
					if (*Character.Get<Reflection::BotProfile::NAME>() == *pValue)
						iCount++;

					if (iCount > 1)
						return "Duplicated name.";
				}
			}
		}
		[[fallthrough]];

	case offsetof(BotProfile_t, m_szSkin):
	case offsetof(BotProfile_t, m_szVoiceBank):
		if constexpr (std::is_same_v<T, std::string>)
		{
			if (pValue->empty())
				return "Empty field.";
			else if (pValue->starts_with("Error - "))
				return "Field with default value.";
			else if (pValue->find_first_of(" \f\n\r\t\v") != std::string::npos)
				return "Field contains space(s).";
		}
		break;

	case offsetof(BotProfile_t, m_szTeam):
		if constexpr (std::is_same_v<T, std::string>)
		{
			if (_stricmp(pValue->c_str(), "CT") && _stricmp(pValue->c_str(), "T") && _stricmp(pValue->c_str(), "ANY"))
				return "String must be either 'CT', 'T' or 'ANY'.";
		}
		break;

	case offsetof(BotProfile_t, m_rgszWpnPreference):
		if constexpr (std::is_same_v<T, Weapons_t>)
		{
			if (m_rgszWpnPreference.empty())
				return "Weapon preference list should never be empty!";

			bool bRet = 
				(m_rgszWpnPreference.size() == 1 && !_stricmp(m_rgszWpnPreference.front().c_str(), "none"))
				|| std::find_if(m_rgszWpnPreference.begin(), m_rgszWpnPreference.end(), [](const std::string& szWpn) { return !_stricmp(szWpn.c_str(), "none"); }) == m_rgszWpnPreference.end();

			if (!bRet)
				return "Placeholder value 'none' should not be place along with weapon names.";
		}
		break;

	case offsetof(BotProfile_t, m_flAttackDelay):
	case offsetof(BotProfile_t, m_flReactionTime):
		if constexpr (std::floating_point<T>)
		{
			if (*pValue < 0.0f)
				return "Value must be greater than or equal to 0.";
		}
		break;

	case offsetof(BotProfile_t, m_bitsDifficulty):
	case offsetof(BotProfile_t, m_iVoicePitch):
		if constexpr (std::integral<T>)
		{
			if (*pValue <= 0)
				return "Value must be greater than 0.";
		}
		break;

	case offsetof(BotProfile_t, m_iAggression):
	case offsetof(BotProfile_t, m_iSkill):
	case offsetof(BotProfile_t, m_iTeamwork):
		if constexpr (std::integral<T>)
		{
			if (*pValue < 0)
				return "Value must be greater than or equal to 0.";
			else if (*pValue > 100)
				return "Value must be smaller than or equal to 100.";
		}
		break;

	case offsetof(BotProfile_t, m_iCost):
		if constexpr (std::integral<T>)
		{
			if (*pValue < 1 || *pValue > 5)
				return "Value must be 1, 2, 3, 4 or 5.";
		}
		break;

	default:
		std::cout << "Error in calling BotProfile_t::AttribSanity() with reflection offset " << iOffset << ": Not a proper member.\n";
		return nullptr;
	}

	return nullptr;
}

template<size_t iOffset>
inline decltype(auto) BotProfile_t::Get(const BotProfile_t** ppSource) const noexcept	// #HACKHACK undefined behavior involved.
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;

	if (ppSource)
		*ppSource = this;

	if (AttribValid<iOffset>())
		return reinterpret_cast<T*>(size_t(this) + iOffset);

	for (auto iter = m_rgszRefTemplates.rbegin(); iter != m_rgszRefTemplates.rend(); ++iter)	// From the last template to the first template.
	{
		BotProfile_t* pRef = &BotProfileMgr::m_Templates[*iter];

		if (ppSource)
			*ppSource = pRef;

		if (pRef->AttribValid<iOffset>())
			return reinterpret_cast<T*>(size_t(pRef) + iOffset);
	}

	if (ppSource)
		*ppSource = &BotProfileMgr::m_Default;

	// Don't forget 'Default' template. The last thing we can turn to.
	return reinterpret_cast<T*>(size_t(&BotProfileMgr::m_Default) + iOffset);	// #POTENTIAL_BUG Unknow reason: there will be two set of address of m_Default. And m_szSkin contains different value within.
}

template<size_t iOffset>
inline decltype(auto) BotProfile_t::GetSelf(void) const noexcept
{
	return reinterpret_cast<Reflection::BotProfile::TypeOf<iOffset>*>(size_t(this) + iOffset);
}

template<size_t iOffset>
inline consteval const char* BotProfile_t::GetAttribIntro(void) noexcept
{
	switch (iOffset)
	{
	case Reflection::BotProfile::NAME:
		return m_pszNameIntro;
	case Reflection::BotProfile::WEAPON_PREFERENCE:
		return m_pszWeaponPreferenceIntro;
	case Reflection::BotProfile::ATTACK_DELAY:
		return m_pszAttackDelayIntro;
	case Reflection::BotProfile::REACTION_TIME:
		return m_pszReactionTimeIntro;
	case Reflection::BotProfile::DIFFICULTY:
		return m_pszDifficultyIntro;
	case Reflection::BotProfile::AGGRESSION:
		return m_pszAggressionIntro;
	case Reflection::BotProfile::COST:
		return m_pszCostIntro;
	case Reflection::BotProfile::SKILL:
		return m_pszSkillIntro;
	case Reflection::BotProfile::TEAMWORK:
		return m_pszTeamworkIntro;
	case Reflection::BotProfile::VOICEPITCH:
		return m_pszVoicePitchIntro;
	case Reflection::BotProfile::SKIN:
		return m_pszSkinIntro;
	case Reflection::BotProfile::TEAM:
		return m_pszTeamIntro;
	case Reflection::BotProfile::VOICEBANK:
		return m_pszVoicebankIntro;
	default:
		std::cout << "Error in calling BotProfile_t::GetAttribIntro() with reflection offset " << iOffset << ": Not a proper member.\n";
		return nullptr;
	}
}

template<size_t iOffset>
inline consteval const char* BotProfile_t::GetAttribName(void) noexcept
{
	switch (iOffset)
	{
	case Reflection::BotProfile::NAME:
		return "Name";
	case Reflection::BotProfile::WEAPON_PREFERENCE:
		return "Weapon Preference";
	case Reflection::BotProfile::ATTACK_DELAY:
		return "Attack Delay";
	case Reflection::BotProfile::REACTION_TIME:
		return "Reaction Time";
	case Reflection::BotProfile::DIFFICULTY:
		return "Difficulty";
	case Reflection::BotProfile::AGGRESSION:
		return "Aggression";
	case Reflection::BotProfile::COST:
		return "Cost";
	case Reflection::BotProfile::SKILL:
		return "Skill";
	case Reflection::BotProfile::TEAMWORK:
		return "Teamwork";
	case Reflection::BotProfile::VOICEPITCH:
		return "Voice Pitch";
	case Reflection::BotProfile::SKIN:
		return "Skin";
	case Reflection::BotProfile::TEAM:
		return "Preferred Team";
	case Reflection::BotProfile::VOICEBANK:
		return "Voicebank";
	default:
		std::cout << "Error in calling BotProfile_t::GetAttribName() with reflection offset " << iOffset << ": Not a proper member.\n";
		return nullptr;
	}
}

template<size_t iOffset>
inline consteval bool BotProfile_t::AttribOptional(void) noexcept
{
	using namespace Reflection::BotProfile;

	switch (iOffset)
	{
	case NAME:
	case WEAPON_PREFERENCE:
	case ATTACK_DELAY:
	case REACTION_TIME:
	case AGGRESSION:
	case COST:
	case SKILL:
	case TEAMWORK:
	case VOICEPITCH:
		return false;

	case DIFFICULTY:
	case VOICEBANK:
	case TEAM:
	case SKIN:
		return true;

	default:
		std::cout << "Error in calling BotProfile_t::AttribOptional() with reflection offset " << iOffset << ": Not a proper member.\n";
		return true;
	}
}

template<size_t iOffset>
inline void BotProfile_t::Invalidate(void) noexcept
{
	using T = Reflection::BotProfile::TypeOf<iOffset>;
	T* pValue = GetSelf<iOffset>();

	if constexpr (std::is_same_v<T, Weapons_t>)
	{
		pValue->clear();
	}
	else if constexpr (std::is_same_v<T, std::string>)	// Does my reference has no value for themself?
	{
		*pValue = "Error - Using inherited value.";
	}
	else
	{
		if constexpr (iOffset == Reflection::BotProfile::DIFFICULTY)
			*pValue = 0;
		else
			*pValue = static_cast<T>(-1);
	}
}

template<size_t iOffset>
inline void BotProfile_t::Rationalize(void) noexcept
{
	// The value is already making sense. Skip it.
	if (!AttribSanity<iOffset>(true))
		return;

	using namespace Reflection::BotProfile;
	using T = TypeOf<iOffset>;

	T* pValue = GetSelf<iOffset>();

	switch (iOffset)
	{
	case offsetof(BotProfile_t, m_szName):
	case offsetof(BotProfile_t, m_szSkin):
	case offsetof(BotProfile_t, m_szVoiceBank):
		if constexpr (std::is_same_v<T, std::string>)
			*pValue = "Default string. Change me!";

		break;

	case offsetof(BotProfile_t, m_szTeam):
		if constexpr (std::is_same_v<T, std::string>)
			*pValue = "ANY";

		break;

	case offsetof(BotProfile_t, m_rgszWpnPreference):
		if constexpr (std::is_same_v<T, Weapons_t>)
			*pValue = { "none" };

		break;

	case offsetof(BotProfile_t, m_flAttackDelay):
	case offsetof(BotProfile_t, m_flReactionTime):
		if constexpr (std::floating_point<T>)
			*pValue = 0.0f;

		break;

	case offsetof(BotProfile_t, m_bitsDifficulty):
		if constexpr (std::integral<T>)
			*pValue = (1 << Difficulty_e::EASY);

		break;

	case offsetof(BotProfile_t, m_iVoicePitch):
	case offsetof(BotProfile_t, m_iAggression):
	case offsetof(BotProfile_t, m_iSkill):
	case offsetof(BotProfile_t, m_iTeamwork):
		if constexpr (std::integral<T>)
			*pValue = 50;

		break;

	case offsetof(BotProfile_t, m_iCost):
		if constexpr (std::integral<T>)
			*pValue = 1;

		break;

	default:
		std::cout << "Error in calling BotProfile_t::Rationalize() with reflection offset " << iOffset << ": Not a proper member.\n";
	}
}

template<size_t iOffset>
inline decltype(auto) BotProfileMgr::Deduce(const TemplateNames_t& rgszTemplates, bool bIncludeDefault, const BotProfile_t** ppSource) noexcept
{
	using namespace Reflection::BotProfile;

	using T = TypeOf<iOffset>;

	if (ppSource)
		*ppSource = nullptr;

	for (auto iter = rgszTemplates.rbegin(); iter != rgszTemplates.rend(); ++iter)	// From the last template to the first template.
	{
		BotProfile_t* pRef = &BotProfileMgr::m_Templates[*iter];

		if (ppSource)
			*ppSource = pRef;

		if (pRef->AttribValid<iOffset>())
			return reinterpret_cast<T*>(size_t(pRef) + iOffset);
	}

	if (!bIncludeDefault)
		return (T*)nullptr;

	if (ppSource)
		*ppSource = &BotProfileMgr::m_Default;

	// Don't forget 'Default' template. The last thing we can turn to.
	return reinterpret_cast<T*>(size_t(&BotProfileMgr::m_Default) + iOffset);
}

#pragma endregion Templated Functions
