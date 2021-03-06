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

/// @file egolib/Math/Functors/Closure_Sphere_AxisAlignedBox.hpp
/// @brief Enclose axis aligned boxes in spheres.
/// @author Michael Heilmann

#pragma once

#include "egolib/platform.h"

namespace idlib {

/// @brief Specialization of idlib::enclose_functor.
/// Encloses an axis aligned box in a sphere.
/// @detail A sphere \f$b\f$ enclosing an axis aligned box \f$a\f$ has the center \f$c\f$
/// of the axis aligned box. Its radius is given by \f$r:=|x - c|\f$ where \x\f$ is the
/// minimum point of the axis aligned box and \f$|.|\f$ is the Euclidean length of a vector.
/// @tparam P the point type of the geometries
template <typename P>
struct enclose_functor<sphere<P>, axis_aligned_box<P>>
{
    auto operator()(const axis_aligned_box<P>& source) const
	{
		const auto center = source.get_center();
		const auto radius = euclidean_norm(source.get_min() - center);
        return sphere<P>(center, radius);
    }
}; // struct enclose_functor

} // namespace idlib
