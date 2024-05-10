#if defined(_WIN_PLATFORM_)

#include "exports.h"

bool TlsOnce = false;
// this runs way before dllmain
void __stdcall TlsCallback(PVOID hModule, DWORD fdwReason, PVOID pContext) {
	if (!TlsOnce) {
		// for version.dll proxy
		// load exports as early as possible
		Exports::Load();
		TlsOnce = true;
	}
}

#pragma comment (linker, "/INCLUDE:_tls_used")
#pragma comment (linker, "/INCLUDE:tls_callback_func")
#pragma const_seg(".CRT$XLF")
EXTERN_C const PIMAGE_TLS_CALLBACK tls_callback_func = TlsCallback;

#endif