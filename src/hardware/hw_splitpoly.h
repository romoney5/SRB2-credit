// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file hw_splitpoly.h
/// \brief Convex polygon splitting

#ifndef __HWR_SPLITPOLY_H__
#define __HWR_SPLITPOLY_H__

#define DIVLINE_VERTEX_DIFF 0.45f

typedef struct
{
	float x, y;
	float dx, dy;
} FDivLine;

boolean HWR_SplitPolygon (F2DLine *line,
                         FOutVector *poly, INT32 numpts,
                         FOutVector **frontpoly, INT32 *frontpts,
                         FOutVector **backpoly, INT32 *backpts);

#endif // __HWR_SPLITPOLY_H__
