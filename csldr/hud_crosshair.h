extern int currentWeaponId;

extern int (*Og_MsgFunc_CurWeapon)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);

extern int (*Og_MsgFunc_HideWeapon)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf);

void HudInit(void);
int Hk_HudRedraw(float time, int intermission);
int Hk_HudVidInit(void);
