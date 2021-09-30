#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include "Third Party/ImGui/imgui.h"
#include "Third Party/ImGui/imgui_impl_win32.h"
#include "Third Party/ImGui/imgui_impl_dx11.h"
//#include "../console.h"
#include "options.h"
#include "detours.h"
#include "Util/util.h"
#include "main.h"
#include "Util/helper.h"
#include "Third Party/Kiero/kiero.h"

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
static bool show = false;
static bool bSpeedHack = false;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (GetAsyncKeyState(VK_F8) & 0x8000) { // BUG: You gotta press the button multiple times.
		show = !show;
		ImGui::GetIO().MouseDrawCursor = show;
	}

	if (show)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowBgAlpha(0.8);
		ImGui::SetNextWindowSize(ImVec2(560, 345));

		ImGui::Begin(NAME, 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);
		static int tab = 0;
		static int fov = 80;
		static int currentFov = 80; // float?
		static std::string isAuthStr = btcc(bHasAuthority);
		if (ImGui::BeginTabBar("", 0)) {

			if (ImGui::BeginTabItem("Player")) {
				tab = 0;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Fun")) {
				tab = 1;
				ImGui::EndTabItem();
			}
#ifdef DEVELOPMENT
			if (ImGui::BeginTabItem("Debug"))
			{
				tab = 2;
				ImGui::EndTabItem();
			}
#endif
			ImGui::EndTabBar();
		}

		switch (tab)
		{
		case 0:
			ImGui::SliderInt("Fov", &fov, 30, 160, "%.03f");
			if (currentFov != fov) // credit jacobb
			{
				std::wstring command(L"fov " + std::to_wstring(fov));
				Console::ExecuteConsoleCommand(command.c_str());
				currentFov = fov;
			}
			if (ImGui::Button(XOR("Die")))
			{
				Console::ExecuteConsoleCommand(L"suicide");
			}
			break;
		case 1:
			if (ImGui::Button(XOR("Speed Hack")))
			{
				bSpeedHack = !bSpeedHack;
				if (bSpeedHack) Console::ExecuteConsoleCommand(L"demospeed 1.12");
				else Console::ExecuteConsoleCommand(L"demospeed 1.00");
			}
			else if (ImGui::Button((isAuthStr + " Authority").c_str())) // the names dont work
			{
				bHasAuthority = !bHasAuthority;
				isAuthStr = btcc(bHasAuthority);

				ToggleAuthority();
			}
			break;
#ifdef DEVELOPMENT
		case 2:
			//char* countc{};
			//ImGui::InputText(XOR("Ammo Count: "), countc, sizeof countc);

			if (ImGui::Button(XOR("Log Roles")))
			{
				auto roles = GetRoles();
				// std::cout << "LocalRole: " << roles[0] << " RemoteRole: " << roles[1] << std::endl;
			}
			if (ImGui::Button(XOR("Apply Thanos")))
			{
				Skins::ApplySkin();
				Log("Applied thanos parts to pawn.");
			}
			if (ImGui::Button(XOR("Show Character Parts")))
			{
				Skins::ShowSkin();
			}
			/* if (ImGui::Button(XOR("Is In Lobby")))
			{
				Log(IsInLobby());
			} */
			if (ImGui::Button(XOR("FORCE")))
			{
				ToggleAuthority(ENetRole::ROLE_Authority);
			}
			/* if (ImGui::Button(XOR("Give Pickaxe Test")))
			{
				GiveWeapon(L"Wavecrest", 3, 2);
			} */
			if (ImGui::Button(XOR("Give Ammo To Player")))
			{
				//const char* countcc = const_cast<const char*>(countc);
				//int32_t count = std::stoi(countcc);
				//std::cout << count;
				GiveAmmoToPlayer(1000);
				
			}
			if (ImGui::Button(XOR("Autonomous Proxy")))
			{
				ToggleAuthority(ENetRole::ROLE_AutonomousProxy);
			}
			if (ImGui::Button(XOR("Simulated Proxy")))
			{
				ToggleAuthority(ENetRole::ROLE_SimulatedProxy);
			}
			/* if (ImGui::Button(XOR("Dump GObjects")))
			{

			} */
			/*if (ImGui::Button(XOR("Log PlayerID")))
			{

				auto UACClass = UE4::FindObject(XOR(L"Class /Script/UACCommon.UACNetworkComponent"));
				UObject** PlayerID = reinterpret_cast<UObject**>(__int64(like wth do i make this its not in a class) + __int64(0xb0));

			} */
			break;
#endif
		}
		ImGui::End();

		ImGui::Render();

		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI GuiThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	Log("Initialized ImGUI!\n");
	return TRUE;
}