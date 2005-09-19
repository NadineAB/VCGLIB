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
Revision 1.7  2005/06/15 11:44:47  pietroni
minor changes

Revision 1.6  2005/06/01 13:47:59  pietroni
resolved hash code conflicts

Revision 1.5  2005/03/15 09:50:44  ganovelli
there was a debug line, now removed

Revision 1.4  2005/03/14 15:11:18  ganovelli
ClosestK added and other minor changes

Revision 1.3  2005/03/11 15:25:29  ganovelli
added ClosersIterator and other minor changes. Not compatible with the previous version.
Still other modifications to do (temporary commit)

Revision 1.2  2005/02/21 12:13:25  ganovelli
added vcg header



****************************************************************************/

#ifndef VCGLIB_SPATIAL_HASHING
#define VCGLIB_SPATIAL_HASHING


#define P0 73856093
#define P1 19349663
#define P2 83492791

#include <vcg/space/index/grid_util.h>
//#include <map>
#include <vector>
#include <algorithm>
#ifdef WIN32
#include <hash_map>
#define STDEXT stdext
#else
#include <ext/hash_map>
#define STDEXT __gnu_cxx
#endif


namespace vcg{
	/** Spatial Hash Table
	Spatial Hashing as described in
	"Optimized Spatial Hashing for Coll	ision Detection of Deformable Objects", 
	Matthias Teschner and Bruno Heidelberger and Matthias Muller and Danat Pomeranets and Markus Gross
	*/
	template < typename ContainerType,class FLT=float >
	class SpatialHashTable:public BasicGrid<FLT>
	{

	public:

		typedef typename ContainerType::value_type ObjType;
		typedef ObjType* ObjPtr;
		typedef typename ObjType::ScalarType ScalarType;
		typedef Point3<ScalarType> CoordType;


		//type of container of pointer to object in a Cell
		//typedef typename std::pair<ObjType*,int> EntryType ;
		class  EntryType : public std::pair<ObjType*,int>
		{
		public:
			EntryType(ObjType* sim,const int  &_tempMark)
			{
				first=sim;
				second=_tempMark;
			}

			ObjType& operator *(){return (*this->first);}
		};

		//typedef typename std::map<ObjType*,int> CellContainerType;
		typedef typename std::vector<EntryType> CellContainerType;
		typedef typename CellContainerType::iterator IteMap;
		typedef typename EntryType* CellIterator;

		////iterator to elems on a cell (need to define method Elem() for Space Iterators)
		//class CellIterator :public IteMap
		//{
		//	//std::vector<int>::iterator i;
		//public:
		//	CellIterator(){}
		//	inline CellIterator & operator = ( IteMap const &c) 
		//	{this->); };
		//	ObjType* operator *(){return ((* this)->first);}
		//};

			//This Class Identify the cell
		struct Cell
		{
		private:

			///the min and max point corresponding to the cell - used to inverse hashing
			//CoordType min;//coordinate min of the cell
			//CoordType max;//coordinate max of the cell

			Point3i cell_n;//cell number
			//iterator to the map element into the cell
			//typedef typename CellContainerType::iterator IteMap;

		public:

			//elements 
			CellContainerType _entries;

			Cell()
			{}

			Cell(ObjType* sim,Point3i _cell,const int  &_tempMark)
			{
				_entries.push_back(EntryType(sim,_tempMark));
				/*min=_min;
				max=_max;
				assert(min<max);*/
				cell_n=_cell;
			}

			/////return true if the element is in the cell
			//bool IsIn(ObjType* sim)
			//{
			//	int n=elem.count(sim);
			//	assert((n==0)||(n==1));
			//	return (n==1);
			//}

			///return the number of elements stored in the cell
			int Size()
			{return (int)(_entries.size());}

			///find the simplex into the cell
			bool Find(ObjType* sim,IteMap &I)
			{
			for (I=_entries.begin();I<_entries.end();I++)
				if ((*I).first==sim)
					return true;
			return false;
			}

			///update or insert an element into a cell
			void Update(ObjType* sim, const int & _tempMark)
			{
				IteMap I;
				if (Find(sim,I))///update temporary mark
					(*I).second=_tempMark;
				else
					_entries.push_back(EntryType(sim,_tempMark));

				//at the end update the temporary mark on the simplex
				sim->Mark()=_tempMark;
				//Assert();
			}

			///given an iterator to the instance of the entry in the cell
			///return true if the the entry is valid
			///(using temporary mark).
			bool IsUpdated(IteMap &I)
			{
				return ((*I).second >= (*I).first->Mark());
			}

			///given an simplex pointer
			///return true if the the entry corripondent to that 
			///simplex is valid or not
			///(using temporary mark).
			bool IsUpdated(ObjType* sim)
			{
				IteMap I;
				if (Find(sim,I))
					return(IsUpdated(I));
				else
					return false;
			}

			Point3i CellN()
			{return cell_n;}

			bool operator ==(const Cell &h)  
			{return (cell_n==h.CellN());}

			bool operator !=(const Cell &h)  
			{return ((cell_n!=h.CellN()));}


		}; // end struct Cell

