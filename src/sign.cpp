#include "sign.h"

#include <mutex>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

#include <thread>

// #define _LINUX_PLATFORM_
// #define _WIN_PLATFORM_

#define _X64_ARCH_

#if defined(_WIN_PLATFORM_)
#include <Windows.h>
#include <psapi.h>
#elif defined(_MAC_PLATFORM_) || defined(_LINUX_PLATFORM_)
#include "proc_maps.h"
#include <cstring>
#endif

// 签名函数定义
#if defined(_WIN_PLATFORM_)
#define CURRENT_VERSION "9.9.9-23361"
#if defined(_X64_ARCH_)
std::map<std::string, uint64_t> addrMap = {
	{"9.9.2-16183", 0x2E0D0},
	{"9.9.9-23361", 0x2EB50}};
#elif defined(_X86_ARCH_)
std::map<std::string, uint64_t> addrMap = {
	{"9.9.2-15962", 0x2BD70},
	{"9.9.2-16183", 0x2BD70}};
#endif
#elif defined(_MAC_PLATFORM_)
#define CURRENT_VERSION "6.9.20-17153"
#if defined(_X64_ARCH_)
std::map<std::string, uint64_t> addrMap = {
	{"6.9.19-16183", 0x1B29469}};
#elif defined(_ARM64_ARCH_)
std::map<std::string, uint64_t> addrMap = {
	{"6.9.20-17153", 0x1c73dd0}};
#endif
#elif defined(_LINUX_PLATFORM_)
#define CURRENT_VERSION "3.2.7-23361"
#if defined(_X64_ARCH_)
std::map<std::string, uint64_t> addrMap = {
	{"3.1.2-12912", 0x33C38E0},
	{"3.1.2-13107", 0x33C3920},
	{"3.2.7-23361", 0x4C93C57}};
#endif
#endif

int SignOffsets = 767; // 562 before 3.1.2-13107, 767 in others
int ExtraOffsets = 511;
int TokenOffsets = 255;

std::vector<uint8_t> Hex2Bin(std::string_view str) {
	if (str.length() % 2 != 0) {
        throw std::invalid_argument("Hex string length must be even");
    }
	std::vector<uint8_t> bin(str.size() / 2);
	std::string extract("00");
	for (size_t i = 0; i < str.size() / 2; i++) {
		extract[0] = str[2 * i];
		extract[1] = str[2 * i + 1];
		bin[i] = std::stoi(extract, nullptr, 16);
	}
	return bin;
}

std::string Bin2Hex(const uint8_t *ptr, size_t length) {
	const char table[] = "0123456789ABCDEF";
	std::string str;
	str.resize(length * 2);
	for (size_t i = 0; i < length; ++i) {
		str[2 * i] = table[ptr[i] / 16];
		str[2 * i + 1] = table[ptr[i] % 16];
	}
	return str;
}

Sign::Sign() {
	std::thread t(&Sign::InitEx, this);
	t.detach();
}

void Sign::Init() {
	uint64_t HookAddress = 0;
#if defined(_WIN_PLATFORM_)
	HMODULE wrapperModule = GetModuleHandleW(L"wrapper.node");
	if (wrapperModule == NULL) {
        throw std::runtime_error("Can't find wrapper.node module");
    }
	HookAddress = reinterpret_cast<uint64_t>(wrapperModule) + addrMap[CURRENT_VERSION];
	printf("HookAddress: %llx\n", HookAddress);
#elif defined(_MAC_PLATFORM_)
	auto pmap = hak::get_maps();
	do {
		if (pmap->module_name.find("wrapper.node") != std::string::npos && pmap->offset == 0) {
			HookAddress = pmap->start() + addrMap[CURRENT_VERSION];
			printf("HookAddress: %llx\n", HookAddress);
			break;
		}
	} while ((pmap = pmap->next()) != nullptr);
#elif defined(_LINUX_PLATFORM_)
	auto pmap = hak::get_maps();
	do {
		if (pmap->module_name.find("wrapper.node") != std::string::npos && pmap->offset == 0) {
			HookAddress = pmap->start() + addrMap[CURRENT_VERSION];
			printf("HookAddress: %lx\n", HookAddress);
			break;
		}
	} while ((pmap = pmap->next()) != nullptr);
#endif
	if (HookAddress == 0) {
        throw std::runtime_error("Can't find hook address");
    }
	SignFunction = reinterpret_cast<SignFunctionType>(HookAddress);
}

void Sign::InitEx() {
	while (true) {
        try {
            Init();
            break;
        }
        catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

std::tuple<std::string, std::string, std::string> Sign::Call(const std::string_view cmd, const std::string_view src, int seq) {
	if (SignFunction == nullptr) {
        throw std::runtime_error("Sign function not initialized");
    }

	const std::vector<uint8_t> signArgSrc = Hex2Bin(src);

	size_t resultSize = 1024;
	auto *signResult = new uint8_t[resultSize];

	SignFunction(cmd.data(), signArgSrc.data(), signArgSrc.size(), seq, signResult);

	std::string signDataHex = Bin2Hex(signResult + 512, *(signResult + SignOffsets));
	std::string extraDataHex = Bin2Hex(signResult + 256, *(signResult + ExtraOffsets));
	std::string tokenDataHex = Bin2Hex(signResult, *(signResult + TokenOffsets));

	return std::make_tuple(signDataHex, extraDataHex, tokenDataHex);
}