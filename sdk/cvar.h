#define FCVAR_ARCHIVE (1 << 0)
#define FCVAR_USERINFO (1 << 1)
#define FCVAR_SERVER (1 << 2)
#define FCVAR_EXTDLL (1 << 3)
#define FCVAR_CLIENTDLL (1 << 4)
#define FCVAR_PROTECTED (1 << 5)
#define FCVAR_SPONLY (1 << 6)
#define FCVAR_PRINTABLEONLY (1 << 7)
#define FCVAR_UNLOGGED (1 << 8)
#define FCVAR_NOEXTRAWHITEPACE (1 << 9)

typedef struct cvar_s
{
	const char *name;
	const char *string;
	int flags;
	float value;
	struct cvar_s *next;
} cvar_t;
