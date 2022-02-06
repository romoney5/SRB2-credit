// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2020-2022 by Jaime Ita Passos.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_patchrotation.h
/// \brief Patch rotation.

#include "r_patch.h"
#include "r_picformats.h"

rotsprite_t *RotatedPatch_Create(INT32 numangles);
void RotatedPatch_DoRotation(rotsprite_t *rotsprite, patch_t *patch, angle_t angle, INT32 xpivot, INT32 ypivot, boolean flip);
void RotatedPatch_CalculateDimensions(
	INT32 width, INT32 height, angle_t angle,
	INT32 *newwidth, INT32 *newheight);

extern fixed_t rollcosang[ROTANGLES];
extern fixed_t rollsinang[ROTANGLES];
