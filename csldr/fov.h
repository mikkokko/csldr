extern float fovDifference;
extern float currentFov;

void FovThink(void);

extern int (*Og_MsgFunc_SetFOV)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_SetFOV(const char *pszName, int iSize, void *pbuf);
