// BLANKART
//-----------------------------------------------------------------------------
// Copyright (C) 2024 by Kart Krew.
// Copyright (C) 2020 by Sonic Team Junior.
// Copyright (C) 2000 by DooM Legacy Team.
// Copyright (C) 1996 by id Software, Inc.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  r_draw_column.cpp
/// \brief column drawer functions
/// \note  no includes because this is included as part of r_draw.cpp

// ==========================================================================
// COLUMNS
// ==========================================================================

// A column is a vertical slice/span of a wall texture that uses
// a has a constant z depth from top to bottom.
//

#include "r_main.h"
#include "r_draw.h"
#include <tracy/tracy/Tracy.hpp>


#include "r_draw_flush.cpp"

enum DrawColumnType
{
	DC_BASIC			= 0x0000,
	DC_COLORMAP			= 0x0001,
	DC_TRANSMAP			= 0x0002,
	DC_BRIGHTMAP		= 0x0004,
	DC_HOLES			= 0x0008,
	DC_LIGHTLIST		= 0x0010,
	DC_DIRECT			= 0x0020, // draw our columns directly to screen!
};

template<DrawColumnType Type>
FUNCINLINE static ATTRINLINE constexpr UINT8 R_GetColumnTranslated(drawcolumndata_t* dc, UINT8 col)
{
	if constexpr (Type & DrawColumnType::DC_COLORMAP)
	{
		return dc->translation[col];
	}
	else
	{
		return col;
	}
}

template<DrawColumnType Type>
FUNCINLINE static ATTRINLINE constexpr UINT8 R_GetColumnBrightmapped(drawcolumndata_t* dc, UINT32 bit, UINT8 col)
{
	col = R_GetColumnTranslated<Type>(dc, col);

	if constexpr (Type & DrawColumnType::DC_BRIGHTMAP)
	{
		if (dc->brightmap[bit] == BRIGHTPIXEL)
		{
			return dc->fullbright[col];
		}
	}

	return dc->colormap[col];
}

// translucency is handled on flush side now!
template<DrawColumnType Type>
FUNCINLINE static ATTRINLINE constexpr UINT8 R_GetColumnTranslucent(drawcolumndata_t* dc, UINT8 *dest, UINT32 bit, UINT8 col)
{
	col = R_GetColumnBrightmapped<Type>(dc, bit, col);

	if constexpr (Type & DrawColumnType::DC_TRANSMAP)
	{
		return *(dc->transmap + (col << 8) + (*dest));
	}
	else
	{
		return col;
	}
}

template<DrawColumnType Type>
FUNCINLINE static ATTRINLINE constexpr UINT8 R_DrawColumnPixel(drawcolumndata_t* dc, UINT8 *dest, UINT32 bit)
{
	UINT8 col = dc->source[bit];

	if constexpr (Type & DrawColumnType::DC_HOLES)
	{
		if (col == TRANSPARENTPIXEL)
		{
			return *dest;
		}
	}

	// if we dont buffer our columns, we need to handle translucency again
	return R_GetColumnTranslucent<Type>(dc, dest, bit, col);
}

// Function flow: Holes -> Translation -> Brightmap -> Colormap -> Translucency
// "Why are these in nested functions for standard columns?" Uhhhhhh iunno lol
// I make-a da code-a

