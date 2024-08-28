#include "DirectX11.h"
#include <vector>
#include "main.h"
#include <iostream>
#include <numbers>
#include <cmath>
#include <windows.h>
#include <thread>
#include <tlhelp32.h>
#include "vector.hpp";
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
struct pointer {
	uintptr_t a;
	std::vector<uintptr_t> b;
	//if u need more add c d e f g etc...
};
namespace offset {
	constexpr std::ptrdiff_t dwViewMatrix = 0x19CA480;
	constexpr std::ptrdiff_t dwEntityList = 0x19684F8;
	constexpr std::ptrdiff_t dwLocalPlayerPawn = 0x17D37F0;
	constexpr std::ptrdiff_t dwViewAngles = 0x19D98E8;
	constexpr std::ptrdiff_t m_vecVelocity = 0x3E0;
	constexpr std::ptrdiff_t m_vecViewOffset = 0xC50;
	constexpr std::ptrdiff_t m_hPawn = 0x60C;
	constexpr std::ptrdiff_t m_hPlayerPawn = 0x7EC;
	constexpr std::ptrdiff_t m_iHealth = 0x324;
	constexpr std::ptrdiff_t m_iTeamNum = 0x3C3;
	constexpr std::ptrdiff_t m_vOldOrigin = 0x1274;
	// Ask on unknowncheats for these offsets or find on the internet
}
//client.dll
pointer GoldEsp = { 0x19B7FA0, { 0x40 } }; //r_aoproxy_show 1
pointer BlinkLess = { 0x19C5DC0, { 0x4F0 } }; //r_extra_render_frames 1
pointer NumberEsp = { 0x19CD3E8, { 0x40 } }; //cl_player_proximity_debug 1
pointer NoSkybox =  { 0x19D9E78, { 0x90 } }; //r_drawskybox 0

//scenesystem.dll
pointer H_nolights = { 0x597F70, { 0x40 } }; //HIDDEN CVAR lb_enable_lights
// HOW TO DUMP POINTERS
	// Go to bots game, open console and type sv_cheats 1. Copy the command u want and change to 1.
	// Then open cheat engine and if val is 1 scan for 1 (byte) then 0 after u change it, then 1 then 0 and until u get good results
	// U can change value directly in cheat engine to know if its right. If changes are visible in game, its right.
	// After u got right value click Generate Pointer Map, once its done generating Pointer Scan for this Address
	// Select Use Saved Pointermap, Set Maximum Offset Value to 65535 and Max Level to 1
	// Click Ok and search for values. You will get a few maybe 10-500
	// Look for pointers that start with "client.dll" These should be all good
	// Double click on the pointer u think is good and restart game, then select process again
	// If value on pointer is 0 its probably good. Go in game then change to 1, and u will know for 100%
	// After that click on the address, last section will be "client.dll"+019C5DC0 for example
	// And above that u have 4F0 for example. Then that means, pointer is 019C5DC0+4F0
	// You can add that already or do 0x19C5DC0+0x4F0, both are valid. Congrats, u have your pointer! Continue process to get all.



uintptr_t clientbase = (DWORD64)GetModuleHandle("client.dll");
uintptr_t cs2base = (DWORD64)GetModuleHandle("cs2.exe"); //probably wont need it
uintptr_t scenebase = (DWORD64)GetModuleHandle("scenesystem.dll"); //probably wont need it



class C_CSPlayerPawn
{
public:
	int health, team;

	Vector Position;

	uintptr_t pCSPlayerPawn;
};

inline C_CSPlayerPawn CCSPlayerPawn;

class Reader
{
public:
	uintptr_t client = clientbase;

	std::vector<C_CSPlayerPawn> playerList;

	void ThreadLoop();
private:
	void FilterPlayers();
};

inline Reader reader;
class Aimbot
{
public:
	void doAimbot();
	void doRageAimbot(); //we are using CallRage() instead but u can use this if u really want
private:
	Vector findClosest(const std::vector<Vector> playerPositions);
	void MoveMouseToPlayer(Vector position, float smoothingFactor);
};
inline Aimbot aimbot;


void Reader::ThreadLoop()
{
	FilterPlayers();
}


float smoothingFactor = 4.5f;


float BoneIndex = 63.0f;
int HitBone = 0;

