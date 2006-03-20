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
Revision 1.21  2006/01/20 16:35:51  pietroni
added Intersection_Segment_Box function

Revision 1.20  2005/10/03 16:07:50  ponchio
Changed order of functions intersection_line_box and
intersectuion_ray_box

Revision 1.19  2005/09/30 13:11:39  pietroni
corrected 1 compiling error on Ray_Box_Intersection function

Revision 1.18  2005/09/29 15:30:10  pietroni
Added function RayBoxIntersection, renamed intersection line box from "Intersection" to "Intersection_Line_Box"

Revision 1.17  2005/09/29 11:48:00  m_di_benedetto
Added functor RayTriangleIntersectionFunctor.

Revision 1.16  2005/09/28 19:40:55  m_di_benedetto
Added intersection for ray-triangle (with Ray3 type).

Revision 1.15  2005/06/29 15:28:31  callieri
changed intersection names to more specific to avoid ambiguity

Revision 1.14  2005/03/15 11:22:39  ganovelli
added intersection between tow planes (porting from old vcg lib)

Revision 1.13  2005/01/26 10:03:08  spinelli
aggiunta intersect  ray-box

Revision 1.12  2004/10/13 12:45:51  cignoni
Better Doxygen documentation

Revision 1.11  2004/09/09 14:41:32  ponchio
forgotten typename SEGMENTTYPE::...

Revision 1.10  2004/08/09 09:48:43  pietroni
correcter .dir to .Direction and .ori in .Origin()

Revision 1.9  2004/08/04 20:55:02  pietroni
added rey triangle intersections funtions

Revision 1.8  2004/07/11 22:08:04  cignoni
Added a cast to remove a warning

Revision 1.7  2004/05/14 03:14:29  ponchio
Fixed some minor bugs

Revision 1.6  2004/05/13 23:43:54  ponchio
minor bug

Revision 1.5  2004/05/05 08:21:55  cignoni
syntax errors in inersection plane line.

Revision 1.4  2004/05/04 02:37:58  ganovelli
Triangle3<T> replaced by TRIANGLE
Segment<T> replaced by EDGETYPE

Revision 1.3  2004/04/29 10:48:44  ganovelli
error in plane segment corrected

Revision 1.2  2004/04/26 12:34:50  ganovelli
plane line
plane segment
triangle triangle added

Revision 1.1  2004/04/21 14:22:27  cignoni
Initial Commit


****************************************************************************/



#ifndef __VCGLIB_INTERSECTION_3
#define __VCGLIB_INTERSECTION_3

#include <vcg/space/point3.h>
#include <vcg/space/line3.h>
#include <vcg/space/ray3.h>
#include <vcg/space/plane3.h>
#include <vcg/space/segment3.h>
#include <vcg/space/sphere3.h>
#include <vcg/space/triangle3.h>
#include <vcg/space/intersection/triangle_triangle3.h>




namespace vcg {
/** \addtogroup space */
/*@{*/
/** 
    Function computing the intersection between couple of geometric primitives in
    3 dimension
*/
  /// interseciton between sphere and line
  template<class T>
    inline bool IntersectionLineSphere( const Sphere3<T> & sp, const Line3<T> & li, Point3<T> & p0,Point3<T> & p1 ){

    // Per prima cosa si sposta il sistema di riferimento 
    // fino a portare il centro della sfera nell'origine
    Point3<T> neworig=li.Origin()-sp.Center();
    // poi si risolve il sistema di secondo grado (con maple...)
    T t1 = li.Direction().X()*li.Direction().X();
    T t2 = li.Direction().Y()*li.Direction().Y();
    T t3 = li.Direction().Z()*li.Direction().Z();
    T t6 = neworig.Y()*li.Direction().Y();
    T t7 = neworig.X()*li.Direction().X();
    T t8 = neworig.Z()*li.Direction().Z();
    T t15 = sp.Radius()*sp.Radius();
    T t17 = neworig.Z()*neworig.Z();
    T t19 = neworig.Y()*neworig.Y();
    T t21 = neworig.X()*neworig.X();
    T t28 = T(2.0*t7*t6+2.0*t6*t8+2.0*t7*t8+t1*t15-t1*t17-t1*t19-t2*t21+t2*t15-t2*t17-t3*t21+t3*t15-t3*t19);
    if(t28<0) return false;
    T t29 = sqrt(t28);      
    T val0 = 1/(t1+t2+t3)*(-t6-t7-t8+t29); 
    T val1 = 1/(t1+t2+t3)*(-t6-t7-t8-t29);

    p0=li.P(val0);
    p1=li.P(val1);
    return true;
  }