template<DrawColumnType Type>
FUNCINLINE static ATTRINLINE constexpr UINT16 R_DrawColumnAffinePixel(drawcolumndata_t* dc, UINT8 *dest, INT32 bit)
{
	const UINT16 pixel = reinterpret_cast<UINT16 *>(dc->source)[bit];

	UINT8 col = (UINT8)(pixel & 0xff);

	if (pixel < 0xff00)
	{
		return 0;
	}

	if constexpr (Type & DrawColumnType::DC_HOLES)
	{
		if (col == TRANSPARENTPIXEL)
		{
			return 0;
		}
	}

	if constexpr (Type & DrawColumnType::DC_COLORMAP)
	{
		// Remap to the current translation
		col = dc->translation[col];
	}

	boolean was_brightmapped = false;

	if constexpr (Type & DrawColumnType::DC_BRIGHTMAP)
	{
		// For affine drawers, dc->brightmap points to a *flat*, and not a *column*.
		// Keep that in mind before you do something that causes a bunch of segfaults!

		if (dc->brightmap)
		{
			const UINT16 bmpixel = reinterpret_cast<UINT16 *>(dc->brightmap)[bit];

			if ((bmpixel & 0xff) == BRIGHTPIXEL)
			{
				// Pixel is part of the brightmap
				col = dc->fullbright[col];
				was_brightmapped = true;
			}
		}
	}

	if (!was_brightmapped)
	{
		col = dc->colormap[col];
	}

	if constexpr (Type & DrawColumnType::DC_TRANSMAP)
	{
		// Pixel is translucent
		col = *(dc->transmap + (col << 8) + (*dest));
	}

	*dest = col;

	return (0xff00 | col);
}

