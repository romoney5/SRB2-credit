// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 1998-2000 by DooM Legacy Team.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file hw_splitpoly.c
/// \brief Convex polygon splitting

#include "../doomdef.h"
#include "../doomstat.h"

#ifdef HWRENDER
#include "hw_glob.h"
#include "hw_splitpoly.h"

static FOutVector *IntersectLines (FOutVector *v1, FOutVector *v2, F2DLine *line, float *outfrac)
{
	static FOutVector pt;
	double frac;
	double num;
	double den;
	double v1x,v1y,v1dx,v1dy;
	double v2x,v2y,v2dx,v2dy;

	// a segment of a polygon
	v1x  = v1->x;
	v1y  = v1->y;
	v1dx = v2->x - v1->x;
	v1dy = v2->y - v1->y;

	// the intersection line
	v2x  = line->v1.x;
	v2y  = line->v1.y;
	v2dx = line->v2.x - line->v1.x;
	v2dy = line->v2.y - line->v1.y;

	den = v2dy*v1dx - v2dx*v1dy;
	if (fabsf((float)den) < 1.0E-36f) // avoid checking exactly for 0.0
		return NULL;       // parallel

	// first check the frac along the polygon segment,
	// (do not accept hit with the extensions)
	num = (v2x - v1x)*v2dy + (v1y - v2y)*v2dx;
	frac = num / den;
	if (frac < 0.0l || frac > 1.0l)
		return NULL;

	// now get the frac along the line
	// which is useful to determine what is left, what is right
	num = (v2x - v1x)*v1dy + (v1y - v2y)*v1dx;
	frac = num / den;
	*outfrac = (float)frac;

	// find the interception point along the partition line
	pt.x = (float)(v2x + v2dx*frac);
	pt.y = (float)(v2y + v2dy*frac);

	return &pt;
}

// if two vertice coords have a x and/or y difference
// of less or equal than 1 FRACUNIT, they are considered the same
// point. Note: hardcoded value, 1.0f could be anything else.
static boolean SameVertice (FOutVector *p1, FOutVector *p2)
{
	float ep = DIVLINE_VERTEX_DIFF;
	if (fabsf( p2->x - p1->x ) > ep )
		return false;
	if (fabsf( p2->y - p1->y ) > ep )
		return false;
	// p1 and p2 are considered the same vertex
	return true;
}

// Splits a convex polygon into two convex polygons
boolean HWR_SplitPolygon (F2DLine *line,
                          FOutVector *poly, INT32 numpts,
                          FOutVector **frontpoly, INT32 *frontpts,
                          FOutVector **backpoly, INT32 *backpts)
{
	INT32 i, j;
	FOutVector *pv;

	INT32 ps = -1, pe = -1;
	INT32 nptfront, nptback;
	FOutVector vs = {0,0,0,0,0};
	FOutVector ve = {0,0,0,0,0};
	FOutVector lastpv = {0,0,0,0,0};
	float fracs = 0.0f, frace = 0.0f;
	INT32 psonline = 0, peonline = 0;

	static FOutVector polyfront[16];
	static FOutVector polyback[16];

	for (i = 0; i < numpts; i++)
	{
		float frac;

		j = i + 1;
		if (j == numpts) j = 0;

		// start & end points
		pv = IntersectLines(&poly[i], &poly[j], line, &frac);

		if (pv == NULL)
			continue;

		if (ps < 0)
		{
			// first point
			ps = i;
			vs = *pv;
			fracs = frac;
		}
		else
		{
			// the partition line traverse a junction between two segments
			// or the two points are so close, they can be considered as one
			// thus, don't accept, since split 2 must be another vertex
			if (SameVertice(pv, &lastpv))
			{
				if (pe < 0)
				{
					ps = i;
					psonline = 1;
				}
				else
				{
					pe = i;
					peonline = 1;
				}
			}
			else
			{
				if (pe < 0)
				{
					pe = i;
					ve = *pv;
					frace = frac;
				}
				else
				{
					// a frac, not same vertice as last one
					// we already got pt2 so pt 2 is not on the line,
					// so we probably got back to the start point
					// which is on the line
					if (SameVertice(pv, &vs))
						psonline = 1;
					break;
				}
			}
		}

		// remember last point intercept to detect identical points
		lastpv = *pv;
	}

	// no split: the partition line is either parallel and
	// aligned with one of the poly segments, or the line is totally
	// out of the polygon and doesn't traverse it
	if (ps < 0 || pe < 0)
	{
		*frontpoly = poly;
		*frontpts = numpts;
		*backpoly = NULL;
		*backpts = 0;
		return false;
	}
	else if (pe <= ps)
	{
		*frontpoly = NULL;
		*frontpts = 0;
		*backpoly = NULL;
		*backpts = 0;
		return false;
	}

	// number of points on each side, _not_ counting those
	// that may lie just one the line
	nptback  = pe - ps - peonline;
	nptfront = numpts - peonline - psonline - nptback;

	if (nptback > 0)
	{
		*backpoly = polyback;
		*backpts = 2 + nptback;
	}
	else
	{
		*backpoly = NULL;
		*backpts = 0;
	}

	if (nptfront > 0)
	{
		*frontpoly = polyfront;
		*frontpts = 2 + nptfront;
	}
	else
	{
		*frontpoly = NULL;
		*frontpts = 0;
	}

	// generate FRONT poly
	if (*frontpoly)
	{
		pv = *frontpoly;
		*pv++ = vs;
		*pv++ = ve;
		i = pe;
		do
		{
			if (++i == numpts)
				i = 0;
			*pv++ = poly[i];
		} while (i != ps && --nptfront);
	}

	// generate BACK poly
	if (*backpoly)
	{
		pv = *backpoly;
		*pv++ = ve;
		*pv++ = vs;
		i = ps;
		do
		{
			if (++i == numpts)
				i = 0;
			*pv++ = poly[i];
		} while (i != pe && --nptback);
	}

	// make sure frontpoly is the one on the 'right' side
	// of the partition line
	if (fracs > frace)
	{
		FOutVector *swappoly = *backpoly;
		INT32 swappts = *backpts;
		*backpoly = *frontpoly;
		*backpts = *frontpts;
		*frontpoly = swappoly;
		*frontpts = swappts;
	}

	return true;
}

#endif
