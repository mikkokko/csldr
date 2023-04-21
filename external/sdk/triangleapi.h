typedef enum {
  TRI_FRONT = 0,
  TRI_NONE = 1
} TRICULLSTYLE;

typedef struct
{
  int version;
  void (*RenderMode)(int);
  void (*Begin)(int);
  void (*End)(void);
  void (*Color4f)(float, float, float, float);
  void (*Color4ub)(unsigned char, unsigned char, unsigned char, unsigned char);
  void (*TexCoord2f)(float, float);
  void (*Vertex3fv)(float*);
  void (*Vertex3f)(float, float, float);
  void (*Brightness)(float);
  void (*CullFace)(TRICULLSTYLE);
  int (*SpriteTexture)(model_t*, int);
  int (*WorldToScreen)(float*, float*);
  void (*Fog)(float*, float, float, int);
  void (*ScreenToWorld)(float*, float*);
  void (*GetMatrix)(int, float*);
  int (*BoxInPVS)(float*, float*);
  void (*LightAtPoint)(float*, float*);
  void (*Color4fRendermode)(float, float, float, float, int);
  void (*FogParams)(float, int);
} triangleapi_t;
