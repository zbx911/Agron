// This whole thing took me way longer than it should have.

// defines

// #define LOGARGS

// c++ includes

#include <filesystem>
#ifdef LOGARGS
#include <fstream>
#endif

#pragma warning (disable : 6276 ) // cast warning idk
#pragma warning (disable : 4267 ) // loss of data warning (i should probably fix this but im to lazy)

#include "injector.h"

namespace fs = std::filesystem;

std::string anticheat = "NONE";

void Exit(const char* errorMsg, int exitCode) // TODO: Add support for closing (child) other applications.
{
	std::cout << errorMsg << std::endl;
	std::cout << "Exiting in 3 seconds..." << std::endl;
	Sleep(3000);
	exit(exitCode);
}

std::wstring widen(const std::string& s)
{
	std::vector<wchar_t> buf(MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size() + 1, 0, 0));

	MultiByteToWideChar(CP_ACP, 0, s.c_str(), s.size() + 1, &buf[0], buf.size());

	return std::wstring(&buf[0]);
}

struct ProcessParams
{
	std::string exeName;
	std::string exeArguments = " ";
	const char* dllName = "";
	DWORD creationFlags = CREATE_NEW_CONSOLE;
	bool exeIsFullPath = false;
	bool dllIsFullPath = false;
};

void NewProcess(const ProcessParams& params)
{
	// TODO: Add path checking to see if it exists/is a dll/is a exe.

	// Setting wpath and path to the full path

	std::string path = params.exeName;
	std::string args = params.exeArguments;
	const char* dllPath = params.dllName;
	DWORD creationFlags = params.creationFlags;
	bool fullDirectory = params.exeIsFullPath;

	if (path.find(".exe") == std::string::npos) path = path + ".exe"; // TODO: Test path.append(".exe");

	std::wstring wpath;
	fs::path fspath = fs::current_path() / path;
	if (!fullDirectory)
	{
		// path = fspath.string(); // not sure if we need to update this as we may be able to just pass fspath.string() directly into wpath.
		wpath = widen(fspath.string());
		// std::cout << fspath.string() << std::endl;
	}

	// Make sure arguments isn't nothing.

	std::wstring wargs = widen(args);
	if (wargs.size() != 0)
	{
		if (wargs[0] != L' ') wargs.insert(0, L" ");
	}

	// memory stuff

	wchar_t* wcp = new wchar_t[wargs.size() + 1];
	const wchar_t* temp = wargs.c_str();
	wcscpy_s(wcp, wargs.size() + 1, temp);

	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	std::cout << "Starting " << params.exeName << std::endl;

	if (!CreateProcessW(const_cast<LPCWSTR>(wpath.c_str()),
		wcp,
		NULL,
		NULL,
		FALSE,
		creationFlags,
		NULL,
		NULL,
		&si,
		&pi)
		)
	{
		std::cout << "Couldn't create process " + fspath.filename().string() + " and the error code is " << GetLastError() << std::endl;
	}
	else
	{

		if (dllPath != "")
		{
			std::string fullDllPath = (fs::current_path() / dllPath).string();
			std::cout << InjectDLL(pi.dwProcessId, fullDllPath, dllPath ? "Injected DLL successfully!" : "Couldn't inject DLL.") << std::endl;
		}
		WaitForSingleObject(pi.hProcess, INFINITE);
		// std::cout << GetExitCodeProcess(pi.hProcess, idk what to put here) << std::endl;
		delete[]wcp; // we may be able to do this earlier but i havent tested.
		wcp = 0;
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// Getting whole path and parsing it to only the filename.

		char path[MAX_PATH] = ""; // __FILE__ could also be a solution, but I'm pretty sure its on compile time.
		std::string strAppName;

		GetModuleFileNameA(0, path, MAX_PATH);

		// Extract name
		strAppName = path;
		strAppName = strAppName.substr(strAppName.rfind("\\") + 1);

		// Starting BE/EAC

		if (anticheat == "BE") system("net stop BEService"); // we could check in args what kind of anticheat it is but imo this is better.
		else if (anticheat == "EAC") system("net stop EasyAnticheat");
		else std::cout << "Major issue. Please send this to Milxnor: " << (fs::current_path() / strAppName).string() << std::endl;

		Sleep(1000);
		exit(0);
	}
}

int main(int argc, char** argv)
{

	// Getting whole path and parsing it to only the filename.

	char path[MAX_PATH] = ""; // __FILE__ could also be a solution, but I'm pretty sure its on compile time.
	std::string strAppName;

	GetModuleFileNameA(0, path, MAX_PATH);

	// Extract name
	strAppName = path;
	strAppName = strAppName.substr(strAppName.rfind("\\") + 1);

	// Starting BE/EAC

	if (strAppName.find("BE") != std::string::npos) anticheat = "BE";
	else if (strAppName.find("EAC") != std::string::npos) anticheat = "EAC"; // we could check in args what kind of anticheat it is but imo this is better.
	SetConsoleTitleA(std::string("Argon's Fake " + anticheat).c_str());
	if (anticheat == "BE") system("net start BeService");
	else if (anticheat == "EAC") system("net start EasyAnticheat");
	else std::cout << "Major issue. Please send this to Milxnor: " << (fs::current_path() / strAppName).string() << std::endl;

	// Converting the args to a string

	std::string argumentsStr = "";
	if (argc > 0)
	{
#ifdef LOGARGS
		std::ofstream outfile("args.txt");
#endif
		for (int i = 0; i < argc; ++i) {
#ifdef LOGARGS
			outfile << argv[i] << " ";
#endif
			argumentsStr.append(std::string(argv[i]).append(" "));
		}
#ifdef LOGARGS 
		outfile.close();
#endif
	}

	// Starting main shipping process.

	ProcessParams params;

	params.exeName = "FortniteClient-Win64-Shipping.exe";
	params.exeArguments = argumentsStr;
	params.dllName = "Argon.dll";

	NewProcess(params);

	std::cin.get();
}