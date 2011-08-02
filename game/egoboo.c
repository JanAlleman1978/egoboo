//********************************************************************************************
//*
//*    This file is part of Egoboo.
//*
//*    Egoboo is free software: you can redistribute it and/or modify it
//*    under the terms of the GNU General Public License as published by
//*    the Free Software Foundation, either version 3 of the License, or
//*    (at your option) any later version.
//*
//*    Egoboo is distributed in the hope that it will be useful, but
//*    WITHOUT ANY WARRANTY; without even the implied warranty of
//*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//*    General Public License for more details.
//*
//*    You should have received a copy of the GNU General Public License
//*    along with Egoboo.  If not, see <http://www.gnu.org/licenses/>.
//*
//********************************************************************************************

/// @file egoboo.c
/// @brief Code for the main program process
/// @details

#include <SDL.h>
#include <SDL_image.h>

#define DECLARE_GLOBALS

#include "egoboo.h"

#include <egolib/log.h>
#include <egolib/system.h>
#include <egolib/font_bmp.h>
#include <egolib/fileutil.h>
#include <egolib/egoboo_setup.h>
#include <egolib/vfs.h>
#include <egolib/console.h>
#include <egolib/strutil.h>

#include <egolib/file_formats/scancode_file.h>
#include <egolib/file_formats/controls_file.h>
#include <egolib/file_formats/treasure_table_file.h>
#include <egolib/extensions/SDL_extensions.h>
#include <egolib/clock.h>

#include "graphic.h"
#include "network.h"
#include "sound.h"
#include "ui.h"
#include "input.h"
#include "game.h"
#include "menu.h"
#include "player.h"
#include "graphic_texture.h"

#include "char.inl"
#include "particle.inl"
#include "enchant.inl"
#include "collision.h"
#include "profile.inl"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

static int do_ego_proc_begin( ego_process_t * eproc );
static int do_ego_proc_running( ego_process_t * eproc );
static int do_ego_proc_leaving( ego_process_t * eproc );
static int do_ego_proc_run( ego_process_t * eproc, double frameDuration );

static void memory_cleanUp( void );
static int  ego_init_SDL( void );

static void object_systems_begin( void );
static void object_systems_end( void );

static void _quit_game( ego_process_t * pgame );

static ego_process_t * ego_process_init( ego_process_t * eproc, int argc, char **argv );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

#if defined(__cplusplus)
extern "C"
{
#endif
    extern bool_t config_download( egoboo_config_t * pcfg );
    extern bool_t config_upload( egoboo_config_t * pcfg );
#if defined(__cplusplus)
}
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
static ClockState_t    * _gclock = NULL;
static ego_process_t     _eproc;

static bool_t  screenshot_keyready  = btrue;

