extern cvar_t *cl_righthand;

void ShellInit(void);

extern int (*Og_MsgFunc_Brass)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_Brass(const char *pszName, int iSize, void *pbuf);

extern TEMPENTITY *(*Og_TempModel)(float *pos, float *dir, float *angles, float life, int modelIndex, int soundtype);
TEMPENTITY *Hk_TempModel(float *pos, float *dir, float *angles, float life, int modelIndex, int soundtype);

void Hk_HookEvent(const char *name, void (*pfnEvent)(event_args_t *));
