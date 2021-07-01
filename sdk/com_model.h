UNIMPLEMENTED_TYPE(dmodel_t);
UNIMPLEMENTED_TYPE(mplane_t);
UNIMPLEMENTED_TYPE(mleaf_t);
UNIMPLEMENTED_TYPE(mvertex_t);
UNIMPLEMENTED_TYPE(medge_t);
UNIMPLEMENTED_TYPE(mnode_t);
UNIMPLEMENTED_TYPE(mtexinfo_t);
UNIMPLEMENTED_TYPE(msurface_t);
UNIMPLEMENTED_TYPE(dclipnode_t);
UNIMPLEMENTED_TYPE(texture_t);

typedef enum
{
	mod_brush,
	mod_sprite,
	mod_alias,
	mod_studio
} modtype_t;

typedef enum
{
	ST_SYNC,
	ST_RAND
} synctype_t;

typedef struct
{
	dclipnode_t *clipnodes;
	mplane_t *planes;
	int firstclipnode;
	int lastclipnode;
	vec3_t clip_mins;
	vec3_t clip_maxs;
} hull_t;

typedef struct
{
	void *data;
} cache_user_t;

typedef struct
{
	char name[64];
	qboolean needload;
	modtype_t type;
	int numframes;
	synctype_t synctype;
	int flags;
	vec3_t mins;
	vec3_t maxs;
	float radius;
	int firstmodelsurface;
	int nummodelsurfaces;
	int numsubmodels;
	dmodel_t *submodels;
	int numplanes;
	mplane_t *planes;
	int numleafs;
	mleaf_t *leafs;
	int numvertexes;
	mvertex_t *vertexes;
	int numedges;
	medge_t *edges;
	int numnodes;
	mnode_t *nodes;
	int numtexinfo;
	mtexinfo_t *texinfo;
	int numsurfaces;
	msurface_t *surfaces;
	int numsurfedges;
	int *surfedges;
	int numclipnodes;
	dclipnode_t *clipnodes;
	int nummarksurfaces;
	msurface_t **marksurfaces;
	hull_t hulls[4];
	int numtextures;
	texture_t **textures;
	byte *visdata;
	color24 *lightdata;
	const char *entities;
	cache_user_t cache;
} model_t;
