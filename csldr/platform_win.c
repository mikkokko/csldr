#include "pch.h"

#if defined(_WIN32)

HMODULE hModule;

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

void Plat_Error(const char *error)
{
	MessageBoxA(NULL, error, "Error", MB_OK);
	ExitProcess(1);
}

/* warzone */
void RemoveGTlib(void)
{
	if (SetFileAttributesW(L"GTlib.asi", FILE_ATTRIBUTE_NORMAL))
		DeleteFileW(L"GTlib.asi");
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
