// SONIC ROBO BLAST 2
//-----------------------------------------------------------------------------
// Copyright (C) 2026-2026 by romoney5
//
// This program is free software distributed under the
// terms of the GNU General Public License, version 2.
// See the 'LICENSE' file for more details.
//-----------------------------------------------------------------------------
/// \file  lua_servlib.c
/// \brief something

#include "doomdef.h"
#include "fastcmp.h"
#include "r_skins.h"
#include "sounds.h"
#include "netcode/client_connection.h"
#include "netcode/i_net.h" // ligma
#include "netcode/d_netfil.h" // stigma
#include "i_time.h" // weiga

#include "lua_script.h"
#include "lua_libs.h"

enum serverelem {
	serverelem_valid = 0,
	serverelem_node,
	serverelem_info
};

static const char* const serverelem_opt[] = {
	"valid",
	"node",
	"info",
	NULL };

static int serverelem_fields_ref = LUA_NOREF;

static int serverelem_get(lua_State* L)
{
	serverelem_t* serv = *((serverelem_t**)luaL_checkudata(L, 1, META_SERVERELEM));
	enum serverelem field = Lua_optoption(L, 2, -1, serverelem_fields_ref);

	if (!serv)
	{
		if (field == serverelem_valid)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		return LUA_ErrInvalid(L, "serverelem_t");
	}

	switch (field)
	{
	case serverelem_valid:
		lua_pushboolean(L, true);
		break;
	case serverelem_node:
		lua_pushinteger(L, serv->node);
		break;
	case serverelem_info:
		LUA_PushUserdata(L, &serv->info, META_SERVERINFO_PAK);
		break;
	}

	return 1;
}

static int serverelem_set(lua_State* L)
{
	return luaL_error(L, LUA_QL("serverelem_t") " struct cannot be edited by Lua.");
}


enum serverinfo_pak {
	serverinfo_pak_valid = 0,
	serverinfo_pak__255,
	serverinfo_pak_packetversion,
	serverinfo_pak_application,
	serverinfo_pak_version,
	serverinfo_pak_subversion,
	serverinfo_pak_numberofplayer,
	serverinfo_pak_maxplayer,
	serverinfo_pak_refusereason,
	serverinfo_pak_gametypename,
	serverinfo_pak_modifiedgame,
	serverinfo_pak_cheatsenabled,
	serverinfo_pak_flags,
	serverinfo_pak_fileneedednum,
	serverinfo_pak_time,
	serverinfo_pak_leveltime, // romoney5: why is this even here?
	serverinfo_pak_servername,
	serverinfo_pak_mapname, // may change with future release
	serverinfo_pak_maptitle,
	serverinfo_pak_mapmd5,
	serverinfo_pak_actnum,
	serverinfo_pak_iszone,
	serverinfo_pak_httpsource,
	serverinfo_pak_fileneeded,
};

static const char* const serverinfo_pak_opt[] = {
	"valid",
	"_255",
	"packetversion",
	"application",
	"version",
	"subversion",
	"numberofplayer",
	"maxplayer",
	"refusereason",
	"gametypename",
	"modifiedgame",
	"cheatsenabled",
	"flags",
	"fileneedednum",
	"time",
	"leveltime",
	"servername",
	"mapname",
	"maptitle",
	"mapmd5",
	"actnum",
	"iszone",
	"httpsource",
	"fileneeded",
	NULL };

static int serverinfo_pak_fields_ref = LUA_NOREF;

