#include "pch.h"

/* animate camera using a viewmodel attachment */

static cvar_t *camera_movement_scale;
static cvar_t *camera_movement_interp;

void CameraInit(void)
{
	CVAR_ARCHIVE_FAST(camera_movement_scale, 1);
	CVAR_ARCHIVE_FAST(camera_movement_interp, 0);
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

static void UnpackRotation(vec3_t angles, int frame, mstudiobone_t *bone, mstudioanim_t *anim)
{
	/* unpack the bone rotation */
	angles[0] = Degrees(bone->value[3] + bone->scale[3] *
		UnpackScale(frame, anim, anim->offset[3]));

	angles[1] = Degrees(bone->value[4] + bone->scale[4] *
		UnpackScale(frame, anim, anim->offset[4]));

	angles[2] = Degrees(bone->value[5] + bone->scale[5] *
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

static void CameraCalcMovement(cl_entity_t *vm, vec3_t angles)
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

	VectorClear(angles);

	model = vm->model;
	if (!model)
		return;

	hdr = (studiohdr_t *)model->cache.data;
	if (!hdr)
		return;

	bone_index = GetCameraBone(hdr);
	if (bone_index == -1)
		return;

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

	AngleLerp(angles1, angles2, t, angles);
	return;
}

void CameraApplyMovement(ref_params_t *pparams)
{
	static studiohdr_t *last_hdr;
	static vec3_t prevMove, curMove;
	static float lerpTime;

	vec3_t new_movement;
	float scale;
	cl_entity_t *vm;
	model_t *model;
	studiohdr_t *hdr;

	vm = gEngfuncs.GetViewModel();
	CameraCalcMovement(vm, new_movement);

	/* check if weapon has changed */
	model = vm->model;
	hdr = model ? (studiohdr_t *)model->cache.data : NULL;

	if (hdr != last_hdr)
	{
		last_hdr = hdr;

		if (camera_movement_interp->value)
		{
			lerpTime = clientTime;
			VectorCopy(curMove, prevMove);
		}
	}

	if (lerpTime)
	{
		float frac = (clientTime - lerpTime) / camera_movement_interp->value;

		if (frac < 0 || frac >= 1)
		{
			/* just copy */
			VectorCopy(new_movement, curMove);
			lerpTime = 0;
		}
		else
		{
			AngleLerp(prevMove, new_movement, frac, curMove);
		}
	}
	else
	{
		/* just copy */
		VectorCopy(new_movement, curMove);
	}

	scale = camera_movement_scale->value;

	pparams->viewangles[0] += curMove[0] * scale;
	pparams->viewangles[1] += curMove[1] * scale;
	pparams->viewangles[2] += curMove[2] * scale;
}
