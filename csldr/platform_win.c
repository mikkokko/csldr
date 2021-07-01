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
	FARPROC hresult = GetProcAddress((HMODULE)handle, name);
	void *result = *(void **)(&hresult);

	return result;
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

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			hModule = hinstDLL;
			PassInit();
			break;
		case DLL_PROCESS_DETACH:
			PassQuit();
			break;
	}

	return TRUE;
}

#endif
