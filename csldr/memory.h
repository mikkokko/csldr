// for quickly allocating memory that doesn't need to be freed
void Mem_Init(void);
void Mem_Shutdown(void);
void *Mem_Alloc(size_t size);
char *Mem_Strdup(const char *s);

// same as Mem_Alloc but doesn't move the pointer so the
// next call to Mem_Alloc/Mem_AllocTemp will replace it
void *Mem_AllocTemp(size_t size);

void Mem_GetInfo(size_t *used, size_t *size);
