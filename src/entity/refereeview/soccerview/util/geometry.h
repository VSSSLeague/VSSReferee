//========================================================================
//  This software is free: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License Version 3,
//  as published by the Free Software Foundation.
//
//  This software is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  Version 3 in the file COPYING that came with this distribution.
//  If not, see <http://www.gnu.org/licenses/>.
//========================================================================
/*!
  \file    geometry.h
  \brief   Meta-header to include all significant geometry-related classes
  \author  James R. Bruce <jbruce@cs.cmu.edu>, (C) 2007
  \author  Stefan Zickler, (C) 2008
*/
//========================================================================

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

//==== Vector types ====//

#include "gvector.h"

typedef GVector::vector2d<double> vector2d;

//==== Angle conversion, and normally missing constant(s) ====//

#define RAD(deg) ((deg) * (M_PI / 180.0)) /* convert radians to degrees */
#define DEG(rad) ((rad) * (180.0 / M_PI)) /* convert degrees to radians */


#endif // __GEOMETRY_H__
