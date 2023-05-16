typedef struct
{
	float brightness;
	float gamma;
	float lightgamma;
	float texgamma;

	float g;
	float g3;

	byte textable[256];
	byte lineartable[256];
} gammavars_t;

extern gammavars_t gammavars;

void GammaInit(void);
void GammaUpdate(void);
