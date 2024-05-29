//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
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
// DESCRIPTION:  none
//

#include <stdio.h>
#include <stdlib.h>

#include "doom_config.h"
#include "doomtype.h"

//#include "gusconf.h"
#include "i_sound.h"
#include "i_video.h"
#include "m_argv.h"
#include "m_config.h"

// Sound sample rate to use for digital output (Hz)

int snd_samplerate = 44100;

// Maximum number of bytes to dedicate to allocated sound effects.
// (Default: 64MB)

int snd_cachesize = 64 * 1024 * 1024;

// Config variable that controls the sound buffer size.
// We default to 28ms (1000 / 35fps = 1 buffer per tic).

int snd_maxslicetime_ms = 28;

// External command to invoke to play back music.

char *snd_musiccmd = "";

// Whether to vary the pitch of sound effects
// Each game will set the default differently

int snd_pitchshift = -1;

// Low-level sound and music modules we are using

static sound_module_t *sound_module;
static music_module_t *music_module;

int snd_musicdevice = SNDDEVICE_SB;
int snd_sfxdevice = SNDDEVICE_SB;

// Sound modules
/* NRFD-TODO?
extern void I_InitTimidityConfig(void);
extern sound_module_t sound_sdl_module;
extern sound_module_t sound_pcsound_module;
extern music_module_t music_sdl_module;
extern music_module_t music_opl_module;

// For OPL module:

extern opl_driver_ver_t opl_drv_ver;
extern int opl_io_port;

// For native music module:

extern char *music_pack_path;
extern char *timidity_cfg_path;
*/


extern sound_module_t sound_i2s_module;

// DOS-specific options: These are unused but should be maintained
// so that the config file can be shared between chocolate
// doom and doom.exe

static int snd_sbport = 0;
static int snd_sbirq = 0;
static int snd_sbdma = 0;
static int snd_mport = 0;

// Compiled-in sound modules:

static sound_module_t *sound_modules[] =
{
    // NRFD-TODO?
    &sound_i2s_module,
    // &sound_sdl_module,
    //&sound_pcsound_module,
    NULL,
};

// Compiled-in music modules:

static music_module_t *music_modules[] =
{
    // NRFD-TODO?
    //&music_sdl_module,
    //&music_opl_module,
    NULL,
};

// Check if a sound device is in the given list of devices

static boolean SndDeviceInList(snddevice_t device, snddevice_t *list,
                               int len)
{
    /* X-HEEP COMMENT
    int i;

    for (i=0; i<len; ++i)
    {
        if (device == list[i])
        {
            return true;
        }
    }
X-HEEP COMMENT END */
    return false;
}

// Find and initialize a sound_module_t appropriate for the setting
// in snd_sfxdevice.

static void InitSfxModule(boolean use_sfx_prefix)
{
    return;
    /* X-HEEP COMMENT
    int i;

    sound_module = NULL;

    /* NRFD-Exclude
    for (i=0; sound_modules[i] != NULL; ++i)
    {
        // Is the sfx device in the list of devices supported by
        // this module?

        if (SndDeviceInList(snd_sfxdevice, 
                            sound_modules[i]->sound_devices,
                            sound_modules[i]->num_sound_devices))
        {
            // Initialize the module
    *//*X-HEEP COMMENT
        if (1) { // NRFD-TODO: Setting?
            i = 0;
            if (sound_modules[i]->Init(use_sfx_prefix))
            {
                sound_module = sound_modules[i];
                return;
            }
        }
    /*
        }
    }
    */
}

// Initialize music according to snd_musicdevice.

static void InitMusicModule(void)
{
    return;
    /* X-HEEP COMMENT
    int i;

    music_module = NULL;

    for (i=0; music_modules[i] != NULL; ++i)
    {
        // Is the music device in the list of devices supported
        // by this module?

        if (SndDeviceInList(snd_musicdevice, 
                            music_modules[i]->sound_devices,
                            music_modules[i]->num_sound_devices))
        {
            // Initialize the module

            if (music_modules[i]->Init())
            {
                music_module = music_modules[i];
                return;
            }
        }
    } X-HEEP COMMENT END */
}

//
// Initializes sound stuff, including volume
// Sets channels, SFX and music volume,
//  allocates channel buffer, sets S_sfx lookup.
//

