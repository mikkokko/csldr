#include "pch.h"

#ifndef SHADERS_FROM_DISK /* xxd'd shaders */
#include "studio_cpu_vert.h"
#include "studio_frag.h"
#include "studio_gpu_vert.h"
#endif

static struct
{
	GLuint program;

	GLuint u_vieworg;
	GLuint u_ambientlight;
	GLuint u_shadelight;
	GLuint u_lightcolor;
	GLuint u_lightvec;
	GLuint u_texture;
	GLuint u_tex_flatshade;
	GLuint u_tex_chrome;
	GLuint u_tex_fullbright;
	GLuint u_tex_masked;
} shader_studio;

enum
{
	shader_studio_a_pos = 0,
	shader_studio_a_normal = 1,
	shader_studio_a_texcoord = 2,

	// only for gpu skinning
	shader_studio_a_bones = 3
};

static const attribute_t studio_attributes[] =
{
	{ shader_studio_a_pos, "a_pos" },
	{ shader_studio_a_normal, "a_normal" },
	{ shader_studio_a_texcoord, "a_texcoord" },

	// only for gpu skinning
	{ shader_studio_a_bones, "a_bones" }
};

static const uniform_t studio_uniforms[] =
{
	{ &shader_studio.u_vieworg, "u_vieworg" },

	{ &shader_studio.u_ambientlight, "u_ambientlight" },
	{ &shader_studio.u_shadelight, "u_shadelight" },
	{ &shader_studio.u_lightcolor, "u_lightcolor" },
	{ &shader_studio.u_lightvec, "u_lightvec" },

	{ &shader_studio.u_texture, "u_texture" },

	{ &shader_studio.u_tex_flatshade, "u_tex_flatshade" },
	{ &shader_studio.u_tex_chrome, "u_tex_chrome" },
	{ &shader_studio.u_tex_fullbright, "u_tex_fullbright" },
	{ &shader_studio.u_tex_masked, "u_tex_masked" }
};

// only for gpu skinning
static GLuint studio_ubo;
static GLint bones_offset;

void R_StudioInit(void)
{
	if (studio_gpuskin)
		LOAD_SHADER(studio, studio_gpu, studio);
	else
		LOAD_SHADER(studio, studio_cpu, studio);

	// textures stay constant (mikkotodo check)
	glUniform1i(shader_studio.u_texture, 0);

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
	ctx->bonetransform = (void *)IEngineStudio.StudioGetBoneTransform();
}

void R_StudioSetupLighting(studio_context_t *ctx, alight_t *lighting)
{
	ctx->ambientlight = (float)lighting->ambientlight;
	ctx->shadelight = (float)lighting->shadelight;
	VectorCopy(ctx->lightcolor, lighting->color);
	VectorCopy(ctx->lightvec, lighting->plightvec);
}

void R_StudioSetupRenderer(studio_context_t *ctx)
{
	glPushAttrib(GL_TEXTURE_BIT);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(shader_studio.program);
	glUniform3fv(shader_studio.u_vieworg, 1, v_vieworg);

	glBindBuffer(GL_UNIFORM_BUFFER, studio_ubo);

	// calling GetViewEntity is ok here
	if (cl_righthand->value && ctx->entity == IEngineStudio.GetViewEntity())
		glDisable(GL_CULL_FACE);

	glBindBuffer(GL_ARRAY_BUFFER, ctx->cache->studio_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->cache->studio_ebo);

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

		glBufferSubData(GL_UNIFORM_BUFFER, bones_offset, ctx->header->numbones * sizeof(mat3x4_t), &(*ctx->bonetransform)[0][0][0]);
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


	glUniform1f(shader_studio.u_ambientlight, ctx->ambientlight);
	glUniform1f(shader_studio.u_shadelight, ctx->shadelight);
	glUniform3fv(shader_studio.u_lightcolor, 1, ctx->lightcolor);
	glUniform3fv(shader_studio.u_lightvec, 1, ctx->lightvec);
}

