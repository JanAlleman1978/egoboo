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

/// @file egolib/state_machine.h

#pragma once

/// The various states that an egoboo state machine can occupy.
enum ego_state_t
{
    ego_state_invalid = 0,  ///< A state to set the machine in if it is not valid.
    ego_state_begin,        ///< The creation of the machine. Should be run once.
    ego_state_entering,     ///< The initialization of the machine. An entry point for re-initializing an already created machine. Run as many times as needed.
    ego_state_running,      ///< The normal state of a running machine. Run as many times as desired.
    ego_state_leaving,      ///< The deinitialization of the machine. Run as many times as needed.
    ego_state_finish        ///< The final destruction of the machine. Should be run once.
};

#if 0
// this typedef must be after the enum definition or gcc has a fit
typedef enum e_ego_states ego_state_t;
#endif