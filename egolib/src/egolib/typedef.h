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

/// @file    egolib/egolib_typedef.h
/// @brief   Basic types used throughout the code.
/// @details Some basic types that are used throughout the game code.

#pragma once

// this include must be the absolute last include
#include "egolib/egolib_config.h"

// this include must be the absolute last include
#include "egolib/mem.h"

//--------------------------------------------------------------------------------------------
// place the definition of the lambda operator in a macro
#define LAMBDA(AA,BB,CC) ((AA) ? (BB) : (CC))

//--------------------------------------------------------------------------------------------
// portable definition of assert. the c++ version can be activated below.
// make assert into a warning if _DEBUG is not defined
void non_fatal_assert( bool val, const char * format, ... ) GCC_PRINTF_FUNC( 2 );



//--------------------------------------------------------------------------------------------
// a replacement for memset()

#    if !defined(BLANK_STRUCT_PTR)
#       define BLANK_STRUCT_PTR(XX)  memset( XX, 0, sizeof( *(XX) ) );
#    endif

#    if !defined(BLANK_ARY)
#       define BLANK_ARY(XX)  memset( XX, 0, sizeof( XX ) );
#    endif

//--------------------------------------------------------------------------------------------
// BOOLEAN

#if !defined(TO_EGO_BOOL)
	#if defined(__cplusplus)
		#define TO_C_BOOL(VAL)   LAMBDA(VAL, true, false)
	#else
		#define TO_C_BOOL(VAL) (VAL)
	#endif
#endif

//--------------------------------------------------------------------------------------------

	/**
	 * @brief
	 *	Special return values.
	 * @todo
	 *	Rename to Ego::Result.
	 */
    enum egolib_rv
    {
        rv_error   = -1,
        rv_fail    = false,
        rv_success = true
    };

//--------------------------------------------------------------------------------------------
// 24.8 fixed point types

    typedef Uint32 UFP8_T;
    typedef Sint32 SFP8_T;

    /// fast version of V1 / 256
#   define UFP8_TO_UINT(V1)   ( ((unsigned)(V1)) >> 8 )
    /// signed version of V1 / 256
#   define SFP8_TO_SINT(V1)   LAMBDA( (V1) < 0, -((signed)UFP8_TO_UINT(-V1)), (signed)UFP8_TO_UINT(V1) )

    /// fast version of V1 / 256
#   define UINT_TO_UFP8(V1)   ( ((unsigned)(V1)) << 8 )
    /// signed version of V1 / 256
#   define SINT_TO_SFP8(V1)   LAMBDA( (V1) < 0, -((signed)UINT_TO_UFP8(-V1)), (signed)UINT_TO_UFP8(V1) )

    /// version of V1 / 256.0f
#   define FP8_TO_FLOAT(V1)   ( (float)(V1) * INV_0100 )
    /// version of V1 * 256.0f
#   define FLOAT_TO_FP8(V1)   ( (Uint32)((V1) * (float)(0x0100) ) )

#   define FP8_MUL(V1, V2)    ( ((V1)*(V2)) >> 8 )               ///< this may overflow if V1 or V2 have non-zero bits in their upper 8 bits
#   define FP8_DIV(V1, V2)    ( ((V1)<<8) / (V2) )               ///< this  will fail if V1 has bits in the upper 8 bits

//--------------------------------------------------------------------------------------------
    /// the type for the 16-bit value used to store angles
    typedef Uint16   FACING_T;

    /// the type for the 14-bit value used to store angles
    typedef FACING_T TURN_T;

#   define TO_FACING(X) ((FACING_T)(X))
#   define TO_TURN(X)   ((TURN_T)((TO_FACING(X)>>2) & TRIG_TABLE_MASK))

//--------------------------------------------------------------------------------------------
// 16.16 fixed point types

    typedef Uint32 UFP16_T;
    typedef Sint32 SFP16_T;

#   define FLOAT_TO_FP16( V1 )  ( (Uint32)((V1) * 0x00010000) )
#   define FP16_TO_FLOAT( V1 )  ( (float )((V1) * 0.0000152587890625f ) )