static int serverinfo_pak_get(lua_State* L)
{
	serverinfo_pak* serv = *((serverinfo_pak**)luaL_checkudata(L, 1, META_SERVERINFO_PAK));
	enum serverinfo_pak field = Lua_optoption(L, 2, -1, serverinfo_pak_fields_ref);

	if (!serv)
	{
		if (field == serverinfo_pak_valid)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		return LUA_ErrInvalid(L, "serverinfo_pak");
	}

	switch (field)
	{
	case serverinfo_pak_valid:
		lua_pushboolean(L, true);
		break;
	case serverinfo_pak__255:
		lua_pushinteger(L, serv->_255);
		break;
	case serverinfo_pak_packetversion:
		lua_pushinteger(L, serv->packetversion);
		break;
	case serverinfo_pak_application:
		lua_pushstring(L, serv->application);
		break;
	case serverinfo_pak_version:
		lua_pushinteger(L, serv->version);
		break;
	case serverinfo_pak_subversion:
		lua_pushinteger(L, serv->subversion);
		break;
	case serverinfo_pak_numberofplayer:
		lua_pushinteger(L, serv->numberofplayer);
		break;
	case serverinfo_pak_maxplayer:
		lua_pushinteger(L, serv->maxplayer);
		break;
	case serverinfo_pak_refusereason:
		lua_pushinteger(L, serv->refusereason);
		break;
	case serverinfo_pak_gametypename:
		lua_pushstring(L, serv->gametypename);
		break;
	case serverinfo_pak_modifiedgame:
		lua_pushinteger(L, serv->modifiedgame);
		break;
	case serverinfo_pak_cheatsenabled:
		lua_pushinteger(L, serv->cheatsenabled);
		break;
	case serverinfo_pak_flags:
		lua_pushinteger(L, serv->flags);
		break;
	case serverinfo_pak_fileneedednum:
		lua_pushinteger(L, serv->fileneedednum);
		break;
	case serverinfo_pak_time:
		lua_pushinteger(L, serv->time);
		break;
	case serverinfo_pak_leveltime:
		lua_pushinteger(L, serv->leveltime);
		break;
	case serverinfo_pak_servername:
		lua_pushstring(L, serv->servername);
		break;
	case serverinfo_pak_mapname:
		lua_pushstring(L, serv->mapname);
		break;
	case serverinfo_pak_maptitle:
		lua_pushstring(L, serv->maptitle);
		break;
	case serverinfo_pak_mapmd5:
		// lua_pushstring(L, serv->mapmd5);
		lua_pushlightuserdata(L, serv->mapmd5);
		break;
	case serverinfo_pak_actnum:
		lua_pushinteger(L, serv->actnum);
		break;
	case serverinfo_pak_iszone:
		lua_pushinteger(L, serv->iszone);
		break;
	case serverinfo_pak_httpsource:
		lua_pushstring(L, serv->httpsource);
		break;
	case serverinfo_pak_fileneeded:
		// lazily referenced from http library
		// this might not be the right way to push lists
		// or arrays whatever they're called
		//lua_newtable(L); // lua_createtable?

		//for (size_t i = 0; i < MAXFILENEEDED; i++) {
		//	lua_pushinteger(L, serv->fileneeded[i]);
		//	lua_rawseti(L, -2, i); // + 1
		//}

		// mmMMI dunno..
		lua_pushlightuserdata(L, serv->fileneeded);
		break;
	}

	return 1;
}

static int serverinfo_pak_set(lua_State* L)
{
	return luaL_error(L, LUA_QL("serverinfo_pak") " struct cannot be edited by Lua.");
}


static int lib_getServerelem(lua_State* L)
{
	UINT32 i;

	// find server by number
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		i = luaL_checkinteger(L, 2);
		if (i >= 126)
			return luaL_error(L, "serverlist[] index %d out of range (0 - %d)", i, 126 - 1); // - 1?
		if (i >= serverlistcount)
			return 0;
		LUA_PushUserdata(L, &serverlist[i], META_SERVERELEM);
		return 1;
	}

	return 0;
}

static int lib_numServerlist(lua_State* L)
{
	lua_pushinteger(L, serverlistcount);
	return 1;
}


enum plrinfo_pak {
	plrinfo_pak_valid = 0,
	plrinfo_pak_num,
	plrinfo_pak_name,
	plrinfo_pak_address,
	plrinfo_pak_team,
	plrinfo_pak_skin,
	plrinfo_pak_data,
	plrinfo_pak_score,
	plrinfo_pak_timeinserver,
};

