/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004-2012                                           \/)\/    *
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
#include<vcg/complex/complex.h>

#include <vcg/complex/algorithms/update/bounding.h>
#include <vcg/complex/algorithms/update/topology.h>
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/update/flag.h>
#include <vcg/complex/algorithms/clustering.h>

// input output
#include <wrap/io_trimesh/import_ply.h>
#include <wrap/io_trimesh/export_ply.h>

using namespace vcg;
using namespace std;

class MyFace;
class MyVertex;

struct MyUsedTypes : public UsedTypes<	Use<MyVertex>		::AsVertexType,
																				Use<MyFace>			::AsFaceType>{};

class MyVertex  : public Vertex< MyUsedTypes, vertex::Coord3f, vertex::Normal3f, vertex::BitFlags  >{};
class MyFace    : public Face < MyUsedTypes, face::VertexRef, face::Normal3f, face::BitFlags > {};
class MyMesh    : public vcg::tri::TriMesh< vector<MyVertex>, vector<MyFace> > {};

int  main(int argc, char **argv)
{
 if(argc<3)
	{
		printf(
		"\n                  trimesh_clustering ("__DATE__")\n"
			"						Visual Computing Group I.S.T.I. C.N.R.\n"
      "Usage: PlyRefine filein.ply fileout.ply [opt] \n"
      "options: \n"
      "-k cellnum     approx number of cluster that should be defined; (default 10e5)\n"
      "-s size        in absolute units the size of the clustering cell (override the previous param)\n"
			"-d             enable the duplication of faces for double surfaces\n"
			);
		exit(0);
	}

  int i=3;  
  int CellNum=100000; 
  float CellSize=0; 
  bool DupFace=false;

	while(i<argc)
		{
			if(argv[i][0]!='-')
				{printf("Error unable to parse option '%s'\n",argv[i]); exit(0);}
			switch(argv[i][1])
			{				
				case 'k' :	CellNum=atoi(argv[i+1]); ++i; printf("Using %i clustering cells\n",CellNum); break;
				case 's' :	CellSize=atof(argv[i+1]); ++i; printf("Using %5f as clustering cell size\n",CellSize); break;
				case 'd' :	DupFace=true; printf("Enabling the duplication of faces for double surfaces\n"); break;

				default : {printf("Error unable to parse option '%s'\n",argv[i]); exit(0);}
			}
			++i;
		}

	MyMesh m;

	if(vcg::tri::io::ImporterPLY<MyMesh>::Open(m,argv[1])!=0)
		{
      printf("Error reading file  %s\n",argv[1]);
			exit(0);
		}
  vcg::tri::UpdateBounding<MyMesh>::Box(m);
  vcg::tri::UpdateNormal<MyMesh>::PerFace(m);
  printf("Input mesh  vn:%i fn:%i\n",m.vn,m.fn);
  vcg::tri::Clustering<MyMesh, vcg::tri::AverageColorCell<MyMesh> > Grid;
  Grid.DuplicateFaceParam=DupFace;
  Grid.Init(m.bbox,CellNum,CellSize);
  
  printf("Clustering to %i cells\n",Grid.Grid.siz[0]*Grid.Grid.siz[1]*Grid.Grid.siz[2] );
  printf("Grid of %i x %i x %i cells\n",Grid.Grid.siz[0],Grid.Grid.siz[1],Grid.Grid.siz[2]);
  printf("with cells size of %.2f x %.2f x %.2f units\n",Grid.Grid.voxel[0],Grid.Grid.voxel[1],Grid.Grid.voxel[2]);
  
  int t0=clock();
  Grid.AddMesh(m);
  int t1=clock();
  Grid.ExtractMesh(m);
  int t2=clock();
  printf("Output mesh vn:%i fn:%i\n",m.vn,m.fn);
  printf("Simplified in :%i msec (%i+%i)\n",t2-t0,t1-t0,t2-t1);
	
  vcg::tri::io::PlyInfo pi;
	vcg::tri::io::ExporterPLY<MyMesh>::Save(m,argv[2],pi.mask);
	return 0;
	}
