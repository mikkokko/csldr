#include "pch.h"

#define BLOCK_SIZE (128 << 20)

static byte *mem_block;
static size_t mem_used;

void Mem_Init(void)
{
	mem_block = (byte *)malloc(BLOCK_SIZE);
	if (!mem_block)
		Plat_Error("Memory allocation failed\n");
}

void Mem_Shutdown(void)
{
	free(mem_block);
}

void *Mem_Alloc(size_t size)
{
	void *ptr = Mem_AllocTemp(size);
	mem_used += size;
	return ptr;
}

char *Mem_Strdup(const char *s)
{
	size_t n = strlen(s) + 1;
	char *ptr = Mem_Alloc(n);
	memcpy(ptr, s, n);
	return ptr;
}

void *Mem_AllocTemp(size_t size)
{
	/* special case */
	if (!size)
		return mem_block;

	size = (size + 15) & ~15;

	if (mem_used + size > BLOCK_SIZE)
	{
		Plat_Error("Out of memory\n");
		return NULL; /* never reached */
	}

	return &mem_block[mem_used];
}

void Mem_GetInfo(size_t *used, size_t *size)
{
	*used = mem_used;
	*size = BLOCK_SIZE;
}
