#include <Windows.h>
#include <iostream>

#include "gui.h"
#include "options.h"
#include "main.h"

void Main()
{
    FILE* fptr;

    AllocConsole();

    freopen_s(&fptr, "CONOUT$", "w", stdout);
    freopen_s(&fptr, "CONOUT$", "w", stderr);
    freopen_s(&fptr, "CONIN$", "r", stdin);

    printf("Initializing Argon...");

#ifndef ONLYPROCESSEVENT
    CreateThread(0, 0, GuiThread, 0, 0, 0);
#endif

    Init();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Main();
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        // FreeConsole();
        //FreeLibraryAndExitThread(hModule, 0);
        break;
    }
    return TRUE;
}