		////This Class Identify the cell
		//struct Cell
		//{
		//private:

		//	///the min and max point corresponding to the cell - used to inverse hashing
		//	//CoordType min;//coordinate min of the cell
		//	//CoordType max;//coordinate max of the cell

		//	Point3i cell_n;//cell number
		//	//iterator to the map element into the cell
		//	//typedef typename CellContainerType::iterator IteMap;

		//public:

		//	//elements 
		//	CellContainerType _entries;

		//	Cell()
		//	{}

		//	Cell(ObjType* sim,Point3i _cell,const int  &_tempMark)
		//	{
		//		_entries.insert(EntryType(sim,_tempMark));
		//		/*min=_min;
		//		max=_max;
		//		assert(min<max);*/
		//		cell_n=_cell;
		//	}

		//	///return true if the element is in the cell
		//	bool IsIn(ObjType* sim)
		//	{
		//		int n=elem.count(sim);
		//		assert((n==0)||(n==1));
		//		return (n==1);
		//	}

		//	///return the number of elements stored in the cell
		//	int Size()
		//	{return (int)(_entries.size());}

		//	///update or insert an element into a cell
		//	void Update(ObjType* sim, const int & _tempMark)
		//	{
		//		IteMap I=_entries.find(sim);

		//		if (I!=_entries.end())//the entry exist in the cell
		//			(*I).second=_tempMark;
		//		else
		//			_entries.insert(_entries.begin(),EntryType(sim,_tempMark));

		//		//at the end update the temporary mark on the simplex
		//		sim->Mark()=_tempMark;
		//		//Assert();
		//	}

		//	///given an iterator to the instance of the entry in the cell
		//	///return true if the the entry is valid
		//	///(using temporary mark).
		//	bool IsUpdated(CellIterator &I)
		//	{
		//		return ((*I).second >= (*I).first->Mark());
		//	}

		//	///given an simplex pointer
		//	///return true if the the entry corripondent to that 
		//	///simplex is valid or not
		//	///(using temporary mark).
		//	bool IsUpdated(ObjType* sim)
		//	{
		//		IteMap I=_entries.find(sim);
		//		if (I!=_entries.end())
		//			return(IsUpdated(I));
		//		else
		//			return false;
		//	}

		//	////add to the vector all simplexes of the map that have a right timestamp or are not deleted
		//	//void  Elems(std::vector<ObjType*> & res)
		//	//{
		//	//	for (IteMap ite=_entries.begin();ite!=_entries.end();ite++)
		//	//	{
		//	//		ObjType* sim=(*ite).first;
		//	//		if (IsUpdated(ite)&&(!sim->IsD()))
		//	//			res.push_back(sim);
		//	//	}
		//	//}


		//	Point3i CellN()
		//	{return cell_n;}

		//	bool operator ==(const Cell &h)  
		//	{return (cell_n==h.CellN());}

		//	bool operator !=(const Cell &h)  
		//	{return ((cell_n!=h.CellN()));}


		//}; // end struct Cell


		//hash table definition
		typedef typename STDEXT::hash_multimap<int,Cell> Htable;
		//record of the hash table
		typedef typename std::pair<int,Cell> HRecord;
		//iterator to the hash table
		typedef typename Htable::iterator IteHtable;

		SpatialHashTable(){HashSpace=1000;};//default value for hash_space
		~SpatialHashTable(){};

		int tempMark;

	protected:

		Htable hash_table;
		
		///number of possible hash code [0...HashSpace]
		int HashSpace;

		///number of conflicts created
		int conflicts;

		///insert a new cell
		void _InsertNewHentry(ObjType* s,Point3i cell)
		{
			int h=Hash(cell);
			hash_table.insert(HRecord(h,Cell(s,cell,tempMark)));
			s->Mark()=tempMark;
			//Assert();	
		}
		
		///return true and return the iterator to the cell if exist
		bool _IsInHtable(Point3i cell,IteHtable &result)
		{
			int h=Hash(cell);
			int count=hash_table.count(h);
			if (count==0)///in this case there is no entry for that key
				return false;
			else
			{
				////std::pair<Htable::const_iterator, Htable::const_iterator> p =hash_table.equal_range(h);
				std::pair<IteHtable, IteHtable> p =hash_table.equal_range(h);
				IteHtable i = p.first;

				while((i != p.second)&&((*i).second.CellN()!=cell))++i;

				if (i==p.second)///the scan is terminated and we have not found the right cell
				{
					conflicts++;
					return false;
				}
				else	///we have found the right cell
				{
					result=i;
					return true;
				}
			}
		}

		///insert an element in a specified cell if the cell doesn't exist than
		///create it.
		void _InsertInCell(ObjType* s,Point3i cell)
		{
			IteHtable I;
			if (!_IsInHtable(cell,I))
				_InsertNewHentry(s,cell);
			else///there is the entry specified by the iterator I so update only the temporary mark
				(*I).second.Update(s,tempMark);
		}

