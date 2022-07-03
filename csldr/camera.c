#include "pch.h"

/* animate camera using a viewmodel attachment */

cvar_t *camera_movement_scale;

void CameraInit(void)
{
	camera_movement_scale = gEngfuncs.pfnRegisterVariable("camera_movement_scale", "1", FCVAR_ARCHIVE);
}

mstudioanim_t *GetAnim(studiohdr_t *hdr, model_t *model, mstudioseqdesc_t *seqdesc)
{
	byte *data;
	cache_user_t *seqs;

	if (!seqdesc->seqgroup)
		return (mstudioanim_t *)((byte *)hdr + seqdesc->animindex);

	seqs = (cache_user_t *)model->submodels;
	data = (byte *)seqs[seqdesc->seqgroup].data;

	return (mstudioanim_t *)&data[seqdesc->animindex];
}

void CalcBoneAngles(int frame, mstudiobone_t *bone, mstudioanim_t *anim, vec3_t angles)
{
	int j, k;
	mstudioanimvalue_t *animvalue;

	for (j = 0; j < 3; j++)
	{
		if (anim->offset[j + 3] == 0)
			angles[j] = bone->value[j + 3];
		else
		{
			animvalue = (mstudioanimvalue_t *)((byte *)anim + anim->offset[j + 3]);
			k = frame;

			if (animvalue->num.total < animvalue->num.valid)
				k = 0;

			while (animvalue->num.total <= k)
			{
				k -= animvalue->num.total;
				animvalue += animvalue->num.valid + 1;

				if (animvalue->num.total < animvalue->num.valid)
					k = 0;
			}

			if (animvalue->num.valid > k)
			{
				angles[j] = animvalue[k + 1].value;
			}
			else
			{
				angles[j] = animvalue[animvalue->num.valid].value;
			}

			angles[j] = bone->value[j + 3] + angles[j] * bone->scale[j + 3];
		}
	}

	angles[0] = DEG(angles[0]);
	angles[1] = DEG(angles[1]);
	angles[2] = DEG(angles[2]);
}

int GetCameraBone(studiohdr_t *hdr)
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

/* these are probably from wikipedia */

void AnglesToQuat(vec_t *in, vec_t *out)
{
	float yaw, pitch, roll;
	float sy, cy, sp, cp, sr, cr;

	yaw = RAD(in[1]);
	pitch = RAD(in[0]);
	roll = RAD(in[2]);

	/* Abbreviations for the various angular functions */
	cy = cos(yaw * 0.5);
	sy = sin(yaw * 0.5);
	cp = cos(pitch * 0.5);
	sp = sin(pitch * 0.5);
	cr = cos(roll * 0.5);
	sr = sin(roll * 0.5);

	out[3] = cr * cp * cy + sr * sp * sy;
	out[0] = sr * cp * cy - cr * sp * sy;
	out[1] = cr * sp * cy + sr * cp * sy;
	out[2] = cr * cp * sy - sr * sp * cy;
}

void QuatToAngles(vec_t *in, vec_t *out)
{
	double sinr_cosp, cosr_cosp;
	double sinp;
	double siny_cosp, cosy_cosp;

	/* roll (x-axis rotation) */
	sinr_cosp = 2 * (in[3] * in[0] + in[1] * in[2]);
	cosr_cosp = 1 - 2 * (in[0] * in[0] + in[1] * in[1]);
	out[ROLL] = DEG(atan2(sinr_cosp, cosr_cosp));

	/* pitch (y-axis rotation) */
	sinp = 2 * (in[3] * in[1] - in[2] * in[0]);
	if (abs(sinp) >= 1)
		out[PITCH] = DEG(copysign(M_PI / 2, sinp)); /* use 90 degrees if out of range */
	else
		out[PITCH] = DEG(asin(sinp));

	/* yaw (z-axis rotation) */
	siny_cosp = 2 * (in[3] * in[2] + in[0] * in[1]);
	cosy_cosp = 1 - 2 * (in[1] * in[1] + in[2] * in[2]);
	out[YAW] = DEG(atan2(siny_cosp, cosy_cosp));
}

void QuatSlerp(vec_t *a, vec_t *b, float t, vec_t *out)
{
	float angle;

	angle = acos(a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]);

	if (fabs(angle) >= 0.00001)
	{
		float sine;
		float s1, s2;

		sine = sin(angle);
		s1 = sin((1 - t) * angle) / sine;
		s2 = sin(t * angle) / sine;

		out[0] = s1 * a[0] + s2 * b[0];
		out[1] = s1 * a[1] + s2 * b[1];
		out[2] = s1 * a[2] + s2 * b[2];
		out[3] = s1 * a[3] + s2 * b[3];
	}
	else
	{
		float s = 1.0f - t;

		out[0] = (a[0] * s) + (b[0] * t);
		out[1] = (a[1] * s) + (b[1] * t);
		out[2] = (a[2] * s) + (b[2] * t);
		out[3] = (a[3] * s) + (b[3] * t);
	}
}

void SlerpAngles(vec_t *a, vec_t *b, float f, vec_t *out)
{
	vec4_t qa, qb, qout;

	AnglesToQuat(a, qa);
	AnglesToQuat(b, qb);
	QuatSlerp(qa, qb, f, qout);
	QuatToAngles(qout, out);
}

bool CameraCalcMovement(cl_entity_t *vm, vec_t *angles)
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

	CalcBoneAngles(frame, bone, anim, angles1);
	CalcBoneAngles(next_frame, bone, anim, angles2);

	SlerpAngles(angles1, angles2, t, angles);

	return true;
}

void CameraApplyMovement(ref_params_t *pparams)
{
	float scale;
	vec3_t cameraAngles;

	if (!CameraCalcMovement(gEngfuncs.GetViewModel(), cameraAngles))
		return; /* no movement */

	scale = camera_movement_scale->value;

	pparams->viewangles[0] += cameraAngles[0] * scale;
	pparams->viewangles[1] += cameraAngles[1] * scale;
	pparams->viewangles[2] += cameraAngles[2] * scale;
}
