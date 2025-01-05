extern int currentWeaponId;

extern int xhairPlayerFlags;
extern float xhairPlayerSpeed;
extern int xhairWeaponFlags;
extern int xhairShotsFired;

extern int (*Og_MsgFunc_CurWeapon)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_CurWeapon(const char *pszName, int iSize, void *pbuf);

extern int (*Og_MsgFunc_HideWeapon)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_HideWeapon(const char *pszName, int iSize, void *pbuf);

extern int (*Og_MsgFunc_NVGToggle)(const char *pszName, int iSize, void *pbuf);
int Hk_MsgFunc_NVGToggle(const char *pszName, int iSize, void *pbuf);

void Hk_FillRGBABlend(int x, int y, int w, int h, int r, int g, int b, int a);

void HudInit(void);
int Hk_HudRedraw(float time, int intermission);
int Hk_HudVidInit(void);
