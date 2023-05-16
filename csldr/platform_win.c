#include "pch.h"

#if defined(_WIN32)

#include <windows.h>

static HMODULE hModule;

#if defined(_WIN32)
int isWarzone;
#endif

void *Plat_Dlopen(const char *filename)
{
	return LoadLibraryA(filename);
}

void *Plat_Dlsym(void *handle, const char *name)
{
	/* shut up compiler */
	FARPROC res = GetProcAddress((HMODULE)handle, name);
	return *(void **)(&res);
}

void Plat_Dlclose(void *handle)
{
	FreeLibrary((HMODULE)handle);
}

void Plat_CurrentModuleName(char *name, size_t size)
{
	GetModuleFileNameA(hModule, name, size);
}

void Plat_Error(const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];

	va_start(ap, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	MessageBoxA(NULL, buffer, "Error", MB_OK);
	ExitProcess(1);
}

/* warzone */
static void RemoveGTlib(void)
{
	if (SetFileAttributesW(L"GTlib.asi", FILE_ATTRIBUTE_NORMAL))
	{
		DeleteFileW(L"GTlib.asi");
		isWarzone = 1;
	}

	if (SetFileAttributesW(L"GTProtector.asi", FILE_ATTRIBUTE_NORMAL))
	{
		DeleteFileW(L"GTProtector.asi");
		isWarzone = 2;
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			hModule = hinstDLL;
			ProxyInit();
			RemoveGTlib();
			break;
		case DLL_PROCESS_DETACH:
			ProxyQuit();
			break;
	}

	return TRUE;
}

#endif
