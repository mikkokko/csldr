#include "pch.h"

studio_globals_t studio_globals;

void R_StudioInit(void)
{
	studio_globals.r_glowshellfreq = gEngfuncs.pfnGetCvarPointer("r_glowshellfreq");

	R_StudioCompileShaders();

	if (studio_uboable)
	{
		// bold size assumption
		glGenBuffers(1, &studio_globals.ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, studio_globals.ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(mat3x4_t) * 128, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, studio_globals.ubo);
	}

	// get pointer to first elight
	studio_globals.elights = gEngfuncs.pEfxAPI->CL_AllocElight(0);
}

void R_StudioNewFrame(void)
{
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

	cl_entity_t *entity = ctx->entity;

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
			assert(attachment >= 0 && attachment < 4);

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

void R_StudioInitContext(studio_context_t *ctx, cl_entity_t *entity, model_t *model, studiohdr_t *header, studio_cache_t *cache)
{
	assert(entity);
	assert(model);
	assert(header);

	ctx->entity = entity;
	ctx->model = model;
	ctx->header = header;
	ctx->cache = cache;
	ctx->used_texflags = 0;

	mat3x4_t(*bonetransform)[] = (mat3x4_t(*)[])IEngineStudio.StudioGetBoneTransform();

	for (int i = 0; i < cache->num_gpubones; i++)
	{
		int index = cache->map_gpubones[i];
		memcpy(ctx->gpu_bonetransform[i], (*bonetransform)[index], sizeof(mat3x4_t));
	}
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

static void R_StudioGetOptions(studio_context_t *ctx, studio_options_t *options)
{
	if (ctx->entity->curstate.rendermode == kRenderTransAdd)
		options->LIGHTING_MODE = LIGHTING_MODE_ADDITIVE;
	else if (ctx->entity->curstate.renderfx == kRenderFxGlowShell)
	{
		options->LIGHTING_MODE = LIGHTING_MODE_GLOWSHELL;
		assert(ctx->used_texflags & STUDIO_NF_CHROME);
		//options->CAN_CHROME = 1; // set by forceflags
	}
	else if (ctx->elight_num)
		options->LIGHTING_MODE = LIGHTING_MODE_ELIGHTS;

	if (studio_globals.fog_mode == GL_LINEAR)
		options->FOG_MODE = FOG_MODE_LINEAR;
	else if (studio_globals.fog_mode == GL_EXP2)
		options->FOG_MODE = FOG_MODE_EXP2;

	if (ctx->used_texflags & STUDIO_NF_FLATSHADE)
		options->CAN_FLATSHADE = 1;

	if (ctx->used_texflags & STUDIO_NF_CHROME)
		options->CAN_CHROME = 1;

	if (ctx->used_texflags & STUDIO_NF_FULLBRIGHT)
		options->CAN_FULLBRIGHT = 1;

	if (ctx->used_texflags & STUDIO_NF_MASKED)
		options->CAN_MASKED = 1;
}

static void R_StudioSetupRenderer(studio_context_t *ctx)
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

	ctx->forceflags = IEngineStudio.GetForceFaceFlags();
	ctx->used_texflags |= ctx->forceflags;

	studio_options_t options = { 0 };
	R_StudioGetOptions(ctx, &options);

	studio_shader_t *shader = R_StudioSelectShader(&options);
	ctx->shader = shader;

	// setup our 70 different uniforms
	glUseProgram(shader->program);

	float colormix[4];

	if (options.LIGHTING_MODE == LIGHTING_MODE_GLOWSHELL)
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

	glUniform4fv(shader->u_color, 1, colormix);

	if (options.LIGHTING_MODE == LIGHTING_MODE_DEFAULT || options.LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
	{
		glUniform1f(shader->u_ambientlight, ctx->ambientlight);
		glUniform1f(shader->u_shadelight, ctx->shadelight);
		glUniform3fv(shader->u_lightvec, 1, ctx->lightvec);

		glUniform1f(shader->u_lightgamma, gammavars.lightgamma);
		glUniform1f(shader->u_brightness, gammavars.brightness);
		glUniform1f(shader->u_g3, gammavars.g3);
		glUniform1f(shader->u_invgamma, gammavars.g);
	}

	if (options.CAN_CHROME)
	{
		vec3_t chrome_origin;

		// bruh
		if (options.LIGHTING_MODE == LIGHTING_MODE_GLOWSHELL)
		{
			chrome_origin[0] = cosf(studio_globals.r_glowshellfreq->value * clientTime) * 4000.0f;
			chrome_origin[1] = sinf(studio_globals.r_glowshellfreq->value * clientTime) * 4000.0f;
			chrome_origin[2] = cosf(studio_globals.r_glowshellfreq->value * clientTime * 0.33f) * 4000.0f;
		}
		else
		{
			VectorCopy(v_vieworg, chrome_origin);
		}

		glUniform3fv(shader->u_chromeorg, 1, chrome_origin);
		glUniform3fv(shader->u_chromeright, 1, v_viewright);
	}

	if (options.LIGHTING_MODE == LIGHTING_MODE_ELIGHTS)
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

	if (studio_uboable)
	{
		glBindBuffer(GL_UNIFORM_BUFFER, studio_globals.ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, ctx->cache->num_gpubones * sizeof(mat3x4_t), &ctx->gpu_bonetransform[0][0]);
	}
	else
	{
		glUniformMatrix3x4fv(shader->u_bones, ctx->cache->num_gpubones, GL_FALSE, &ctx->gpu_bonetransform[0][0][0]);
	}

	glEnableVertexAttribArray(shader_studio_a_pos);
	glEnableVertexAttribArray(shader_studio_a_normal);
	glEnableVertexAttribArray(shader_studio_a_texcoord);
	glEnableVertexAttribArray(shader_studio_a_bones);

	glVertexAttribPointer(shader_studio_a_pos, 3, GL_FLOAT, GL_FALSE, sizeof(studio_vert_t), (void *)Q_OFFSETOF(studio_vert_t, pos));
	glVertexAttribPointer(shader_studio_a_normal, 3, GL_FLOAT, GL_FALSE, sizeof(studio_vert_t), (void *)Q_OFFSETOF(studio_vert_t, norm));
	glVertexAttribPointer(shader_studio_a_texcoord, 2, GL_FLOAT, GL_FALSE, sizeof(studio_vert_t), (void *)Q_OFFSETOF(studio_vert_t, texcoord));
	glVertexAttribPointer(shader_studio_a_bones, 2, GL_FLOAT, GL_FALSE, sizeof(studio_vert_t), (void *)Q_OFFSETOF(studio_vert_t, bones));
}

static void R_StudioRestoreRenderer(studio_context_t *ctx)
{
	glUseProgram(0);

	// restore opengl state
	if (cl_righthand->value && ctx->entity == IEngineStudio.GetViewEntity())
		glEnable(GL_CULL_FACE);

	glDisableVertexAttribArray(shader_studio_a_pos);
	glDisableVertexAttribArray(shader_studio_a_normal);
	glDisableVertexAttribArray(shader_studio_a_texcoord);
	glDisableVertexAttribArray(shader_studio_a_bones);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (studio_uboable)
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static void R_StudioEmitDrawCalls(studio_context_t *ctx, studio_shader_t *shader)
{
	studiohdr_t *header = ctx->header;
	studiohdr_t *textureheader = R_LoadTextures(ctx->model, header);
	mstudiotexture_t *textures = (mstudiotexture_t *)((byte *)textureheader + textureheader->textureindex);

	assert(ctx->forceflags == IEngineStudio.GetForceFaceFlags());
	assert(ctx->forceflags == 0 || ctx->forceflags == STUDIO_NF_CHROME);

	for (int i = 0; i < textureheader->numtextures; i++)
	{
		mem_texture_t *mem_texture = &ctx->cache->textures[i];
		if (!mem_texture->num_elements)
			continue;

		mstudiotexture_t *texture = &textures[i];

		int texture_flags = texture->flags | ctx->forceflags;

		if (shader->u_tex_flatshade != -1)
			glUniform1i(shader->u_tex_flatshade, (texture_flags & STUDIO_NF_FLATSHADE) ? true : false);

		if (shader->u_tex_chrome != -1)
			glUniform1i(shader->u_tex_chrome, (texture_flags & STUDIO_NF_CHROME) ? true : false);

		if (shader->u_tex_fullbright != -1)
			glUniform1i(shader->u_tex_fullbright, (texture_flags & STUDIO_NF_FULLBRIGHT) ? true : false);

		if (shader->u_tex_masked != -1)
			glUniform1i(shader->u_tex_masked, (texture_flags & STUDIO_NF_MASKED) ? true : false);

		bool additive = ((texture_flags & STUDIO_NF_ADDITIVE) && ctx->entity->curstate.rendermode == kRenderNormal);
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

			IEngineStudio.StudioSetupSkin(textureheader, i);

			texture->name[0] = old_name;
			texture->index = old_index;
		}
		else
		{
			IEngineStudio.StudioSetupSkin(textureheader, i);
		}

		glMultiDrawElements(GL_TRIANGLES,
			(GLint *)mem_texture->counts,
			GL_UNSIGNED_INT,
			(const void *const *)mem_texture->offsets,
			mem_texture->num_elements);

		if (additive)
		{
			glDisable(GL_BLEND);
			glDepthMask(GL_TRUE);
		}

		mem_texture->num_elements = 0;
	}
}

void R_StudioFinish(studio_context_t *ctx)
{
	R_StudioSetupRenderer(ctx);
	R_StudioEmitDrawCalls(ctx, ctx->shader);
	R_StudioRestoreRenderer(ctx);
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

	for (int i = 0; i < submodel->nummesh; i++)
	{
		mstudiomesh_t *mesh = &meshes[i];
		mem_mesh_t *mem_mesh = &mem_submodel->meshes[i];

		mem_texture_t *call = &ctx->cache->textures[skins[mesh->skinref]];
		call->counts[call->num_elements] = mem_mesh->num_indices;
		call->offsets[call->num_elements] = (void *)(size_t)mem_mesh->ofs_indices;
		call->num_elements++;

		mstudiotexture_t *texture = &textures[skins[mesh->skinref]];
		ctx->used_texflags |= texture->flags;
	}
}