//63.0f head
//50.0f chest
//35.5f legs
//10.0f foot
// all of this is a shit approximation which doesnt really make sense but "works"





void Aimbot::doAimbot()
{
	auto view_matrix = *reinterpret_cast<view_matrix_t*>(reader.client + offset::dwViewMatrix);

	std::vector<Vector> playerPositions;

	playerPositions.clear();

	for (const auto& player : reader.playerList)
	{

		Vector playerPosition = *reinterpret_cast<Vector*>(player.pCSPlayerPawn + offset::m_vOldOrigin);
		Vector headPos = { playerPosition.x += 0.0, playerPosition.y += 0.0, playerPosition.z += BoneIndex };
		//we are using relative position here which is a really bad method
		// when player is ducking or using anti aim, will not aim properly. 
		// as such, use entityeyepos if u want only head. if you want other bones boneindex will work fine
		// can do both easily since we are internal

		Vector f, h;

		if (Vector::world_to_screen(view_matrix, playerPosition, f) &&
			Vector::world_to_screen(view_matrix, headPos, h))
		{
			playerPositions.push_back(h);
		}
	}

	if (GetAsyncKeyState(VK_SHIFT))
	{
		auto closest_player = findClosest(playerPositions);

		MoveMouseToPlayer(closest_player, smoothingFactor);
	}
}
float calculateDistanceSquared(const vec3& a, const vec3& b) {
	return std::pow(b.x - a.x, 2) + std::pow(b.y - a.y, 2) + std::pow(b.z - a.z, 2);
}
vec3 FindClosest(const vec3& localEyePos, const std::vector<vec3>& playerPositions) {
	if (playerPositions.empty()) {
		printf("Empty");
		return { 0.0f, 0.0f, 0.0f };
	}

	float lowestDistance = 10000.0f;
	int index = -1;

	for (int i = 0; i < playerPositions.size(); i++) {
		float distance = calculateDistanceSquared(localEyePos, playerPositions[i]);

		if (distance < lowestDistance) {
			lowestDistance = distance;
			index = i;
		}
	}

	if (index == -1) {
		return { 0.0f, 0.0f, 0.0f };
	}

	return playerPositions[index];
}
float distance(vec3 p1, vec3 p2)
{
	return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2) + pow(p1.z - p2.z, 2));
}
\


void SimulateMouseClick() //can use for triggerbot, works fine.
{
	INPUT input = { 0 };

	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &input, sizeof(INPUT));

	input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &input, sizeof(INPUT));
}

void Triggerbot()
{
	//add your own here
}

Vector Aimbot::findClosest(const std::vector<Vector> playerPositions)
{
	if (playerPositions.empty()) {

		printf("Empty");
		return { 0.0f, 0.0f, 0.0f };
	}

	Vector center_of_screen{ (float)GetSystemMetrics(0) / 2, (float)GetSystemMetrics(1) / 2, 0.0f };

	float lowestDistance = 10000;

	int index = -1;

	for (int i = 0; i < playerPositions.size(); i++) 
	{
		float distance(std::pow(playerPositions[i].x - center_of_screen.x, 2) + std::pow(playerPositions[i].y - center_of_screen.y, 2));

		if (distance < lowestDistance)
		{
			lowestDistance = distance;
			index = i;
		}
	}


	if (index == -1) {
		return { 0.0f, 0.0f, 0.0f };
	}

	return { playerPositions[index].x, playerPositions[index].y, 0.0f };

	//works fine

}

void Aimbot::MoveMouseToPlayer(Vector position, float smoothingFactor)
{
	if (position.IsZero())
		return;

	Vector center_of_screen{ (float)GetSystemMetrics(0) / 2, (float)GetSystemMetrics(1) / 2, 0.0f };

	auto deltaX = position.x - center_of_screen.x;
	auto deltaY = position.y - center_of_screen.y;

	deltaX /= smoothingFactor;
	deltaY /= smoothingFactor;

	mouse_event(MOUSEEVENTF_MOVE, static_cast<int>(deltaX), static_cast<int>(deltaY), 0, 0);
}

