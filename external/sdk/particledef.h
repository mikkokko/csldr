typedef enum {
  pt_static = 0,
  pt_grav = 1,
  pt_slowgrav = 2,
  pt_fire = 3,
  pt_explode = 4,
  pt_explode2 = 5,
  pt_blob = 6,
  pt_blob2 = 7,
  pt_vox_slowgrav = 8,
  pt_vox_grav = 9,
  pt_clientcustom = 10
} ptype_t;

typedef struct particle_s {
  Vector org;
  short color;
  short packedColor;
  struct particle_s* next;
  Vector vel;
  float ramp;
  float die;
  ptype_t type;
  void (*deathfunc)(struct particle_s*);
  void (*callback)(struct particle_s*, float);
  unsigned char context;
} particle_t;
