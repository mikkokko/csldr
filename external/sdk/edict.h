typedef struct edict_s {
  qboolean free;
  int serialnumber;
  link_t area;
  int headnode;
  int num_leafs;
  short leafnums[48];
  float freetime;
  void* pvPrivateData;
  entvars_t v;
} edict_t;