  /// intersection between line and plane
  template<class T>
    inline bool IntersectionLinePlane( const Plane3<T> & pl, const Line3<T> & li, Point3<T> & po){
    const T epsilon = T(1e-8);

    T k = pl.Direction() * li.Direction();						// Compute 'k' factor
    if( (k > -epsilon) && (k < epsilon))
      return false;
    T r = (pl.Offset() - pl.Direction()*li.Origin())/k;	// Compute ray distance
    po = li.Origin() + li.Direction()*r;
    return true;
  }
	
   /// intersection between segment and plane
  template<class T>
    inline bool IntersectionSegmentPlane( const Plane3<T> & pl, const Segment3<T> & s, Point3<T> & po){
	vcg::Line3<T> l;
	l.Set(s.P0(),s.P1()-s.P0());
	l.Normalize();
	if (IntersectionLinePlane(pl,l,po))
		return((po-s.P0()).Norm()<=(s.P0()-s.P1()).Norm());
    return false;
  }

  /// intersection between segment and plane
  template<typename SEGMENTTYPE>
    inline bool Intersection( const Plane3<typename SEGMENTTYPE::ScalarType> & pl, 
			      const SEGMENTTYPE & sg, 
			      Point3<typename SEGMENTTYPE::ScalarType> & po){
    typedef typename SEGMENTTYPE::ScalarType T;
    const T epsilon = T(1e-8);

    T k = pl.Direction() * (sg.P1()-sg.P0());
    if( (k > -epsilon) && (k < epsilon))
      return false;
    T r = (pl.Offset() - pl.Direction()*sg.P0())/k;	// Compute ray distance
    if( (r<0) || (r > 1.0))
      return false;
    po = sg.P0()*(1-r)+sg.P1() * r;
    return true;
  }

  /// intersection between plane and triangle 
  // not optimal: uses plane-segment intersection (and the fact the two or none edges can be intersected)
  template<typename TRIANGLETYPE> 
    inline bool Intersection( const Plane3<typename TRIANGLETYPE::ScalarType> & pl, 
			      const  TRIANGLETYPE & tr, 
			      Segment3<typename TRIANGLETYPE::ScalarType> & sg){
    typedef typename TRIANGLETYPE::ScalarType T;
    if(Intersection(pl,Segment3<T>(tr.P(0),tr.P(1)),sg.P0())){
      if(Intersection(pl,Segment3<T>(tr.P(0),tr.P(2)),sg.P1()))
	return true;
      else
	{
	  Intersection(pl,Segment3<T>(tr.P(1),tr.P(2)),sg.P1());
	  return true;
	}
    }else
      {
	if(Intersection(pl,Segment3<T>(tr.P(1),tr.P(2)),sg.P0()))
	  {
	    Intersection(pl,Segment3<T>(tr.P(0),tr.P(2)),sg.P1());
	    return true;
	  }
      }
    return false;
  }

  /// intersection between two triangles
  template<typename TRIANGLETYPE> 
    inline bool Intersection(const TRIANGLETYPE & t0,const TRIANGLETYPE & t1){
    return NoDivTriTriIsect(t0.P0(0),t0.P0(1),t0.P0(2),
			    t1.P0(0),t1.P0(1),t1.P0(2));
  }
  template<class T>
    inline bool Intersection(Point3<T> V0,Point3<T> V1,Point3<T> V2,
			     Point3<T> U0,Point3<T> U1,Point3<T> U2){
    return NoDivTriTriIsect(V0,V1,V2,U0,U1,U2);
  }

