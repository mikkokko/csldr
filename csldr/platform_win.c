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

void *Plat_CheckedDlopen(const char *filename)
{
	HMODULE handle = LoadLibraryA(filename);
	if (!handle)
	{
		wchar_t errorString[2048];

		if (!FormatMessageW(
			FORMAT_MESSAGE_FROM_SYSTEM
			| FORMAT_MESSAGE_IGNORE_INSERTS
			| FORMAT_MESSAGE_MAX_WIDTH_MASK,
			NULL,
			GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			errorString,
			Q_ARRAYSIZE(errorString),
			NULL))
		{
			wcscpy(errorString, L"Unknown error");
		}

		wchar_t buffer[2048];
		_snwprintf(buffer, Q_ARRAYSIZE(buffer), L"Could not load '%S':\n%s", filename, errorString);
		buffer[Q_ARRAYSIZE(buffer) - 1] = L'\0';

		MessageBoxW(NULL, buffer, L"Client-side loader", MB_OK | MB_ICONERROR);
		ExitProcess(1);

		return NULL; /* never reached */
	}

	return handle;
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

size_t Plat_CurrentModuleName(char *name, size_t size)
{
	DWORD length = GetModuleFileNameA(hModule, name, size);
	if (!length || length >= size)
		return 0;

	return length;
}

void Plat_Error(const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];

	va_start(ap, fmt);
	_vsnprintf(buffer, sizeof(buffer), fmt, ap);
	buffer[sizeof(buffer) - 1] = '\0';
	va_end(ap);

	MessageBoxA(NULL, buffer, "Client-side loader", MB_OK | MB_ICONERROR);
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
	(void)lpReserved;

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
