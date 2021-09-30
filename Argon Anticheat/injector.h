#pragma once

#include <Windows.h>
#include <string>
#include <iostream>

static bool InjectDLL(DWORD process_id_, std::string full_dll_pathstr, const char* processname) // most of it from https://gist.github.com/jonobrien/0fbcfa77bc658dc653cbd2c4988a40d1#file-injector-cpp-L68
{
	const char* full_dll_path = full_dll_pathstr.c_str();
	// get the function LoadLibraryA
	LPVOID load_library = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (load_library == NULL)
	{
		return false;
	}

	// open the process
	HANDLE process_handle = OpenProcess(PROCESS_ALL_ACCESS, false, process_id_);
	if (process_handle == NULL)
	{
		return false;
	}

	// allocate space to write the dll location
	LPVOID dll_parameter_address = VirtualAllocEx(process_handle, 0, strlen(full_dll_path), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (dll_parameter_address == NULL)
	{
		CloseHandle(process_handle);
		return false;
	}

	// write the dll location to the space we previously allocated
	BOOL wrote_memory = WriteProcessMemory(process_handle, dll_parameter_address, full_dll_path, strlen(full_dll_path), NULL);
	if (wrote_memory == false)
	{
		CloseHandle(process_handle);
		return false;
	}

	// TODO: Get name from process id.
	std::cout << "Loading DLL into " + std::string(processname) << std::endl;

	// launch the dll using LoadLibraryA
	HANDLE dll_thread_handle = CreateRemoteThread(process_handle, 0, 0, (LPTHREAD_START_ROUTINE)load_library, dll_parameter_address, 0, 0);
	if (dll_thread_handle == NULL)
	{
		CloseHandle(process_handle);
		return false;
	}

	CloseHandle(dll_thread_handle);
	CloseHandle(process_handle);
	return true;
}