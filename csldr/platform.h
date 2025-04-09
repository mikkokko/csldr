#if defined(__cplusplus)
#define EXTERNC extern "C"
#else
#define EXTERNC
#endif

#if defined(_MSC_VER)
#define EXPORT EXTERNC __declspec(dllexport)
#elif defined(__GNUC__)
#define EXPORT EXTERNC __attribute__((visibility("default")))
#endif

#if defined(_WIN32)
extern int isWarzone;
#endif

void *Plat_Dlopen(const char *filename);

/* same as Plat_Dlopen, but exits with an error message on failure */
void *Plat_CheckedDlopen(const char *filename);

void *Plat_Dlsym(void *handle, const char *name);
void Plat_Dlclose(void *handle);
size_t Plat_CurrentModuleName(char *name, size_t size);
void Plat_Error(const char *fmt, ...);
