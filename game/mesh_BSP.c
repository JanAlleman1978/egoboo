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

/// @file map_BSP.c
/// @brief Implementation of functions for the mpd BSP
/// @details

#include "mesh_BSP.h"

#include "../egolib/_math.inl"

#include "frustum.h"
#include "obj_BSP.h"

#include "bbox.inl"
#include "mesh.inl"
#include "bsp.inl"

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

static ego_bool _mpd_BSP_system_initialized = ego_false;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

mesh_BSP_t mesh_BSP_root = MAP_BSP_INIT;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

static ego_bool mesh_BSP_insert( mesh_BSP_t * pbsp, ego_tile_info_t * ptile, int index );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_system_started( void )
{
    return _mpd_BSP_system_initialized;
}

//--------------------------------------------------------------------------------------------
egolib_rv mesh_BSP_system_begin( ego_mesh_t * pmpd )
{
    // if he system is already started, do a reboot
    if ( _mpd_BSP_system_initialized )
    {
        if ( rv_error == mesh_BSP_system_end() )
        {
            return rv_error;
        }
    }

    // start the system using the given mesh
    if ( NULL != pmpd )
    {
        mesh_BSP_t * rv;

        // initialize the mesh's BSP structure with the mesh tiles
        rv = mesh_BSP_ctor( &mesh_BSP_root, pmpd );

        _mpd_BSP_system_initialized = ( NULL != rv );
    }

    return _mpd_BSP_system_initialized ? rv_success : rv_fail;
}