void Reader::FilterPlayers()
{
	playerList.clear();

	auto entityList = *reinterpret_cast<uintptr_t*>(client + offset::dwEntityList);
	auto localPawn = *reinterpret_cast<uintptr_t*>(client + offset::dwLocalPlayerPawn);


	if (entityList == 0) return;
	if (localPawn == 0) return;
	for (int i = 0; i < 64; i++)
	{
		uintptr_t list_entry1 = *reinterpret_cast<uintptr_t*>(entityList + (((8 * (i & 0x7FFF) >> 9) + 16)));

		if (list_entry1 == 0) continue;

		uintptr_t playerController = *reinterpret_cast<uintptr_t*>(list_entry1 +120 * (i & 0x1FF));

		if (playerController == 0) continue;

		uint32_t playerPawn = *reinterpret_cast<uint32_t*>(playerController + offset::m_hPlayerPawn);

		if (playerPawn == 0) continue;

		uintptr_t list_entry2 = *reinterpret_cast<uintptr_t*>(entityList + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);

		if (list_entry2 == 0) continue;

		uintptr_t pCSPlayerPawnPtr = *reinterpret_cast<uintptr_t*>(list_entry2 + 120 * (playerPawn & 0x1FF));

		if (pCSPlayerPawnPtr == 0) continue;

		int health = *reinterpret_cast<int*>(pCSPlayerPawnPtr + offset::m_iHealth);

		if (health == 0) continue;

		if (health <= 0 || health > 100)
			continue;

		int team = *reinterpret_cast<int*>(pCSPlayerPawnPtr + offset::m_iTeamNum);

		if (team == 0) continue;

		if (team == *reinterpret_cast<int*>(localPawn + offset::m_iTeamNum))
			continue;

		CCSPlayerPawn.pCSPlayerPawn = pCSPlayerPawnPtr;
		playerList.push_back(CCSPlayerPawn);
	}
}

