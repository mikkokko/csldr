#include "pch.h"

#ifndef SHADERS_FROM_DISK /* xxd'd shaders */
#include "studio_cpu_vert.h"
#include "studio_frag.h"
#include "studio_gpu_vert.h"
#endif

// only for gpu skinning
static GLuint studio_ubo;
static GLint bones_offset;

// mikkotodo move elsewhere
static cvar_t *r_glowshellfreq;

// for cpu chrome
int studio_drawcount;

static vec3_t chrome_origin;

static dlight_t *cl_elights;

#define MAX_ELIGHTS 3

// store these globally for unknown reasons
static int elight_num;
static float elight_pos[MAX_ELIGHTS][4];
static float elight_color[MAX_ELIGHTS][3];

static struct
{
	GLuint program;

	GLint u_chromeorg;
	GLint u_chromeright;
	GLint u_ambientlight;
	GLint u_lightvec;
	GLint u_shadelight;
	GLint u_colormix;

	GLint u_texture;

	GLint u_tex_flatshade;
	GLint u_tex_chrome;
	GLint u_tex_fullbright;

	GLint u_lightgamma;
	GLint u_brightness;
	GLint u_invgamma;
	GLint u_g3;

	GLint u_additive;

	GLint u_glowshell;
	GLint u_glowshell_color;

	GLint u_elight_num;
	GLint u_elight_pos;
	GLint u_elight_color;
} shader_studio;

enum
{
	shader_studio_a_pos = 0,
	shader_studio_a_normal = 1,
	shader_studio_a_texcoord = 2,

	// only for gpu skinning
	shader_studio_a_bones = 3
};

static const attribute_t studio_attributes_cpu[] =
{
	{ shader_studio_a_pos, "a_pos" },
	{ shader_studio_a_normal, "a_normal" },
	{ shader_studio_a_texcoord, "a_texcoord" }
};

static const attribute_t studio_attributes_gpu[] =
{
	{ shader_studio_a_pos, "a_pos" },
	{ shader_studio_a_normal, "a_normal" },
	{ shader_studio_a_texcoord, "a_texcoord" },
	{ shader_studio_a_bones, "a_bones" }
};

static const uniform_t studio_uniforms[] =
{
	{ &shader_studio.u_chromeorg, "u_chromeorg" },
	{ &shader_studio.u_chromeright, "u_chromeright" },

	{ &shader_studio.u_ambientlight, "u_ambientlight" },
	{ &shader_studio.u_shadelight, "u_shadelight" },
	{ &shader_studio.u_lightvec, "u_lightvec" },
	{ &shader_studio.u_colormix, "u_colormix" },

	{ &shader_studio.u_texture, "u_texture" },

	{ &shader_studio.u_tex_flatshade, "u_tex_flatshade" },
	{ &shader_studio.u_tex_chrome, "u_tex_chrome" },
	{ &shader_studio.u_tex_fullbright, "u_tex_fullbright" },

	{ &shader_studio.u_lightgamma, "u_lightgamma" },
	{ &shader_studio.u_brightness, "u_brightness" },
	{ &shader_studio.u_invgamma, "u_invgamma" },
	{ &shader_studio.u_g3, "u_g3" },

	{ &shader_studio.u_additive, "u_additive" },

	{ &shader_studio.u_glowshell, "u_glowshell" },
	{ &shader_studio.u_glowshell_color, "u_glowshell_color" },

	{ &shader_studio.u_elight_num, "u_elight_num" },
	{ &shader_studio.u_elight_pos, "u_elight_pos" },
	{ &shader_studio.u_elight_color, "u_elight_color" }
};

void R_StudioInit(void)
{
	r_glowshellfreq = gEngfuncs.pfnGetCvarPointer("r_glowshellfreq");

	if (studio_gpuskin)
	{
		LOAD_SHADER(studio, studio_gpu, studio, studio_attributes_gpu, studio_uniforms);
	}
	else
	{
		LOAD_SHADER(studio, studio_cpu, studio, studio_attributes_cpu, studio_uniforms);
	}

	if (studio_gpuskin)
	{
		GLuint block_index = glGetUniformBlockIndex(shader_studio.program, "bones");

		GLuint bones_index;
		const char *bones_name = "u_bones";
		glGetUniformIndices(shader_studio.program, 1, &bones_name, &bones_index);
		glGetActiveUniformsiv(shader_studio.program, 1, &bones_index, GL_UNIFORM_OFFSET, &bones_offset);
		glUniformBlockBinding(shader_studio.program, block_index, 0);

		// bold size assumption
		glGenBuffers(1, &studio_ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, studio_ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(mat3x4_t) * 128, NULL, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, studio_ubo);
	}

	// get pointer to first elight
	cl_elights = gEngfuncs.pEfxAPI->CL_AllocElight(0);
}