static bool_t _sdl_atexit_registered    = bfalse;
static bool_t _sdl_initialized_base     = bfalse;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
ego_process_t     * EProc   = &_eproc;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int do_ego_proc_begin( ego_process_t * eproc )
{
    // initialize the virtual filesystem first
    vfs_init( NULL );
    setup_init_base_vfs_paths();

    // Initialize logging next, so that we can use it everywhere.
    log_init( vfs_resolveWriteFilename( "/debug/log.txt" ) );
    log_setLoggingLevel( 3 );

    // start initializing the various subsystems
    log_message( "Starting Egoboo " VERSION " ...\n" );
    log_info( "PhysFS file system version %s has been initialized...\n", vfs_getVersion() );

    sys_initialize();
    clk_init();
    _gclock = clk_create( "global clock", -1 );

    // read the "setup.txt" file
    setup_read_vfs();

    // download the "setup.txt" values into the cfg struct
    config_download( &cfg );

    // do basic system initialization
    ego_init_SDL();
    gfx_system_begin();

    // synchronize the config values with the various game subsystems
    // do this after the ego_init_SDL() and ogl_init() in case the config values are clamped
    // to valid values
    config_download( &cfg );

    log_info( "Initializing SDL_Image version %d.%d.%d... ", SDL_IMAGE_MAJOR_VERSION, SDL_IMAGE_MINOR_VERSION, SDL_IMAGE_PATCHLEVEL );
    GLSetup_SupportedFormats();

    // read all the scantags
    scantag_read_all_vfs( "mp_data/scancode.txt" );

    // load input
    input_settings_load_vfs( "/controls.txt", -1 );

    //Ready the mouse input_cursor
    init_mouse_cursor();

    // initialize the console
    egolib_console_begin();

    // initialize network communication
    net_initialize();

    // initialize the sound system
    sound_system_initialize();
    sound_load_all_music_sounds_vfs();

    // initialize the random treasure system
    init_random_treasure_tables_vfs( "mp_data/randomtreasure.txt" );

    // make sure that a bunch of stuff gets initialized properly
    object_systems_begin();
    game_module_init( PMod );
    mesh_ctor( PMesh );
    init_all_graphics();
    profile_system_begin();

    // setup the menu system's gui
    ui_begin( vfs_resolveReadFilename( "mp_data/Bo_Chen.ttf" ), 24 );
    font_bmp_load_vfs( TxTexture_get_valid_ptr(( TX_REF )TX_FONT ), "mp_data/font_new_shadow", "mp_data/font.txt" );  // must be done after init_all_graphics()

    // clear out the import and remote directories
    vfs_empty_temp_directories();

    // register the memory_cleanUp function to automatically run whenever the program exits
    atexit( memory_cleanUp );

    // initialize the game process (not active)
    game_process_init( GProc );

    // initialize the menu process (active)
    menu_process_init( MProc );
    process_start( PROC_PBASE( MProc ) );

    // Initialize the process
    process_start( PROC_PBASE( eproc ) );

    return 1;
}

//--------------------------------------------------------------------------------------------
int do_ego_proc_running( ego_process_t * eproc )
{
    bool_t menu_valid, game_valid;

    if ( !process_validate( PROC_PBASE( eproc ) ) ) return -1;

    eproc->was_active  = eproc->base.valid;

    menu_valid = process_validate( PROC_PBASE( MProc ) );
    game_valid = process_validate( PROC_PBASE( GProc ) );
    if ( !menu_valid && !game_valid )
    {
        process_kill( PROC_PBASE( eproc ) );
        return 1;
    }

    if ( eproc->base.paused ) return 0;

    if ( process_running( PROC_PBASE( MProc ) ) )
    {
        // menu settings
        SDL_WM_GrabInput( SDL_GRAB_OFF );
    }
    else
    {
        // in-game settings
        SDL_ShowCursor( cfg.hide_mouse ? SDL_DISABLE : SDL_ENABLE );
        SDL_WM_GrabInput( cfg.grab_mouse ? SDL_GRAB_ON : SDL_GRAB_OFF );
    }

    // Clock updates each frame
    game_update_timers();
    clk_frameStep( _gclock );
    eproc->base.frameDuration = clk_getFrameDuration( _gclock );

    // read the input values
    input_read_all_devices();

    if ( pickedmodule_ready && !process_running( PROC_PBASE( MProc ) ) )
    {
        // a new module has been picked

        // reset the flag
        pickedmodule_ready = bfalse;

        // start the game process
        process_start( PROC_PBASE( GProc ) );
    }

    // Test the panic button
    if ( SDLKEYDOWN( SDLK_q ) && SDLKEYDOWN( SDLK_LCTRL ) )
    {
        // terminate the program
        process_kill( PROC_PBASE( eproc ) );
    }

    if ( cfg.dev_mode )
    {
        if ( !SDLKEYDOWN( SDLK_F10 ) )
        {
            single_frame_keyready = btrue;
        }
        else if ( single_frame_keyready && SDLKEYDOWN( SDLK_F10 ) )
        {
            if ( !single_frame_mode )
            {
                single_frame_mode = btrue;
            }

            // request one update and one frame
            single_frame_requested  = btrue;
            single_update_requested = btrue;
            single_frame_keyready   = bfalse;
        }

    }

    // Check for screenshots
    if ( !SDLKEYDOWN( SDLK_F11 ) )
    {
        screenshot_keyready = btrue;
    }
    else if ( screenshot_keyready && SDLKEYDOWN( SDLK_F11 ) )
    {
        screenshot_keyready = bfalse;
        screenshot_requested = btrue;
    }

    if ( cfg.dev_mode && SDLKEYDOWN( SDLK_F9 ) && NULL != PMod && PMod->active )
    {
        // super secret "I win" button
        //PMod->beat        = btrue;
        //PMod->exportvalid = btrue;

        CHR_BEGIN_LOOP_ACTIVE( cnt, pchr )
        {
            if ( !VALID_PLA( pchr->is_which_player ) )
            {
                kill_character( cnt, ( CHR_REF )511, bfalse );
            }
        }
        CHR_END_LOOP();
    }

    // handle an escape by passing it on to all active sub-processes
    if ( eproc->escape_requested )
    {
        eproc->escape_requested = bfalse;

        // use the escape key to get out of single frame mode
        single_frame_mode = bfalse;

        if ( process_running( PROC_PBASE( GProc ) ) )
        {
            GProc->escape_requested = btrue;
        }

        if ( process_running( PROC_PBASE( MProc ) ) )
        {
            MProc->escape_requested = btrue;
        }
    }

    // run the sub-processes
    game_process_run( GProc, eproc->base.frameDuration );
    menu_process_run( MProc, eproc->base.frameDuration );

    // toggle the free-running mode on the process timers
    if ( cfg.dev_mode )
    {
        bool_t free_running_keydown = SDLKEYDOWN( SDLK_f ) && SDLKEYDOWN( SDLK_LCTRL );
        if ( free_running_keydown )
        {
            eproc->free_running_latch_requested = btrue;
        }

        if ( !free_running_keydown && eproc->free_running_latch_requested )
        {
            eproc->free_running_latch = btrue;
            eproc->free_running_latch_requested = bfalse;
        }
    }

    if ( eproc->free_running_latch )
    {
        if ( NULL != MProc )
        {
            MProc->gui_timer.free_running = !MProc->gui_timer.free_running;
        }

        if ( NULL != GProc )
        {
            GProc->ups_timer.free_running = !GProc->ups_timer.free_running && !egonet_on();
            GProc->fps_timer.free_running = !GProc->fps_timer.free_running;
        }

        eproc->free_running_latch = bfalse;
    }

    // a heads up display that can be used to debug values that are used by both the menu and the game
    // do_game_hud();

    return 0;
}

