extern vec3_t v_vieworg;
extern vec3_t v_viewforward, v_viewright, v_viewup;

extern cvar_t *viewmodel_fov;
extern cvar_t *viewmodel_hands;

extern cvar_t *fov_horplus;
extern cvar_t *fov_lerp;

void ViewInit(void);
void Hk_CalcRefdef(ref_params_t *pparams);
