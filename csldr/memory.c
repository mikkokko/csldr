#include "pch.h"

#define MEM_ALIGN (8 - 1)

#define BLOCK_SIZE (32 << 20)
#define TEMP_SIZE (32 << 20)

static byte *mem_block;
static size_t mem_used;

static byte *temp_block;
static size_t temp_used;
static int temp_count;

void Mem_Init(void)
{
	mem_block = (byte *)malloc(BLOCK_SIZE);
	if (!mem_block)
		Plat_Error("Memory allocation failed\n");

	temp_block = (byte *)malloc(TEMP_SIZE);
	if (!temp_block)
		Plat_Error("Memory allocation failed\n");
}

void Mem_Shutdown(void)
{
	free(mem_block);
	free(temp_block);
}

void *Mem_Alloc(size_t size)
{
	size = (size + MEM_ALIGN) & ~MEM_ALIGN;

	if (mem_used + size > BLOCK_SIZE)
	{
		Plat_Error("Out of memory\n");
		return NULL; /* never reached */
	}

	void *ptr = &mem_block[mem_used];
	mem_used += size;
	return ptr;
}

char *Mem_Strdup(const char *s)
{
	size_t n = strlen(s) + 1;
	char *ptr = (char *)Mem_Alloc(n);
	memcpy(ptr, s, n);
	return ptr;
}

void *Mem_TempAlloc(size_t size)
{
	size = (size + MEM_ALIGN) & ~MEM_ALIGN;

	if (temp_used + size > TEMP_SIZE)
	{
		Plat_Error("Out of memory\n");
		return NULL; /* never reached */
	}

	void *ptr = &temp_block[temp_used];
	temp_used += size;
	temp_count++;

	return ptr;
}

void Mem_TempFree(void *ptr)
{
	(void)ptr;

	assert(temp_count);
	assert(ptr >= (void *)temp_block && ptr < (void *)&temp_block[TEMP_SIZE]);

	temp_count--;

	if (!temp_count)
	{
		/* nothing uses the temp heap anymore so we can free it */
		temp_used = 0;
	}
}

void Mem_GetInfo(size_t *used, size_t *size)
{
	*used = mem_used + temp_used;
	*size = BLOCK_SIZE + TEMP_SIZE;
}