void R_StudioEntityLight(studio_context_t *ctx)
{
	elight_num = 0;

	float lstrength[MAX_ELIGHTS];
	memset(lstrength, 0, sizeof(lstrength));

	float max_radius = 1000000;
	float min_radius = 0;

	// ctx->entity is not set yet... mikkotodo revisit
	(void)ctx;
	cl_entity_t *entity = IEngineStudio.GetCurrentEntity();

	// asssume that max elights is 64
	for (int i = 0; i < 64; i++)
	{
		dlight_t *elight = &cl_elights[i];

		if (elight->die <= clientTime)
			continue;

		if (elight->radius <= min_radius)
			continue;

		if ((elight->key & 0xFFF) == entity->index)
		{
			int attachment = (elight->key >> 12) & 0xF;

			if (attachment)
				VectorCopy(elight->origin, entity->attachment[attachment]);
			else
				VectorCopy(elight->origin, entity->origin);
		}

		vec3_t dir;
		VectorSubtract(entity->origin, elight->origin, dir);

		float dist = DotProduct(dir, dir);

		float r2 = elight->radius * elight->radius;

		float strength;

		if (dist <= r2)
		{
			strength = 1;
		}
		else
		{
			strength = r2 / dist;

			if (strength <= 0.004f)
				continue;
		}

		int index = elight_num;

		if (elight_num >= MAX_ELIGHTS)
		{
			index = -1;

			for (int j = 0; j < elight_num; j++)
			{
				if (lstrength[j] < max_radius && lstrength[j] < strength)
				{
					index = j;
					max_radius = lstrength[j];
				}
			}
		}

		if (index == -1)
			continue;

		lstrength[index] = strength;

		VectorCopy(elight_pos[index], elight->origin);
		elight_pos[index][3] = r2;

		elight_color[index][0] = (float)gammavars.lineartable[elight->color.r] * (1.0f / 255.0f);
		elight_color[index][1] = (float)gammavars.lineartable[elight->color.g] * (1.0f / 255.0f);
		elight_color[index][2] = (float)gammavars.lineartable[elight->color.b] * (1.0f / 255.0f);

		if (index >= elight_num)
		{
			elight_num = index + 1;
		}
	}
}

void R_StudioInitContext(studio_context_t *ctx, cl_entity_t *entity, model_t *model, studiohdr_t *header)
{
	assert(entity);
	assert(model);
	assert(header);

	ctx->entity = entity;
	ctx->model = model;
	ctx->header = header;
	ctx->cache = GetStudioCache(ctx->model, header);

	// mikkotodo move? this never changes
	ctx->bonetransform = (mat3x4_t(*)[])IEngineStudio.StudioGetBoneTransform();
}

void R_StudioSetupLighting(studio_context_t *ctx, alight_t *lighting)
{
	ctx->ambientlight = (float)lighting->ambientlight * (1.0f / 255.0f);
	ctx->shadelight = (float)lighting->shadelight * (1.0f / 255.0f);
	VectorCopy(ctx->lightcolor, lighting->color);
	VectorCopy(ctx->lightvec, lighting->plightvec);
}

// different to engine's but doesn't matter here
inline static int RandomLong(int low, int high)
{
	return low + (rand() % (high - low + 1));
}