static const char* const plrinfo_pak_opt[] = {
	"valid",
	"num",
	"name",
	"address",
	"team",
	"skin",
	"data",
	"score",
	"timeinserver",
	NULL };

static int plrinfo_pak_fields_ref = LUA_NOREF;

static int plrinfo_pak_get(lua_State* L)
{
	plrinfo_pak* plr = *((plrinfo_pak**)luaL_checkudata(L, 1, META_PLRINFO_PAK));
	enum plrinfo_pak field = Lua_optoption(L, 2, -1, plrinfo_pak_fields_ref);

	if (!plr)
	{
		if (field == plrinfo_pak_valid)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		return LUA_ErrInvalid(L, "plrinfo_pak");
	}


	/*
	typedef struct
	{
		UINT8 num;
		char name[MAXPLAYERNAME+1];
		UINT8 address[4]; // sending another string would run us up against MAXPACKETLENGTH
		UINT8 team;
		UINT8 skin;
		UINT8 data; // Color is first four bits, hasflag, isit and issuper have one bit each, the last is unused.
		UINT32 score;
		UINT16 timeinserver; // In seconds.
	} ATTRPACK plrinfo_pak;
	*/

	switch (field)
	{
	case plrinfo_pak_valid:
		lua_pushboolean(L, true);
		break;
	case plrinfo_pak_name:
		lua_pushstring(L, plr->name);
		break;
	case plrinfo_pak_address:
		lua_pushlightuserdata(L, plr->address);
		break;
	case plrinfo_pak_team:
		lua_pushinteger(L, plr->team);
		break;
	case plrinfo_pak_skin:
		lua_pushinteger(L, plr->skin);
		break;
	case plrinfo_pak_data:
		lua_pushinteger(L, plr->data);
		break;
	case plrinfo_pak_score:
		lua_pushinteger(L, plr->score);
		break;
	case plrinfo_pak_timeinserver:
		lua_pushinteger(L, plr->timeinserver);
		break;
	}

	return 1;
}

static int plrinfo_pak_set(lua_State* L)
{
	return luaL_error(L, LUA_QL("serverelem_t") " struct cannot be edited by Lua.");
}

static int lib_getPlrinfo(lua_State* L)
{
	INT32 i;

	// find player by number
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		i = luaL_checkinteger(L, 2);
		if (i < 0 || i >= MAXPLAYERS)
			return luaL_error(L, "playerinfo[] index %d out of range (0 - %d)", i, MAXPLAYERS - 1);
		if (playerinfo[i].num >= 255)
			return 0;
		LUA_PushUserdata(L, &playerinfo[i], META_PLRINFO_PAK);
		return 1;
	}

	return 0;
}

static int lib_numPlayerinfo(lua_State* L)
{
	lua_pushinteger(L, MAXPLAYERS - 1);
	return 1;
}



