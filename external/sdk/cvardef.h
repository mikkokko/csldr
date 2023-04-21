#define FCVAR_ARCHIVE (1 << 0)

typedef struct cvar_s {
  char* name;
  char* string;
  int flags;
  float value;
  struct cvar_s* next;
} cvar_t;
