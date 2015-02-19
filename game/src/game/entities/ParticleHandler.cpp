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

/// @file  game/entities/ParticleHandler.cpp
/// @brief Handler of particle entities.
/// @details

#define GAME_ENTITIES_PRIVATE 1
#include "game/entities/ParticleHandler.hpp"
#include "game/egoboo_object.h"
#include "game/entities/Particle.hpp"

//--------------------------------------------------------------------------------------------
// testing macros
//--------------------------------------------------------------------------------------------

// macros without range checking
#define INGAME_PPRT_BASE_RAW(PPRT)      ( ACTIVE_PBASE( POBJ_GET_PBASE(PPRT) ) && ON_PBASE( POBJ_GET_PBASE(PPRT) ) )
#define DEFINED_PPRT_RAW( PPRT )        ( ALLOCATED_PBASE ( POBJ_GET_PBASE(PPRT) ) && !TERMINATED_PBASE ( POBJ_GET_PBASE(PPRT) ) )
#define ALLOCATED_PPRT_RAW( PPRT )      ALLOCATED_PBASE( POBJ_GET_PBASE(PPRT) )
#define ACTIVE_PPRT_RAW( PPRT )         ACTIVE_PBASE( POBJ_GET_PBASE(PPRT) )
#define WAITING_PPRT_RAW( PPRT )        WAITING_PBASE   ( POBJ_GET_PBASE(PPRT) )
#define TERMINATED_PPRT_RAW( PPRT )     TERMINATED_PBASE( POBJ_GET_PBASE(PPRT) )

//--------------------------------------------------------------------------------------------
//Inline
//--------------------------------------------------------------------------------------------

bool VALID_PRT_RANGE(const PRT_REF IPRT) { return PrtList.isValidRef(IPRT); }
bool DEFINED_PRT(const PRT_REF IPRT) { return (VALID_PRT_RANGE(IPRT) && DEFINED_PPRT_RAW(PrtList.get_ptr(IPRT))); }
bool ALLOCATED_PRT(const PRT_REF IPRT) { return (VALID_PRT_RANGE(IPRT) && ALLOCATED_PPRT_RAW(PrtList.get_ptr(IPRT))); }
bool ACTIVE_PRT(const PRT_REF IPRT) { return (VALID_PRT_RANGE(IPRT) && ACTIVE_PPRT_RAW(PrtList.get_ptr(IPRT))); }
bool WAITING_PRT(const PRT_REF IPRT) { return (VALID_PRT_RANGE(IPRT) && WAITING_PPRT_RAW(PrtList.get_ptr(IPRT))); }
bool TERMINATED_PRT(const PRT_REF IPRT) { return (VALID_PRT_RANGE(IPRT) && TERMINATED_PPRT_RAW(PrtList.get_ptr(IPRT))); }
PRT_REF GET_REF_PPRT(const prt_t *PPRT) { return LAMBDA(NULL == (PPRT), INVALID_PRT_REF, GET_INDEX_POBJ(PPRT, INVALID_PRT_REF)); }
bool  VALID_PRT_PTR(const prt_t *PPRT) { return ((NULL != (PPRT)) && VALID_PRT_RANGE(GET_REF_POBJ(PPRT, INVALID_PRT_REF))); }
bool  DEFINED_PPRT(const prt_t *PPRT) { return (VALID_PRT_PTR(PPRT) && DEFINED_PPRT_RAW(PPRT)); }
bool ALLOCATED_PPRT(const prt_t *PPRT) { return (VALID_PRT_PTR(PPRT) && ALLOCATED_PPRT_RAW(PPRT)); }
bool ACTIVE_PPRT(const prt_t *PPRT) { return (VALID_PRT_PTR(PPRT) && ACTIVE_PPRT_RAW(PPRT)); }
bool WAITING_PPRT(const prt_t *PPRT) { return (VALID_PRT_PTR(PPRT) && WAITING_PPRT_RAW(PPRT)); }
bool TERMINATED_PPRT(const prt_t *PPRT) { return (VALID_PRT_PTR(PPRT) && TERMINATED_PPRT_RAW(PPRT)); }
bool INGAME_PRT_BASE(const PRT_REF IPRT) { return (VALID_PRT_RANGE(IPRT) && INGAME_PPRT_BASE_RAW(PrtList.get_ptr(IPRT))); }
bool INGAME_PPRT_BASE(const prt_t *PPRT) { return (VALID_PRT_PTR(PPRT) && INGAME_PPRT_BASE_RAW(PPRT)); }
bool DISPLAY_PRT(const PRT_REF IPRT) { return INGAME_PRT_BASE(IPRT); }
bool DISPLAY_PPRT(const prt_t *PPRT) { return INGAME_PPRT_BASE(PPRT); }
bool INGAME_PRT(const PRT_REF IPRT) { return LAMBDA(Ego::Entities::spawnDepth > 0, DEFINED_PRT(IPRT), INGAME_PRT_BASE(IPRT) && (!PrtList.get_ptr(IPRT)->is_ghost)); }
bool INGAME_PPRT(const prt_t *PPRT) { return LAMBDA(Ego::Entities::spawnDepth > 0, INGAME_PPRT_BASE(PPRT), DISPLAY_PPRT(PPRT) && (!(PPRT)->is_ghost)); }

