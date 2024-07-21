#include "pch.h"

#if !defined(_WIN32)

#define __USE_GNU 1 /* stupid hack for dladdr on linux */
#include <dlfcn.h>

void *Plat_CheckedDlopen(const char *filename)
{
	void *handle = dlopen(filename, RTLD_NOW);
	if (!handle)
	{
		Plat_Error("Could not load '%s': %s\n", filename, dlerror());
		return NULL; /* never reached */
	}

	return handle;
}

void *Plat_Dlsym(void *handle, const char *name)
{
	return dlsym(handle, name);
}

void Plat_Dlclose(void *handle)
{
	dlclose(handle);
}

size_t Plat_CurrentModuleName(char *name, size_t size)
{
	Dl_info info;
	if (!dladdr(Plat_CurrentModuleName, &info))
		return 0;

	size_t length = strlen(info.dli_fname);
	if (length >= size)
		return 0;

	memcpy(name, info.dli_fname, length + 1);
	return length;
}

void Plat_Error(const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];

	va_start(ap, fmt);
	int length = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	fwrite(buffer, 1, length, stderr);

#if defined(__linux__)
	// display a message box if we can
	// i don't want to link to sdl2 directly nor do i want it as a build dependency
	void *libSDL2 = dlopen("libSDL2.so", RTLD_LAZY);
	if (libSDL2)
	{
		typedef int (*SDL_ShowSimpleMessageBox_t)(uint32 flags, const char *title, const char *message, void *window);
		SDL_ShowSimpleMessageBox_t SDL_ShowSimpleMessageBox = (SDL_ShowSimpleMessageBox_t)dlsym(libSDL2, "SDL_ShowSimpleMessageBox");

		if (SDL_ShowSimpleMessageBox)
		{
			SDL_ShowSimpleMessageBox(0, "Client-side loader", buffer, NULL);
		}
	}
#elif defined(__APPLE__)
	CFStringRef header = CFStringCreateWithCString(kCFAllocatorDefault, "Client-side loader", kCFStringEncodingUTF8);
	CFStringRef message = CFStringCreateWithCString(kCFAllocatorDefault, buffer, kCFStringEncodingUTF8);

	CFOptionFlags result;
	CFUserNotificationDisplayAlert(0,
		kCFUserNotificationStopAlertLevel,
		NULL,
		NULL,
		NULL,
		header,
		message,
		NULL,
		NULL,
		NULL,
		&result);

	CFRelease(header);
	CFRelease(message);
#endif

	exit(1);
}

void __attribute__((constructor)) Init(void)
{
	ProxyInit();
}

void __attribute__((destructor)) Quit(void)
{
	ProxyQuit();
}

#endif
