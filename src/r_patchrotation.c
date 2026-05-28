// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2020-2023 by Lactozilla.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_patchrotation.c
/// \brief Patch rotation.

#include "r_patchrotation.h"
#include "r_things.h" // FEETADJUST (todo: is this needed anymore? -- Monster Iestyn 21 Sep 2023 )
#include "z_zone.h"
#include "w_wad.h"
#include "r_main.h" // R_PointToAngle
#include "p_mobj.h"

#ifdef ROTSPRITE
fixed_t rollcosang[ROTANGLES];
fixed_t rollsinang[ROTANGLES];

angle_t R_ModelRotationAngle(interpmobjstate_t *interp)
{
	return interp->spriteroll;
}

angle_t R_SpriteRotationAngle(interpmobjstate_t *interp)
{
	if (cv_pitchroll_rotation.value)
	{
		angle_t viewingAngle = R_PointToAngle(interp->x, interp->y);

		fixed_t pitchMul = -FINESINE(viewingAngle >> ANGLETOFINESHIFT);
		fixed_t rollMul = FINECOSINE(viewingAngle >> ANGLETOFINESHIFT);

		angle_t rollOrPitch = FixedMul(interp->pitch, pitchMul) + FixedMul(interp->roll, rollMul);

		if (interp->flipped)
		{
			rollOrPitch *= -1;
		}

		return (rollOrPitch + R_ModelRotationAngle(interp));
	}
	else
<<<<<<< HEAD
		return R_ModelRotationAngle(interp);
=======
	{
		// Regular Mobjs don't air tilt.
		viewingAngle = mobj->angle;
	}

	pitchMul = -FINESINE(viewingAngle >> ANGLETOFINESHIFT);
	rollMul = FINECOSINE(viewingAngle >> ANGLETOFINESHIFT);

	if (!cv_sloperoll.value || (mobj->player && (mobj->player->pogospring > 0)))
	{
		sloperoll = 0;
		slopepitch = 0;
	}

	rollOrPitch = FixedMul(interp->pitch + sloperoll, pitchMul) + FixedMul(interp->roll + slopepitch, rollMul);

	return rollOrPitch;
}

static angle_t R_PlayerSpriteRotation(player_t *player, player_t *viewPlayer, boolean fliptilt)
{
	angle_t viewingAngle = R_PointToAnglePlayer(viewPlayer, player->mo->x, player->mo->y);
	angle_t angleDelta = (viewingAngle - player->mo->angle);

	INT32 nulltilt = (K_NullDriftTiltEnabled()) ? player->nulldrifttilt : 0;
	INT32 aiztilt = (cv_sliptidetilt.value) ? player->aizdrifttilt : 0;

	boolean nullBeforeSlip = (abs(aiztilt) < abs(nulltilt));
	INT32 liftsign = ((nullBeforeSlip) ? intsign(nulltilt) : intsign(aiztilt));

	// Sliptide tilt: Nulldrifts take priority if and ONLY if they're the larger value compared to sliptides.
	angle_t sliptideLift = max(abs(aiztilt), abs(nulltilt)) * liftsign;

	if (fliptilt)
	{
		// Vertical flip affine hack: invert the angle so it still looks right.
		sliptideLift *= -1;
	}

	angle_t rollAngle = 0;

	if (sliptideLift)
	{
		/* (from side) tilt downward if turning
		   toward camera, upward if away. */
		rollAngle +=
			FixedMul(sliptideLift, FINESINE(AbsAngle(angleDelta) >> ANGLETOFINESHIFT)) +
			FixedMul(sliptideLift, FINECOSINE(angleDelta >> ANGLETOFINESHIFT));
	}

	if (player->pogospring)
	{
		rollAngle = 0;
	}

	return rollAngle;
}

// Hacky boolean to check if we're the rainbow S-Monitor overlay
boolean R_IsOverlayingSMonitorPlayer(mobj_t* mobj)
{
	return ((mobj->type == MT_OVERLAY) &&
			(mobj->target) && (mobj->extravalue2) && (mobj->target->player) &&
			(mobj->target->player->smonitortimer));
}

angle_t R_ModelRotationAngle(mobj_t *mobj, player_t *viewPlayer, boolean fliptilt)
{
	angle_t rollAngle = mobj->rollangle;

	if (mobj->player)
	{
		rollAngle += R_PlayerSpriteRotation(mobj->player, viewPlayer, fliptilt);
	}
	else if (R_IsOverlayingSMonitorPlayer(mobj))
	{
		rollAngle += R_PlayerSpriteRotation(mobj->target->player, viewPlayer, fliptilt);
	}

	return rollAngle;
}

