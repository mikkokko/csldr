#include "pch.h"

#if defined(__linux__)

#define __USE_GNU 1 /* Dl_info */
#include <dlfcn.h>

void *Plat_Dlopen(const char *filename)
{
	return dlopen(filename, RTLD_NOW);
}

void *Plat_Dlsym(void *handle, const char *name)
{
	return dlsym(handle, name);
}

void Plat_Dlclose(void *handle)
{
	dlclose(handle);
}

void Plat_CurrentModuleName(char *name, size_t size)
{
	Dl_info info;

	dladdr(Plat_CurrentModuleName, &info);
	/* mikkotodo strncpy bad */
	strncpy(name, info.dli_fname, size);
	name[size - 1] = '\0';
}

void Plat_Error(const char *fmt, ...)
{
	va_list ap;
	char buffer[1024];

	va_start(ap, fmt);
	int length = vsnprintf(buffer, sizeof(buffer), fmt, ap);
	va_end(ap);

	fwrite(buffer, 1, length, stderr);

	// display a message box if we can
	// i don't want to link to sdl2 directly nor do i want it as a build dependency
	void *libSDL2 = dlopen("libSDL2.so", RTLD_LAZY);
	if (libSDL2)
	{
		typedef int (*SDL_ShowSimpleMessageBox_t)(uint32 flags, const char *title, const char *message, void *window);
		SDL_ShowSimpleMessageBox_t SDL_ShowSimpleMessageBox = (SDL_ShowSimpleMessageBox_t)dlsym(libSDL2, "SDL_ShowSimpleMessageBox");
	
		if (SDL_ShowSimpleMessageBox)
		{
			SDL_ShowSimpleMessageBox(0, "Error", buffer, NULL);
		}
	}

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