//--------------------------------------------------------------------------------------------

ParticleManager PrtList;

ParticleManager& ParticleManager::getSingleton()
{
    return PrtList;
}

//--------------------------------------------------------------------------------------------
void ParticleManager::prune_used_list()
{
    // prune the used list
    for (size_t i = 0; i < usedCount; ++i)
    {
        bool removed = false;

        PRT_REF ref = used_ref[i];

        if (!isValidRef(ref) || !DEFINED_PRT(ref))
        {
            removed = remove_used_idx(i);
        }

        if (removed && !lst[ref].obj_base.in_free_list)
        {
            add_free_ref(ref);
        }
    }
}

//--------------------------------------------------------------------------------------------
void ParticleManager::prune_free_list()
{
    for (size_t i = 0; i < freeCount; ++i)
    {
        bool removed = false;

        PRT_REF ref = free_ref[i];

        if (isValidRef(ref) && INGAME_PRT_BASE(ref))
        {
            removed = remove_free_idx(i);
        }

        if (removed && !lst[ref].obj_base.in_free_list)
        {
            push_used(ref);
        }
    }
}

//--------------------------------------------------------------------------------------------
void ParticleManager::update_used()
{
    prune_used_list();
    prune_free_list();

    // Go through the object list to see if there are any dangling objects.
    for (PRT_REF ref = 0; ref < getCount(); ++ref)
    {
        if (!isValidRef(ref)) continue; /// @todo Redundant.
        prt_t *x = get_ptr(ref);
        if (!ALLOCATED_PPRT_RAW(x)) continue;

        if (DISPLAY_PPRT(x))
        {
            if (!x->obj_base.in_used_list)
            {
                push_used(ref);
            }
        }
        else if (!DEFINED_PPRT(x))
        {
            if (!x->obj_base.in_free_list)
            {
                add_free_ref(ref);
            }
        }
    }

    // Blank out the unused elements of the used list.
    for (size_t i = getUsedCount(); i < getCount(); ++i)
    {
        used_ref[i] = INVALID_PRT_REF;
    }

    // Blank out the unused elements of the free list.
    for (size_t i = getFreeCount(); i < getCount(); ++i)
    {
        free_ref[i] = INVALID_PRT_REF;
    }
}

//--------------------------------------------------------------------------------------------
bool ParticleManager::free_one(const PRT_REF ref)
{
    /// @details Stick an object back into the free object list.

    // Ensure that we have a valid reference.
    if (!isValidRef(ref)) return false;
    prt_t *obj = get_ptr(ref);

    // If the object is not allocated (i.e. in the state range ["constructing","destructing"])
    // then its reference is in the free list.
    if (!ALLOCATED_PPRT(obj)) return false;

    Ego::Entity *parentObj = POBJ_GET_PBASE(obj);

    // If we are inside an iteration, do not actually change the length of the list.
    // This would invalidate all iterators.
    if (getLockCount() > 0)
    {
        return add_termination(ref);
    }
    else
    {
        // Ensure that the entity reaches the "destructing" state.
        // @todo This is redundant.
        obj = prt_t::config_deinitialize(obj, 100);
        if (!obj) return false;
        // Ensure that the entity reaches the "terminated" state.
        obj = prt_t::config_deconstruct(obj, 100);
        if (!obj) return false;

        if (parentObj->in_used_list)
        {
            remove_used_ref(ref);
        }

        if (parentObj->in_free_list)
        {
            return true;
        }
        else
        {
            return add_free_ref(ref);
        }
    }
}