bool ShowMenu = true;
bool ImGui_Initialised = false;
DWORD64 GetPointerAddress(DWORD64 ptr, std::vector<DWORD64> offsets)
{

	DWORD64 addr = ptr;
	for (int i = 0; i < offsets.size(); ++i) {
		addr = *(DWORD64*)addr;
		addr += offsets[i];
	}
	return addr;
}
namespace Process {
	DWORD ID;
	HANDLE Handle;
	HWND Hwnd;
	HMODULE Module;
	WNDPROC WndProc;
	int WindowWidth;
	int WindowHeight;
	LPCSTR Title;
	LPCSTR ClassName;
	LPCSTR Path;
}
namespace DirectX11Interface {
	ID3D11Device* Device;
	ID3D11DeviceContext* DeviceContext;
	ID3D11RenderTargetView* RenderTargetView;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT APIENTRY WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (ShowMenu) {
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		return true;
	}
	return CallWindowProc(Process::WndProc, hwnd, uMsg, wParam, lParam);
}
bool IsKeyPressed(int key) {
	return (GetAsyncKeyState(key) & 0x8000) != 0;

}
void SetRichImGuiStyle() //chat GPT made me this style looks cool ig
{
	// Get current style
	ImGuiStyle& style = ImGui::GetStyle();

	// Rich colors
	ImVec4 goldColor = ImVec4(1.0f, 0.84f, 0.0f, 1.0f);  // Gold
	ImVec4 darkBgColor = ImVec4(0.1f, 0.1f, 0.1f, 1.0f); // Dark background
	ImVec4 deepBlack = ImVec4(0.05f, 0.05f, 0.05f, 1.0f); // Even darker background
	ImVec4 richHover = ImVec4(1.0f, 0.9f, 0.0f, 1.0f);   // Bright gold on hover
	ImVec4 richActive = ImVec4(0.9f, 0.7f, 0.0f, 1.0f);  // Slightly darker gold on click

	// Overall window style
	style.WindowPadding = ImVec2(15, 15);  // Padding inside the window
	style.WindowRounding = 12.0f;          // Rounded corners for windows
	style.FramePadding = ImVec2(10, 10);   // Padding inside UI elements
	style.FrameRounding = 8.0f;            // Rounded corners for buttons and frames
	style.ItemSpacing = ImVec2(15, 8);     // Spacing between UI elements
	style.ItemInnerSpacing = ImVec2(10, 10); // Spacing within UI elements

	// Colors
	style.Colors[ImGuiCol_Text] = goldColor;                  // Text color
	style.Colors[ImGuiCol_WindowBg] = darkBgColor;            // Background color of windows
	style.Colors[ImGuiCol_FrameBg] = deepBlack;               // Background color of widgets
	style.Colors[ImGuiCol_FrameBgHovered] = richHover;        // Hover effect on widgets
	style.Colors[ImGuiCol_FrameBgActive] = richActive;        // Click effect on widgets
	style.Colors[ImGuiCol_Button] = goldColor;                // Button color
	style.Colors[ImGuiCol_ButtonHovered] = richHover;         // Button hover effect
	style.Colors[ImGuiCol_ButtonActive] = richActive;         // Button active/click effect
	style.Colors[ImGuiCol_Header] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);          // Darker color for the header when not active
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);                      // Brighter color on hover
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.3f, 0.3f, 0.3f, 1.0f);    // Slightly darker gold when active
	style.Colors[ImGuiCol_Border] = goldColor;                // Border color
	style.Colors[ImGuiCol_BorderShadow] = darkBgColor;        // Shadow effect for borders
	style.Colors[ImGuiCol_PopupBg] = deepBlack;               // Popup background
	style.Colors[ImGuiCol_TitleBg] = deepBlack;             // Title background
	style.Colors[ImGuiCol_TitleBgActive] = deepBlack;         // Active title background
	style.Colors[ImGuiCol_TitleBgCollapsed] = deepBlack;      // Collapsed title background
	style.Colors[ImGuiCol_ScrollbarBg] = darkBgColor;         // Scrollbar background
	style.Colors[ImGuiCol_ScrollbarGrab] = goldColor;         // Scrollbar grab
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = richHover;  // Scrollbar hover effect
	style.Colors[ImGuiCol_ScrollbarGrabActive] = richActive;  // Scrollbar active effect
	style.Colors[ImGuiCol_CheckMark] = goldColor;             // Checkmark color
	style.Colors[ImGuiCol_SliderGrab] = goldColor;            // Slider grab color
	style.Colors[ImGuiCol_SliderGrabActive] = richActive;     // Slider active effect
	style.Colors[ImGuiCol_Tab] = darkBgColor;                 // Tab color
	style.Colors[ImGuiCol_TabHovered] = darkBgColor;            // Tab hover effect
	style.Colors[ImGuiCol_TabActive] = darkBgColor;            // Tab active effect
	style.Colors[ImGuiCol_Separator] = goldColor;             // Separator color
	style.Colors[ImGuiCol_SeparatorHovered] = richHover;      // Separator hover effect
	style.Colors[ImGuiCol_SeparatorActive] = richActive;      // Separator active effect
	style.Colors[ImGuiCol_ResizeGrip] = goldColor;            // Resize grip color
	style.Colors[ImGuiCol_ResizeGripHovered] = richHover;     // Resize grip hover effect
	style.Colors[ImGuiCol_ResizeGripActive] = richActive;     // Resize grip active effect
	style.Colors[ImGuiCol_TextSelectedBg] = richHover;        // Text selection color
	style.Colors[ImGuiCol_DragDropTarget] = richHover;        // Drag and drop target effect
	style.Colors[ImGuiCol_NavHighlight] = richHover;          // Highlight for navigation
	style.Colors[ImGuiCol_NavWindowingHighlight] = richHover; // Window highlight for navigation
	style.Colors[ImGuiCol_NavWindowingDimBg] = deepBlack;     // Dimming effect for navigation
	style.Colors[ImGuiCol_ModalWindowDimBg] = deepBlack;      // Dimming effect for modals

	// Customizing some additional elements for a more prestigious feel
	style.GrabRounding = 8.0f;     // Rounded corners for sliders and grabbers
	style.ScrollbarRounding = 12.0f;  // Rounded corners for the scrollbar

	// Setting window title alignment to center for a more balanced look
	style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
}

void LoadCustomFont()
{
	ImGuiIO& io = ImGui::GetIO();
	ImFont* customFont = io.Fonts->AddFontFromFileTTF("C:/ProggyTiny.ttf", 13.0f); //not Using, but u can add ur own. Just call LoadCustomFont

	if (customFont == nullptr)
	{
	}

	io.Fonts->Build();
}
bool bunnyhop = false;
bool trigger = false;
bool numberesp = false;
bool goldenesp = false;
bool noskybox = false;
bool nolights = false;
bool rageaimboot = false;
bool aimboot = false;