  template<class T>
    inline bool Intersection(Point3<T> V0,Point3<T> V1,Point3<T> V2,
			     Point3<T> U0,Point3<T> U1,Point3<T> U2,int *coplanar,
			     Point3<T> &isectpt1,Point3<T> &isectpt2){

    return tri_tri_intersect_with_isectline(V0,V1,V2,U0,U1,U2,
					    coplanar,isectpt1,isectpt2);
  }

  template<typename TRIANGLETYPE,typename SEGMENTTYPE >
    inline bool Intersection(const TRIANGLETYPE & t0,const TRIANGLETYPE & t1,bool &coplanar,
			     SEGMENTTYPE  & sg){
    Point3<typename SEGMENTTYPE::PointType> ip0,ip1; 
    return  tri_tri_intersect_with_isectline(t0.P0(0),t0.P0(1),t0.P0(2),
					     t1.P0(0),t1.P0(1),t1.P0(2),
					     coplanar,sg.P0(),sg.P1()
					     );              
  }

	
  // ray-triangle, gives barycentric coords of intersection and distance along ray
template<class T>
bool Intersection( const Line3<T> & ray, const Point3<T> & vert0, 
				  const Point3<T> & vert1, const Point3<T> & vert2,
				  T & a ,T & b, T & dist)
{
	// small (hum) borders around triangle
	const T EPSILON2= T(1e-8); 
	
	const T EPSILON = T(1e-8);
	
	Point3<T> edge1 = vert1 - vert0;
	Point3<T> edge2 = vert2 - vert0;

	// determinant 
	Point3<T> pvec  = ray.Direction() ^ edge2;

	T det = edge1*pvec;

	// if determinant is near zero, ray lies in plane of triangle
  if (fabs(det) < EPSILON) return false;

  // calculate distance from vert0 to ray origin 
  Point3<T> tvec = ray.Origin()- vert0;

  // calculate A parameter and test bounds 
  a = tvec * pvec;
  if (a < -EPSILON2*det || a > det+det*EPSILON2) return false;

  // prepare to test V parameter 
  Point3<T> qvec = tvec ^ edge1;

  // calculate B parameter and test bounds 
  b = ray.Direction() * qvec ;
  if (b < -EPSILON2*det || b + a > det+det*EPSILON2) return false;

  // calculate t, scale parameters, ray intersects triangle 
  dist = edge2 * qvec;
	if (dist<0) return false;
  T inv_det = T(1.0) / det;
  dist *= inv_det;
  a *= inv_det;
  b *= inv_det;

  return true;
}