//--------------------------------------------------------------------------------------------
// BIT FIELDS
    typedef Uint32 BIT_FIELD;                              ///< A big string supporting 32 bits

#   define FULL_BIT_FIELD      0x7FFFFFFF                  ///< A bit string where all bits are flagged as 1
#   define EMPTY_BIT_FIELD     0                           ///< A bit string where all bits are flagged as 0
#   define FILL_BIT_FIELD(XX)  (XX) = FULL_BIT_FIELD       ///< Fills up all bits in a bit pattern
#   define RESET_BIT_FIELD(XX) (XX) = EMPTY_BIT_FIELD      ///< Resets all bits in a BIT_FIELD to 0

#    if !defined(SET_BIT)
#       define SET_BIT(XX, YY) (XX) |= (YY)
#    endif

#    if !defined(UNSET_BIT)
#       define UNSET_BIT(XX, YY) (XX) &= ~(YY)
#    endif

#    if !defined(BOOL_TO_BIT)
#       define BOOL_TO_BIT(XX)       LAMBDA(XX, 1, 0 )
#    endif

#    if !defined(BIT_TO_BOOL)
#       define BIT_TO_BOOL(XX)       (1 == (XX))
#    endif

#    if !defined(HAS_SOME_BITS)
#       define HAS_SOME_BITS(XX,YY) (0 != ((XX)&(YY)))
#    endif

#    if !defined(HAS_ALL_BITS)
#       define HAS_ALL_BITS(XX,YY)  ((YY) == ((XX)&(YY)))
#    endif

#    if !defined(HAS_NO_BITS)
#       define HAS_NO_BITS(XX,YY)   (0 == ((XX)&(YY)))
#    endif

#    if !defined(MISSING_BITS)
#       define MISSING_BITS(XX,YY)  (HAS_SOME_BITS(XX,YY) && !HAS_ALL_BITS(XX,YY))
#    endif

#   define CLIP_TO_08BITS( V1 )  ( (V1) & 0xFF       )
#   define CLIP_TO_16BITS( V1 )  ( (V1) & 0xFFFF     )
#   define CLIP_TO_24BITS( V1 )  ( (V1) & 0xFFFFFF   )
#   define CLIP_TO_32BITS( V1 )  ( (V1) & 0xFFFFFFFF )

//--------------------------------------------------------------------------------------------
// RECTANGLE

	namespace Ego
	{
		/**
		 * @brief A rectangle in a 2 dimensional Cartesian coordinate system.
		 * @invariant
		 */
		template <typename Type>
		struct Rectangle
		{
			Type _left;   ///< @brief The coordinate of the left side   of the rectangle.
			              ///< @invariant <tt>left <= right</tt>.
			Type _bottom; ///< @brief The coordinate of the bottom side of the rectangle.
			              ///< @invariant <tt>bottom <= top</tt>.
			Type _right;  ///< @brief The coordinate of the right side  of the rectangle.
			Type _top;    ///< @brief The coordinate of the top side    of the rectangle.
			/**
			 * @brief
			 *	Construct an empty rectangle.
			 */
			Rectangle() : _left(), _bottom(), _right(), _top()
			{
			}
			/**
			 * @brief
			 *	Construct this rectangle with the specified sides.
			 * @param left
			 *	the coordinate of the left side
			 * @param bottom
			 *	the coordinate of the bottom side
			 * @param right
			 *	the coordinate of the right side
			 * @param top
			 *	the coordinate of the top side
			 * @throws std::domain_error
			 *	if <tt>left > right</tt> or <tt>bottom > top</tt>
			 */
			Rectangle(const Type& left, const Type& bottom, const Type& right, const Type& top)
			{
				if (left > right)
				{
					throw std::domain_error("the coordinate of the left side must be smaller than or equal to the coordinate of the right side");
				}
				if (bottom > top)
				{
					throw std::domain_error("the coordinate of the bottom side must be smaller than or equal to the coordinate of the top side");
				}
				_left = left;
				_bottom = bottom;
				_right = right;
				_top = top;
			}
			bool point_inside(const Type& x, const Type& y)
			{
				if (x < _left || x > _right) return false;
				if (y < _top  || y > _bottom) return false;
				return true;
			}
		};
	};

    struct irect_t
    {
        int left;
        int right;
        int top;
        int bottom;
		irect_t() :left(0), right(0), top(0), bottom(0)
		{
		}
		bool point_inside(int x, int y)
		{
			if (x < left || x > right)  return false;
			if (y < top  || y > bottom) return false;
			return true;
		}
    };

    struct frect_t
    {
        float left;
        float right;
        float top;
        float bottom;
		frect_t() :left(0.0f), right(0.0f), top(0.0f), bottom(0.0f)
		{
		}
		bool point_inside(float x, float y)
		{
			if (x < left || x > right) return false;
			if (y < top  || y > bottom) return false;
			return true;
		}
    };

	struct ego_irect_t
    {
        int xmin, ymin;
        int xmax, ymax;
    };

    struct ego_frect_t
    {
        float xmin, ymin;
        float xmax, ymax;
    };

