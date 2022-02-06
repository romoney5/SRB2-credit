// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2021-2022 by Jaime Ita Passos.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------

void DC_ROTATION_FUNC(void)
{
	UINT8 *dest;
	fixed_t frac;
	fixed_t ca, sa;

	fixed_t precalc_x;
	fixed_t precalc_x_ca;
	fixed_t precalc_x_sa;

#ifdef RANGECHECK
	if ((unsigned)dc_x >= (unsigned)vid.width || dc_yl < 0 || dc_yh >= vid.height)
		return;
#endif

	dest = &topleft[dc_yl*vid.width + dc_x];
	frac = dc_texturemid + FixedMul((dc_yl << FRACBITS) - centeryfrac, dc_iscale);
	ca = dc_rotation.precalc_cos;
	sa = dc_rotation.precalc_sin;

#ifdef DC_ROTATION_INTEGER
	precalc_x = (dc_rotation.column<<FRACBITS) - dc_rotation.midpoint_x;
#else
	precalc_x = dc_rotation.column - dc_rotation.midpoint_x;
#endif

	precalc_x_ca = FixedMul(precalc_x, ca);
	precalc_x_sa = FixedMul(precalc_x, sa);

	for (; dc_yl <= dc_yh; frac += dc_iscale, dc_yl++)
	{
		fixed_t y = frac;
		fixed_t sx, sy;

#ifdef DC_ROTATION_INTEGER
		y >>= FRACBITS;
		y <<= FRACBITS;
#endif

		y -= dc_rotation.midpoint_y;

		sx = precalc_x_ca + FixedMul(y, sa) + dc_rotation.patch_center_x;
		sy = -precalc_x_sa + FixedMul(y, ca) + dc_rotation.patch_center_y;

		sx >>= FRACBITS;
		sy >>= FRACBITS;

		if (sx >= 0 && sy >= 0 && sx < dc_rotation.patch_width && sy < dc_rotation.patch_height)
		{
			UINT16 src_pixel = dc_rotation.source[sy * dc_rotation.patch_width + sx];
			if (src_pixel & 0xFF00)
				*dest = DC_ROTATION_PIXELFUNC;
		}

		dest += vid.width;
	}
}