angle_t R_SpriteRotationAngle(mobj_t *mobj, player_t *viewPlayer, interpmobjstate_t *interp, boolean fliptilt)
{
	angle_t rollOrPitch;
	
	if (R_IsOverlayingSMonitorPlayer(mobj))
	{
		rollOrPitch = R_GetPitchRollAngle(mobj->target, viewPlayer, interp);
	}
	else
	{
		rollOrPitch = R_GetPitchRollAngle(mobj, viewPlayer, interp);
	}
	return (rollOrPitch + R_ModelRotationAngle(mobj, viewPlayer, fliptilt));
>>>>>>> 6e99c9b5cd (Merge pull request '[FEAT] Affine sprite rendering' (#223) from softwarehell into next)
}

INT32 R_GetRollAngle(angle_t rollangle)
{
	INT32 ra = AngleFixed(rollangle)>>FRACBITS;
#if (ROTANGDIFF > 1)
	ra += (ROTANGDIFF/2);
#endif
	ra /= ROTANGDIFF;
	ra %= ROTANGLES;
	return ra;
}

<<<<<<< HEAD
=======
#define VISROTMUL (ANG1 * ROTANGDIFF)

// Simulates "rollangle" angling
angle_t R_ConvToRollAngle(angle_t ang)
{
	return (cv_fakerollangle.value) ? (R_GetRollAngle(ang) * VISROTMUL) : ang;
}

vector2_t* R_RotateSpriteOffsetsByPitchRoll(
	mobj_t* mobj,
	boolean vflip,
	boolean hflip,
	boolean affine,
	interpmobjstate_t *interp,
	vector2_t* out,
	vector2_t* rolloffs)
{
	fixed_t rotcos, rotsin, finx, finy;
	vector2_t xvec, yvec;

	// input offsets
	fixed_t xoffs, yoffs, xpiv, ypiv;

	// final offsets
	INT16 visx, visy, visz;
	INT16 vxpiv, vypiv;

	// visual rotation
	angle_t visrollang;

	// camera angle
	angle_t viewingAngle = R_PointToAngle(mobj->x, mobj->y);

	// rotate ourselves entirely by the sprite's own rotation angle
	angle_t visrot = R_SpriteRotationAngle(mobj, NULL, interp, false);

	if (vflip)
	{
		// Invert the angle for vertically flipped sprites.
		visrot = InvAngle(visrot);
	}

	// xoffs = (-cos(xoff) + sin(yoff))
	xoffs =
		FixedMul(mobj->bakeyoff, -FINECOSINE(mobj->angle >> ANGLETOFINESHIFT)) +
		FixedMul(mobj->bakexoff, FINESINE(mobj->angle >> ANGLETOFINESHIFT));
	xpiv =
		FixedMul(mobj->bakeypiv, -FINECOSINE(mobj->angle >> ANGLETOFINESHIFT)) +
		FixedMul(mobj->bakexpiv, FINESINE(mobj->angle >> ANGLETOFINESHIFT));

	// yoffs = (-sin(yoff) + cos(xoff))
	yoffs =
		FixedMul(mobj->bakeyoff, -FINESINE(mobj->angle >> ANGLETOFINESHIFT)) +
		FixedMul(mobj->bakexoff, FINECOSINE(mobj->angle >> ANGLETOFINESHIFT));
	ypiv =
		FixedMul(mobj->bakeypiv, -FINESINE(mobj->angle >> ANGLETOFINESHIFT)) +
		FixedMul(mobj->bakexpiv, FINECOSINE(mobj->angle >> ANGLETOFINESHIFT));

	visrollang = (R_GetRollAngle(visrot) * VISROTMUL) * (hflip ? -1 : 1);

	// get pitch and roll multipliers, mainly used to align the
	// viewpoint with the camera
	fixed_t pitchMul = -FINESINE(viewingAngle >> ANGLETOFINESHIFT);
	fixed_t rollMul = FINECOSINE(viewingAngle >> ANGLETOFINESHIFT);

	// get visual positions
	visz = visy = visx = 0;
	visz = (INT16)(-(mobj->bakezoff / FRACUNIT));
	visx = (INT16)(FixedMul((yoffs / FRACUNIT), rollMul));
	visy = (INT16)(FixedMul((xoffs / FRACUNIT), pitchMul));

	vxpiv = (INT16)(FixedMul((ypiv / FRACUNIT), rollMul));
	vypiv = (INT16)(FixedMul((xpiv / FRACUNIT), pitchMul));

	// rotate by rollangle
	finx = (visx + visy);
	finy = -visz;

	rotcos = FINECOSINE(visrollang >> ANGLETOFINESHIFT);
	rotsin = FINESINE(visrollang >> ANGLETOFINESHIFT);

	xvec.x = FixedMul(finx, rotcos);
	xvec.y = FixedMul(finx, -rotsin);

	yvec.x = FixedMul(finy, rotsin);
	yvec.y = FixedMul(finy, -rotcos);

	rolloffs->x = (fixed_t)(FixedMul(mobj->rollingxoffset, rotcos) + FixedMul(mobj->rollingyoffset, rotsin));
	rolloffs->y = (fixed_t)(FixedMul(mobj->rollingxoffset, -rotsin) - FixedMul(mobj->rollingyoffset, -rotcos));

	// set finalized offsets
	out->x = (fixed_t)(xvec.x + yvec.x + vxpiv + vypiv);
	out->y = (fixed_t)(xvec.y - yvec.y) + (mobj->bakezpiv / FRACUNIT);

	// flip based on vflip and hflip
	// flip the view angle if we're horizontally flipped
	if (hflip)
	{
		out->x *= -1;
		rolloffs->x *= 1;
	}

	if (vflip)
	{
		out->y *= -1;
		rolloffs->y *= 1;
	}
	return out;
}

#undef VISROTMUL

>>>>>>> 6e99c9b5cd (Merge pull request '[FEAT] Affine sprite rendering' (#223) from softwarehell into next)
patch_t *Patch_GetRotated(patch_t *patch, INT32 angle, boolean flip)
{
	rotsprite_t *rotsprite = patch->rotated;
	if (angle < 1 || angle >= ROTANGLES)
		return NULL;

	if (rotsprite == NULL)
	{
		rotsprite = RotatedPatch_Create(ROTANGLES);
		patch->rotated = rotsprite;
	}

	if (flip)
		angle += rotsprite->angles;

	if (rotsprite->patches[angle] == NULL)
	{
		INT32 xpivot = 0, ypivot = 0;
		xpivot = patch->leftoffset;
		ypivot = patch->topoffset;
		RotatedPatch_DoRotation(rotsprite, patch, angle, xpivot, ypivot, flip);
	}
	return rotsprite->patches[angle];
}

patch_t *Patch_GetRotatedSprite(
	spriteframe_t *sprite,
	size_t frame, size_t spriteangle,
	boolean flip,
	void *info, INT32 rotationangle)
{
	rotsprite_t *rotsprite = sprite->rotated[spriteangle];
	spriteinfo_t *sprinfo = (spriteinfo_t *)info;
	INT32 idx = rotationangle;

	if (rotationangle < 1 || rotationangle >= ROTANGLES)
		return NULL;

	if (rotsprite == NULL)
	{
		rotsprite = RotatedPatch_Create(ROTANGLES);
		sprite->rotated[spriteangle] = rotsprite;
	}

	if (flip)
		idx += rotsprite->angles;

	if (rotsprite->patches[idx] == NULL)
	{
		patch_t *patch;
		INT32 xpivot = 0, ypivot = 0;
		lumpnum_t lump = sprite->lumppat[spriteangle];

		if (lump == LUMPERROR)
			return NULL;

		patch = W_CachePatchNum(lump, PU_SPRITE);

		if (sprinfo->available)
		{
			xpivot = sprinfo->pivot[frame].x;
			ypivot = sprinfo->pivot[frame].y;
		}
		else
		{
			xpivot = patch->leftoffset;
			ypivot = patch->height / 2;
		}

		RotatedPatch_DoRotation(rotsprite, patch, rotationangle, xpivot, ypivot, flip);
	}

	return rotsprite->patches[idx];
}

void Patch_Rotate(patch_t *patch, INT32 angle, INT32 xpivot, INT32 ypivot, boolean flip)
{
	if (patch->rotated == NULL)
		patch->rotated = RotatedPatch_Create(ROTANGLES);
	RotatedPatch_DoRotation(patch->rotated, patch, angle, xpivot, ypivot, flip);
}

rotsprite_t *RotatedPatch_Create(INT32 numangles)
{
	rotsprite_t *rotsprite = Z_Calloc(sizeof(rotsprite_t), PU_STATIC, NULL);
	rotsprite->angles = numangles;
	rotsprite->patches = Z_Calloc(rotsprite->angles * 2 * sizeof(void *), PU_STATIC, NULL);
	return rotsprite;
}

static void RotatedPatch_CalculateDimensions(
	INT32 width, INT32 height,
	fixed_t ca, fixed_t sa,
	INT32 *newwidth, INT32 *newheight)
{
	fixed_t fixedwidth = (width * FRACUNIT);
	fixed_t fixedheight = (height * FRACUNIT);

	fixed_t w1 = abs(FixedMul(fixedwidth, ca) - FixedMul(fixedheight, sa));
	fixed_t w2 = abs(FixedMul(-fixedwidth, ca) - FixedMul(fixedheight, sa));
	fixed_t h1 = abs(FixedMul(fixedwidth, sa) + FixedMul(fixedheight, ca));
	fixed_t h2 = abs(FixedMul(-fixedwidth, sa) + FixedMul(fixedheight, ca));

	w1 = FixedInt(FixedCeil(w1 + (FRACUNIT/2)));
	w2 = FixedInt(FixedCeil(w2 + (FRACUNIT/2)));
	h1 = FixedInt(FixedCeil(h1 + (FRACUNIT/2)));
	h2 = FixedInt(FixedCeil(h2 + (FRACUNIT/2)));

	*newwidth = max(width, max(w1, w2));
	*newheight = max(height, max(h1, h2));
}

void RotatedPatch_DoRotation(rotsprite_t *rotsprite, patch_t *patch, INT32 angle, INT32 xpivot, INT32 ypivot, boolean flip)
{
	patch_t *rotated;
	UINT16 *rawdst, *rawconv;
	size_t size;
	pictureflags_t bflip = (flip) ? PICFLAGS_XFLIP : 0;

	INT32 width = patch->width;
	INT32 height = patch->height;
	INT32 leftoffset = patch->leftoffset;
	INT32 newwidth, newheight;

	fixed_t ca = rollcosang[angle];
	fixed_t sa = rollsinang[angle];
	fixed_t xcenter, ycenter;
	INT32 idx = angle;
	INT32 x, y;
	INT32 sx, sy;
	INT32 dx, dy;
	INT32 ox, oy;
	INT32 minx, miny, maxx, maxy;

	// Don't cache angle = 0
	if (angle < 1 || angle >= ROTANGLES)
		return;

	if (flip)
	{
		idx += rotsprite->angles;
		xpivot = width - xpivot;
		leftoffset = width - leftoffset;
	}

	if (rotsprite->patches[idx])
		return;

	// Find the dimensions of the rotated patch.
	RotatedPatch_CalculateDimensions(width, height, ca, sa, &newwidth, &newheight);

	xcenter = (xpivot * FRACUNIT);
	ycenter = (ypivot * FRACUNIT);

	if (xpivot != width / 2 || ypivot != height / 2)
	{
		newwidth *= 2;
		newheight *= 2;
	}

	minx = newwidth;
	miny = newheight;
	maxx = 0;
	maxy = 0;

	// Draw the rotated sprite to a temporary buffer.
	size = (newwidth * newheight);
	if (!size)
		size = (width * height);
	rawdst = Z_Calloc(size * sizeof(UINT16), PU_STATIC, NULL);

	for (dy = 0; dy < newheight; dy++)
	{
		for (dx = 0; dx < newwidth; dx++)
		{
			x = (dx - (newwidth / 2)) * FRACUNIT;
			y = (dy - (newheight / 2)) * FRACUNIT;
			sx = FixedMul(x, ca) + FixedMul(y, sa) + xcenter;
			sy = -FixedMul(x, sa) + FixedMul(y, ca) + ycenter;

			sx >>= FRACBITS;
			sy >>= FRACBITS;

			if (sx >= 0 && sy >= 0 && sx < width && sy < height)
			{
				void *input = Picture_GetPatchPixel(patch, PICFMT_PATCH, sx, sy, bflip);
				if (input != NULL)
				{
					rawdst[(dy * newwidth) + dx] = (0xFF00 | (*(UINT8 *)input));
					if (dx < minx)
						minx = dx;
					if (dy < miny)
						miny = dy;
					if (dx > maxx)
						maxx = dx;
					if (dy > maxy)
						maxy = dy;
				}
			}
		}
	}

	ox = (newwidth / 2) + (leftoffset - xpivot);
	oy = (newheight / 2) + (patch->topoffset - ypivot);
	width = (maxx - minx);
	height = (maxy - miny);

	if ((unsigned)(width * height) > size)
	{
		UINT16 *src, *dest;

		size = (width * height);
		rawconv = Z_Calloc(size * sizeof(UINT16), PU_STATIC, NULL);

		src = &rawdst[(miny * newwidth) + minx];
		dest = rawconv;
		dy = height;

		while (dy--)
		{
			M_Memcpy(dest, src, width * sizeof(UINT16));
			dest += width;
			src += newwidth;
		}

		ox -= minx;
		oy -= miny;

		Z_Free(rawdst);
	}
	else
	{
		rawconv = rawdst;
		width = newwidth;
		height = newheight;
	}

	// make patch
	rotated = (patch_t *)Picture_Convert(PICFMT_FLAT16, rawconv, PICFMT_PATCH, 0, NULL, width, height, 0, 0, 0);

	Z_ChangeTag(rotated, PU_PATCH_ROTATED);
	Z_SetUser(rotated, (void **)(&rotsprite->patches[idx]));
	Z_Free(rawconv);

	rotated->leftoffset = ox;
	rotated->topoffset = oy;
}
#endif
