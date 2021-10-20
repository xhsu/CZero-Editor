// CZeroEditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

// C++
#include <array>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <list>
#include <string>
#include <thread>

// C
#include <cstdio>

// ImGUI
#include "imgui.h"
#include "misc/cpp/imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// OpenGL
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

import UtlKeyValues;
import UtlString;

namespace fs = std::filesystem;

using namespace std::literals;
using namespace std::string_literals;
using namespace std::literals::string_literals;


#define IMGUI_GREEN	ImVec4(0, 153.0f / 255.0f, 0, 1)
#define IMGUI_RED	ImVec4(204.0f / 255.0f, 0, 0, 1)

#define CAST_TO_CHAR	reinterpret_cast<const char*>
#define CAST_TO_UTF8	reinterpret_cast<const char8_t*>
#define CAST_TO_U8S(x)	*reinterpret_cast<std::u8string*>(&x)
#define CAST_TO_STR(x)	*reinterpret_cast<std::string*>(&x)

// Dummy
struct Task_t;
struct Map_t;

using ConsoleCmd_t = std::string;
using CostAvailability_t = std::array<int, 5>;
using Maps_t = std::list<Map_t>;
using Name_t = std::string;
using Names_t = std::list<Name_t>;
using Tasks_t = std::list<Task_t>;

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

enum class TaskType_e : unsigned __int8
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

template<typename E>
concept GoodEnum = requires(E e)
{
	{E::_LAST};
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

template<GoodEnum Enumerator_t>
constexpr std::strong_ordering operator<=> (const Enumerator_t& lhs, std::integral auto rhs)
{
	using T = std::decay_t<decltype(rhs)>;

	return static_cast<T>(lhs) <=> rhs;
}

enum class Weapon_e : unsigned __int8
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

struct Task_t
{
	Task_t() {}
	Task_t(const std::string& sz) { Parse(sz); }
	virtual ~Task_t() {}

	void Parse(const std::string& sz)
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

	std::string ToString(void)
	{
		std::string ret = g_rgszTaskNames[(unsigned)m_iType] + " "s;

		if (m_iCount)
			ret += std::to_string(m_iCount) + ' ';

		if (m_iWeapon != Weapon_e::_LAST)
			ret += g_rgszWeaponNames[(unsigned)m_iWeapon] + " "s;

		if (m_bSurvive)
			ret += "survive"s + ' ';

		if (m_bInARow)
			ret += "inarow"s + ' ';

		return ret;
	}

	TaskType_e m_iType{ TaskType_e::kill };
	int m_iCount{ 0 };
	Weapon_e m_iWeapon{ Weapon_e::_LAST };
	bool m_bSurvive{ false };
	bool m_bInARow{ false };
};

struct Map_t
{
	Map_t() {}
	Map_t(NewKeyValues* pkv) { Parse(pkv); }
	virtual ~Map_t() {}

	void Parse(NewKeyValues* pkv)
	{
		m_rgszBots.clear();
		m_Tasks.clear();

		m_szMapName = pkv->GetName();

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

	Name_t			m_szMapName{ ""s };
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
		m_rgszCharacters.clear();
		m_Maps.clear();

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

	int m_iInitialPoints{ 2 };
	int m_iMatchWins{ 3 };
	int m_iMatchWinBy{ 2 };
	Names_t m_rgszCharacters{};
	CostAvailability_t m_rgiCostAvailability{ 1, 6, 10, 15, 99 };
	Maps_t m_Maps;
};


inline GLFWwindow*			g_hGLFWWindow = nullptr;
inline bool					g_bShowDebugWindow = false;
inline std::string			g_szInputPath;
inline fs::path				g_InputPath;
inline NewKeyValues*		g_pKV = nullptr;
inline std::atomic<bool>	g_bReady = false;
inline CareerGame_t			g_CareerGame;


void ListKeyValue(NewKeyValues* pkv)
{
	if (!pkv)
		return;

	if (!ImGui::TreeNode(pkv->GetName()))
		return;

	NewKeyValues* pSub = pkv->GetFirstValue();
	while (pSub)
	{
		ImGui::BulletText("(Value) %s: %s", pSub->GetName(), pSub->GetString());
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

void OnValidatingPath(void)
{
	g_bReady = false;

	if (g_pKV)
	{
		g_pKV->deleteThis();	// #MEM_FREED
		g_pKV = nullptr;
	}

	if (!g_pKV)
		g_pKV = new NewKeyValues("KV");	// #MEM_ALLOC

	g_pKV->LoadFromFile(g_szInputPath.c_str());

	g_CareerGame.Parse(g_pKV);

	g_bReady = true;
}

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
	g_hGLFWWindow = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
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
		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (ImGui::IsKeyPressed(0x60))
			g_bShowDebugWindow = !g_bShowDebugWindow;

		if (g_bShowDebugWindow)
			ImGui::ShowDemoWindow(&g_bShowDebugWindow);
#pragma endregion Debug Window

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Save", "Ctrl+S") && g_pKV)
					g_pKV->SaveToFile("Test.txt");

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		if (ImGui::Begin("Config"))
		{
			if (ImGui::InputText("Path", &g_szInputPath) && fs::exists(g_szInputPath))
			{
				g_InputPath = g_szInputPath;

				g_bReady = false;
				std::thread t(OnValidatingPath);
				t.detach();
			}

			if (!g_bReady)
				goto LAB_PATH_WINDOW_SKIP;

			ListKeyValue(g_pKV);
		}

	LAB_PATH_WINDOW_SKIP:;
		ImGui::End();	// Path

		// Rendering
		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(g_hGLFWWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(0.45f, 0.55f, 0.60f, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(g_hGLFWWindow);
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