// like engine's CL_FxBlend but doesn't modify renderamt (already done by engine)
static int CalcFxBlend(const cl_entity_t *ent)
{
	int renderfx = ent->curstate.renderfx;
	float offset = (float)ent->curstate.number * 363;

	int amount;

	switch (renderfx)
	{
	case kRenderFxPulseSlow:
		amount = (int)(sinf(clientTime + clientTime + offset) * 16 + ent->curstate.renderamt);
		break;

	case kRenderFxPulseFast:
		amount = (int)(sinf(clientTime * 8 + offset) * 16 + ent->curstate.renderamt);
		break;

	case kRenderFxPulseSlowWide:
		amount = (int)(sinf(clientTime + clientTime + offset) * 64 + ent->curstate.renderamt);
		break;

	case kRenderFxPulseFastWide:
		amount = (int)(sinf(clientTime * 8 + offset) * 64 + ent->curstate.renderamt);
		break;

	case kRenderFxFadeSlow:
		amount = ent->curstate.renderamt;
		break;

	case kRenderFxFadeFast:
		amount = ent->curstate.renderamt;
		break;

	case kRenderFxSolidSlow:
		amount = ent->curstate.renderamt;
		break;

	case kRenderFxSolidFast:
		amount = ent->curstate.renderamt;
		break;

	case kRenderFxStrobeSlow:
		if ((sinf(clientTime * 4 + offset) * 20) >= 0)
			amount = ent->curstate.renderamt;
		else
			amount = 0;
		break;

	case kRenderFxStrobeFast:
		if ((sinf(clientTime * 16 + offset) * 20) >= 0)
			amount = ent->curstate.renderamt;
		else
			amount = 0;
		break;

	case kRenderFxStrobeFaster:
		if ((sinf(clientTime * 36 + offset) * 20) >= 0)
			amount = ent->curstate.renderamt;
		else
			amount = 0;
		break;

	case kRenderFxFlickerSlow:
		if (((sinf(clientTime + clientTime) + sinf(clientTime * 17 + offset)) * 20) >= 0)
			amount = ent->curstate.renderamt;
		else
			amount = 0;
		break;

	case kRenderFxFlickerFast:
		if (((sinf(clientTime * 16) + sinf(clientTime * 23 + offset)) * 20) >= 0)
			amount = ent->curstate.renderamt;
		else
			amount = 0;
		break;

	case kRenderFxDistort:
	case kRenderFxHologram:
	{
		vec3_t dir;
		VectorSubtract(ent->origin, v_vieworg, dir);
		float dist = DotProduct(v_viewforward, dir);

		if (renderfx == kRenderFxDistort)
			dist = 1;

		if (dist <= 0)
		{
			amount = 0;
		}
		else
		{
			if (dist > 100)
				amount = RandomLong(-32, 31) + (int)((1 - (dist - 100) * 0.0025f) * 180);
			else
				amount = RandomLong(-32, 31) + 180;
		}
		break;
	}

	default:
		amount = ent->curstate.renderamt;
		break;
	}

	return CLAMP(amount, 0, 255);
}

