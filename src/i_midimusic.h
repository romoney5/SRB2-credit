//
// Copyright(C) 2021-2022 Roman Fomin
// Copyright(C) 2022 ceski
// Copyright(C) 2024 Sonic Team Junior
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//

#include "doomtype.h"

// TODO: Add more functions here when they're needed by mixer_sound.c

// Initializes MIDI devices
boolean I_MID_InitMusic(int device);

// Plays a MIDI song
void I_MID_PlaySong(void *handle, boolean looping);

// Sets MIDI volume
void I_MID_SetMusicVolume(int volume);

// Stops a MIDI song
void I_MID_StopSong(void *handle);

// Registers a MIDI song
void *I_MID_RegisterSong(void *data, int len);

// Unregisters a MIDI song
void I_MID_UnRegisterSong(void *handle);
