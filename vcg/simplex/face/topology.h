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
Revision 1.26  2005/12/19 13:47:26  corsini
Rewrite SwapEdge to fix problems with borders

Revision 1.25  2005/12/16 11:01:26  corsini
Remove trivial warnings

Revision 1.24  2005/12/16 10:47:48  corsini
Add further comment to FlipEdge

Revision 1.23  2005/12/16 10:43:23  corsini
Fix one bug

Revision 1.22  2005/12/16 10:29:10  corsini
Add CheckOrientation
Reimplement SwapEdge function

Revision 1.21  2005/12/01 23:54:59  cignoni
Removed excess ';' from end of template functions, for gcc compiling

Revision 1.20  2005/11/23 13:04:26  ganovelli
changed IsBOrder

Revision 1.19  2005/11/10 15:49:32  cignoni
Made IsManifold Constant

Revision 1.18  2005/10/13 08:34:19  cignoni
Removed reference to IsBorder() member of face and substituted with the face templated function version.

Revision 1.17  2005/04/11 09:17:24  pietroni
Changed detach to FFdetach , compiled tested in manifold cases

Revision 1.16  2005/03/18 16:35:53  fiorin
minor changes to comply gcc compiler

Revision 1.15  2004/10/22 13:41:06  fiorin
Added CheckFlipEdge and FlipEdge

Revision 1.14  2004/10/18 17:15:45  ganovelli
minor change

Revision 1.13  2004/08/06 01:47:57  pietroni
corrected errors on vfappend

Revision 1.12  2004/08/05 22:27:00  pietroni
added VFAppend funtion

Revision 1.10  2004/07/27 09:49:23  cignoni
Removed warning about postfix incremnet of VFIterator

Revision 1.9  2004/07/15 12:03:07  ganovelli
minor changes

Revision 1.8  2004/07/15 11:26:48  ganovelli
VFDetach corrected

Revision 1.7  2004/05/12 12:23:23  cignoni
Conformed C++ syntax to GCC requirements

Revision 1.6  2004/05/11 16:03:18  ganovelli
changed from "thi" to "&f" in Vfdetach

Revision 1.5  2004/05/10 15:20:49  cignoni
Updated names of POS and adj functions to the new standards for many functions

Revision 1.4  2004/03/18 16:00:10  cignoni
minor changes


****************************************************************************/