void R_StudioSetupRenderer(studio_context_t *ctx)
{
	// done to avoid texture bugs
	glPushAttrib(GL_TEXTURE_BIT);
	glActiveTexture(GL_TEXTURE0);

	// setup our 70 different uniforms
	glUseProgram(shader_studio.program);

	glUniform1i(shader_studio.u_texture, 0);

	glUniform1f(shader_studio.u_ambientlight, ctx->ambientlight);
	glUniform1f(shader_studio.u_shadelight, ctx->shadelight);
	glUniform3fv(shader_studio.u_lightvec, 1, ctx->lightvec);

	float colormix[4];
	colormix[0] = ctx->lightcolor[0];
	colormix[1] = ctx->lightcolor[1];
	colormix[2] = ctx->lightcolor[2];
	colormix[3] = CalcFxBlend(ctx->entity) * (1.0f / 255);
	glUniform4fv(shader_studio.u_colormix, 1, colormix);

	glUniform1f(shader_studio.u_lightgamma, gammavars.lightgamma);
	glUniform1f(shader_studio.u_brightness, gammavars.brightness);
	glUniform1f(shader_studio.u_invgamma, gammavars.g);
	glUniform1f(shader_studio.u_g3, gammavars.g3);

	glUniform1i(shader_studio.u_additive, (ctx->entity->curstate.rendermode == kRenderTransAdd));

	// bruh
	if (ctx->entity->curstate.renderfx == kRenderFxGlowShell)
	{
		chrome_origin[0] = cosf(r_glowshellfreq->value * clientTime) * 4000.0f;
		chrome_origin[1] = sinf(r_glowshellfreq->value * clientTime) * 4000.0f;
		chrome_origin[2] = cosf(r_glowshellfreq->value * clientTime * 0.33f) * 4000.0f;
		glUniform1i(shader_studio.u_glowshell, true);

		float data[4];
		data[0] = (float)ctx->entity->curstate.rendercolor.r * (1.0f / 255);
		data[1] = (float)ctx->entity->curstate.rendercolor.g * (1.0f / 255);
		data[2] = (float)ctx->entity->curstate.rendercolor.b * (1.0f / 255);
		data[3] = (float)ctx->entity->curstate.renderamt * 0.05f;
		glUniform4fv(shader_studio.u_glowshell_color, 1, data);
	}
	else
	{
		VectorCopy(chrome_origin, v_vieworg);
		glUniform1i(shader_studio.u_glowshell, false);
	}

	glUniform3fv(shader_studio.u_chromeorg, 1, chrome_origin);
	glUniform3fv(shader_studio.u_chromeright, 1, v_viewright);

	glUniform1i(shader_studio.u_elight_num, elight_num);

	if (elight_num)
	{
		glUniform4fv(shader_studio.u_elight_pos, elight_num, &elight_pos[0][0]);
		glUniform3fv(shader_studio.u_elight_color, elight_num, &elight_color[0][0]);
	}

	// setup other stuff

	// calling GetViewEntity is ok here
	if (cl_righthand->value && ctx->entity == IEngineStudio.GetViewEntity())
		glDisable(GL_CULL_FACE);

	glBindBuffer(GL_ARRAY_BUFFER, ctx->cache->studio_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->cache->studio_ebo);

	if (studio_gpuskin)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, studio_ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, bones_offset, ctx->header->numbones * sizeof(mat3x4_t), &(*ctx->bonetransform)[0][0][0]);
	}

	if (studio_gpuskin)
	{
		glEnableVertexAttribArray(shader_studio_a_pos);
		glEnableVertexAttribArray(shader_studio_a_normal);
		glEnableVertexAttribArray(shader_studio_a_texcoord);
		glEnableVertexAttribArray(shader_studio_a_bones);

		glVertexAttribPointer(shader_studio_a_pos, 3, GL_FLOAT, GL_FALSE, sizeof(studio_gpu_vert_t), (void *)Q_OFFSETOF(studio_gpu_vert_t, pos));
		glVertexAttribPointer(shader_studio_a_normal, 3, GL_FLOAT, GL_FALSE, sizeof(studio_gpu_vert_t), (void *)Q_OFFSETOF(studio_gpu_vert_t, norm));
		glVertexAttribPointer(shader_studio_a_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(studio_gpu_vert_t), (void *)Q_OFFSETOF(studio_gpu_vert_t, texcoord));
		glVertexAttribPointer(shader_studio_a_bones, 2, GL_FLOAT, GL_FALSE, sizeof(studio_gpu_vert_t), (void *)Q_OFFSETOF(studio_gpu_vert_t, bones));
	}
	else
	{
		glEnableVertexAttribArray(shader_studio_a_pos);
		glEnableVertexAttribArray(shader_studio_a_normal);
		glEnableVertexAttribArray(shader_studio_a_texcoord);

		glVertexAttribPointer(shader_studio_a_pos, 3, GL_FLOAT, GL_FALSE, sizeof(studio_cpu_vert_t), (void *)Q_OFFSETOF(studio_cpu_vert_t, pos));
		glVertexAttribPointer(shader_studio_a_normal, 3, GL_FLOAT, GL_FALSE, sizeof(studio_cpu_vert_t), (void *)Q_OFFSETOF(studio_cpu_vert_t, norm));
		glVertexAttribPointer(shader_studio_a_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(studio_cpu_vert_t), (void *)Q_OFFSETOF(studio_cpu_vert_t, texcoord));
	}
}