/*enum doomdata {
	doomdata_valid = 0,
	doomdata_packettype,

	doomdata_clientpak,
	doomdata_client2pak,
	doomdata_serverpak,
	doomdata_servercfg,
	doomdata_textcmd,
	doomdata_filetxpak,
	doomdata_fileack,
	doomdata_filereceived,
	doomdata_clientcfg,
	doomdata_md5sum,
	doomdata_serverinfo,
	doomdata_serverrefuse,
	doomdata_askinfo,
	doomdata_msaskinfo,
	doomdata_playerinfo,
	doomdata_playerconfig,
	doomdata_filesneedednum,
	doomdata_filesneededcfg,
	doomdata_pingtable,
};

static const char* const doomdata_opt[] = {
	"valid",
	"packettype",

	"clientpak",
	"client2pak",
	"serverpak",
	"servercfg",
	"textcmd",
	"filetxpak",
	"fileack",
	"filereceived",
	"clientcfg",
	"md5sum",
	"serverinfo",
	"serverrefuse",
	"askinfo",
	"msaskinfo",
	"playerinfo",
	"playerconfig",
	"filesneedednum",
	"filesneededcfg",
	"pingtable",
	NULL };

static int doomdata_fields_ref = LUA_NOREF;

static int doomdata_get(lua_State* L)
{
	doomdata_t* dat = *((doomdata_t**)luaL_checkudata(L, 1, META_DOOMDATA));
	enum doomdata field = Lua_optoption(L, 2, -1, doomdata_fields_ref);

	if (!dat)
	{
		if (field == doomdata_valid)
		{
			lua_pushboolean(L, false);
			return 1;
		}
		return LUA_ErrInvalid(L, "doomdata_t");
	}


	/a*
	typedef struct
	{
		UINT32 checksum;
		UINT8 ack; // If not zero the node asks for acknowledgement, the receiver must resend the ack
		UINT8 ackreturn; // The return of the ack number

		UINT8 packettype;
		UINT8 reserved; // Padding
		union
		{
			clientcmd_pak clientpak;
			client2cmd_pak client2pak;
			servertics_pak serverpak;
			serverconfig_pak servercfg;
			UINT8 textcmd[MAXTEXTCMD+1];
			filetx_pak filetxpak;
			fileack_pak fileack;
			UINT8 filereceived;
			clientconfig_pak clientcfg;
			UINT8 md5sum[16];
			serverinfo_pak serverinfo;
			serverrefuse_pak serverrefuse;
			askinfo_pak askinfo;
			msaskinfo_pak msaskinfo;
			plrinfo_pak playerinfo[MAXPLAYERS];
			plrconfig_pak playerconfig[MAXPLAYERS];
			INT32 filesneedednum;
			filesneededconfig_pak filesneededcfg;
			UINT32 pingtable[MAXPLAYERS+1];
		} u; // This is needed to pack diff packet types data together
	} ATTRPACK doomdata_t;
	*a/

	switch (field)
	{
	case doomdata_valid:
		lua_pushboolean(L, true);
		break;
	case doomdata_packettype:
		lua_pushstring(L, dat->packettype);
		break;

	}

	return 1;
}

static int doomdata_set(lua_State* L)
{
	return luaL_error(L, LUA_QL("serverelem_t") " struct cannot be edited by Lua.");
}

static int lib_getNetbuffer(lua_State* L)
{
	INT32 i;

	// find player by number
	if (lua_type(L, 2) == LUA_TNUMBER)
	{
		i = luaL_checkinteger(L, 2);
		if (i < 0 || i >= MAXPLAYERS)
			return luaL_error(L, "playerinfo[] index %d out of range (0 - %d)", i, MAXPLAYERS - 1);
		if (playerinfo[i].num >= 255)
			return 0;
		LUA_PushUserdata(L, &playerinfo[i], META_PLRINFO_PAK);
		return 1;
	}

	return 0;
}
*/


// this is my file, i get to use whatever naming scheme i want
static int lib_ClearMultiplayer(lua_State* L)
{
	(void)L;
	netgame = multiplayer = false;
	return 0;
}

static int lib_SetMultiplayer(lua_State* L)
{
	(void)L;
	netgame = multiplayer = true;
	return 0;
}

static int lib_I_NetOpenSocket(lua_State* L)
{
	(void)L;
	I_NetOpenSocket(); // romoney5: surely netopensocket is always defined, nothing will ever go wrong!
	return 0; // .. ..right?
}

static int lib_I_NetMakeNodewPort(lua_State* L)
{
	//HUDSAFE
	const char* address = luaL_checkstring(L, 1);
	const char* port = luaL_checkstring(L, 2);
	SINT8 node = I_NetMakeNodewPort(address, port); // romoney5 TODO: expose "connect any"
	lua_pushinteger(L, node);

	return 1;
}

