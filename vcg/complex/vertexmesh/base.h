/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

$Log: not supported by cvs2svn $

****************************************************************************/

#pragma warning( disable : 4804 )

#include <vcg/space/box3.h>
#include <vcg/space/color4.h>


/*
People should subclass his vertex class from these one...
*/

#ifndef __VCGLIB_VERTEXMESH
#define __VCGLIB_VERTEXMESH

namespace vcg {
namespace vertex {
/** \addtogroup vertexmesh */
/*@{*/

/** \class VertexMesh.
    This is class for definition of a mesh.
		@param VertContainerType (Template Parameter) Specifies the type of the vertices container any the vertex type.
		@param PointContainerType (Template Parameter) Specifies the type of the faces container any the face type.
 */
template < class VertContainerType >
class VertexMesh{
	public:
	typedef VertContainerType VertexContainer;
	typedef typename VertContainerType::value_type VertexType;
	typedef typename VertContainerType::value_type::ScalarType ScalarType;
	typedef typename VertContainerType::value_type::CoordType CoordType;
	typedef typename VertContainerType::iterator VertexIterator;
	typedef typename VertContainerType::const_iterator ConstVertexIterator;
	typedef VertexType * VertexPointer;
	typedef const VertexType * ConstVertexPointer;
	typedef Box3<ScalarType> BoxType;

	/// Set of vertices 
	VertContainerType vert;
	/// Real number of vertices
	int vn;

	/// Bounding box of the mesh
	Box3<ScalarType> bbox;
	
	/// Global color
private:
	Color4b c;
public:

	inline const Color4b & C() const
	{
		return c;
	}

	inline Color4b & C()
	{
		return c;
	}


	/// Default constructor
	VertexMesh()
	{
		vn = 0;
		imark = 0;
	}

	inline int MemUsed() const
	{
		return sizeof(VertexMesh)*vert.size();
	}

	inline int MemNeeded() const
	{
		return sizeof(VertexMesh)*vn;
	}



/// Function to destroy the mesh
void Clear()
{
	vert.clear();
	vn = 0;
}

/// Reflection functions that speak about vertex and face properties.
static bool HasPerVertexNormal()  { return VertexType::HasNormal() ; }
static bool HasPerVertexColor()   { return VertexType::HasColor()  ; }
static bool HasPerVertexMark()    { return VertexType::HasMark()   ; }
static bool HasPerVertexQuality() { return VertexType::HasQuality(); }
static bool HasPerVertexTexture() { return VertexType::HasTexture(); }

/// Initialize the imark-system of the faces
void InitPointIMark()
{
	VertexIterator f;
	
	for(f=vert.begin();f!=vert.end();++f)
		if( !(*f).IsDeleted() && (*f).IsR() && (*f).IsW() )
			(*f).InitIMark();
}

/// Initialize the imark-system of the vertices
void InitVertexIMark()
{
	VertexIterator vi;

	for(vi=vert.begin();vi!=vert.end();++vi)
		if( !(*vi).IsDeleted() && (*vi).IsRW() )
			(*vi).InitIMark();
}

/// The incremental mark
int imark;

/** Check if the vertex incremental mark matches the one of the mesh. 
*/
inline bool IsMarked( ConstVertexPointer  v ) const { return v->IMark() == imark; }
/** Set the vertex incremental mark of the vertex to the one of the mesh.
*/
inline void Mark( ConstVertexPointer v ) const { v->IMark() = imark; }
/// Unmark the mesh
inline void UnMarkAll() { ++imark; }

};	// end class VertexMesh

/*@}*/
}	 // end namespace
}	 // end namespace


#endif