std::uint8_t* PatternScan(const char* module_name, const char* signature) noexcept {
	const auto module_handle = GetModuleHandleA(module_name);

	if (!module_handle)
		return nullptr;

	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + std::strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;

				if (*current == '?')
					++current;

				bytes.push_back(-1);
			}
			else {
				bytes.push_back(std::strtoul(current, &current, 16));
			}
		}
		return bytes;
		};

	auto dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_handle);
	auto nt_headers =
		reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<std::uint8_t*>(module_handle) + dos_header->e_lfanew);

	auto size_of_image = nt_headers->OptionalHeader.SizeOfImage;
	auto pattern_bytes = pattern_to_byte(signature);
	auto scan_bytes = reinterpret_cast<std::uint8_t*>(module_handle);

	auto s = pattern_bytes.size();
	auto d = pattern_bytes.data();

	for (auto i = 0ul; i < size_of_image - s; ++i) {
		bool found = true;

		for (auto j = 0ul; j < s; ++j) {
			if (scan_bytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found)
			return &scan_bytes[i];
	}

	throw std::runtime_error(std::string("Wrong signature: ") + signature);
}

class CCSGOInput //dump in reclass or grab from asphyxia or maybe uc
{
public:

};



typedef void(__fastcall* oCreateMoveFn)(CCSGOInput*, int, char);
inline oCreateMoveFn oCreateMove = nullptr;
void __fastcall CreateMoveHook(CCSGOInput* ecx, int edx, char a2);

void __fastcall CreateMoveHook(CCSGOInput* ecx, int edx, char a2)
{
	std::cout << "Golden ESP HOOKED";
	oCreateMove(ecx, edx, a2);

}

void CallRage() {
	auto entityList = *reinterpret_cast<uintptr_t*>(reader.client + offset::dwEntityList);
	auto localPlayerPawn = *reinterpret_cast<uintptr_t*>(reader.client + offset::dwLocalPlayerPawn);

	BYTE team = *reinterpret_cast<BYTE*>(localPlayerPawn + offset::m_iTeamNum);

	vec3 localEyePos = *reinterpret_cast<vec3*>(localPlayerPawn + offset::m_vOldOrigin) + *reinterpret_cast<vec3*>(localPlayerPawn + offset::m_vecViewOffset);
	float closest_distance = -1;
	vec3 enemyPos;

	for (int i = 0; i < 64; i++)
	{
		uintptr_t listEntry = *reinterpret_cast<uintptr_t*>(entityList + ((8 * (i & 0x7ff) >> 9) + 16));
		if (!listEntry) continue;

		uintptr_t entityController = *reinterpret_cast<uintptr_t*>(listEntry + 120 * (i & 0x1ff));
		if (!entityController) continue;

		uintptr_t entityControllerPawn = *reinterpret_cast<uintptr_t*>(entityController + offset::m_hPlayerPawn);
		if (!entityControllerPawn) continue;

		uintptr_t entityPawn = *reinterpret_cast<uintptr_t*>(listEntry + 120 * (entityControllerPawn & 0x1ff));
		if (!entityPawn) continue;

		int health = *reinterpret_cast<int*>(entityPawn + offset::m_iHealth);
		if (health == 0) continue;
		if (team == *reinterpret_cast<BYTE*>(entityPawn + offset::m_iTeamNum))
			continue;

		if (health <= 0 || health > 100)
			continue;

		//Grabbing entityeyepos is better than relative pos, But we can only grab head.
		//If you want other bones, u should add boneindex instead.
		vec3 entityEyePos = *reinterpret_cast<vec3*>(entityPawn + offset::m_vOldOrigin) + *reinterpret_cast<vec3*>(entityPawn + offset::m_vecViewOffset);


		//Not working too well, legit aim has a better current distance.
		float current_distance = distance(localEyePos, entityEyePos);

		if (closest_distance < 0 || current_distance < closest_distance)
		{
			closest_distance = current_distance;
			enemyPos = entityEyePos;
		}
	}
	float timeFactor = 0.04f; //Try 0.03f or something else if u want
	vec3 localPlayerVelocity = *reinterpret_cast<vec3*>(localPlayerPawn + offset::m_vecVelocity);

	vec3 adjustedEnemyPos = enemyPos - (localPlayerVelocity * timeFactor);

	vec3 relativeAngle = (adjustedEnemyPos - localEyePos).RelativeAngle(localEyePos, adjustedEnemyPos);

	vec3* ViewAngles = reinterpret_cast<vec3*>(clientbase + offset::dwViewAngles);
	if (ViewAngles != nullptr) {
		*ViewAngles = relativeAngle;
	}

}