void R_StudioRestoreRenderer(studio_context_t *ctx)
{
	// reset opengl state
	if (cl_righthand->value && ctx->entity == IEngineStudio.GetViewEntity())
		glEnable(GL_CULL_FACE);

	glDisableVertexAttribArray(shader_studio_a_pos);
	glDisableVertexAttribArray(shader_studio_a_normal);
	glDisableVertexAttribArray(shader_studio_a_texcoord);

	if (studio_gpuskin)
	{
		glDisableVertexAttribArray(shader_studio_a_bones);
	}

	glPopAttrib();

	glUseProgram(0);

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
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

static void R_StudioAnimateCPU(studio_context_t *ctx, mem_model_t *mem_model)
{
	studio_cpu_vert_t *anim_verts = Mem_AllocTemp(sizeof(studio_cpu_vert_t) * mem_model->num_verts);

	for (unsigned int i = 0; i < mem_model->num_verts; i++)
	{
		studio_cpu_vert_t *src = &ctx->cache->verts[mem_model->ofs_verts + i];
		studio_vertbone_t *srcbone = &ctx->cache->vertbones[mem_model->ofs_verts + i];
		studio_cpu_vert_t *dst = &anim_verts[i];

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

	// mikkotodo this copies way too much data
	glBufferSubData(GL_ARRAY_BUFFER,
		mem_model->ofs_verts * sizeof(studio_cpu_vert_t),
		mem_model->num_verts * sizeof(studio_cpu_vert_t),
		anim_verts);
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
		R_StudioAnimateCPU(ctx, mem_submodel);

	for (int i = 0; i < submodel->nummesh; i++)
	{
		mstudiomesh_t *mesh = &meshes[i];
		mstudiotexture_t *texture = &textures[skins[mesh->skinref]];
		mem_texture_t *mem_texture = &ctx->cache->textures[skins[mesh->skinref]];
		mem_mesh_t *mem_mesh = &mem_submodel->meshes[i];

		glUniform1i(shader_studio.u_tex_flatshade, (texture->flags & STUDIO_NF_FLATSHADE) ? true : false);
		glUniform1i(shader_studio.u_tex_chrome, (texture->flags & STUDIO_NF_CHROME) ? true : false);
		glUniform1i(shader_studio.u_tex_fullbright, (texture->flags & STUDIO_NF_FULLBRIGHT) ? true : false);
		glUniform1i(shader_studio.u_tex_masked, (texture->flags & STUDIO_NF_MASKED) ? true : false);

		bool additive = ((texture->flags & STUDIO_NF_ADDITIVE) && ctx->entity->curstate.rendermode == 0); // kRenderNormal

		if (additive)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);
		}

		if (mem_texture->diffuse)
			glBindTexture(GL_TEXTURE_2D, mem_texture->diffuse);
		else
			glBindTexture(GL_TEXTURE_2D, texture->index);

		glDrawElements(GL_TRIANGLES, mem_mesh->num_indices, GL_UNSIGNED_INT, (void *)mem_mesh->ofs_indices);

		if (additive)
		{
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}
}

studiohdr_t *R_LoadTextures(model_t *model, studiohdr_t *header)
{
	assert(model);

	if (header->textureindex)
		return header;

	model_t *texmodel = (model_t *)model->texinfo;
	if (texmodel && IEngineStudio.Cache_Check(&texmodel->cache))
		return (studiohdr_t *)texmodel->cache.data;

	char path[128];
	strcpy(path, model->name);

	// unsafe but the engine does it too...
	// also lower case for linux??? what the fuck
	strcpy(path + strlen(path) - 4, "t.mdl");

	texmodel = IEngineStudio.Mod_ForName(path, true);
	model->texinfo = (mtexinfo_t *)texmodel;

	// not sure why this is done but ok
	studiohdr_t *textureheader = (studiohdr_t *)texmodel->cache.data;
	strcpy(textureheader->name, path);
	return textureheader;
}
