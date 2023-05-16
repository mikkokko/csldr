#if defined(_WIN32)
#define LIB_EXT ".dll"
#elif defined(__linux__)
#define LIB_EXT ".so"
#endif

#if defined(__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#if defined(_WIN32)
#define EXPORT EXTERNC __declspec(dllexport)
#elif defined(__linux__)
#define EXPORT EXTERNC __attribute__((visibility("default")))
#endif

#if defined(_WIN32)
extern int isWarzone;
#endif

void *Plat_Dlopen(const char *filename);
void *Plat_Dlsym(void *handle, const char *name);
void Plat_Dlclose(void *handle);
void Plat_CurrentModuleName(char *name, size_t size);
void Plat_Error(const char *fmt, ...);
