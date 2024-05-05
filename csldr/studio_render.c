#include "pch.h"

studio_globals_t studio_globals;

void R_StudioInit(void)
{
	studio_globals.r_glowshellfreq = gEngfuncs.pfnGetCvarPointer("r_glowshellfreq");
	studio_globals.gl_fog = gEngfuncs.pfnGetCvarPointer("gl_fog");

	R_StudioCompileShaders();

	if (studio_gpuskin)
	{
		// bold size assumption
		glGenBuffers(1, &studio_globals.ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, studio_globals.ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(mat3x4_t) * 128, NULL, GL_STREAM_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, studio_globals.ubo);
	}

	// get pointer to first elight
	studio_globals.elights = gEngfuncs.pEfxAPI->CL_AllocElight(0);
}

void R_StudioNewFrame(void)
{
	if (!studio_globals.framecount && studio_fastpath)
		R_StudioCompileShaders();

	studio_globals.framecount++;
}

void R_StudioEntityLight(studio_context_t *ctx)
{
	ctx->elight_num = 0;
	memset(ctx->elight_color, 0, sizeof(ctx->elight_color));
	memset(ctx->elight_pos, 0, sizeof(ctx->elight_pos));

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
		dlight_t *elight = &studio_globals.elights[i];

		if (elight->die <= clientTime)
			continue;

		if (elight->radius <= min_radius)
			continue;

		if ((elight->key & 0xFFF) == entity->index)
		{
			int attachment = (elight->key >> 12) & 0xF;

			if (attachment)
				VectorCopy(entity->attachment[attachment], elight->origin);
			else
				VectorCopy(entity->origin, elight->origin);
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

		int index = ctx->elight_num;

		if (ctx->elight_num >= MAX_ELIGHTS)
		{
			index = -1;

			for (int j = 0; j < ctx->elight_num; j++)
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

		VectorCopy(elight->origin, ctx->elight_pos[index]);
		ctx->elight_pos[index][3] = r2;

		ctx->elight_color[index][0] = (float)gammavars.lineartable[elight->color.r] * (1.0f / 255.0f);
		ctx->elight_color[index][1] = (float)gammavars.lineartable[elight->color.g] * (1.0f / 255.0f);
		ctx->elight_color[index][2] = (float)gammavars.lineartable[elight->color.b] * (1.0f / 255.0f);

		if (index >= ctx->elight_num)
		{
			ctx->elight_num = index + 1;
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
	VectorCopy(lighting->color, ctx->lightcolor);
	VectorCopy(lighting->plightvec, ctx->lightvec);
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

static int R_StudioGetOptions(studio_context_t *ctx)
{
	int options = 0;

	if (ctx->entity->curstate.rendermode == kRenderTransAdd)
		options |= HAVE_ADDITIVE;
	else if (ctx->entity->curstate.renderfx == kRenderFxGlowShell)
		options |= HAVE_GLOWSHELL;

	// mikkotodo revisit
	if (studio_globals.fog_mode == GL_LINEAR)
		options |= (HAVE_FOG | HAVE_FOG_LINEAR);
	else if (studio_globals.fog_mode == GL_EXP2)
		options |= HAVE_FOG;

	if (ctx->elight_num)
		options |= HAVE_ELIGHTS;

	if (ctx->cache->texflags & STUDIO_NF_FLATSHADE)
		options |= CAN_FLATSHADE;

	if (ctx->cache->texflags & STUDIO_NF_CHROME)
		options |= CAN_CHROME;

	if (ctx->cache->texflags & STUDIO_NF_FULLBRIGHT)
		options |= CAN_FULLBRIGHT;

	if (ctx->cache->texflags & STUDIO_NF_MASKED)
		options |= CAN_MASKED;

	return options;
}

void R_StudioSetupRenderer(studio_context_t *ctx)
{
	static int framecount;

	if (framecount != studio_globals.framecount)
	{
		framecount = studio_globals.framecount;

		if (glIsEnabled(GL_FOG))
			glGetIntegerv(GL_FOG_MODE, &studio_globals.fog_mode);
		else
			studio_globals.fog_mode = 0;
	}

	int options = R_StudioGetOptions(ctx);
	studio_shader_t *shader = R_StudioSelectShader(options);
	ctx->shader = shader;

	// setup our 70 different uniforms
	glUseProgram(shader->program);

	glUniform1i(shader->u_texture, 0);

	float colormix[4];

	if (options & HAVE_GLOWSHELL)
	{
		colormix[0] = (float)ctx->entity->curstate.rendercolor.r * (1.0f / 255);
		colormix[1] = (float)ctx->entity->curstate.rendercolor.g * (1.0f / 255);
		colormix[2] = (float)ctx->entity->curstate.rendercolor.b * (1.0f / 255);
		colormix[3] = (float)ctx->entity->curstate.renderamt * 0.05f;
	}
	else
	{
		colormix[0] = ctx->lightcolor[0];
		colormix[1] = ctx->lightcolor[1];
		colormix[2] = ctx->lightcolor[2];
		colormix[3] = CalcFxBlend(ctx->entity) * (1.0f / 255);
	}

	glUniform4fv(shader->u_colormix, 1, colormix);

	// mikkotodo is this correct
	if ((options & (HAVE_ADDITIVE | HAVE_GLOWSHELL)) == 0)
	{
		glUniform1f(shader->u_ambientlight, ctx->ambientlight);
		glUniform1f(shader->u_shadelight, ctx->shadelight);
		glUniform3fv(shader->u_lightvec, 1, ctx->lightvec);

		glUniform1f(shader->u_lightgamma, gammavars.lightgamma);
		glUniform1f(shader->u_brightness, gammavars.brightness);
		glUniform1f(shader->u_g3, gammavars.g3);
	}

	if ((options & HAVE_ELIGHTS) || (options & (HAVE_ADDITIVE | HAVE_GLOWSHELL)) == 0)
		glUniform1f(shader->u_invgamma, gammavars.g);

	// bruh
	if (options & HAVE_GLOWSHELL)
	{
		ctx->chrome_origin[0] = cosf(studio_globals.r_glowshellfreq->value * clientTime) * 4000.0f;
		ctx->chrome_origin[1] = sinf(studio_globals.r_glowshellfreq->value * clientTime) * 4000.0f;
		ctx->chrome_origin[2] = cosf(studio_globals.r_glowshellfreq->value * clientTime * 0.33f) * 4000.0f;
	}
	else
	{
		VectorCopy(v_vieworg, ctx->chrome_origin);
	}

	if (options & CAN_CHROME)
	{
		glUniform3fv(shader->u_chromeorg, 1, ctx->chrome_origin);
		glUniform3fv(shader->u_chromeright, 1, v_viewright);
	}

	if (options & HAVE_ELIGHTS)
	{
		glUniform4fv(shader->u_elight_pos, ctx->elight_num, &ctx->elight_pos[0][0]);
		glUniform3fv(shader->u_elight_color, ctx->elight_num, &ctx->elight_color[0][0]);
	}

	// setup other stuff

	// calling GetViewEntity is ok here
	if (cl_righthand->value && ctx->entity == IEngineStudio.GetViewEntity())
		glDisable(GL_CULL_FACE);

	glBindBuffer(GL_ARRAY_BUFFER, ctx->cache->studio_vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ctx->cache->studio_ebo);

	if (studio_gpuskin)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, studio_globals.ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, ctx->header->numbones * sizeof(mat3x4_t), &(*ctx->bonetransform)[0][0][0]);
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

static void CalcChromeCPU(studio_context_t *ctx, float *out, int bone_id, mat3x4_t bone, vec3_t normal)
{
	// cache the results like the engine does
	static int chrome_age[128];
	static vec3_t chrome_up[128];
	static vec3_t chrome_side[128];

	float *up_anim = chrome_up[bone_id];
	float *side_anim = chrome_side[bone_id];

	if (chrome_age[bone_id] != studio_globals.drawcount)
	{
		chrome_age[bone_id] = studio_globals.drawcount;

		vec3_t dir;
		dir[0] = bone[0][3] - ctx->chrome_origin[0];
		dir[1] = bone[1][3] - ctx->chrome_origin[1];
		dir[2] = bone[2][3] - ctx->chrome_origin[2];

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

	int forceflags = IEngineStudio.GetForceFaceFlags();

	if (!studio_gpuskin)
	{
		studio_cpu_vert_t *anim_verts = (studio_cpu_vert_t *)Mem_TempAlloc(sizeof(studio_cpu_vert_t) * mem_submodel->num_verts);
		int vert_ofs = 0;

		for (int i = 0; i < submodel->nummesh; i++)
		{
			mstudiomesh_t *mesh = &meshes[i];
			mem_mesh_t *mem_mesh = &mem_submodel->meshes[i];
			mstudiotexture_t *texture = &textures[skins[mesh->skinref]];

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

					CalcChromeCPU(ctx, dst->texcoord, srcbone->bones[1], (*ctx->bonetransform)[srcbone->bones[1]], src->norm);
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

		Mem_TempFree(anim_verts);
	}

	for (int i = 0; i < submodel->nummesh; i++)
	{
		mstudiomesh_t *mesh = &meshes[i];
		mstudiotexture_t *texture = &textures[skins[mesh->skinref]];
		mem_texture_t *mem_texture = &ctx->cache->textures[skins[mesh->skinref]];
		mem_mesh_t *mem_mesh = &mem_submodel->meshes[i];

		int flags = texture->flags | forceflags;

		if (ctx->shader->u_tex_flatshade != -1)
			glUniform1i(ctx->shader->u_tex_flatshade, (flags & STUDIO_NF_FLATSHADE) ? true : false);

		if (ctx->shader->u_tex_chrome != -1)
			glUniform1i(ctx->shader->u_tex_chrome, (flags & STUDIO_NF_CHROME) ? true : false);

		if (ctx->shader->u_tex_fullbright != -1)
			glUniform1i(ctx->shader->u_tex_fullbright, (flags & STUDIO_NF_FULLBRIGHT) ? true : false);

		if (ctx->shader->u_tex_masked != -1)
			glUniform1i(ctx->shader->u_tex_masked, (flags & STUDIO_NF_MASKED) ? true : false);

		bool additive = ((flags & STUDIO_NF_ADDITIVE) && ctx->entity->curstate.rendermode == kRenderNormal);

		if (additive)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
			glDepthMask(GL_FALSE);
		}

		if (mem_texture->diffuse)
		{
			// mikkotodo revisit, hack to make texture bindings go through GL_Bind
			char old_name = texture->name[0]; // make sure remapping won't happen
			int old_index = texture->index;

			texture->name[0] = '\0';
			texture->index = mem_texture->diffuse;

			IEngineStudio.StudioSetupSkin(textureheader, skins[mesh->skinref]);

			texture->name[0] = old_name;
			texture->index = old_index;
		}
		else
		{
			IEngineStudio.StudioSetupSkin(textureheader, skins[mesh->skinref]);
		}

		glDrawElements(GL_TRIANGLES, mem_mesh->num_indices, GL_UNSIGNED_INT, (void *)mem_mesh->ofs_indices);

		if (additive)
		{
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}
	}
}
