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
	  return ( f.FFp(j) == &f || &f == f.FFp(j)->FFp(f.FFi(j)) );
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
	  return f.IsBorder(j);
  else 
    return true;
}


/// Count border edges of the face
template <class FaceType>
inline int BorderCount(FaceType const & f) 
{
  if(FaceType::HasFFAdjacency())
  {
    int t = 0;
	  if( f.IsBorder(0) ) ++t;
	  if( f.IsBorder(1) ) ++t;
	  if( f.IsBorder(2) ) ++t;
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

/** This function change the orientation of the face. Inverting the index of two vertex 
@param z Index of the edge
*/
template <class SwapFaceType>
void Swap (SwapFaceType &f, const int z )
{
  int i;
  SwapFaceType *tmp, *prec;
  int t, precz;

  swap ( f.V((z  )%3),f.V((z+1)%3));

  if(f.HasFFAdjacency() )
  {
    // TODO!!!
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
};

/*!
* Flip the z-th edge of the face f.
* Check for topological correctness first using <CODE>CheckFlipFace()</CODE>.
*	\param f	pointer to the face
*	\param z	the edge index
*/
template <class FaceType>
static void FlipEdge(FaceType &f, const int z)
{	
	assert(z>=0);
	assert(z<3);
	assert( !f.IsBorder(z) );
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
};


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

