// for quickly allocating memory that doesn't need to be freed
void Mem_Init(void);
void Mem_Shutdown(void);
void *Mem_Alloc(size_t size);
char *Mem_Strdup(const char *s);

void *Mem_TempAlloc(size_t size);
void Mem_TempFree(void *ptr);

void Mem_GetInfo(size_t *used, size_t *size);
