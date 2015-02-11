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

/// @file game/ModuleProfile.cpp
/// @author Johan Jansen

#include "game/profiles/ModuleProfile.hpp"

#include "egolib/file_formats/quest_file.h"

#include "egolib/log.h"

#include "egolib/vfs.h"
#include "egolib/strutil.h"
#include "egolib/fileutil.h"
#include "egolib/platform.h"

const uint8_t ModuleProfile::RESPAWN_ANYTIME;
static const size_t SUMMARYLINES = 8;

ModuleProfile::ModuleProfile() :
	_loaded(false),
	_name("*UNKNOWN*"),
    _rank(0),
    _reference(),
    _importAmount(1),
    _allowExport(false),
    _minPlayers(0),
    _maxPlayers(0),
    _respawnValid(false),
	_summary(),
    _moduleType(FILTER_SIDE_QUEST),
    _beaten(false),
	_icon(),
	_vfsPath(_name),
    _folderName(_name)
{
    //ctor
}

ModuleProfile::~ModuleProfile()
{
	oglx_texture_release(&_icon);
}

bool ModuleProfile::isModuleUnlocked() const
{
    // First check if we are in developers mode or that the right module has been beaten before
    if ( cfg.dev_mode )
    {
        return true;
    }

    if (moduleHasIDSZ(_reference.c_str(), _unlockQuest.id, 0, nullptr))
    {
        return true;
    }

//ZF> TODO: re-enable
/*
    if (base.importamount > 0)
    {
        // If that did not work, then check all selected players directories, but only if it isn't a starter module
        for(const std::shared_ptr<LoadPlayerElement> &player : _selectedPlayerList)
        {
            // find beaten quests or quests with proper level
            if(!player->hasQuest(base.unlockquest.id, base.unlockquest.level)) {
                return false;
            }
        }
    }
*/

    return true;
}

ModuleFilter ModuleProfile::getModuleType() const
{
    return _moduleType;
}


std::shared_ptr<ModuleProfile> ModuleProfile::loadFromFile(const std::string &folderPath)
{
    STRING buffer;

    // see if we can open menu.txt file (required)
    vfs_FILE * fileread = vfs_openRead((folderPath + "/gamedat/menu.txt").c_str());
    if (!fileread) return nullptr;

    //Allocate memory
    std::shared_ptr<ModuleProfile> result = std::make_shared<ModuleProfile>();

    // Read basic data
    vfs_get_next_name(fileread, buffer, SDL_arraysize(buffer));
    result->_name = buffer;

    vfs_get_next_string(fileread, buffer, SDL_arraysize(buffer));
    result->_reference = buffer;

    result->_unlockQuest.id    = vfs_get_next_idsz( fileread );
    result->_unlockQuest.level = vfs_get_int( fileread );

    result->_importAmount = vfs_get_next_int( fileread );
    result->_allowExport  = vfs_get_next_bool( fileread );
    result->_minPlayers   = vfs_get_next_int( fileread );
    result->_maxPlayers   = vfs_get_next_int( fileread );

    switch(vfs_get_next_char(fileread))
    {
        case 'T':
            result->_respawnValid = true;
        break;

        case 'A':
            result->_respawnValid = RESPAWN_ANYTIME;
        break;

        default:
            result->_respawnValid = false;
        break;
    }

    //Skip char
    vfs_get_next_char(fileread);

    vfs_get_next_string(fileread, buffer, SDL_arraysize(buffer));
    str_trim(buffer);
    result->_rank = strlen(buffer);

    // convert the special ranks of "unranked" or "-" ("rank 0")
    if ( '-' == buffer[0] || 'U' == char_toupper(buffer[0]) )
    {
        result->_rank = 0;
    }

    // Read the summary
    for (size_t cnt = 0; cnt < SUMMARYLINES; cnt++)
    {
        // load the string
        vfs_get_next_string( fileread,  buffer, SDL_arraysize(buffer) );

        // remove the '_' characters
        str_decode(buffer, SDL_arraysize(buffer),  buffer);

        result->_summary.push_back(buffer);
    }

    // Assume default module type as a sidequest
    result->_moduleType = FILTER_SIDE_QUEST;

    // Read expansions
    while (goto_colon_vfs(nullptr, fileread, true))
    {
        IDSZ idsz = vfs_get_idsz(fileread);

        // Read module type
        if ( idsz == MAKE_IDSZ( 'T', 'Y', 'P', 'E' ) )
        {
            // parse the expansion value
            switch(char_toupper(vfs_get_first_letter(fileread)))
            {
                case 'M': result->_moduleType = FILTER_MAIN; break;
                case 'S': result->_moduleType = FILTER_SIDE_QUEST; break;
                case 'T': result->_moduleType = FILTER_TOWN; break;
                case 'F': result->_moduleType = FILTER_FUN; break;
                //case 'S': result->_moduleType = FILTER_STARTER; break;
            }
        }
        else if ( idsz == MAKE_IDSZ( 'B', 'E', 'A', 'T' ) )
        {
            result->_beaten = true;
        }
    }

    //Done!
    result->_loaded = true;
    vfs_close(fileread);

    // save the module path
    result->_vfsPath = folderPath;

    /// @note just because we can't load the title image DOES NOT mean that we ignore the module
    // load title image
    ego_texture_load_vfs(&result->_icon, (folderPath + "/gamedat/title").c_str(), INVALID_KEY );

    /// @note This is kinda a cheat since we know that the virtual paths all begin with "mp_" at the moment.
    // If that changes, this line must be changed as well.
    result->_folderName = folderPath.substr(11);

    return result;
}

