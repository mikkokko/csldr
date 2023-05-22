extern float fovScale;

void FovThink(void);
float GetCurrentFov(void);

extern int (*Og_MsgFunc_SetFOV)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf);
