#if defined(_WIN_PLATFORM_)

#include "exports.h"
#include <filesystem>

FARPROC OriginalFuncs_version[17];

void Exports::Load() {
	char szSystemDirectory[MAX_PATH]{};
	GetSystemDirectoryA(szSystemDirectory, MAX_PATH);

	std::string OriginalPath = szSystemDirectory;
	OriginalPath += "\\version.dll";

	HMODULE version = LoadLibraryA(OriginalPath.c_str());
	// load version.dll from system32
	if (!version) {
        throw std::runtime_error("Failed to load version.dll from system32\n");
    }

	std::vector<std::string> ExportNames_version = {
		"GetFileVersionInfoA",
		"GetFileVersionInfoByHandle",
		"GetFileVersionInfoExA",
		"GetFileVersionInfoExW",
		"GetFileVersionInfoSizeA",
		"GetFileVersionInfoSizeExA",
		"GetFileVersionInfoSizeExW",
		"GetFileVersionInfoSizeW",
		"GetFileVersionInfoW",
		"VerFindFileA",
		"VerFindFileW",
		"VerInstallFileA",
		"VerInstallFileW",
		"VerLanguageNameA",
		"VerLanguageNameW",
		"VerQueryValueA",
		"VerQueryValueW"
	};

	// get addresses of original functions
	for (int i = 0; i < 17; i++) {
		OriginalFuncs_version[i] = GetProcAddress(version, ExportNames_version[i].c_str());
		if (!OriginalFuncs_version[i]) {
            throw std::runtime_error("Failed to get address of " + ExportNames_version[i] + "\n");
        }
	}
}
#endif