#ifndef _VCG_FACE_TOPOLOGY
#define _VCG_FACE_TOPOLOGY
#include <vcg/simplex/face/pos.h>
namespace vcg {
namespace face {
/** \addtogroup face */
/*@{*/

/** Return a boolean that indicate if the face is complex.
    @param j Index of the edge
	@return true se la faccia e' manifold, false altrimenti
*/
template <class FaceType>
inline bool IsManifold( FaceType const & f, const int j ) 
{
  if(FaceType::HasFFAdjacency())
	  return ( f.cFFp(j) == &f || &f == f.cFFp(j)->cFFp(f.cFFi(j)) );
  else 
    return true;
}

/** Return a boolean that indicate if the j-th edge of the face is a border.
	@param j Index of the edge
	@return true if j is an edge of border, false otherwise
*/
template <class FaceType>
inline bool IsBorder(FaceType const & f,  const int j ) 
{
  if(FaceType::HasFFAdjacency())
	  return f.cFFp(j)==&f;
    //return f.IsBorder(j);
  
  assert(0);
  return true;
}


/// Count border edges of the face
template <class FaceType>
inline int BorderCount(FaceType const & f) 
{
  if(FaceType::HasFFAdjacency())
  {
    int t = 0;
	  if( IsBorder(f,0) ) ++t;
	  if( IsBorder(f,1) ) ++t;
	  if( IsBorder(f,2) ) ++t;
	  return t;
  }
	else 	return 3;
}


/// Counts the number of incident faces in a complex edge
template <class FaceType>
inline int ComplexSize(FaceType const & f, const int e)
{
if(FaceType::HasFFAdjacency())
{
  Pos< FaceType > fpos(&f,e); 
  int cnt=0;
  do
  {
		fpos.NextF();
		++cnt;
	}
	while(fpos.f=&f);
	return cnt;
}
  assert(0);
	return 2;
}

/*Funzione di detach che scollega una faccia da un ciclo 
(eventualmente costituito da due soli elementi) incidente su un edge*/
/** This function detach the face from the adjacent face via the edge e. It's possible to use it also in non-two manifold situation.
		The function cannot be applicated if the adjacencies among faces aren't define.
		@param e Index of the edge
*/
template <class FaceType>
void FFDetach(FaceType & f, const int e)
{
	assert(!IsBorder<FaceType>(f,e));
	Pos< FaceType > EPB(&f,e,f.V(e));//build the half edge
	//vcg::face::Pos< FaceType > pos(&f, (z+2)%3, f.V2(z));
	EPB.NextF();
	int cnt=0;

	///then in case of non manifold face continue to switch the 
	///set of faces that share the edge until I find the one that 
	///preceed the one I want to erase

	while ( EPB.f->FFp(EPB.z) != &f)
	{ 
		assert(!IsManifold<FaceType>(f,e));   // Si entra in questo loop solo se siamo in una situazione non manifold.
		assert(!IsBorder<FaceType>(*EPB.f,e));
		EPB.NextF();
		cnt++;
	}

	assert(EPB.f->FFp(EPB.z)==&f);

	EPB.f->FFp(EPB.z) = f.FFp(e);
	EPB.f->FFi(EPB.z) = f.FFi(e);
	
	f.FFp(e) = &f;
	f.FFi(e) = e;

}


/** This function attach the face (via the edge z1) to another face (via the edge z2). It's possible to use it also in non-two manifold situation.
		The function cannot be applicated if the adjacencies among faces aren't define.
		@param z1 Index of the edge
		@param f2 Pointer to the face
		@param z2 The edge of the face f2 
*/
template <class FaceType>
void Attach(FaceType * &f, int z1, FaceType *&f2, int z2)
{
	//typedef FEdgePosB< FACE_TYPE > ETYPE;
	Pos< FaceType > EPB(f2,z2);
	Pos< FaceType > TEPB;
	TEPB = EPB;
	EPB.NextF();
	while( EPB.f != f2)  //Alla fine del ciclo TEPB contiene la faccia che precede f2
	{
		TEPB = EPB;
		EPB.NextF();
	}
	//Salvo i dati di f1 prima di sovrascrivere
	FaceType *f1prec = f.FFp(z1);  
	int z1prec = f.FFi(z1);
	//Aggiorno f1
	f->FFp(z1) = TEPB.f->FFp(TEPB.z);  
	f->FFi(z1) = TEPB.f->FFi(TEPB.z);
	//Aggiorno la faccia che precede f2
	TEPB.f->FFp(TEPB.z) = f1prec;
	TEPB.f->FFi(TEPB.z) = z1prec;
}


template <class FaceType>
void AssertAdj(FaceType & f)
{
	assert(f.FFp(0)->FFp(f.FFi(0))==&f);
	assert(f.FFp(1)->FFp(f.FFi(1))==&f);
	assert(f.FFp(2)->FFp(f.FFi(2))==&f);

	assert(f.FFp(0)->FFi(f.FFi(0))==0);
	assert(f.FFp(1)->FFi(f.FFi(1))==1);
	assert(f.FFp(2)->FFi(f.FFi(2))==2); 
}
// Funzione di supporto usata da swap?
//template <class FaceType>
//inline void Nexts(  *&f, int &z )
//{
//    int t;
//    t = z;
//    z = (*f).Z(z);
//    f = (*f).F(t);
//}

/**
 * Check if the given face is oriented as the one adjacent to the specified edge.
 * @param f Face to check the orientation
 * @param z Index of the edge
 */
template <class FaceType>
bool CheckOrientation(FaceType &f, int z)
{
	if (IsBorder(f, z))
		return true;
	else
	{
		FaceType *g = f.FFp(z);
		int gi = f.FFi(z);
		if (f.V0(z) == g->V1(gi))
			return true;
		else
			return false;
	}
}

/** 
 * This function change the orientation of the face by inverting the index of two vertex.
 * @param z Index of the edge
 */
template <class FaceType>
void SwapEdge(FaceType &f, const int z) { SwapEdge<FaceType,true>(f,z); }

template <class FaceType, bool UpdateTopology>
void SwapEdge(FaceType &f, const int z)
{
	// swap V0(z) with V1(z)
	swap(f.V0(z), f.V1(z));

	if(f.HasFFAdjacency() && UpdateTopology)
	{
		// store information to preserve topology
		int z1 = (z+1)%3;
		int z2 = (z+2)%3;
		FaceType *g1p = f.FFp(z1);
		FaceType *g2p = f.FFp(z2);
		int g1i = f.FFi(z1);
		int g2i = f.FFi(z2);

		// g0 face topology is not affected by the swap

		if (g1p != &f)
		{
			g1p->FFi(g1i) = z2;
			f.FFi(z2) = g1i;
		}
		else
		{
			f.FFi(z2) = z2;
		}

		if (g2p != &f)
		{
			g2p->FFi(g2i) = z1;
			f.FFi(z1) = g2i;
		}
		else
		{
			f.FFi(z1) = z1;
		}

		// finalize swap
		f.FFp(z1) = g2p;
		f.FFp(z2) = g1p;
	}
}

/*!
* Check if the z-th edge of the face f can be flipped.
*	\param f	pointer to the face
*	\param z	the edge index
*/
template <class FaceType>
static bool CheckFlipEdge(FaceType &f, int z)
{
	if (z<0 || z>2)
		return false;

	// boundary edges cannot be flipped
	if (face::IsBorder(f, z))
		return false;

	FaceType *g = f.FFp(z);
	int		 w = f.FFi(z);

	// check if the vertices of the edge are the same
	if (g->V(w)!=f.V1(z) || g->V1(w)!=f.V(z) )
		return false;

	// check if the flipped edge is already present in the mesh
	typedef typename FaceType::VertexType VertexType;
	VertexType *f_v2 = f.V2(z);
	VertexType *g_v2 = g->V2(w);
	if (f_v2 == g_v2)
		return false;

	vcg::face::Pos< FaceType > pos(&f, (z+2)%3, f.V2(z));
	do
	{
		pos.NextE();
		if (g_v2==pos.f->V1(pos.z))
			return false;
	}
	while (&f!=pos.f);

	return true;
}

/*!
* Flip the z-th edge of the face f.
* Check for topological correctness first using <CODE>CheckFlipFace()</CODE>.
*	\param f	pointer to the face
*	\param z	the edge index
*
* Note: For <em>edge flip</em> we intend the swap of the diagonal of the rectangle 
*       formed by the face \a f and the face adjacent to the specified edge.
*/
template <class FaceType>
static void FlipEdge(FaceType &f, const int z)
{	
	assert(z>=0);
	assert(z<3);
	assert( !IsBorder(f,z) );
	assert( face::IsManifold<FaceType>(f, z));

 	FaceType *g = f.FFp(z);
	int		 w = f.FFi(z);
	
	assert( g->V(w)	== f.V1(z) );
	assert( g->V1(w)== f.V(z) );
	assert( g->V2(w)!= f.V(z) );
	assert( g->V2(w)!= f.V1(z) );
	assert( g->V2(w)!= f.V2(z) );

	f.V1(z) = g->V2(w);
	g->V1(w) = f.V2(z);
	
	f.FFp(z)				= g->FFp1(w);
	f.FFi(z)				= g->FFi((w+1)%3);
	g->FFp(w)				= f.FFp1(z);
	g->FFi(w)				= f.FFi((z+1)%3);
	f.FFp1(z)				= g;
	f.FFi((z+1)%3)	= (w+1)%3;
	g->FFp1(w)			= &f;
	g->FFi((w+1)%3) = (z+1)%3;

	if(f.FFp(z)==g)
	{
		f.FFp(z) = &f;
		f.FFi(z) = z;
	}
	else
	{
		f.FFp(z)->FFp( f.UberZ(z) ) = &f;
		f.FFp(z)->FFi( f.UberZ(z) ) = z;
	}
	if(g->FFp(w)==&f)
	{
		g->FFp(w)=g;
		g->FFi(w)=w;
	}
	else
	{
		g->FFp(w)->FFp( g->UberZ(w) ) = g;
		g->FFp(w)->FFi( g->UberZ(w) ) = w;
	}
}


// Stacca la faccia corrente dalla catena di facce incidenti sul vertice z, 
// NOTA funziona SOLO per la topologia VF!!!
// usata nelle classi di collapse
template <class FaceType>
void VFDetach(FaceType & f, int z)
{
	if(f.V(z)->VFp()==&f )  //if it is the first face detach from the begin
	{
		int fz = f.V(z)->VFi();
		f.V(z)->VFp() = f.VFp(fz);
		f.V(z)->VFi() = f.VFi(fz);
	}
	else  // scan the list of faces in order to finde the current face f to be detached
	{
    VFIterator<FaceType> x(f.V(z)->VFp(),f.V(z)->VFi());
    VFIterator<FaceType> y;

		for(;;)
		{
			y = x;
			++x;
			assert(x.f!=0);
			if(x.f==&f) // found!
			{
				y.f->VFp(y.z) = f.VFp(z);
				y.f->VFi(y.z) = f.VFi(z);
				break;
			}
		}
	}
}

/// Append a face in VF list of vertex f->V(z) 
template <class FaceType>
void VFAppend(FaceType* & f, int z)
{
	typename FaceType::VerteType *v = f->V(z);
	if (v->VFp()!=0)
	{
		FaceType *f0=v->VFp();	
		int z0=v->VFi();
		//append
		f->VFp(z)=f0;
		f->VFi(z)=z0;
	}
	v->VFp()=f;
	v->VFi()=z;
}

/*@}*/
}	 // end namespace
}	 // end namespace

#endif

