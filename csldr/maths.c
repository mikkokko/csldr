#include "pch.h"

void AngleVectors(const vec3_t angles, vec3_t forward, vec3_t side, vec3_t up)
{
	float yaw, pitch, roll;
	float sy, cy, sp, cp, sr, cr;

	yaw = Radians(angles[1]);
	pitch = Radians(angles[0]);
	roll = Radians(angles[2]);

	cy = cosf(yaw);
	sy = sinf(yaw);
	cp = cosf(pitch);
	sp = sinf(pitch);
	cr = cosf(roll);
	sr = sinf(roll);

	if (forward)
	{
		forward[0] = cy * cp;
		forward[1] = cp * sy;
		forward[2] = -sp;
	}

	if (side)
	{
		side[0] = cr * sy - cy * sp * sr;
		side[1] = -(sy * sp * sr + cy * cr);
		side[2] = -(cp * sr);
	}

	if (up)
	{
		up[0] = cy * cr * sp + sy * sr;
		up[1] = cr * sy * sp - cy * sr;
		up[2] = cp * cr;
	}
}

void AngleLerp(const vec3_t a, const vec3_t b, float t, vec3_t dst)
{
	/* crappy but it's fast */
	int i;
	float dt;

	for (i = 0; i < 3; i++)
	{
		dt = b[i] - a[i];

		if (dt < -180)
			dt += 360;
		else if (dt > 180)
			dt -= 360;

		dst[i] = a[i] + t * dt;
	}
}