bool ModuleProfile::moduleHasIDSZ(const char *szModName, IDSZ idsz, size_t buffer_len, char * buffer)
{
    /// @author ZZ
    /// @details This function returns true if the named module has the required IDSZ

    vfs_FILE *fileread;
    STRING newloadname;
    Uint32 newidsz;
    bool foundidsz;
    int cnt;

    if ( idsz == IDSZ_NONE ) return true;

    if ( 0 == strcmp( szModName, "NONE" ) ) return false;

    snprintf( newloadname, SDL_arraysize( newloadname ), "mp_modules/%s/gamedat/menu.txt", szModName );

    fileread = vfs_openRead( newloadname );
    if ( NULL == fileread ) return false;

    // Read basic data
    goto_colon_vfs( NULL, fileread, false );  // Name of module...  Doesn't matter
    goto_colon_vfs( NULL, fileread, false );  // Reference directory...
    goto_colon_vfs( NULL, fileread, false );  // Reference IDSZ...
    goto_colon_vfs( NULL, fileread, false );  // Import...
    goto_colon_vfs( NULL, fileread, false );  // Export...
    goto_colon_vfs( NULL, fileread, false );  // Min players...
    goto_colon_vfs( NULL, fileread, false );  // Max players...
    goto_colon_vfs( NULL, fileread, false );  // Respawn...
    goto_colon_vfs( NULL, fileread, false );  // BAD! NOT USED
    goto_colon_vfs( NULL, fileread, false );  // Rank...

    // Summary...
    for ( cnt = 0; cnt < SUMMARYLINES; cnt++ )
    {
        goto_colon_vfs( NULL, fileread, false );
    }

    // Now check expansions
    foundidsz = false;
    while ( goto_colon_vfs( NULL, fileread, true ) )
    {
        newidsz = vfs_get_idsz( fileread );
        if ( newidsz == idsz )
        {
            foundidsz = true;
            break;
        }
    }

    if ( NULL != buffer )
    {
        if ( buffer_len < 1 )
        {
            /* nothing */
        }
        else if ( 1 == buffer_len )
        {
            buffer[0] = CSTR_END;
        }
        else
        {
            vfs_gets( buffer, buffer_len, fileread );
        }
    }

    vfs_close( fileread );

    return foundidsz;
}

bool ModuleProfile::moduleAddIDSZ(const char *szModName, IDSZ idsz, size_t buffer_len, const char * buffer)
{
    /// @author ZZ
    /// @details This function appends an IDSZ to the module's menu.txt file

    vfs_FILE *filewrite;
    bool retval = false;

    // Only add if there isn't one already
    if ( !moduleHasIDSZ( szModName, idsz, 0, NULL ) )
    {
        STRING src_file, dst_file;

        // make sure that the file exists in the user data directory since we are WRITING to it
        snprintf( src_file, SDL_arraysize( src_file ), "mp_modules/%s/gamedat/menu.txt", szModName );
        snprintf( dst_file, SDL_arraysize( dst_file ), "/modules/%s/gamedat/menu.txt", szModName );
        vfs_copyFile( src_file, dst_file );

        // Try to open the file in append mode
        filewrite = vfs_openAppend( dst_file );
        if ( NULL != filewrite )
        {
            // output the expansion IDSZ
            vfs_printf( filewrite, "\n:[%s]", undo_idsz( idsz ) );

            // output an optional parameter
            if ( NULL != buffer && buffer_len > 1 )
            {
                vfs_printf( filewrite, " %s", undo_idsz( idsz ) );
            }

            // end the line
            vfs_printf( filewrite, "\n" );

            // success
            retval = true;

            // close the file
            vfs_close( filewrite );
        }
    }

    return retval;
}