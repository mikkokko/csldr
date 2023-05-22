#define FCVAR_ARCHIVE (1 << 0)

typedef struct cvar_s {
  sdk_string_const char* name;
  sdk_string_const char* string;
  int flags;
  float value;
  struct cvar_s* next;
} cvar_t;
