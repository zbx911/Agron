#pragma once

#include <iostream>

#include "mem.h"
#include "UE4/structs.h"
#include "options.h"
#include "UE4/finder.h"
#include "UE4/enums.h"
#include "Third Party/MinHook/MinHook.h"
#include "Util/helper.h"
#include "Util/log.h"
#include "detours.h"

#define VALIDATE_ADDRESS(address, error) \
    if (!address) { \
        MessageBoxA(0, error, NAME, MB_OK); \
        FreeLibraryAndExitThread(GetModuleHandle(NULL), 0); \
        return 0; \
    }

#define RELATIVE_ADDRESS(address, size) ((PBYTE)((UINT_PTR)(address) + *(PINT)((UINT_PTR)(address) + ((size) - sizeof(INT))) + (size)))

DWORD GEngineLoop(LPVOID)
{
	while (!GEngine) // cuz gengine isnt initialized on startup.
	{
		GEngine = UE4::FindObject<UEngine*>(XOR(L"FortEngine /Engine/Transient.FortEngine_"));
	}
	const auto vtable = *reinterpret_cast<void***>(GEngine);
	auto ProcessEventAdd = (uintptr_t)vtable[0x44]; // or sig, or findstring.

	ProcessEvent = decltype(ProcessEvent)(ProcessEventAdd);
	MH_CreateHook((void*)ProcessEventAdd, ProcessEventDetour, (void**)&ProcessEvent);
	MH_EnableHook(reinterpret_cast<void*>(ProcessEventAdd));
#ifdef CONSTRUCTCONSOLE
	Console::ConstructConsole();
#endif
	return 0;
}

inline DWORD InputThread(LPVOID)
{
	while (1)
	{
		if (GetAsyncKeyState(VK_F10) & 0x8000 /* & GetPlayerPawn() != nullptr */) { ToggleAuthority(); }
		if (GetAsyncKeyState(VK_F7) & 0x8000) { ToggleAuthority(ENetRole::ROLE_Authority); }
		if (GetAsyncKeyState(VK_F6) & 0x8000) { ToggleAuthority(ENetRole::ROLE_AutonomousProxy); }
		if (GetAsyncKeyState(VK_F5) & 0x8000) { Console::CreateCheatManager(); }
	}
}

static bool Init()
{
	if (MH_Initialize() != MH_OK)
	{
		MessageBoxA(nullptr, XOR("MinHook failed to initialize, exiting thread."), XOR(NAME), MB_OK);
		FreeLibraryAndExitThread(GetModuleHandle(nullptr), 0);
	}
#ifndef ONLYPROCESSEVENT
	//GObject Array
	auto GObjectsAdd = FindPattern(Patterns::bGlobal::GObjects, Masks::bGlobal::GObjects);
	VALIDATE_ADDRESS(GObjectsAdd, XOR("Failed to find GObjects Address."));

	GObjs = decltype(GObjs)(RELATIVE_ADDRESS(GObjectsAdd, 7));
#endif
	auto FNameToStringAdd = FindPattern(Patterns::New::FNameToString, Masks::New::FNameToString);
	VALIDATE_ADDRESS(FNameToStringAdd, XOR("Failed to find FNameToString Address."));

	/*const auto offset = *reinterpret_cast<int32_t*>(FNameToStringAdd + 1);
	FNameToStringAdd = FNameToStringAdd + 5 + offset;*/

	FNameToString = decltype(FNameToString)(FNameToStringAdd);

#ifndef ONLYPROCESSEVENT
	CreateThread(0, 0, GEngineLoop, 0, 0, 0);
#endif

	//GEngine = decltype(GEngine)(Util::SigScan(XOR("A0 60 05 4F 33 02 00 00"))); // A0 60 05 4F 33 02 00 00
#ifdef USECURL
	unsigned __int64* CurlEasyOptP = sigscan(XOR("89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9"));

	MH_CreateHook((void*)CurlEasyOptP, cURLDetour, (void**)&curl_easy_setopt_original);
	MH_EnableHook((void*)CurlEasyOptP);
#endif

#ifdef ONLYPROCESSEVENT
	auto ProcessEventAdd = sigscan(XOR("40 55 53 56 57 41 54 41 56 41 57 48 81 ec"));
	ProcessEvent = decltype(ProcessEvent)(ProcessEventAdd);
	MH_CreateHook(reinterpret_cast<void*>(ProcessEventAdd), ProcessEventDetour, reinterpret_cast<void**>(&ProcessEvent));
	MH_EnableHook(reinterpret_cast<void*>(ProcessEventAdd));
#endif

#ifndef ONLYPROCESSEVENT
	CreateThread(0, 0, InputThread, 0, 0, 0);
#endif

	return true;
}