HRESULT APIENTRY MJPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
	if (!ImGui_Initialised) {
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&DirectX11Interface::Device))){
			ImGui::CreateContext();

			ImGuiIO& io = ImGui::GetIO(); (void)io;
			ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantTextInput || ImGui::GetIO().WantCaptureKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

			DirectX11Interface::Device->GetImmediateContext(&DirectX11Interface::DeviceContext);

			DXGI_SWAP_CHAIN_DESC Desc;
			pSwapChain->GetDesc(&Desc);
			WindowHwnd = Desc.OutputWindow;

			ID3D11Texture2D* BackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&BackBuffer);
			DirectX11Interface::Device->CreateRenderTargetView(BackBuffer, NULL, &DirectX11Interface::RenderTargetView);
			BackBuffer->Release();

			ImGui_ImplWin32_Init(WindowHwnd);
			ImGui_ImplDX11_Init(DirectX11Interface::Device, DirectX11Interface::DeviceContext);

			//LoadCustomFont(); ur own font
			ImGui_ImplDX11_CreateDeviceObjects();
			ImGui::GetIO().ImeWindowHandle = Process::Hwnd;
			Process::WndProc = (WNDPROC)SetWindowLongPtr(Process::Hwnd, GWLP_WNDPROC, (__int3264)(LONG_PTR)WndProc);
			ImGui_Initialised = true;
			/*
			AllocConsole();
			freopen("CONOUT$", "w", stdout);

			auto createmove = PatternScan("client.dll", "85 D2 0F 85 ? ? ? ? 48 8B C4 44 88 40 18");
			if (!createmove)
				return NULL;

			MH_Initialize();

			MH_CreateHook((LPVOID)createmove, (LPVOID)CreateMoveHook, (LPVOID*)&oCreateMove);
			MH_EnableHook(MH_ALL_HOOKS);
			*/
			/* to call off
			MH_DisableHook(MH_ALL_HOOKS);
			MH_RemoveHook(MH_ALL_HOOKS);
			MH_Uninitialize();
			*/
		}
	}
	if (GetAsyncKeyState(VK_INSERT) & 1) ShowMenu = !ShowMenu;
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImGui::GetIO().MouseDrawCursor = ShowMenu;
	SetRichImGuiStyle();

	if (ShowMenu == true) {
		ImGui::SetNextWindowSize(ImVec2(400, 400));
		ImGui::Begin("BillionaireWare");
		//ImGui::Checkbox("Bunny Hop", &bunnyhop);
		ImGui::Checkbox("Rage Aimbot [SHIFT]", &rageaimboot);
		ImGui::Checkbox("Legit Aimbot [SHIFT]", &aimboot);
		ImGui::Combo("[Only for Legit]", &HitBone, ("Head\0Chest\0Legs\0Feet\0"));
		ImGui::SliderFloat("Smoothing", &smoothingFactor, 1.0f, 50.0f);
		ImGui::Checkbox("Number ESP", &numberesp);
		ImGui::Checkbox("Golden ESP", &goldenesp);
		ImGui::Checkbox("Disable Skybox", &noskybox);
		ImGui::Checkbox("Disable Lights", &nolights);
		ImGui::Text("Client base address: 0x%X", clientbase);
		ImGui::Text("CS2 base address: 0x%X", cs2base);
		ImGui::Text("Developed by bly", cs2base);
		ImGui::End();
	}

	std::once_flag flag;
	std::once_flag flag2;
	std::once_flag flag4;
	std::once_flag flag7;
	std::once_flag flag8;
	uintptr_t* goldesp = (uintptr_t*)GetPointerAddress(clientbase + GoldEsp.a, GoldEsp.b); //r_aoproxy_show 1
	uintptr_t* blinkless = (uintptr_t*)GetPointerAddress(clientbase + BlinkLess.a, BlinkLess.b); //r_r_extra_render_frames 1
	uintptr_t* number = (uintptr_t*)GetPointerAddress(clientbase + NumberEsp.a, NumberEsp.b); //cl_player_proximity_debug 1
	uintptr_t* skybox = (uintptr_t*)GetPointerAddress(clientbase + NoSkybox.a, NoSkybox.b); //r_drawskybox 0
	uintptr_t* no_lights = (uintptr_t*)GetPointerAddress(scenebase + H_nolights.a, H_nolights.b); //HIDDEN CVAR
	if (goldenesp) {
		std::call_once(flag2, [&]() { *goldesp = 1; });
		std::call_once(flag4, [&]() { *blinkless = 1; });
	}
	else {
		std::call_once(flag2, [&]() { *goldesp = 0; });
		std::call_once(flag4, [&]() { *blinkless = 0; });
	}

	if (noskybox) {
		std::call_once(flag7, [&]() { *skybox = 0; });
	}
	else {
		std::call_once(flag7, [&]() { *skybox = 1; });
	}

	if (nolights) {
		std::call_once(flag8, [&]() { *no_lights = 0; });
	}
	else {
		std::call_once(flag8, [&]() { *no_lights = 1; });
	}

	if (numberesp) {
		std::call_once(flag, [&]() { *number = 1; });
	}
	else {
		std::call_once(flag, [&]() { *number = 0; });
	}

	if (trigger) {
		//not working, to be fixed
		if (GetAsyncKeyState(VK_CAPITAL)){
		Triggerbot();
		}
		if (GetAsyncKeyState(VK_SHIFT)) {
			SimulateMouseClick();
		}
	}
	if (rageaimboot) {
		if (GetAsyncKeyState(VK_SHIFT) && 0x8000) {
			CallRage();
		}
	}
	if (aimboot) {
		aimbot.doAimbot();
		reader.ThreadLoop();
		if (HitBone == 0) {
			BoneIndex = 59.0f; // estimated obv. head
		}
		if (HitBone == 1) {
			BoneIndex = 50.0f; //chest 
		}
		if (HitBone == 2) {
			BoneIndex = 35.5f; //legs
		}
		if (HitBone == 3) {
			BoneIndex = 10.0f; //foot
		}
	}
	ImGui::EndFrame();
	ImGui::Render();
	DirectX11Interface::DeviceContext->OMSetRenderTargets(1, &DirectX11Interface::RenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oIDXGISwapChainPresent(pSwapChain, SyncInterval, Flags);
}

