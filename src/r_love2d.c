
#include "doomdef.h"
#include "r_defs.h"
#include "d_player.h"

__declspec(dllexport) void (*LOVECallback_OnRender) (void) = NULL;
__declspec(dllexport) void (*LOVECallback_DrawSoftwareBuffer) (UINT8 *buffer, INT32 width, INT32 height) = NULL;
__declspec(dllexport) void (*LOVECallback_DrawStretchyFixedPatch)
	(patch_t *patch, fixed_t x, fixed_t y, fixed_t pscale, fixed_t vscale, INT32 scrn, const UINT8 *colormap) = NULL;
__declspec(dllexport) void (*LOVECallback_RenderPlayerView)
	(player_t *player) = NULL;

__declspec(dllexport) void (*LOVECallback_SetMouseGrab)
	(boolean grab) = NULL;

void LOVE_OnRender(void)
{
	if (LOVECallback_OnRender)
		LOVECallback_OnRender();
	else
		I_OutputMsg("LOVE_OnRender(): callback not set\n");
}

void LOVE_DrawSoftwareBuffer(UINT8 *buffer, INT32 width, INT32 height)
{
	if (LOVECallback_DrawSoftwareBuffer)
		LOVECallback_DrawSoftwareBuffer(buffer, width, height);
	else
		I_OutputMsg("LOVE_DrawSoftwareBuffer(): callback not set\n");
}

void LOVE_DrawStretchyFixedPatch(patch_t *patch, fixed_t x, fixed_t y, fixed_t pscale, fixed_t vscale, INT32 scrn, const UINT8 *colormap)
{
	if (LOVECallback_DrawStretchyFixedPatch)
		LOVECallback_DrawStretchyFixedPatch(patch, x, y, pscale, vscale, scrn, colormap);
	else
		I_OutputMsg("LOVE_DrawStretchyFixedPatch(): callback not set\n");
}

void LOVE_RenderPlayerView(player_t *player)
{
	if (LOVECallback_RenderPlayerView)
		LOVECallback_RenderPlayerView(player);
}


void LOVE_SetMouseGrab(boolean grab)
{
	if (LOVECallback_SetMouseGrab)
		LOVECallback_SetMouseGrab(grab);
}