//--------------------------------------------------------------------------------------------
egolib_rv  mesh_BSP_system_end( void )
{
    if ( _mpd_BSP_system_initialized )
    {
        mesh_BSP_dtor( &mesh_BSP_root );
    }

    _mpd_BSP_system_initialized = ego_false;

    return rv_success;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
mesh_BSP_t * mesh_BSP_ctor( mesh_BSP_t * pbsp, const ego_mesh_t * pmesh )
{
    /// @author BB
    /// @details Create a new BSP tree for the mesh.
    ///     These parameters duplicate the max resolution of the old system.

    int grids_x, grids_y;
    float x_min, x_max, y_min, y_max, bsp_size;
    int depth;

    if ( NULL == pbsp ) return NULL;

    BLANK_STRUCT_PTR( pbsp )

    if ( NULL == pmesh ) return pbsp;

    // get the nominal physical size of the mesh
    x_min = 0.0f;
    x_max = pmesh->gmem.edge_x;
    y_min = 0.0f;
    y_max = pmesh->gmem.edge_y;
    bsp_size = MAX( x_max - x_min, y_max - y_min );

    // determine the number of bifurcations necessary to get cells the size of the "blocks"
    grids_x = pmesh->gmem.grids_x;
    grids_y = pmesh->gmem.grids_y;
    depth = CEIL( LOG( 0.5f * MAX( grids_x, grids_y ) ) / LOG( 2.0f ) );

    // make a 2D BSP tree with "max depth" depth
    // this automatically allocates all data
    BSP_tree_ctor( &( pbsp->tree ), 2, depth );

    // !!!!SET THE BSP SIZE HERE!!!!
    // enlarge it a bit
    pbsp->tree.bsp_bbox.mins.ary[kX] = x_min - 0.25f * bsp_size;
    pbsp->tree.bsp_bbox.maxs.ary[kX] = x_max + 0.25f * bsp_size;
    pbsp->tree.bsp_bbox.mids.ary[kX] = 0.5f * ( pbsp->tree.bsp_bbox.mins.ary[kX] + pbsp->tree.bsp_bbox.maxs.ary[kX] );

    pbsp->tree.bsp_bbox.mins.ary[kY] = y_min - 0.25f * bsp_size;
    pbsp->tree.bsp_bbox.maxs.ary[kY] = y_max + 0.25f * bsp_size;
    pbsp->tree.bsp_bbox.mids.ary[kY] = 0.5f * ( pbsp->tree.bsp_bbox.mins.ary[kY] + pbsp->tree.bsp_bbox.maxs.ary[kY] );

    // initialize the volume
    oct_bb_ctor( &( pbsp->volume ) );

    // do any additional allocation
    mesh_BSP_alloc( pbsp );

    return pbsp;
}

//--------------------------------------------------------------------------------------------
mesh_BSP_t * mesh_BSP_dtor( mesh_BSP_t * pbsp )
{
    if ( NULL == pbsp ) return NULL;

    // destroy the tree
    BSP_tree_dtor( &( pbsp->tree ) );

    // free any other allocated memory
    mesh_BSP_free( pbsp );

    BLANK_STRUCT_PTR( pbsp )

    return pbsp;
}

//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_alloc( mesh_BSP_t * pbsp )
{
    if ( NULL == pbsp ) return ego_false;

    // BSP_tree_alloc() is called by BSP_tree_ctor(), so there is no need to
    // do any allocation here

    return ego_true;
}

//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_free( mesh_BSP_t * pbsp )
{
    if ( NULL == pbsp ) return ego_false;

    // no other data allocated, so nothing else to do

    return ego_true;
}

//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_fill( mesh_BSP_t * pbsp, const ego_mesh_t * pmpd )
{
    int cnt;

    size_t tcount;
    ego_tile_info_t * tlist, *ptile;

    // error trap
    if ( NULL == pbsp || NULL == pmpd ) return ego_false;
    tcount = pmpd->tmem.tile_count;
    tlist  = pmpd->tmem.tile_list;

    // make sure the mesh is allocated
    if ( 0 == tcount || NULL == tlist ) return ego_false;

    // initialize the bsp volume
    // assumes tlist[0] is insterted
    oct_bb_copy( &( pbsp->volume ), &( tlist[0].oct ) );

    // insert each tile
    for ( cnt = 0, ptile = tlist; cnt < tcount; cnt++, ptile++ )
    {
        // try to insert the BSP
        if ( mesh_BSP_insert( pbsp, ptile, cnt ) )
        {
            // add this tile's volume to the bsp's volume
            oct_bb_self_union( &( pbsp->volume ), &( ptile->oct ) );
        }
    }

    return pbsp->count > 0;
}

//--------------------------------------------------------------------------------------------
int mesh_BSP_collide_aabb( const mesh_BSP_t * pbsp, const aabb_t * paabb, BSP_leaf_test_t * ptest, BSP_leaf_pary_t * colst )
{
    /// @author BB
    /// @details fill the collision list with references to tiles that the object volume may overlap.
    ///      Return the number of collisions found.

    if ( NULL == pbsp || NULL == paabb || NULL == colst ) return 0;

    return BSP_tree_collide_aabb( &( pbsp->tree ), paabb, ptest, colst );
}

//--------------------------------------------------------------------------------------------
int mesh_BSP_collide_frustum( const mesh_BSP_t * pbsp, const egolib_frustum_t * pfrust, BSP_leaf_test_t * ptest, BSP_leaf_pary_t * colst )
{
    /// @author BB
    /// @details fill the collision list with references to tiles that the object volume may overlap.
    ///      Return the number of collisions found.

    if ( NULL == pbsp || NULL == pfrust || NULL == colst ) return 0;

    return BSP_tree_collide_frustum( &( pbsp->tree ), pfrust, ptest, colst );
}

//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_insert( mesh_BSP_t * pbsp, ego_tile_info_t * ptile, int index )
{
    /// @author BB
    /// @details insert a tile's BSP_leaf_t into the BSP_tree_t

    ego_bool       retval;
    BSP_leaf_t * pleaf;
    BSP_tree_t * ptree;

    if ( NULL == pbsp || NULL == ptile ) return ego_false;
    ptree = &( pbsp->tree );

    // grab the leaf from the tile
    pleaf = &( ptile->bsp_leaf );

    // make sure everything is kosher
    if ( ptile != ( ego_tile_info_t * )( pleaf->data ) )
    {
        // some kind of error. re-initialize the data.
        pleaf->data      = ptile;
        pleaf->index     = index;
        pleaf->data_type = BSP_LEAF_TILE;
    };

    // convert the octagonal bounding box to an aabb
    ego_aabb_from_oct_bb( &( pleaf->bbox ), &( ptile->oct ) );

    // insert the leaf
    retval = BSP_tree_insert_leaf( ptree, pleaf );

    if ( retval )
    {
        // log all successes
        pbsp->count++;
    }

    return retval;
}

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_can_collide( BSP_leaf_t * pleaf )
{
    /// @author BB
    /// @details a test function passed to BSP_*_collide_* functions to determine whether a leaf
    ///               can be added to a collision list

    ego_tile_info_t * ptile;

    // make sure we have a character leaf
    if ( NULL == pleaf || NULL == pleaf->data || BSP_LEAF_TILE != pleaf->data_type )
    {
        return ego_false;
    }
    ptile = ( ego_tile_info_t * )( pleaf->data );

    if ( TILE_IS_FANOFF( *ptile ) ) return ego_false;

    return ego_true;
}

//--------------------------------------------------------------------------------------------
ego_bool mesh_BSP_is_visible( BSP_leaf_t * pleaf )
{
    /// @author BB
    /// @details a test function passed to BSP_*_collide_* functions to determine whether a leaf
    ///               can be added to a collision list

    ego_tile_info_t * ptile;

    // make sure we have a character leaf
    if ( NULL == pleaf || NULL == pleaf->data || BSP_LEAF_TILE != pleaf->data_type )
    {
        return ego_false;
    }
    ptile = ( ego_tile_info_t * )( pleaf->data );

    if ( TILE_IS_FANOFF( *ptile ) ) return ego_false;

    return ego_true;
}