//--------------------------------------------------------------------------------------------
PRT_REF ParticleManager::allocate(const bool force)
{
    PRT_REF iprt;

    // Return INVALID_PRT_REF if we can't find one.
    iprt = INVALID_PRT_REF;

    if (0 == freeCount)
    {
        if (force)
        {
            PRT_REF found        = INVALID_PRT_REF;
            size_t  min_life     = ( size_t )( ~0 );
            PRT_REF min_life_idx = INVALID_PRT_REF;
            size_t  min_anim     = ( size_t )( ~0 );
            PRT_REF min_anim_idx = INVALID_PRT_REF;

            // Gotta find one, so go through the list and replace a unimportant one.
            for (iprt = 0; iprt < getCount(); iprt++)
            {
                bool was_forced = false;
                prt_t *pprt;

                // Is this an invalid particle? The particle allocation count is messed up! :(
                if (!DEFINED_PRT(iprt))
                {
                    found = iprt;
                    break;
                }
                pprt =  get_ptr(iprt);

                // Does it have a valid profile?
                if (!LOADED_PIP(pprt->pip_ref))
                {
                    found = iprt;
                    end_one_particle_in_game(iprt);
                    break;
                }

                // Do not bump another.
                was_forced = TO_C_BOOL(PipStack.lst[pprt->pip_ref].force);

                if (WAITING_PRT(iprt))
                {
                    // If the particle has been "terminated" but is still waiting around,
                    // bump it to the front of the list.

                    size_t min_time = std::min( pprt->lifetime_remaining, pprt->frames_remaining );

                    if ( min_time < std::max( min_life, min_anim ) )
                    {
                        min_life     = pprt->lifetime_remaining;
                        min_life_idx = iprt;

                        min_anim     = pprt->frames_remaining;
                        min_anim_idx = iprt;
                    }
                }
                else if (!was_forced)
                {
                    // If the particle has not yet died, let choose the worst one.
                    if ( pprt->lifetime_remaining < min_life )
                    {
                        min_life     = pprt->lifetime_remaining;
                        min_life_idx = iprt;
                    }

                    if ( pprt->frames_remaining < min_anim )
                    {
                        min_anim     = pprt->frames_remaining;
                        min_anim_idx = iprt;
                    }
                }
            }

            if (isValidRef(found))
            {
                // Found a "bad" particle.
                iprt = found;
            }
            else if (isValidRef(min_anim_idx))
            {
                // Found a "terminated" particle.
                iprt = min_anim_idx;
            }
            else if (isValidRef(min_life_idx))
            {
                // Found a particle that closest to death.
                iprt = min_life_idx;
            }
            else
            {
                // Found nothing. This should only happen if all the particles are forced.
                iprt = INVALID_PRT_REF;
            }
        }
    }
    else
    {
        if ( freeCount > ( getCount() / 4 ) )
        {
            // Just grab the next one
            iprt = pop_free();
        }
        else if ( force )
        {
            iprt = pop_free();
        }
    }

    // return a proper value
    iprt = ( iprt >= getCount() ) ? INVALID_PRT_REF : iprt;

    if (isValidRef(iprt))
    {
        // If the particle is already being used, make sure to destroy the old one.
        if (DEFINED_PRT(iprt))
        {
            end_one_particle_in_game(iprt);
        }

        // allocate the new one
        get_ptr(iprt)->obj_base.allocate(REF_TO_INT(iprt));
    }

    if (ALLOCATED_PRT(iprt))
    {
        // construct the new structure
        prt_t::config_construct(get_ptr( iprt ), 100);
    }

    return iprt;
}

//--------------------------------------------------------------------------------------------
void ParticleManager::maybeRunDeferred()
{
    // Go through the list and activate all the particles that
    // were created while the list was iterating.
    for (size_t i = 0; i < activation_count; i++)
    {
        PRT_REF iprt = activation_list[i];

        if (!ALLOCATED_PRT(iprt)) continue;
        prt_t *pprt = get_ptr(iprt);

        if (!pprt->obj_base.turn_me_on) continue;

        pprt->obj_base.on         = true;
        pprt->obj_base.turn_me_on = false;
    }
    activation_count = 0;

    // Go through and delete any particles that
    // were supposed to be deleted while the list was iterating.
    for (size_t i = 0; i < termination_count; i++ )
    {
        free_one(termination_list[i]);
    }
    termination_count = 0;
}

//--------------------------------------------------------------------------------------------
void ParticleManager::reset_all()
{
    /// @author ZZ
    /// @details This resets all particle data and reads in the coin and water particles

    const char *loadpath;

    //release_all_local_pips();
    PipStack.release_all();

    // Load in the standard global particles ( the coins for example )
    loadpath = "mp_data/1money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_COIN1 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/5money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_COIN5 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/25money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_COIN25 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/100money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_COIN100 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/200money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_GEM200 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/500money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_GEM500 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/1000money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_GEM1000 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/2000money.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_GEM2000 ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    // Load module specific information
    loadpath = "mp_data/weather4.txt";
    PipStack.load_one( loadpath, ( PIP_REF )PIP_WEATHER );              //It's okay if weather particles fail

    loadpath = "mp_data/weather5.txt";
    PipStack.load_one( loadpath, ( PIP_REF )PIP_WEATHER_FINISH );

    loadpath = "mp_data/splash.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_SPLASH ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    loadpath = "mp_data/ripple.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_RIPPLE ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    // This is also global...
    loadpath = "mp_data/defend.txt";
    if ( INVALID_PIP_REF == PipStack.load_one( loadpath, ( PIP_REF )PIP_DEFEND ) )
    {
        log_error( "Data file was not found! (\"%s\")\n", loadpath );
    }

    PipStack.count = GLOBAL_PIP_COUNT;
}