  // ray-triangle, gives barycentric coords of intersection and distance along ray.
	// Ray3 type used.
template<class T>
bool Intersection( const Ray3<T> & ray, const Point3<T> & vert0, 
				  const Point3<T> & vert1, const Point3<T> & vert2,
				  T & a ,T & b, T & dist)
{
	// small (hum) borders around triangle
	const T EPSILON2= T(1e-8); 

	const T EPSILON = T(1e-8);
	
	Point3<T> edge1 = vert1 - vert0;
	Point3<T> edge2 = vert2 - vert0;

	// determinant 
	Point3<T> pvec  = ray.Direction() ^ edge2;

	T det = edge1*pvec;

	// if determinant is near zero, ray lies in plane of triangle
  if (fabs(det) < EPSILON) return false;

  // calculate distance from vert0 to ray origin 
  Point3<T> tvec = ray.Origin()- vert0;

  // calculate A parameter and test bounds 
  a = tvec * pvec;
  if (a < -EPSILON2*det || a > det+det*EPSILON2) return false;

  // prepare to test V parameter 
  Point3<T> qvec = tvec ^ edge1;

  // calculate B parameter and test bounds 
  b = ray.Direction() * qvec ;
  if (b < -EPSILON2*det || b + a > det+det*EPSILON2) return false;

  // calculate t, scale parameters, ray intersects triangle 
  dist = edge2 * qvec;
	if (dist<0) return false;
  T inv_det = T(1.0) / det;
  dist *= inv_det;
  a *= inv_det;
  b *= inv_det;

  return true;
}

// ray-triangle, gives intersection 3d point and distance along ray
template<class T>
bool Intersection( const Line3<T> & ray, const Point3<T> & vert0, 
				  const Point3<T> & vert1, const Point3<T> & vert2,
			   Point3<T> & inte)
{

	// small (hum) borders around triangle
	const T EPSILON2= T(1e-8); 

	const T EPSILON = T(1e-8);
	
	Point3<T> edge1 = vert1 - vert0;
	Point3<T> edge2 = vert2 - vert0;

	// determinant 
	Point3<T> pvec  = ray.Direction() ^ edge2;

	T det = edge1*pvec;

	// if determinant is near zero, ray lies in plane of triangle
  if (fabs(det) < EPSILON) return false;

  // calculate distance from vert0 to ray origin 
  Point3<T> tvec = ray.Origin() - vert0;

  // calculate A parameter and test bounds 
  T a = tvec * pvec;
  if (a < -EPSILON2*det || a > det+det*EPSILON2) return false;

  // prepare to test V parameter 
  Point3<T> qvec = tvec ^ edge1;

  // calculate B parameter and test bounds 
  T b = ray.Direction() * qvec ;
  if (b < -EPSILON2*det || b + a > det+det*EPSILON2) return false;

  // calculate t, scale parameters, ray intersects triangle 
  double dist = edge2 * qvec;
	//if (dist<0) return false;
  T inv_det = 1.0 / det;
  dist *= inv_det;
  a *= inv_det;
  b *= inv_det;

	inte = vert0 + edge1*a + edge2*b;
  return true;
}

// line-box
template<class T>
bool Intersection_Line_Box( const Box3<T> & box, const Line3<T> & r, Point3<T> & coord )
{
	const int NUMDIM = 3;
	const int RIGHT  = 0;
	const int LEFT	 = 1;
	const int MIDDLE = 2;

	int inside = 1;
	char quadrant[NUMDIM];
    int i;
    int whichPlane;
    Point3<T> maxT,candidatePlane;
    
	// Find candidate planes; this loop can be avoided if
   	// rays cast all from the eye(assume perpsective view)
    for (i=0; i<NUMDIM; i++)
    {
        if(r.Origin()[i] < box.min[i])
		{
			quadrant[i] = LEFT;
			candidatePlane[i] = box.min[i];
			inside = 0;
		}
		else if (r.Origin()[i] > box.max[i])
		{
			quadrant[i] = RIGHT;
			candidatePlane[i] = box.max[i];
			inside = 0;
		}
		else
		{
			quadrant[i] = MIDDLE;
		}
    }

		// Ray origin inside bounding box
	if(inside){
	    coord = r.Origin();
	    return true;
	}

	// Calculate T distances to candidate planes 
    for (i = 0; i < NUMDIM; i++)
    {
		if (quadrant[i] != MIDDLE && r.Direction()[i] !=0.)
			maxT[i] = (candidatePlane[i]-r.Origin()[i]) / r.Direction()[i];
		else
			maxT[i] = -1.;
    }

	// Get largest of the maxT's for final choice of intersection
    whichPlane = 0;
    for (i = 1; i < NUMDIM; i++)
	    if (maxT[whichPlane] < maxT[i])
			whichPlane = i;

	// Check final candidate actually inside box 
    if (maxT[whichPlane] < 0.) return false;
    for (i = 0; i < NUMDIM; i++)
		if (whichPlane != i)
		{
			coord[i] = r.Origin()[i] + maxT[whichPlane] *r.Direction()[i];
			if (coord[i] < box.min[i] || coord[i] > box.max[i])
				return false;
		}
		else
		{
			coord[i] = candidatePlane[i];
		}
    return true;			// ray hits box
}	

// ray-box
template<class T>
bool Intersection_Ray_Box( const Box3<T> & box, const Ray3<T> & r, Point3<T> & coord )
{
	Line3<T> l;
	l.SetOrigin(r.Origin());
	l.SetDirection(r.Direction());
	return(Intersection_Line_Box<T>(box,l,coord));
}	

// segment-box return fist intersection found  from p0 to p1
template<class T>
bool Intersection_Segment_Box( const Box3<T> & box, const Segment3<T> & s, Point3<T> & coord )
{
	//as first perform box-box intersection
	Box3<T> test;
	test.Add(s.P0());
	test.Add(s.P1());
	if (!test.Collide(box))
		return false;
	else
	{
		Line3<T> l;
		Point3<T> dir=s.P1()-s.P0();
		dir.Normalize();
		l.SetOrigin(s.P0());
		l.SetDirection(dir);
		if(Intersection_Line_Box<T>(box,l,coord))
			return (test.IsIn(coord));
		return false;
	}
}	

// segment-box intersection , return number of intersections and intersection points
template<class T>
int Intersection_Segment_Box( const Box3<T> & box, const Segment3<T> & s, Point3<T> & coord0, Point3<T> & coord1 )
{
	int num=0;
	Segment3<T> test= s
	if (Intersection_Segment_Box(box,test,coord0 ))
	{
		num++;
		Point3<T> swap=test.P0();
		test.P0()=test.P1();
		test.P1()=swap;
		if (Intersection_Segment_Box(box,test,coord1 ))
			num++;
	}
	return num;
}	

template<class T>
bool Intersection (const Plane3<T> & plane0, const Plane3<T> & plane1,
                       Line3<T> & line)
{
    // If Cross(N0,N1) is zero, then either planes are parallel and separated
    // or the same plane.  In both cases, 'false' is returned.  Otherwise,
    // the intersection line is
    //
    //   L(t) = t*Cross(N0,N1) + c0*N0 + c1*N1
    //
    // for some coefficients c0 and c1 and for t any real number (the line
    // parameter).  Taking dot products with the normals,
    //
    //   d0 = Dot(N0,L) = c0*Dot(N0,N0) + c1*Dot(N0,N1)
    //   d1 = Dot(N1,L) = c0*Dot(N0,N1) + c1*Dot(N1,N1)
    //
    // which are two equations in two unknowns.  The solution is
    //
    //   c0 = (Dot(N1,N1)*d0 - Dot(N0,N1)*d1)/det
    //   c1 = (Dot(N0,N0)*d1 - Dot(N0,N1)*d0)/det
    //
    // where det = Dot(N0,N0)*Dot(N1,N1)-Dot(N0,N1)^2.

    T n00 = plane0.Direction()*plane0.Direction();
    T n01 = plane0.Direction()*plane1.Direction();
    T n11 = plane1.Direction()*plane1.Direction();
    T det = n00*n11-n01*n01;

    const T tolerance = (T)(1e-06f);
		if ( math::Abs(det) < tolerance )
        return false;

    T invDet = 1.0f/det;
    T c0 = (n11*plane0.Offset() - n01*plane1.Offset())*invDet;
    T c1 = (n00*plane1.Offset() - n01*plane0.Offset())*invDet;

    line.SetDirection(plane0.Direction()^plane1.Direction());
    line.SetOrigin(plane0.Direction()*c0+ plane1.Direction()*c1);

    return true;
}


// Ray-Triangle Functor
template <bool BACKFACETEST = true>
class RayTriangleIntersectionFunctor {
public:
	template <class TRIANGLETYPE, class SCALARTYPE>
	inline bool operator () (const TRIANGLETYPE & f, const Ray3<SCALARTYPE> & ray, SCALARTYPE & t) {
		typedef SCALARTYPE ScalarType;
		ScalarType a;
		ScalarType b;

		bool bret = Intersection(ray, Point3<SCALARTYPE>::Construct(f.P(0)), Point3<SCALARTYPE>::Construct(f.P(1)), Point3<SCALARTYPE>::Construct(f.P(2)), a, b, t);
		if (BACKFACETEST) {
			if (!bret) {
				bret = Intersection(ray, Point3<SCALARTYPE>::Construct(f.P(0)), Point3<SCALARTYPE>::Construct(f.P(2)), Point3<SCALARTYPE>::Construct(f.P(1)), a, b, t);
			}
		}
		return (bret);
	}
};


/*@}*/

} // end namespace
#endif