//--------------------------------------------------------------------------------------------
// PAIR AND RANGE

    /// Specifies a value between "base" and "base + rand"
    struct IPair
    {
        int base, rand;
    };

    /// Specifies a value from "from" to "to"
    struct FRange
    {
        float from, to;
    };

    void pair_to_range( IPair pair, FRange * prange );
    void range_to_pair( FRange range, IPair * ppair );

    void ints_to_range( int base, int rand, FRange * prange );
    void floats_to_pair( float vmin, float vmax, IPair * ppair );

//--------------------------------------------------------------------------------------------
// IDSZ
    typedef Uint32 IDSZ;

#    if !defined(MAKE_IDSZ)
#       define MAKE_IDSZ(C0,C1,C2,C3) \
    ((IDSZ)( \
             ((((C0)-'A')&0x1F) << 15) |       \
             ((((C1)-'A')&0x1F) << 10) |       \
             ((((C2)-'A')&0x1F) <<  5) |       \
             ((((C3)-'A')&0x1F) <<  0)         \
           ))
#    endif

#   define IDSZ_NONE            MAKE_IDSZ('N','O','N','E')       ///< [NONE]

    const char * undo_idsz( IDSZ idsz );

//--------------------------------------------------------------------------------------------
// STRING
    typedef char STRING[256];

//--------------------------------------------------------------------------------------------
// ego_message_t

    /// the maximum length egoboo messages
#   define EGO_MESSAGE_SIZE      90

    typedef char ego_message_t[EGO_MESSAGE_SIZE];

//--------------------------------------------------------------------------------------------

/// the "base class" of Egoboo profiles
#   define  EGO_PROFILE_STUFF \
    bool loaded;                    /** Was the data read in? */ \
    STRING name;                    /** Usually the source filename */ \
    int    request_count;           /** the number of attempted spawnx */ \
    int    create_count;            /** the number of successful spawns */

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
/// The latch used by the input system
    struct s_latch
    {
        float          x;         ///< the x input
        float          y;         ///< the y input
        BIT_FIELD      b;         ///< the button bits
    };
    typedef struct s_latch latch_t;

    void latch_init( latch_t * platch );

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// References

    /// base reference type
    typedef Uint16 REF_T;

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// definitions for the compiler environment

#if defined(_DEBUG)
#define EGOBOO_ASSERT(expression) assert(expression)
#else
#define EGOBOO_ASSERT(expression) non_fatal_assert(expression, "%s - failed an assert \"%s\"\n", __FUNCTION__, #expression)
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// definition of the c-type reference

#   define DECLARE_REF( NAME ) typedef REF_T NAME

// define the c implementation always
#   define REF_TO_INT(X) ((REF_T)(X))

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// a simple array

#define DECLARE_T_ARY(TYPE, NAME, COUNT)  TYPE   NAME[COUNT]

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// a simple list structure that tracks free elements

#define ACCESS_TYPE_NONE