		// hashing
		const int Hash(Point3i p) const
		{
			return ((p.V(0)*P0 ^ p.V(1)*P1 ^ p.V(2)*P2)%HashSpace);
		}

		
	public:
		
		///We need some extra space for numerical precision.
		template <class Box3Type>
		 void SetBBox( const Box3Type & b )
		{
			bbox.Import( b );
			ScalarType t = bbox.Diag()/100.0;
			if(t == 0) t = ScalarType(1e20);  // <--- Some doubts on this (Cigno 5/1/04)
			bbox.Offset(t);
			dim  = bbox.max - bbox.min;
		}
		
		virtual vcg::Box3i Add( ObjType* s)
		{
			/*std::vector<Point3i> box;
			BoxCells(s->BBox().min,s->BBox().max,box);
			for (std::vector<Point3i>::iterator bi=box.begin();bi<box.end();bi++)*/
			Box3<ScalarType> b;
			s->GetBBox(b);
			vcg::Box3i bb;
			BoxToIBox(b,bb);
			//then insert all the cell of bb
			for (int i=bb.min.X();i<=bb.max.X();i++)
				for (int j=bb.min.Y();j<=bb.max.Y();j++)
					for (int k=bb.min.Z();k<=bb.max.Z();k++)
						_InsertInCell(s,vcg::Point3i(i,j,k));

			return bb;
		}

		/// Insert a mesh in the grid.SetBBox() function must be called before
		/// Hash space is cardinality of hash key set
		void Set( ContainerType & s)
		{
			Set(s,s.size());
		}

		/// Insert a mesh in the grid.SetBBox() function must be called before
		void Set( ContainerType & s,int _size )
		{
			Point3i _siz;
			BestDim( _size, dim, _siz );
			Set(s,_siz);
		}
		
		/// Insert a mesh in the grid.SetBBox() function must be called before
		void Set(ContainerType & s, Point3i _siz)
		{	
			siz=_siz;
			// find voxel size
			voxel[0] = dim[0]/siz[0];
			voxel[1] = dim[1]/siz[1];
			voxel[2] = dim[2]/siz[2];
			typename ContainerType::iterator i; 
			for(i = s.begin(); i!= s.end(); ++i)
				Add(&(*i));
		}
		

		/////initialize the structure HashSpace is one estimation about 
		/////how many keys the system have to generate in order to obtain as less
		/////conflicts as possible
		//void Init(CoordType _min,CoordType _max,ScalarType _l,int HashSpace=1000)
		//{
		//	l=_l;
		//	min=_min;
		//	max=_max;
		//	HashSpace=HashSpace;
		//	tempMark=0;
		//	conflicts=0;
		//}	

		

		/*void AddElem( ObjType* s)
		{
		std::vector<Point3i> box;
		BoxCells(s->BBox().min,s->BBox().max,box);
		for (std::vector<Point3i>::iterator bi=box.begin();bi<box.end();bi++)
		_InsertInCell(s,*bi);
		}*/
		
		///return the simplexes of the cell that contain p
		void Grid( const Point3d & p, CellIterator & first, CellIterator & last )
		{
			IteHtable I;
			vcg::Point3i _c;
			PToIP(p,_c);
			Grid(_c,first,last);
		}

		///return the simplexes on a specified cell 
		void Grid( int x,int y,int z, CellIterator & first, CellIterator & last )
		{
			Grid(vcg::Point3i(x,y,z),first,last);
		}

		///return the simplexes on a specified cell 
		void Grid( const Point3i & _c, CellIterator & first, CellIterator & last )
		{
			IteHtable I;
			if (_IsInHtable(_c,I))//if there is the cell then
			{	///return pointers to first and last element cell elems
				first= &*(*I).second._entries.begin();
				last=  &*(*I).second._entries.end();
			}
			else 
			{	///return 2 equals pointers
				first=&*(*hash_table.begin()).second._entries.begin();
				last= &*(*hash_table.begin()).second._entries.begin();
			}
		}

		void getAtCell(Point3i _c,std::vector<ObjType*> & res)
		{
			IteHtable I;
			if (_IsInHtable(_c,I))//if there is the cell then
				(*I).second.Elems(res);
		}


		std::vector<Point3i> Cells(ObjType *s)
		{
			return BoxCells(s,s->BBox().min,s->BBox().max);
		}

		/*inline Point3i MinCell()
		{
			return PointToCell(min);
		}

		inline Point3i MaxCell()
		{
			return PointToCell(max);
		}*/

		///return the number of elemnts in the cell and the iterator to the cell
		///if the cell exist
		int numElemCell(Point3i _c,IteHtable &I)
		{
			if (_IsInHtable(_c,I))
				return ((*I).second.Size());
			else
				return 0;
		}

		///return the number of cell created
		int CellNumber()
		{return (hash_table.size());}

		int Conflicts()
		{return conflicts;}

		void Clear()
		{
			hash_table.clear();
		}

		void SetHashSpace()
		{
			HashSpace
		}

		void UpdateTmark()
		{tempMark++;}


	}; // end class

}// end namespace

#undef P0
#undef P1
#undef P2

#endif
