extern cl_enginefunc_t gEngfuncs;
extern cldll_func_t cl_funcs;

extern void *(*pCreateInterface)(const char *, int *);

extern bool isCzero;

extern float clientTime;

extern int screenWidth, screenHeight;

extern bool canOpenGL;

void Hk_HudInit(void);
void Hk_HudShutdown(void);
void Hk_HudFrame(double time);
int Hk_Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion);
int Hk_UpdateClientData(client_data_t *pcldata, float flTime);