#define INVALID_UPDATE_GUID ((unsigned)(~((unsigned)0)))

#define DEFINE_LIST_TYPE(TYPE, NAME, COUNT) \
    struct s_c_list__##TYPE__##NAME           \
    {                                         \
        unsigned update_guid;                 \
        int      used_count;                  \
        int      free_count;                  \
        size_t   used_ref[COUNT];             \
        size_t   free_ref[COUNT];             \
        DECLARE_T_ARY(TYPE, lst, COUNT);    \
    }

#define DECLARE_LIST_EXTERN(TYPE, NAME, COUNT) \
    DEFINE_LIST_TYPE(TYPE, NAME, COUNT);       \
    void   NAME##_ctor( void );                  \
    void   NAME##_dtor( void );                  \
    bool NAME##_push_used( const REF_T );        \
    TYPE * NAME##_get_ptr( const size_t );       \
    extern struct s_c_list__##TYPE__##NAME NAME

#define INSTANTIATE_LIST_STATIC(TYPE, NAME, COUNT) \
    DEFINE_LIST_TYPE(TYPE, NAME, COUNT);           \
    static struct s_c_list__##TYPE__##NAME NAME = {INVALID_UPDATE_GUID, 0, 0}

#define INSTANTIATE_LIST(ACCESS,TYPE,NAME, COUNT) \
    ACCESS struct s_c_list__##TYPE__##NAME NAME = {INVALID_UPDATE_GUID, 0, 0}

#ifndef IMPLEMENT_LIST
#define IMPLEMENT_LIST(TYPE, NAME, COUNT)             \
    static int     NAME##_find_free_ref( const REF_T ); \
    static bool  NAME##_push_free( const REF_T );       \
    static size_t  NAME##_pop_free( const int );        \
    static int     NAME##_find_used_ref( const REF_T ); \
    static size_t  NAME##_pop_used( const int );        \
    TYPE * NAME##_get_ptr( const size_t index )   { return LAMBDA(index >= COUNT, NULL, NAME.lst + index); }
#endif

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------
// A stack with a fixed capacity specified at compile-time (cfg. std::array).
template <typename ElementType,size_t Capacity>
struct Stack {
	unsigned update_guid;
	int count; ///< @todo Rename to @a size.
	ElementType lst[Capacity];
	size_t get_size() const { return size; }
	size_t get_capacity() const { return Capacity;  }
	ElementType *get_ptr(size_t index) { return (index >= Capacity) ? NULL : lst + index; }
};

#define DEFINE_STACK_TYPE(ElementType, Name, Capacity)

#define DECLARE_STACK_EXTERN(ElementType, Name, Capacity) \
	ElementType *Name##_get_ptr(size_t index); \
    extern Stack<ElementType,Capacity> Name

#define INSTANTIATE_STACK_STATIC(ElementType, Name, Capacity)  \
    DEFINE_STACK_TYPE(TYPE, NAME, COUNT);            \
    static Stack<ElementType,Capacity> Name = {0}

#define INSTANTIATE_STACK(ACCESS, ElementType, Name, Capacity) \
    ACCESS Stack<ElementType,Capacity> Name = {INVALID_UPDATE_GUID, 0}

#define IMPLEMENT_STACK(ElementType, Name, Capacity)  \
    ElementType * Name##_get_ptr(size_t index) { return Name.get_ptr(index); }

//--------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------

/// a template-like declaration of a statically allocated array
template <typename ElementType,size_t Capacity>
struct StaticArray
{
	int count;
	ElementType ary[Capacity];
	ElementType *get_ptr(size_t index)
	{
		return (index >= Capacity) ? NULL : this->ary + index;
	}
};

#define STATIC_ARY_INIT_VALS {0}

#define DECLARE_EXTERN_STATIC_ARY(ElementType,Name, Capacity) \
    extern StaticArray<ElementType,Capacity> Name

#define INSTANTIATE_STATIC_ARY(ElementType, Name, Capacity) \
    StaticArray<ElementType,Capacity> Name = STATIC_ARY_INIT_VALS;