static int lib_D_ParseFileneeded(lua_State* L)
{
	//HUDSAFE
	INT32 fileneedednum_parm = luaL_checknumber(L, 1);
	UINT8 *fileneededstr = lua_touserdata(L, 2);
	UINT16 firstfile = luaL_checknumber(L, 3);

	if (fileneededstr == NULL || !lua_islightuserdata(L, 2))
		return luaL_error(L, "expected light userdata for fileneededstr");

	D_ParseFileneeded(fileneedednum_parm, fileneededstr, firstfile);

	return 0;
}

static int lib_Net_CloseConnection(lua_State* L)
{
	//HUDSAFE
	INT32 node = luaL_checkinteger(L, 1);

	Net_CloseConnection(node);

	return 0;
}

static int lib_SL_ClearServerList(lua_State* L)
{
	//HUDSAFE
	INT32 connectedserver = luaL_checkinteger(L, 1);

	// terrible but what wi
	for (UINT32 i = 0; i < serverlistcount; i++)
		if (connectedserver != serverlist[i].node)
		{
			Net_CloseConnection(serverlist[i].node | 0x8000/*FORCECLOSE*/);
			serverlist[i].node = 0;
		}
	serverlistcount = 0;

	return 0;
}

static int lib_SendAskInfo(lua_State* L)
{
	//HUDSAFE
	INT32 node = luaL_checkinteger(L, 1);

	// terrible but what wi
	// romoney5 TODO: make this recreateable in lua
	const tic_t asktime = I_GetTime();
	netbuffer->packettype = PT_ASKINFO;
	netbuffer->u.askinfo.version = VERSION;
	netbuffer->u.askinfo.time = (tic_t)LONG(asktime);

	// Even if this never arrives due to the host being firewalled, we've
	// now allowed traffic from the host to us in, so once the MS relays
	// our address to the host, it'll be able to speak to us.
	HSendPacket(node, false, 0, sizeof(askinfo_pak));

	return 0;
}

static luaL_Reg lib[] = {
	{"ClearMultiplayer", lib_ClearMultiplayer}, // romoney5 TODO: function for force setting cvars, not param
	{"SetMultiplayer", lib_SetMultiplayer},
	{"I_NetOpenSocket", lib_I_NetOpenSocket},
	{"I_NetMakeNodewPort", lib_I_NetMakeNodewPort},
	{"D_ParseFileneeded", lib_D_ParseFileneeded},
	{"Net_CloseConnection", lib_Net_CloseConnection},
	{"SL_ClearServerList", lib_SL_ClearServerList},
	{"SendAskInfo", lib_SendAskInfo},

	{NULL, NULL}
};


// mi primer userdata
int LUA_ServLib(lua_State* L)
{
	LUA_RegisterUserdataMetatable(L, META_SERVERELEM, serverelem_get, serverelem_set, NULL);
	LUA_RegisterUserdataMetatable(L, META_SERVERINFO_PAK, serverinfo_pak_get, serverinfo_pak_set, NULL);
	LUA_RegisterUserdataMetatable(L, META_PLRINFO_PAK, plrinfo_pak_get, plrinfo_pak_set, NULL);

	serverelem_fields_ref = Lua_CreateFieldTable(L, serverelem_opt);
	serverinfo_pak_fields_ref = Lua_CreateFieldTable(L, serverinfo_pak_opt);
	plrinfo_pak_fields_ref = Lua_CreateFieldTable(L, plrinfo_pak_opt);

	LUA_RegisterGlobalUserdata(L, "serverlist", lib_getServerelem, NULL, lib_numServerlist);

	LUA_RegisterGlobalUserdata(L, "playerinfo", lib_getPlrinfo, NULL, lib_numPlayerinfo);

	//LUA_RegisterGlobalUserdata(L, "netbuffer", lib_getNetbuffer, NULL, NULL);

	// Set global functions
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	luaL_register(L, NULL, lib);

	return 0;
}
