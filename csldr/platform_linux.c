#include "pch.h"

#if defined(__linux__)

#define __USE_GNU /* Dl_info */
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

	dladdr(__builtin_return_address(0), &info);
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