/**	\brief The R_DrawColumn function
	Experiment to make software go faster. Taken from the Boom source
*/
template<DrawColumnType Type>
static void R_DrawColumnTemplate(drawcolumndata_t *dc)
{
	INT32 count;
	const INT32 vidheight = vid.height;

	// leban 1/17/99:
	// removed the + 1 here, adjusted the if test, and added an increment
	// later.  this helps a compiler pipeline a bit better.  the x86
	// assembler also does this.
	count = dc->yh - dc->yl;

	// leban 1/17/99:
	// this case isn't executed too often.  depending on how many instructions
	// there are between here and the second if test below, this case could
	// be moved down and might save instructions overall.  since there are
	// probably different wads that favor one way or the other, i'll leave
	// this alone for now.
	if (count < 0) // Zero length, column does not exceed a pixel.
	{
		return;
	}

	if ((unsigned)dc->x >= (unsigned)vid.width || dc->yl < 0 || dc->yh >= vidheight)
	{
		return;
	}

	if constexpr (Type & DrawColumnType::DC_LIGHTLIST)
	{
		constexpr DrawColumnType NewType = static_cast<DrawColumnType>(Type & ~DC_LIGHTLIST);
		INT32 i, realyh, height, bheight = 0, solid = 0;
		drawcolumndata_t dc_copy = *dc;

		realyh = dc_copy.yh;

		// This runs through the lightlist from top to bottom and cuts up the column accordingly.
		for (i = 0; i < dc_copy.numlights; i++)
		{
			// If the height of the light is above the column, get the colormap
			// anyway because the lighting of the top should be affected.
			solid = dc_copy.lightlist[i].flags & FOF_CUTSOLIDS;
			height = dc_copy.lightlist[i].height >> LIGHTSCALESHIFT;

			if (solid)
			{
				bheight = dc_copy.lightlist[i].botheight >> LIGHTSCALESHIFT;

				if (bheight < height)
				{
					// confounded slopes sometimes allow partial invertedness,
					// even including cases where the top and bottom heights
					// should actually be the same!
					// swap the height values as a workaround for this quirk
					INT32 temp = height;
					height = bheight;
					bheight = temp;
				}
			}

			if (height <= dc_copy.yl)
			{
				dc_copy.colormap = dc_copy.lightlist[i].rcolormap;
				dc_copy.fullbright = colormaps;

				if (encoremap)
				{
					dc_copy.colormap += COLORMAP_REMAPOFFSET;
					dc_copy.fullbright += COLORMAP_REMAPOFFSET;
				}

				if (solid && dc_copy.yl < bheight)
				{
					dc_copy.yl = bheight;
				}

				continue;
			}

			// Found a break in the column!
			dc_copy.yh = height;

			if (dc_copy.yh > realyh)
			{
				dc_copy.yh = realyh;
			}

			R_DrawColumnTemplate<NewType>(&dc_copy);

			if (solid)
			{
				dc_copy.yl = bheight;
			}
			else
			{
				dc_copy.yl = dc_copy.yh + 1;
			}

			dc_copy.colormap = dc_copy.lightlist[i].rcolormap;
			dc_copy.fullbright = colormaps;
			if (encoremap)
			{
				dc_copy.colormap += COLORMAP_REMAPOFFSET;
				dc_copy.fullbright += COLORMAP_REMAPOFFSET;
			}
		}

		dc_copy.yh = realyh;

		if (dc_copy.yl <= realyh)
		{
			R_DrawColumnTemplate<NewType>(&dc_copy);
		}
	}
	else
	{
		// Inner loop that does the actual texture mapping,
		//  e.g. a DDA-lile scaling.
		// This is as fast as it gets.       (Yeah, right!!! -- killough)
		//
		// killough 2/1/98: more performance tuning

		intptr_t frac;
		// Looks familiar.
		const intptr_t fracstep = dc->iscale;
		const intptr_t heightmask = dc->sourcelength-1; // CPhipps - specify type
		constexpr INT32 npow2min = -1;
		const INT32 npow2max = dc->sourcelength;

		// Framebuffer destination address.
		// SoM: MAGIC
		UINT8 * restrict dest;

		if constexpr (Type & DrawColumnType::DC_DIRECT)
			dest = R_Address(dc->x, dc->yl);
		else if constexpr ((Type & (DrawColumnType::DC_COLORMAP | DrawColumnType::DC_TRANSMAP))
			== (DrawColumnType::DC_COLORMAP | DrawColumnType::DC_TRANSMAP))
			dest = R_GetBufferColormapTrans(dc);
		else if constexpr (Type & DrawColumnType::DC_TRANSMAP)
			dest = R_GetBufferTrans(dc);
		else if constexpr (Type & DrawColumnType::DC_COLORMAP)
			dest = R_GetBufferColormap(dc);
		else
			dest = R_GetBufferOpaque(dc);

		INT32 stride = 8; // SoM: Oh, Oh it's MAGIC! You know...

		if constexpr (Type & DrawColumnType::DC_DIRECT)
			stride = vid.width;

		count++;

		// Determine scaling, which is the only mapping to be done.
		frac = (dc->texturemid + FixedMul((dc->yl << FRACBITS) - centeryfrac, fracstep));

		switch (heightmask)
		{
			case 255:
			case 127:
			{
				while (count--)
				{
					*dest = R_DrawColumnPixel<Type>(dc, dest, (frac>>FRACBITS) & heightmask);

					dest += stride;
					frac += fracstep;
				}
			}
			break;
			case npow2min:
			{
				if (frac < 0)
					// adjust in case we underread
					frac += fracstep;

				// texture has no height, so just go
				while (--count >= 0)
				{
					*dest = R_DrawColumnPixel<Type>(dc, dest, frac>>FRACBITS);

					dest += stride;
					frac += fracstep;
				}
			}
			break;
			default:
			{
				if (!(dc->sourcelength & heightmask))   // power of 2 -- killough
				{
					while ((count -= 2) >= 0) // texture height is a power of 2 -- killough
					{
						*dest = R_DrawColumnPixel<Type>(dc, dest, (frac>>FRACBITS) & heightmask);

						dest += stride;
						frac += fracstep;

						*dest = R_DrawColumnPixel<Type>(dc, dest, (frac>>FRACBITS) & heightmask);

						dest += stride;
						frac += fracstep;
					}

					if (count & 1)
					{
						*dest = R_DrawColumnPixel<Type>(dc, dest, (frac>>FRACBITS) & heightmask);
					}
				}
				else
				{
					const intptr_t fixed_heightmask = dc->texheight << FRACBITS;

					if (frac < 0)
					{
						while ((frac += fixed_heightmask) < 0)
						{
							;
						}
					}
					else
					{
						while (frac >= fixed_heightmask)
						{
							frac -= fixed_heightmask;
						}
					}

					do
					{
						// Re-map color indices from wall texture column
						//  using a lighting/special effects LUT.
						// heightmask is the Tutti-Frutti fix -- killough

						// -1 is the lower clamp bound because column posts have a "safe" byte before the real data
						// and a few bytes after as well
						*dest = R_DrawColumnPixel<Type>(dc, dest, CLAMP((frac >> FRACBITS), npow2min, npow2max));

						dest += stride;

						#if __SIZEOF_POINTER__ < 8  // 64-bit systems have large enough numbers for this to be a non-issue
						// Avoid overflow.
						if (fracstep > 0x7FFFFFFF - frac)
						{
							frac += fracstep - fixed_heightmask;
						}
						else
							#endif
						{
							frac += fracstep;
						}

						while (frac >= fixed_heightmask)
						{
							frac -= fixed_heightmask;
						}
					}
					while (--count);
				}
			}
			break;
		}
	}
}