void APIENTRY MJDrawIndexed(ID3D11DeviceContext* pContext, UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) {

}

DWORD WINAPI MainThread(LPVOID lpParameter) {
	bool WindowFocus = false;
	while (WindowFocus == false) {
		DWORD ForegroundWindowProcessID;
		GetWindowThreadProcessId(GetForegroundWindow(), &ForegroundWindowProcessID);
		if (GetCurrentProcessId() == ForegroundWindowProcessID) {

			Process::ID = GetCurrentProcessId();
			Process::Handle = GetCurrentProcess();
			Process::Hwnd = GetForegroundWindow();

			RECT TempRect;
			GetWindowRect(Process::Hwnd, &TempRect);
			Process::WindowWidth = TempRect.right - TempRect.left;
			Process::WindowHeight = TempRect.bottom - TempRect.top;

			char TempTitle[MAX_PATH];
			GetWindowText(Process::Hwnd, TempTitle, sizeof(TempTitle));
			Process::Title = TempTitle;

			char TempClassName[MAX_PATH];
			GetClassName(Process::Hwnd, TempClassName, sizeof(TempClassName));
			Process::ClassName = TempClassName;

			char TempPath[MAX_PATH];
			GetModuleFileNameEx(Process::Handle, NULL, TempPath, sizeof(TempPath));
			Process::Path = TempPath;

			WindowFocus = true;
		}
	}
	bool InitHook = false;
	while (InitHook == false) {
		if (DirectX11::Init() == true) {
		    CreateHook(8, (void**)&oIDXGISwapChainPresent, MJPresent);
			CreateHook(12, (void**)&oID3D11DrawIndexed, MJDrawIndexed);
			InitHook = true;
		}
	}
	return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		if (ChecktDirectXVersion(DirectXVersion.D3D11) == true) {
			Process::Module = hModule;
			CreateThread(0, 0, MainThread, 0, 0, 0);
		}
		break;
	case DLL_PROCESS_DETACH:
		FreeLibraryAndExitThread(hModule, TRUE);
		DisableAll();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	default:
		break;
	}
	return TRUE;
}
