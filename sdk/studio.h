#define STUDIO_NF_FLATSHADE (1<<0)
#define STUDIO_NF_CHROME (1<<1)
#define STUDIO_NF_FULLBRIGHT (1<<2)
#define STUDIO_NF_NOMIPS (1<<3)
#define STUDIO_NF_ALPHA (1<<4)
#define STUDIO_NF_ADDITIVE (1<<5)
#define STUDIO_NF_MASKED (1<<6)

#define STUDIO_LOOPING (1<<0)

typedef struct
{
	int id;
	int version;
	char name[64];
	int length;
	vec3_t eyeposition;
	vec3_t min;
	vec3_t max;
	vec3_t bbmin;
	vec3_t bbmax;
	int flags;
	int numbones;
	int boneindex;
	int numbonecontrollers;
	int bonecontrollerindex;
	int numhitboxes;
	int hitboxindex;
	int numseq;
	int seqindex;
	int numseqgroups;
	int seqgroupindex;
	int numtextures;
	int textureindex;
	int texturedataindex;
	int numskinref;
	int numskinfamilies;
	int skinindex;
	int numbodyparts;
	int bodypartindex;
	int numattachments;
	int attachmentindex;
	int soundtable;
	int soundindex;
	int soundgroups;
	int soundgroupindex;
	int numtransitions;
	int transitionindex;
} studiohdr_t;

typedef struct
{
	char name[32];
	int parent;
	int flags;
	int bonecontroller[6];
	float value[6];
	float scale[6];
} mstudiobone_t;

typedef struct
{
	int bone;
	int type;
	float start;
	float end;
	int rest;
	int index;
} mstudiobonecontroller_t;

typedef struct
{
	char label[32];
	char name[64];
	int32 unused1;
	int unused2;
} mstudioseqgroup_t;

typedef struct
{
	char label[32];
	float fps;
	int flags;
	int activity;
	int actweight;
	int numevents;
	int eventindex;
	int numframes;
	int numpivots;
	int pivotindex;
	int motiontype;
	int motionbone;
	vec3_t linearmovement;
	int automoveposindex;
	int automoveangleindex;
	vec3_t bbmin;
	vec3_t bbmax;
	int numblends;
	int animindex;
	int blendtype[2];
	float blendstart[2];
	float blendend[2];
	int blendparent;
	int seqgroup;
	int entrynode;
	int exitnode;
	int nodeflags;
	int nextseq;
} mstudioseqdesc_t;

typedef struct
{
	char name[32];
	int type;
	int bone;
	vec3_t org;
	vec3_t vectors[3];
} mstudioattachment_t;

typedef struct
{
	unsigned short offset[6];
} mstudioanim_t;

typedef union
{
	struct
	{
		byte valid;
		byte total;
	} num;
	short value;
} mstudioanimvalue_t;

typedef struct
{
	char name[64];
	int nummodels;
	int base;
	int modelindex;
} mstudiobodyparts_t;

typedef struct
{
	char name[64];
	int flags;
	int width;
	int height;
	int index;
} mstudiotexture_t;

typedef struct
{
	char name[64];
	int type;
	float boundingradius;
	int nummesh;
	int meshindex;
	int numverts;
	int vertinfoindex;
	int vertindex;
	int numnorms;
	int norminfoindex;
	int normindex;
	int numgroups;
	int groupindex;
} mstudiomodel_t;

typedef struct
{
	int numtris;
	int triindex;
	int skinref;
	int numnorms;
	int normindex;
} mstudiomesh_t;
