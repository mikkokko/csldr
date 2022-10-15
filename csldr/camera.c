#include "pch.h"

/* animate camera using a viewmodel attachment */

cvar_t *camera_movement_scale;
cvar_t *camera_movement_interp;

void CameraInit(void)
{
	CVAR_ARHCIVE_FAST(camera_movement_scale, 1);
	CVAR_ARHCIVE_FAST(camera_movement_interp, 0);
}

static mstudioanim_t *GetAnim(studiohdr_t *hdr, model_t *model, mstudioseqdesc_t *seqdesc)
{
	byte *data;
	cache_user_t *seqs;

	if (!seqdesc->seqgroup)
		return (mstudioanim_t *)((byte *)hdr + seqdesc->animindex);

	seqs = (cache_user_t *)model->submodels;
	data = (byte *)seqs[seqdesc->seqgroup].data;

	return (mstudioanim_t *)&data[seqdesc->animindex];
}

static float UnpackScale(int frame, mstudioanim_t *anim, int offset)
{
	int index;
	mstudioanimvalue_t *bone_frame;

	if (!offset)
		return 0;

	bone_frame = (mstudioanimvalue_t *)((byte *)anim + offset);
	index = frame;

	while (1)
	{
		if (index < bone_frame->num.total)
			break;
		index -= bone_frame->num.total;
		bone_frame += bone_frame->num.valid;
		bone_frame++;
	}

	if (index >= bone_frame->num.valid)
		index = bone_frame->num.valid;
	else
		index += 1;

	return (float)bone_frame[index].value;
}

static void UnpackRotation(vec_t *angles, int frame, mstudiobone_t *bone, mstudioanim_t *anim)
{
	/* unpack the bone rotation */
	angles[0] = DEGREES(bone->value[3] + bone->scale[3] *
		UnpackScale(frame, anim, anim->offset[3]));

	angles[1] = DEGREES(bone->value[4] + bone->scale[4] *
		UnpackScale(frame, anim, anim->offset[4]));

	angles[2] = DEGREES(bone->value[5] + bone->scale[5] *
		UnpackScale(frame, anim, anim->offset[5]));
}

static int GetCameraBone(studiohdr_t *hdr)
{
	int i;
	mstudioattachment_t *a;
	mstudiobone_t *b;

	a = (mstudioattachment_t *)((byte *)hdr + hdr->attachmentindex);

	for (i = 0; i < hdr->numattachments; i++, a++)
	{
		b = (mstudiobone_t *)((byte *)hdr + hdr->boneindex) + a->bone;
		if (!strcmp(b->name, "camera"))
			return a->bone;
	}

	return -1;
}

static bool CameraCalcMovement(cl_entity_t *vm, vec_t *quat)
{
	model_t *model;
	studiohdr_t *hdr;
	int bone_index;
	int sequence;
	mstudioseqdesc_t *seqdesc;
	mstudioanim_t *anim;
	mstudiobone_t *bone;
	float anim_time, t;
	int frame, next_frame;
	vec3_t angles1, angles2;
	vec4_t quat1, quat2;

	model = vm->model;
	if (!model)
		return false;

	hdr = (studiohdr_t *)model->cache.data;
	if (!hdr)
		return false;

	bone_index = GetCameraBone(hdr);
	if (bone_index == -1)
		return false;

	sequence = vm->curstate.sequence;
	if (sequence >= hdr->numseq)
		sequence = 0;

	seqdesc = (mstudioseqdesc_t *)((byte *)hdr + hdr->seqindex) + sequence;

	anim = GetAnim(hdr, model, seqdesc) + bone_index;
	bone = (mstudiobone_t *)((byte *)hdr + hdr->boneindex) + bone_index;

	anim_time = (clientTime - vm->curstate.animtime) * seqdesc->fps;

	frame = (int)floor(anim_time);
	next_frame = frame + 1;
	t = anim_time - frame;

	if (seqdesc->flags & STUDIO_LOOPING)
	{
		frame %= seqdesc->numframes;
		next_frame %= seqdesc->numframes;
	}
	else
	{
		frame = MIN(frame, seqdesc->numframes - 1);
		next_frame = MIN(next_frame, seqdesc->numframes - 1);
	}

	UnpackRotation(angles1, frame, bone, anim);
	UnpackRotation(angles2, next_frame, bone, anim);

	AnglesToQuat(angles1, quat1);
	AnglesToQuat(angles2, quat2);

	QuatSlerp(quat1, quat2, t, quat);
	return true;
}

void CameraApplyMovement(ref_params_t *pparams)
{
	vec4_t quat;
	vec3_t angles;
	float scale, timeStep;
	float newTime, frameTime;
	static float lastTime;
	static float accum;
	static vec4_t prevQuat, curQuat;

	if (!camera_movement_interp->value)
	{
		QuatClear(quat);
		CameraCalcMovement(gEngfuncs.GetViewModel(), quat);
	}
	else
	{
		newTime = clientTime;

		/* fix time if needed */
		if (newTime < lastTime)
			lastTime = newTime;

		frameTime = newTime - lastTime;
		lastTime = newTime;

		accum += frameTime;

		timeStep = 1.0f / camera_movement_interp->value;

		while (accum >= timeStep)
		{
			QuatCopy(prevQuat, curQuat);
			QuatClear(curQuat);
			CameraCalcMovement(gEngfuncs.GetViewModel(), curQuat);
			accum -= timeStep;
		}

		QuatSlerp(prevQuat, curQuat, accum / timeStep, quat);
	}

	QuatToAngles(quat, angles);
	scale = camera_movement_scale->value;

	pparams->viewangles[0] += angles[0] * scale;
	pparams->viewangles[1] += angles[1] * scale;
	pparams->viewangles[2] += angles[2] * scale;
}