void R_StudioRestoreRenderer(studio_context_t *ctx)
{
	glPopAttrib();

	glUseProgram(0);

	// restore opengl state
	if (cl_righthand->value && ctx->entity == IEngineStudio.GetViewEntity())
		glEnable(GL_CULL_FACE);

	glDisableVertexAttribArray(shader_studio_a_pos);
	glDisableVertexAttribArray(shader_studio_a_normal);
	glDisableVertexAttribArray(shader_studio_a_texcoord);

	if (studio_gpuskin)
	{
		glDisableVertexAttribArray(shader_studio_a_bones);
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (studio_gpuskin)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
}

void R_StudioSetupModel(studio_context_t *ctx, int bodypart_index)
{
	if (bodypart_index > ctx->header->numbodyparts)
		bodypart_index = 0;

	mstudiobodyparts_t *bodyparts = (mstudiobodyparts_t *)((byte *)ctx->header + ctx->header->bodypartindex);
	mstudiobodyparts_t *bodypart = &bodyparts[bodypart_index];

	mem_bodypart_t *mem_bodypart = &ctx->cache->bodyparts[bodypart_index];

	int model_index = (ctx->entity->curstate.body / bodypart->base) % bodypart->nummodels;

	mstudiomodel_t *submodels = (mstudiomodel_t *)((byte *)ctx->header + bodypart->modelindex);

	ctx->submodel = &submodels[model_index];
	ctx->mem_submodel = &mem_bodypart->models[model_index];
}

static void CalcChromeCPU(float *out, int bone_id, mat3x4_t bone, vec3_t normal)
{
	// cache the results like the engine does
	static int chrome_age[128];
	static vec3_t chrome_up[128];
	static vec3_t chrome_side[128];

	float *up_anim = chrome_up[bone_id];
	float *side_anim = chrome_side[bone_id];

	if (chrome_age[bone_id] != studio_drawcount)
	{
		chrome_age[bone_id] = studio_drawcount;

		vec3_t dir;
		dir[0] = bone[0][3] - chrome_origin[0];
		dir[1] = bone[1][3] - chrome_origin[1];
		dir[2] = bone[2][3] - chrome_origin[2];

		VectorNormalize(dir);

		vec3_t up;
		CrossProduct(dir, v_viewright, up);
		VectorNormalize(up);

		vec3_t side;
		CrossProduct(dir, up, side);
		VectorNormalize(side);

		up_anim[0] = up[0] * bone[0][0] + up[1] * bone[1][0] + up[2] * bone[2][0];
		up_anim[1] = up[0] * bone[0][1] + up[1] * bone[1][1] + up[2] * bone[2][1];
		up_anim[2] = up[0] * bone[0][2] + up[1] * bone[1][2] + up[2] * bone[2][2];

		side_anim[0] = side[0] * bone[0][0] + side[1] * bone[1][0] + side[2] * bone[2][0];
		side_anim[1] = side[0] * bone[0][1] + side[1] * bone[1][1] + side[2] * bone[2][1];
		side_anim[2] = side[0] * bone[0][2] + side[1] * bone[1][2] + side[2] * bone[2][2];
	}

	// mikkotodo why the fuck this this needed???
	out[0] = 1.0f - (DotProduct(normal, side_anim) + 1.0f) * 0.5f;
	out[1] = (DotProduct(normal, up_anim) + 1.0f) * 0.5f;
}

void R_StudioDrawPoints(studio_context_t *ctx)
{
	studiohdr_t *header = ctx->header;
	studiohdr_t *textureheader = R_LoadTextures(ctx->model, header);

	mstudiomodel_t *submodel = ctx->submodel;
	mem_model_t *mem_submodel = ctx->mem_submodel;

	mstudiomesh_t *meshes = (mstudiomesh_t *)((byte *)header + submodel->meshindex);
	mstudiotexture_t *textures = (mstudiotexture_t *)((byte *)textureheader + textureheader->textureindex);

	short *skins = (short *)((byte *)textureheader + textureheader->skinindex);
	int skin = ctx->entity->curstate.skin;

	if (skin && skin < textureheader->numskinfamilies)
	{
		skins = &skins[skin * textureheader->numskinref];
	}

	if (!studio_gpuskin)
	{
		studio_cpu_vert_t *anim_verts = (studio_cpu_vert_t *)Mem_AllocTemp(sizeof(studio_cpu_vert_t) * mem_submodel->num_verts);
		int vert_ofs = 0;

		for (int i = 0; i < submodel->nummesh; i++)
		{
			mstudiomesh_t *mesh = &meshes[i];
			mem_mesh_t *mem_mesh = &mem_submodel->meshes[i];
			mstudiotexture_t *texture = &textures[skins[mesh->skinref]];

			int forceflags = IEngineStudio.GetForceFaceFlags();
			int flags = texture->flags | forceflags;

			if (flags & STUDIO_NF_CHROME)
			{
				for (unsigned int j = 0; j < mem_mesh->num_verts; j++)
				{
					studio_cpu_vert_t *src = &ctx->cache->verts[mem_mesh->ofs_verts + j];
					studio_vertbone_t *srcbone = &ctx->cache->vertbones[mem_mesh->ofs_verts + j];
					studio_cpu_vert_t *dst = &anim_verts[vert_ofs++];

					mat3x4_t *bone = &(*ctx->bonetransform)[srcbone->bones[0]];
					dst->pos[0] = DotProduct(src->pos, (*bone)[0]) + (*bone)[0][3];
					dst->pos[1] = DotProduct(src->pos, (*bone)[1]) + (*bone)[1][3];
					dst->pos[2] = DotProduct(src->pos, (*bone)[2]) + (*bone)[2][3];

					bone = &(*ctx->bonetransform)[srcbone->bones[1]];
					dst->norm[0] = DotProduct(src->norm, (*bone)[0]);
					dst->norm[1] = DotProduct(src->norm, (*bone)[1]);
					dst->norm[2] = DotProduct(src->norm, (*bone)[2]);

					CalcChromeCPU(dst->texcoord, srcbone->bones[1], (*ctx->bonetransform)[srcbone->bones[1]], src->norm);
				}
			}
			else
			{
				for (unsigned int j = 0; j < mem_mesh->num_verts; j++)
				{
					studio_cpu_vert_t *src = &ctx->cache->verts[mem_mesh->ofs_verts + j];
					studio_vertbone_t *srcbone = &ctx->cache->vertbones[mem_mesh->ofs_verts + j];
					studio_cpu_vert_t *dst = &anim_verts[vert_ofs++];

					mat3x4_t *bone = &(*ctx->bonetransform)[srcbone->bones[0]];
					dst->pos[0] = DotProduct(src->pos, (*bone)[0]) + (*bone)[0][3];
					dst->pos[1] = DotProduct(src->pos, (*bone)[1]) + (*bone)[1][3];
					dst->pos[2] = DotProduct(src->pos, (*bone)[2]) + (*bone)[2][3];

					bone = &(*ctx->bonetransform)[srcbone->bones[1]];
					dst->norm[0] = DotProduct(src->norm, (*bone)[0]);
					dst->norm[1] = DotProduct(src->norm, (*bone)[1]);
					dst->norm[2] = DotProduct(src->norm, (*bone)[2]);

					dst->texcoord[0] = src->texcoord[0];
					dst->texcoord[1] = src->texcoord[1];
				}
			}
		}

		// mikkotodo this copies way too much data
		glBufferSubData(GL_ARRAY_BUFFER,
			mem_submodel->ofs_verts * sizeof(studio_cpu_vert_t),
			mem_submodel->num_verts * sizeof(studio_cpu_vert_t),
			anim_verts);
	}

	for (int i = 0; i < submodel->nummesh; i++)
	{
		mstudiomesh_t *mesh = &meshes[i];
		mstudiotexture_t *texture = &textures[skins[mesh->skinref]];
		mem_texture_t *mem_texture = &ctx->cache->textures[skins[mesh->skinref]];
		mem_mesh_t *mem_mesh = &mem_submodel->meshes[i];

		int forceflags = IEngineStudio.GetForceFaceFlags();
		int flags = texture->flags | forceflags;

		glUniform1i(shader_studio.u_tex_flatshade, (flags & STUDIO_NF_FLATSHADE) ? true : false);
		glUniform1i(shader_studio.u_tex_chrome, (flags & STUDIO_NF_CHROME) ? true : false);
		glUniform1i(shader_studio.u_tex_fullbright, (flags & STUDIO_NF_FULLBRIGHT) ? true : false);

		bool alphatest = (flags & STUDIO_NF_MASKED) ? true : false;
		bool additive = ((flags & STUDIO_NF_ADDITIVE) && ctx->entity->curstate.rendermode == kRenderNormal);

		if (alphatest)
		{
			glEnable(GL_ALPHA_TEST);
			glAlphaFunc(GL_GREATER, 0.5f);
			glDepthMask(GL_TRUE);
		}

		if (additive)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);
		}

		if (!(forceflags & STUDIO_NF_CHROME))
		{
			if (mem_texture->diffuse)
				glBindTexture(GL_TEXTURE_2D, mem_texture->diffuse);
			else
				glBindTexture(GL_TEXTURE_2D, texture->index);
		}

		glDrawElements(GL_TRIANGLES, mem_mesh->num_indices, GL_UNSIGNED_INT, (void *)mem_mesh->ofs_indices);

		if (alphatest)
		{
			glAlphaFunc(GL_NOTEQUAL, 0);
			glDisable(GL_ALPHA_TEST);
		}

		if (additive)
		{
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}
}
