#include "sign.h"

#include <mutex>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>

// #define _LINUX_PLATFORM_
#define _WIN_PLATFORM_

#define _X64_ARCH_

#define CURRENT_ARCHITECTURE "x64"

#if defined(_WIN_PLATFORM_)
#include <Windows.h>
#include <psapi.h>
#elif defined(_MAC_PLATFORM_) || defined(_LINUX_PLATFORM_)
#include <cstring>
#include "proc_maps.h"
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
int signOffsets = 767; // 562 in 3.1.2-12912
int ExtraOffsets = 511;
int TokenOffsets = 255;

std::vector<uint8_t> Hex2Bin(std::string str)
{
	if (str.length() % 2 != 0)
		throw std::invalid_argument("Hex string length must be even");
	std::vector<uint8_t> bin(str.size() / 2);
	for (size_t i = 0; i < str.size() / 2; i++)
	{
		std::stringstream ss;
		ss << std::hex << str.substr(i * 2, 2);
		ss >> bin[i];
	}
	return bin;
}

std::string Bin2Hex(const uint8_t *ptr, size_t length)
{
	const char table[] = "0123456789ABCDEF";
	std::string str;
	str.resize(length * 2);
	for (size_t i = 0; i < length; ++i)
	{
		str[2 * i] = table[ptr[i] / 16];
		str[2 * i + 1] = table[ptr[i] % 16];
	}
	return str;
}

typedef int (*SignFunctionType)(const char *cmd, const char *src, size_t src_len, int seq, unsigned char *result);
SignFunctionType SignFunction = nullptr;

void sign::InitSignCall()
{
	uint64_t HookAddress = 0;
#if defined(_WIN_PLATFORM_)
	HMODULE wrapperModule = GetModuleHandleW(L"wrapper.node");
	MODULEINFO modInfo;
	if (wrapperModule == NULL || !GetModuleInformation(GetCurrentProcess(), wrapperModule, &modInfo, sizeof(MODULEINFO)))
		return;
	HookAddress = reinterpret_cast<uint64_t>(wrapperModule) + addrMap[CURRENT_VERSION];
#elif defined(_MAC_PLATFORM_)
	auto pmap = hak::get_maps();
	do
	{
		// printf("start: %llx, end: %llx, offset: %x, module_name: %s\n", pmap->start(), pmap->end(), pmap->offset, pmap->module_name.c_str());
		if (pmap->module_name.find("wrapper.node") != std::string::npos && pmap->offset == 0)
		{
			HookAddress = pmap->start() + addrMap[CURRENT_VERSION];
			printf("HookAddress: %llx\n", HookAddress);
			break;
		}
	} while ((pmap = pmap->next()) != nullptr);
#elif defined(_LINUX_PLATFORM_)
	auto pmap = hak::get_maps();
	do
	{
		// printf("start: %lx, end: %lx, offset: %x, module_name: %s\n", pmap->start(), pmap->end(), pmap->offset, pmap->module_name.c_str());
		if (pmap->module_name.find("wrapper.node") != std::string::npos && pmap->offset == 0)
		{
			HookAddress = pmap->start() + addrMap[CURRENT_VERSION];
			printf("HookAddress: %lx\n", HookAddress);
			break;
		}
	} while ((pmap = pmap->next()) != nullptr);
#endif
	if (HookAddress == 0)
		throw std::runtime_error("Can't find hook address");
	SignFunction = reinterpret_cast<SignFunctionType>(HookAddress);
}

std::tuple<std::string, std::string, std::string> sign::CallSign(const std::string cmd, const std::string src, const int seq)
{
	if (SignFunction == nullptr)
		throw std::runtime_error("Sign function not initialized");

	char *signArgCmd = new char[1024];
	char *signArgSrc = new char[1024];
	int32_t signSrc = 0;
	int32_t signSeq = 0;
	uint8_t *signResult = new uint8_t[1024];
	// 设置最大长度
	size_t str_size = 1024;

	printf("signArgCmd: %s\n", signArgCmd);
	printf("signArgSrc: %s\n", signArgSrc);
	printf("signSrc: %d\n", signSrc);
	printf("signSeq: %d\n", signSeq);

	SignFunction(signArgCmd, signArgSrc, signSrc, signSeq, signResult);

	printf("signResult: %s\n", Bin2Hex(signResult, 1024).c_str());

	// 获取大小
	uint8_t *signSize = (uint8_t *)signResult + signOffsets;
	uint8_t *extraSize = (uint8_t *)signResult + ExtraOffsets;
	uint8_t *tokenSize = (uint8_t *)signResult + TokenOffsets;
	// 读取
	uint32_t signSizeU32 = *signSize;
	uint32_t extraSizeU32 = *extraSize;
	uint32_t tokenSizeU32 = *tokenSize;
	uint8_t *signData = signResult + 512;
	uint8_t *extraData = signResult + 256;
	uint8_t *tokenData = signResult;
	std::string signDataHex = Bin2Hex(signData, signSizeU32);
	std::string extraDataHex = Bin2Hex(extraData, extraSizeU32);
	std::string tokenDataHex = Bin2Hex(tokenData, tokenSizeU32);
	// 回收资源
	delete[] signArgCmd;
	delete[] signArgSrc;
	delete[] signResult;
	return std::make_tuple(signDataHex, extraDataHex, tokenDataHex);
}