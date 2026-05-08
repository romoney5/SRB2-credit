// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2014-2016 by John "JTE" Muniz.
// Copyright (C) 2014-2024 by Sonic Team Junior.
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_custombuild.c
/// \brief Currently only used for lua scripts to detect this build specifically

#include "blua/lua.h"
#include "doomdef.h"
#include "fastcmp.h"
#include "dehacked.h"
#include "deh_lua.h"
#include "lua_script.h"
#include "lua_libs.h"
#include "lua_custombuild.h"

boolean gks_complexlocaladdons = false;
boolean gks_luamenu = false;

INT32 GKS_PushGlobals(lua_State *L, const char *word)
{
    if (fastcmp(word,"gks_custombuild")) {
		lua_pushboolean(L, true);
		return 1;
    } else if (fastcmp(word, "gks_complexlocaladdons")) {
		lua_pushboolean(L, gks_complexlocaladdons);
		return 1;
	} else if (fastcmp(word, "gks_locallyloading")) {
        if (lua_locallyloading)
		    lua_pushboolean(L, true);
        else
            lua_pushboolean(L, false);
		return 1;
    } else if (fastcmp(word, "gks_lumpname")) {
		if (!lua_lumploading)
			lua_pushnil(L);
		else if (lua_lumpname[0])
			lua_pushstring(L, lua_lumpname);
		return 1;
    } else if (fastcmp(word, "gks_luamenu_supported")) {
		lua_pushboolean(L, true);
		return 1;
	} else if (fastcmp(word, "servernode")) {
		lua_pushinteger(L, servernode);
		return 1;
	} else if (fastcmp(word, "gks_luamenu")) {
		lua_pushboolean(L, gks_luamenu);
		return 1;
	}
    return 0;
}

INT32 GKS_CheckGlobals(lua_State *L, const char *word)
{
	if (fastcmp(word, "menuactive"))
		menuactive = luaL_checkboolean(L, 2);
	else if (fastcmp(word, "servernode"))
		servernode = (SINT8)luaL_checkinteger(L, 2);
	else if (fastcmp(word, "gks_luamenu"))
		gks_luamenu = luaL_checkboolean(L, 2);
	else
		return 0;

	return 1;
}
