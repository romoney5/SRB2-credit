
void LOVE_OnRender(void);
void LOVE_DrawSoftwareBuffer(UINT8 *buffer, INT32 width, INT32 height);
void LOVE_DrawStretchyFixedPatch(patch_t *patch, fixed_t x, fixed_t y, fixed_t pscale, fixed_t vscale, INT32 scrn, const UINT8 *colormap);
void LOVE_RenderPlayerView(player_t *player);

void LOVE_SetMouseGrab(boolean grab);