//--------------------------------------------------------------------------------------------
int do_ego_proc_leaving( ego_process_t * eproc )
{
    if ( !process_validate( PROC_PBASE( eproc ) ) ) return -1;

    // make sure that the game is terminated
    if ( !GProc->base.terminated )
    {
        game_process_run( GProc, eproc->base.frameDuration );
    }

    // make sure that the menu is terminated
    if ( !MProc->base.terminated )
    {
        menu_process_run( MProc, eproc->base.frameDuration );
    }

    if ( GProc->base.terminated && MProc->base.terminated )
    {
        process_terminate( PROC_PBASE( eproc ) );
    }

    if ( eproc->base.terminated )
    {
        // hopefully this will only happen once
        object_systems_end();
        clk_destroy( &_gclock );
        egolib_console_end();
        ui_end();
        gfx_system_end();
        setup_clear_base_vfs_paths();
    }

    return eproc->base.terminated ? 0 : 1;
}

//--------------------------------------------------------------------------------------------
int do_ego_proc_run( ego_process_t * eproc, double frameDuration )
{
    int result = 0, proc_result = 0;

    if ( !process_validate( PROC_PBASE( eproc ) ) ) return -1;
    eproc->base.frameDuration = frameDuration;

    if ( !eproc->base.paused ) return 0;

    if ( eproc->base.killme )
    {
        eproc->base.state = proc_leaving;
    }

    switch ( eproc->base.state )
    {
        case proc_begin:
            proc_result = do_ego_proc_begin( eproc );

            if ( 1 == proc_result )
            {
                eproc->base.state = proc_entering;
            }
            break;

        case proc_entering:
            // proc_result = do_ego_proc_entering( eproc );

            eproc->base.state = proc_running;
            break;

        case proc_running:
            proc_result = do_ego_proc_running( eproc );

            if ( 1 == proc_result )
            {
                eproc->base.state = proc_leaving;
            }
            break;

        case proc_leaving:
            proc_result = do_ego_proc_leaving( eproc );

            if ( 1 == proc_result )
            {
                eproc->base.state  = proc_finish;
                eproc->base.killme = bfalse;
            }
            break;

        case proc_finish:
            process_terminate( PROC_PBASE( eproc ) );
            break;

        default:
            /* do nothing */
            break;
    }

    return result;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
int SDL_main( int argc, char **argv )
{
    /// @details ZZ@> This is where the program starts and all the high level stuff happens

    int result = 0;

    // initialize the process
    ego_process_init( EProc, argc, argv );

    // turn on all basic services
    do_ego_proc_begin( EProc );

#if defined(EGOBOO_THROTTLED)
    // update the game at the user-defined rate
    EProc->loop_timer.free_running = bfalse;
    MProc->gui_timer.free_running  = bfalse;
    GProc->ups_timer.free_running  = bfalse;
    GProc->fps_timer.free_running  = bfalse;
#else
    // make the game update as fast as possible
    EProc->loop_timer.free_running = btrue;
    MProc->gui_timer.free_running  = btrue;
    GProc->ups_timer.free_running  = btrue;
    GProc->fps_timer.free_running  = btrue;
#endif

    // run the processes
    request_clear_screen();
    while ( !EProc->base.killme && !EProc->base.terminated )
    {
        if ( !egolib_timer_throttle( &( EProc->loop_timer ), 100.0f ) )
        {
            // let the OS breathe. It may delay as long as 10ms
            SDL_Delay( 1 );
        }
        else
        {

            // clear the screen if needed
            do_clear_screen();

            do_ego_proc_running( EProc );

            // flip the graphics page if need be
            do_flip_pages();

            // let the OS breathe. It may delay as long as 10ms
            if ( !EProc->loop_timer.free_running && update_lag < 3 )
            {
                SDL_Delay( 1 );
            }
        }
    }

    // terminate the game and menu processes
    process_kill( PROC_PBASE( GProc ) );
    process_kill( PROC_PBASE( MProc ) );
    while ( !EProc->base.terminated )
    {
        result = do_ego_proc_leaving( EProc );
    }

    return result;
}

//--------------------------------------------------------------------------------------------
void memory_cleanUp( void )
{
    /// @details ZF@> This function releases all loaded things in memory and cleans up everything properly

    log_info( "memory_cleanUp() - Attempting to clean up loaded things in memory... " );

    // quit any existing game
    _quit_game( EProc );

    // synchronize the config values with the various game subsystems
    config_synch( &cfg );

    // quit the setup system, making sure that the setup file is written
    setup_write_vfs();
    setup_end();

    // delete all the graphics allocated by SDL and OpenGL
    delete_all_graphics();

    // make sure that the current control configuration is written
    input_settings_save_vfs( "controls.txt", -1 );

    // shut down the ui
    ui_end();

    // shut down the network
    if ( egonet_on() )
    {
        net_shutDown();
    }

    // shut down the clock services
    clk_destroy( &_gclock );
    clk_shutdown();

    // deallocate any dynamically allocated collision memory
    collision_system_end();

    // deallocate any dynamically allocated scripting memory
    scripting_system_end();

    // deallocate all dynamically allocated memory for characters, particles, enchants, and models
    object_systems_end();

    log_message( "Success!\n" );
    log_info( "Exiting Egoboo " VERSION " the good way...\n" );

    // shut down the log services
    log_shutdown();
}

//--------------------------------------------------------------------------------------------
int ego_init_SDL()
{
    ego_init_SDL_base();
    input_system_init();

    return _sdl_initialized_base;
}

//--------------------------------------------------------------------------------------------
void ego_init_SDL_base()
{
    if ( _sdl_initialized_base ) return;

    log_info( "Initializing SDL version %d.%d.%d... ", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL );
    if ( SDL_Init( 0 ) < 0 )
    {
        log_message( "Failure!\n" );
        log_error( "Unable to initialize SDL: %s\n", SDL_GetError() );
    }
    else
    {
        log_message( "Success!\n" );
    }

    if ( !_sdl_atexit_registered )
    {
        atexit( SDL_Quit );
        _sdl_atexit_registered = bfalse;
    }

    log_info( "Intializing SDL Timing Services... " );
    if ( SDL_InitSubSystem( SDL_INIT_TIMER ) < 0 )
    {
        log_message( "Failed!\n" );
        log_warning( "SDL error == \"%s\"\n", SDL_GetError() );
    }
    else
    {
        log_message( "Success!\n" );
    }

    log_info( "Intializing SDL Event Threading... " );
    if ( SDL_InitSubSystem( SDL_INIT_EVENTTHREAD ) < 0 )
    {
        log_message( "Failed!\n" );
        log_warning( "SDL error == \"%s\"\n", SDL_GetError() );
    }
    else
    {
        log_message( "Success!\n" );
    }

    _sdl_initialized_base = btrue;
}

//--------------------------------------------------------------------------------------------
void object_systems_begin( void )
{
    /// @details BB@> initialize all the object systems

    particle_system_begin();
    enchant_system_begin();
    character_system_begin();
    model_system_begin();
}

//--------------------------------------------------------------------------------------------
void object_systems_end( void )
{
    /// @details BB@> quit all the object systems

    particle_system_end();
    enchant_system_end();
    character_system_end();
    model_system_end();
}

//--------------------------------------------------------------------------------------------
void _quit_game( ego_process_t * pgame )
{
    /// @details ZZ@> This function exits the game entirely

    if ( process_running( PROC_PBASE( pgame ) ) )
    {
        game_quit_module();
    }

    // tell the game to kill itself
    process_kill( PROC_PBASE( pgame ) );

    // clear out the import and remote directories
    vfs_empty_temp_directories();
}

//--------------------------------------------------------------------------------------------
ego_process_t * ego_process_init( ego_process_t * eproc, int argc, char **argv )
{
    if ( NULL == eproc ) return NULL;

    BLANK_STRUCT_PTR( eproc )

    process_init( PROC_PBASE( eproc ) );

    eproc->argv0 = ( argc > 0 ) ? argv[0] : NULL;

    return eproc;
}

//--------------------------------------------------------------------------------------------
Uint32 egoboo_get_ticks( void )
{
    Uint32 ticks = 0;

    if ( single_frame_mode )
    {
        ticks = UPDATE_SKIP * update_wld;
    }
    else
    {
        ticks = SDL_GetTicks();
    }

    return ticks;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
bool_t config_download( egoboo_config_t * pcfg )
{
    int tmp_maxparticles;
    bool_t rv;

    rv = setup_download( pcfg );
    if ( !rv ) return bfalse;

    // status display
    StatusList.on = pcfg->show_stats;

    // fps display
    fpson = pcfg->fps_allowed;

    // message display
    maxmessage    = CLIP( pcfg->message_count_req, 1, MAX_MESSAGE );
    messageon     = pcfg->message_count_req > 0;
    wraptolerance = pcfg->show_stats ? 90 : 32;

    // Get the particle limit
    // if the particle limit has changed, make sure to make not of it
    // number of particles
    tmp_maxparticles = CLIP( pcfg->particle_count_req, 0, MAX_PRT );
    if ( maxparticles != tmp_maxparticles )
    {
        maxparticles = tmp_maxparticles;
        maxparticles_dirty = btrue;
    }

    // camera options
    cam_options.turn_mode = pcfg->autoturncamera;

    // sound options
    sound_system_download_from_config( &snd, pcfg );

    // renderer options
    gfx_download_from_config( &gfx, pcfg );

    // texture options
    oglx_texture_parameters_download_gfx( &tex_params, pcfg );

    return btrue;
}

//--------------------------------------------------------------------------------------------
bool_t config_upload( egoboo_config_t * pcfg )
{
    if ( NULL == pcfg ) return bfalse;

    pcfg->autoturncamera = cam_options.turn_mode;
    pcfg->fps_allowed    = fpson;

    // number of particles
    pcfg->particle_count_req = CLIP( maxparticles, 0, MAX_PRT );

    // messages
    pcfg->messageon_req     = messageon;
    pcfg->message_count_req = !messageon ? 0 : MAX( 1, MAX_MESSAGE );

    // convert the config values to a setup file
    return setup_upload( pcfg );
}