void I_InitSound(boolean use_sfx_prefix)
{  
    /* X-HEEP COMMENT
    boolean nosound, nosfx, nomusic;

    //!
    // @vanilla
    //
    // Disable all sound output.
    //

    nosound = false; //M_CheckParm("-nosound") > 0;

    //!
    // @vanilla
    //
    // Disable sound effects. 
    //

    nosfx = false; //M_CheckParm("-nosfx") > 0;

    //!
    // @vanilla
    //
    // Disable music.
    //

    // NRFD-TODO: Music
    nomusic = true; //M_CheckParm("-nomusic") > 0;

    // Initialize the sound and music subsystems.

    // NRD-Exclude:
    // if (!nosound && !screensaver_mode)
    // {
    //     // This is kind of a hack. If native MIDI is enabled, set up
    //     // the TIMIDITY_CFG environment variable here before SDL_mixer
    //     // is opened.

    //     if (!nomusic
    //      && (snd_musicdevice == SNDDEVICE_GENMIDI
    //       || snd_musicdevice == SNDDEVICE_GUS))
    //     {
    //         I_InitTimidityConfig();
    //     }

        if (!nosfx)
        {
            InitSfxModule(use_sfx_prefix);
        }

        if (!nomusic)
        {
            InitMusicModule();
        }
    // }
    X-HEEP COMMENT END */

}

void I_ShutdownSound(void)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL)
    {
        sound_module->Shutdown();
    }

    if (music_module != NULL)
    {
        music_module->Shutdown();
    }
    X-HEEP COMMENT END */

}

int I_GetSfxLumpNum(sfxinfo_t *sfxinfo)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL) 
    {
        return sound_module->GetSfxLumpNum(sfxinfo);
    }
    else
    {
        return 0;
    }
    X-HEEP COMMENT END */
    return 0;
}

void I_UpdateSound(void)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL)
    {
        sound_module->Update();
    }

    if (music_module != NULL && music_module->Poll != NULL)
    {
        music_module->Poll();
    }
    X-HEEP COMMENT END */
}

static void CheckVolumeSeparation(int *vol, int *sep)
{
    /* X-HEEP COMMENT
    if (*sep < 0)
    {
        *sep = 0;
    }
    else if (*sep > 254)
    {
        *sep = 254;
    }

    if (*vol < 0)
    {
        *vol = 0;
    }
    else if (*vol > 127)
    {
        *vol = 127;
    }
    X-HEEP COMMENT END */
}

void I_UpdateSoundParams(int channel, int vol, int sep)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL)
    {
        CheckVolumeSeparation(&vol, &sep);
        sound_module->UpdateSoundParams(channel, vol, sep);
    }
    X-HEEP COMMENT END */
}

int I_StartSound(sfxinfo_t *sfxinfo, int channel, int vol, int sep, int pitch)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL)
    {
        CheckVolumeSeparation(&vol, &sep);
        return sound_module->StartSound(sfxinfo, channel, vol, sep, pitch);
    }
    else
    {
        return 0;
    }
    X-HEEP COMMENT END */
    return 0;
}

void I_StopSound(int channel)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL)
    {
        sound_module->StopSound(channel);
    }
    X-HEEP COMMENT END */
}

boolean I_SoundIsPlaying(int channel)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL)
    {
        return sound_module->SoundIsPlaying(channel);
    }
    else
    {
        return false;
    }
    X-HEEP COMMENT END */
    return false;
}

void I_PrecacheSounds(sfxinfo_t *sounds, int num_sounds)
{
    /* X-HEEP COMMENT
    if (sound_module != NULL && sound_module->CacheSounds != NULL)
    {
        sound_module->CacheSounds(sounds, num_sounds);
    }
    X-HEEP COMMENT END */
}

void I_InitMusic(void)
{
}

void I_ShutdownMusic(void)
{

}

void I_SetMusicVolume(int volume)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        music_module->SetMusicVolume(volume);
    }
    X-HEEP COMMENT END */
}

void I_PauseSong(void)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        music_module->PauseMusic();
    }
    X-HEEP COMMENT END */
}

void I_ResumeSong(void)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        music_module->ResumeMusic();
    }
    X-HEEP COMMENT END */
}

void *I_RegisterSong(void *data, int len)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        return music_module->RegisterSong(data, len);
    }
    else
    {
        return NULL;
    }
    X-HEEP COMMENT END */
}

void I_UnRegisterSong(void *handle)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        music_module->UnRegisterSong(handle);
    }
    X-HEEP COMMENT END */
}

void I_PlaySong(void *handle, boolean looping)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        music_module->PlaySong(handle, looping);
    }
    X-HEEP COMMENT END */
}

void I_StopSong(void)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        music_module->StopSong();
    }
    X-HEEP COMMENT END */
}

boolean I_MusicIsPlaying(void)
{
    /* X-HEEP COMMENT
    if (music_module != NULL)
    {
        return music_module->MusicIsPlaying();
    }
    else
    {
        return false;
    }
    X-HEEP COMMENT END */
    return false;
}

void I_BindSoundVariables(void)
{
    // NRFD-Exclude
}

