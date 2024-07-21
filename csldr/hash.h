// 32-bit FNV-1a hash
#define FNV_OFFSET_BASIS32 0x811c9dc5
#define FNV_PRIME32 0x01000193

inline static uint32 HashString(const char *s)
{
	uint32 hash = FNV_OFFSET_BASIS32;

	for (; *s; s++)
	{
		hash ^= (unsigned char)*s;
		hash *= FNV_PRIME32;
	}

	return hash;
}
