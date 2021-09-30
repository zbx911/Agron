#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include <psapi.h>

static unsigned __int64* sigscan(std::string pattern, int times = 0) //not by me
{
	uintptr_t MemoryBase = (uintptr_t)GetModuleHandleA(0);

	static auto patternToByte = [](const char* pattern)
	{
		auto       bytes = std::vector<int>{};
		const auto start = const_cast<char*>(pattern);
		const auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current)
		{
			if (*current == '?')
			{
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else { bytes.push_back(strtoul(current, &current, 16)); }
		}
		return bytes;
	};

	const auto dosHeader = (PIMAGE_DOS_HEADER)MemoryBase;
	const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)MemoryBase + dosHeader->e_lfanew);

	const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
	auto       patternBytes = patternToByte(pattern.c_str());
	const auto scanBytes = reinterpret_cast<std::uint8_t*>(MemoryBase);

	const auto s = patternBytes.size();
	const auto d = patternBytes.data();

	size_t nFoundResults = 0;

	for (auto i = 0ul; i < sizeOfImage - s; ++i)
	{
		bool found = true;
		for (auto j = 0ul; j < s; ++j)
		{
			if (scanBytes[i + j] != d[j] && d[j] != -1)
			{
				found = false;
				break;
			}
		}
		if (found)
		{
			if (times != 0)
			{
				if (nFoundResults < times)
				{
					nFoundResults++;                                   // Skip Result To Get nSelectResultIndex.
					found = false;                                     // Make sure we can loop again.
				}
				else
				{
					return (unsigned __int64*)reinterpret_cast<uintptr_t>(&scanBytes[i]);  // Result By Index.
				}
			}
			else
			{
				return (unsigned __int64*)reinterpret_cast<uintptr_t>(&scanBytes[i]);      // Default/First Result.
			}
		}
	}
	return NULL;
}

static unsigned __int64* offsetscan(int offset)
{
	return reinterpret_cast<unsigned __int64*>(reinterpret_cast<unsigned __int64>(GetModuleHandle(0)) + offset);
}

void WriteMemory(int offset, __int64 value) {

	DWORD old, unused;
	unsigned __int64* address = offsetscan(offset);
	VirtualProtect(address, 1, PAGE_READWRITE, &old);
	WriteProcessMemory(GetModuleHandleW((LPCWSTR)0x0), address, &value, sizeof(value), nullptr);
	VirtualProtect(address, 1, old, &unused);
}

const char* ReadMemory(int offset, int length) {

	const char* res;
	DWORD old, unused;
	unsigned __int64* address = offsetscan(offset);
	VirtualProtect(address, 1, PAGE_READWRITE, &old);
	ReadProcessMemory(GetModuleHandleW((LPCWSTR)0x0), address, &res, length, 0);
	VirtualProtect(address, 1, old, &unused);

	return res;
}

// FROM kem0x/FortKit i take no credit.

enum ASM : DWORD
{
	JMP_REL8 = 0xEB,
	CALL = 0xE8,
	LEA = 0x8D,
	CDQ = 0x99,
	CMOVL = 0x4C,
	CMOVS = 0x48,
	INT3 = 0xCC,
	RETN = 0xC3,
	SKIP
};

static void* FindStringRef(const std::wstring& string)
{
	uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(nullptr));

	const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
	const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

	IMAGE_SECTION_HEADER* textSection = nullptr;
	IMAGE_SECTION_HEADER* rdataSection = nullptr;

	auto sectionsSize = ntHeaders->FileHeader.NumberOfSections;
	auto section = IMAGE_FIRST_SECTION(ntHeaders);

	for (WORD i = 0; i < sectionsSize; section++)
	{
		auto secName = std::string((char*)section->Name);

		if (secName == ".text")
		{
			textSection = section;
		}
		else if (secName == ".rdata")
		{
			rdataSection = section;
		}

		if (textSection && rdataSection)
			break;
	}

	auto textStart = base_address + textSection->VirtualAddress;

	auto rdataStart = base_address + rdataSection->VirtualAddress;
	auto rdataEnd = rdataStart + rdataSection->Misc.VirtualSize;

	const auto scanBytes = reinterpret_cast<std::uint8_t*>(textStart);

	//scan only text section
	for (DWORD i = 0x0; i < textSection->Misc.VirtualSize; i++)
	{
		if (&scanBytes[i])
		{
			if ((scanBytes[i] == ASM::CMOVL || scanBytes[i] == ASM::CMOVS) && scanBytes[i + 1] == ASM::LEA)
			{
				auto stringAdd = reinterpret_cast<uintptr_t>(&scanBytes[i]);
				stringAdd = stringAdd + 7 + *reinterpret_cast<int32_t*>(stringAdd + 3);

				//check if the pointer is actually a valid string by checking if it's inside the rdata section
				if (stringAdd > rdataStart && stringAdd < rdataEnd)
				{
					std::wstring lea((wchar_t*)stringAdd);

					if (lea == string)
					{
						return &scanBytes[i];
					}
				}
			}
		}
	}

	return nullptr;
}

static void* FindByString(const std::wstring& string, std::vector<int> opcodesToFind = {}, bool bRelative = false, uint32_t offset = 0, bool forward = false)
{
	auto ref = FindStringRef(string);

	if (ref)
	{

		const auto scanBytes = static_cast<std::uint8_t*>(ref);

		//scan backwards till we hit a ret (and assume this is the function start)
		for (auto i = 0; forward ? (i < 2048) : (i > -2048); forward ? i++ : i--)
		{
			if (opcodesToFind.size() == 0)
			{
				if (scanBytes[i] == ASM::INT3 || scanBytes[i] == ASM::RETN)
				{
					return &scanBytes[i + 1];
				}
			}
			else
			{
				for (uint8_t byte : opcodesToFind)
				{
					if (scanBytes[i] == byte && byte != ASM::SKIP)
					{
						if (bRelative)
						{
							uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
							address = ((address + offset + 4) + *(int32_t*)(address + offset));
							return (void*)address;
						}
						return &scanBytes[i];
					}
				}
			}
		}
	}

	return nullptr;
}

static __forceinline bool MaskCompare(PVOID pBuffer, LPCSTR lpPattern, LPCSTR lpMask)
{
	for (PBYTE value = static_cast<PBYTE>(pBuffer); *lpMask; ++lpPattern, ++lpMask, ++value)
	{
		if (*lpMask == 'x' && *reinterpret_cast<LPCBYTE>(lpPattern) != *value) return false;
	}
	return true;
}

static __forceinline uintptr_t FindPattern(PVOID pBase, DWORD dwSize, LPCSTR lpPattern, LPCSTR lpMask)
{
	dwSize -= static_cast<DWORD>(strlen(lpMask));
	for (unsigned long index = 0; index < dwSize; ++index)
	{
		PBYTE pAddress = static_cast<PBYTE>(pBase) + index;
		if (MaskCompare(pAddress, lpPattern, lpMask)) return reinterpret_cast<uintptr_t>(pAddress);
	}
	return NULL;
}

static __forceinline uintptr_t FindPattern(LPCSTR lpPattern, LPCSTR lpMask, BOOL SleepBetween = false)
{
	MODULEINFO info = { nullptr };
	GetModuleInformation(GetCurrentProcess(), GetModuleHandle(nullptr), &info, sizeof(info));

	return FindPattern(info.lpBaseOfDll, info.SizeOfImage, lpPattern, lpMask);
}