#define DEFINE_COLUMN_FUNC(name, flags) \
	void name(drawcolumndata_t *dc) \
	{ \
		ZoneScoped; \
		constexpr DrawColumnType opt = static_cast<DrawColumnType>(flags); \
		R_DrawColumnTemplate<opt>(dc); \
	}

#define DEFINE_COLUMN_COMBO(name, flags) \
	DEFINE_COLUMN_FUNC(name, flags|DC_DIRECT) \
	DEFINE_COLUMN_FUNC(name ## _Brightmap, flags|DC_DIRECT|DC_BRIGHTMAP) \
	DEFINE_COLUMN_FUNC(name ## _Flush, flags)

DEFINE_COLUMN_COMBO(R_DrawColumn, DC_BASIC)
DEFINE_COLUMN_COMBO(R_DrawTranslucentColumn, DC_TRANSMAP)
DEFINE_COLUMN_COMBO(R_DrawTranslatedColumn, DC_COLORMAP)
DEFINE_COLUMN_COMBO(R_DrawColumnShadowed, DC_LIGHTLIST)
DEFINE_COLUMN_COMBO(R_DrawTranslatedTranslucentColumn, DC_COLORMAP|DC_TRANSMAP)
DEFINE_COLUMN_COMBO(R_Draw2sMultiPatchColumn, DC_HOLES)
DEFINE_COLUMN_COMBO(R_Draw2sMultiPatchTranslucentColumn, DC_HOLES|DC_TRANSMAP)


void R_DrawFogColumn(drawcolumndata_t *dc)
{
	ZoneScoped;

	INT32 count;
	UINT8 *dest;
	const INT32 vidheight = vid.height;
	const INT32 vidwidth = vid.width;

	count = dc->yh - dc->yl;

	// Zero length, column does not exceed a pixel.
	if (count < 0)
		return;

	if ((unsigned)dc->x >= (unsigned)vidwidth || dc->yl < 0 || dc->yh >= vidheight)
		return;

	// Framebuffer destination address.
	dest = R_Address(dc->x, dc->yl);

	// Determine scaling, which is the only mapping to be done.
	do
	{
		// Simple. Apply the colormap to what's already on the screen.
		*dest = dc->colormap[*dest];
		dest += vidwidth;
	}
	while (count--);
}

void R_DrawDropShadowColumn(drawcolumndata_t *dc)
{
	ZoneScoped;

	// Hack: A cut-down copy of R_DrawTranslucentColumn_8 that does not read texture
	// data since something about calculating the texture reading address for drop shadows is broken.
	// dc_texturemid and dc_iscale get wrong values for drop shadows, however those are not strictly
	// needed for the current design of the shadows, so this function bypasses the issue
	// by not using those variables at all.

	INT32 count;
	UINT8 *dest;
	const INT32 vidwidth = vid.width;

	count = dc->yh - dc->yl + 1;

	if (count <= 0) // Zero length, column does not exceed a pixel.
		return;

	dest = R_Address(dc->x, dc->yl);

	const UINT8 *transmap_offset = dc->transmap + (dc->shadowcolor << 8);
	while ((count -= 2) >= 0)
	{
		*dest = *(transmap_offset + (*dest));
		dest += vidwidth;
		*dest = *(transmap_offset + (*dest));
		dest += vidwidth;
	}

	if (count & 1)
		*dest = *(transmap_offset + (*dest));
}

void R_DrawColumn_Flat(drawcolumndata_t *dc)
{
	ZoneScoped;

	INT32 count;
	UINT8 color = dc->lightmap[dc->r8_flatcolor];
	UINT8 *dest;
	const INT32 vidheight = vid.height;
	const INT32 vidwidth = vid.width;

	count = dc->yh - dc->yl;

	if (count < 0) // Zero length, column does not exceed a pixel.
		return;

	if ((unsigned)dc->x >= (unsigned)vidwidth || dc->yl < 0 || dc->yh >= vidheight)
		return;

	// Framebuffer destination address.
	dest = R_Address(dc->x, dc->yl);

	count++;

	do
	{
		*dest = color;
		dest += vidwidth;
	}
	while (--count);
}

template<DrawColumnType Type>
static void R_DrawAffineColumnTemplate(drawcolumndata_t *dc)
{
	INT32 count;
	const INT32 vidheight = vid.height;

	// leban 1/17/99:
	// removed the + 1 here, adjusted the if test, and added an increment
	// later.  this helps a compiler pipeline a bit better.  the x86
	// assembler also does this.
	count = dc->yh - dc->yl;

	// leban 1/17/99:
	// this case isn't executed too often.  depending on how many instructions
	// there are between here and the second if test below, this case could
	// be moved down and might save instructions overall.  since there are
	// probably different wads that favor one way or the other, i'll leave
	// this alone for now.
	if (count < 0) // Zero length, column does not exceed a pixel.
	{
		return;
	}

	if ((unsigned)dc->x >= (unsigned)vid.width || dc->yl < 0 || dc->yh >= vidheight)
	{
		return;
	}

	{
		// Inner loop that does the actual texture mapping,
		//  e.g. a DDA-lile scaling.
		// This is as fast as it gets.       (Yeah, right!!! -- killough)
		//
		// killough 2/1/98: more performance tuning

		intptr_t frac;
		// Looks familiar.
		const intptr_t fracstep = dc->iscale;
		const intptr_t heightmask = dc->sourcelength-1; // CPhipps - specify type
		constexpr INT32 npow2min = -1;
		const INT32 npow2max = dc->sourcelength;

		// Framebuffer destination address.
		// SoM: MAGIC
		UINT8 * restrict dest;

		if constexpr (Type & DrawColumnType::DC_DIRECT)
			dest = R_Address(dc->x, dc->yl);
		else if constexpr ((Type & (DrawColumnType::DC_COLORMAP | DrawColumnType::DC_TRANSMAP))
			== (DrawColumnType::DC_COLORMAP | DrawColumnType::DC_TRANSMAP))
			dest = R_GetBufferColormapTrans(dc);
		else if constexpr (Type & DrawColumnType::DC_TRANSMAP)
			dest = R_GetBufferTrans(dc);
		else if constexpr (Type & DrawColumnType::DC_COLORMAP)
			dest = R_GetBufferColormap(dc);
		else
			dest = R_GetBufferOpaque(dc);

		const INT32 stride = vid.width; // SoM: Oh, Oh it's MAGIC! You know...

		count++;

		// Determine scaling, which is the only mapping to be done.
		frac = (dc->texturemid + FixedMul((dc->yl << FRACBITS) - centeryfrac, fracstep));

		const affine_t *transform = &dc->affine;
		const affine_bounding_t *bounds = &dc->affinebound;

		const fixed_t a = transform->a;
		const fixed_t b = transform->b;
		const fixed_t c = transform->c;
		const fixed_t d = transform->d;
		fixed_t cx = transform->ox;
		fixed_t cy = transform->oy;

		const INT32 pw = dc->sourcelength, ph = dc->texheight;

		const boolean vflip = (dc->affineystep < 0);
		const fixed_t ystep_delta = abs(dc->affineystep);

		fixed_t ydiff = (bounds->yup * FRACUNIT) - cy;
		fixed_t xdiff = (bounds->xleft * FRACUNIT) - cx;

		xdiff -= (xdiff ? FRACUNIT : 0);
		ydiff -= (ydiff ? FRACUNIT : 0);

		// Offset our X and Y positions by the bounding differences.
		fixed_t cxx = cx + xdiff + dc->affineoffset.x;
		fixed_t cyy = cy + ydiff + dc->affineoffset.y;

		// If we're smaller than usual, mosaic isn't necessary.

		const float y_mosaic = std::max(1.0f, dc->affinemosaic.y);
		const float x_mosaic = std::max(1.0f, dc->affinemosaic.x);

		const boolean mosaic_on = cv_affinemosaic.value;
		const boolean xmosaic_on = (FLOAT_TO_FIXED(x_mosaic) > FRACUNIT);
		const boolean ymosaic_on = (FLOAT_TO_FIXED(y_mosaic) > FRACUNIT);

		fixed_t usefrac = dc->frac;

		if (mosaic_on && xmosaic_on)
		{
			usefrac = FLOAT_TO_FIXED(static_cast<INT32>(FIXED_TO_FLOAT(dc->frac) / x_mosaic) * x_mosaic);
		}

		float ypx = 0.0f;

		// yoinked from NovaSquirrel's mode 7 0preview
		// ...which is in turn yoinked from Mesen's S-PPU code
		// i can't do matrix math to save my life :face_holding_back_tears:
		// (m7xofs and m7yofs are already factored in by destbase)
		fixed_t ux = FixedMul(b, -cyy) + FixedMul(a, -cxx) + FixedMul(a, usefrac) + cx;
		fixed_t uy = FixedMul(d, -cyy) + FixedMul(c, -cxx) + FixedMul(c, usefrac) + cy;
		float ux_base = FIXED_TO_FLOAT(ux);
		float uy_base = FIXED_TO_FLOAT(uy);
		float f_b = FIXED_TO_FLOAT(FixedMul(b, ystep_delta));
		float f_d = FIXED_TO_FLOAT(FixedMul(d, ystep_delta));

		for (; count > 0; dest += stride, --count)
		{
			ypx += 1.0f;

			const INT32 srcx = ux >> FRACBITS;
			const INT32 srcy = (vflip) ? (ph - (uy >> FRACBITS)) : (uy >> FRACBITS);
			INT32 y_real = static_cast<INT32>(ypx);

			if (mosaic_on && ymosaic_on)
			{
				y_real = (static_cast<INT32>(ypx / y_mosaic) * y_mosaic);
			}

			ux = FLOAT_TO_FIXED(ux_base + (f_b * y_real));
			uy = FLOAT_TO_FIXED(uy_base + (f_d * y_real));

			if (srcx < 0 || srcx >= pw || srcy < 0 || srcy >= ph)
			{
				continue;
			}

			R_DrawColumnAffinePixel<Type>(dc, dest, srcy * pw + srcx);
		}
	}
}

#define DEFINE_AFFINE_COLUMN_FUNC(name, flags) \
	void name(drawcolumndata_t *dc) \
	{ \
		ZoneScoped; \
		constexpr DrawColumnType opt = static_cast<DrawColumnType>(flags); \
		R_DrawAffineColumnTemplate<opt>(dc); \
	}

#define DEFINE_AFFINE_COLUMN_SETUP(name, flags) \
	DEFINE_AFFINE_COLUMN_FUNC(name, flags|DC_DIRECT) \
	DEFINE_AFFINE_COLUMN_FUNC(name ## _Brightmap, flags|DC_DIRECT|DC_BRIGHTMAP)

DEFINE_AFFINE_COLUMN_SETUP(R_DrawAffineColumn, DC_BASIC);
DEFINE_AFFINE_COLUMN_SETUP(R_DrawTranslatedAffineColumn, DC_COLORMAP);
DEFINE_AFFINE_COLUMN_SETUP(R_DrawTranslucentAffineColumn, DC_TRANSMAP);
DEFINE_AFFINE_COLUMN_SETUP(R_DrawTranslatedTranslucentAffineColumn, DC_COLORMAP|DC_TRANSMAP);
