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
Revision 1.1  2004/08/26 18:03:48  ponchio
First draft.


****************************************************************************/

#ifdef WIN32
#include "getopt.h"
#else
#include <unistd.h>
#endif

#include <iostream>
using namespace std;

#include "crude.h"
#include "nexus.h"

#include "voronoichain.h"
#include "pintersect.h"
#include "vert_remap.h"
using namespace vcg;
using namespace nxs;

struct RemapLink {
  unsigned int rel_vert;
  unsigned int patch;
  unsigned int abs_vert;
};

/*void RemapFaces(Crude &crude, 
		VoronoiChain &vchain,
                VFile<unsigned int> &face_remap,
		vector<unsigned int> &patch_faces);*/

void RemapVertices(Crude &crude,
		   VertRemap &vert_remap,
		   VFile<unsigned int> &face_remap,	 
		   vector<unsigned int> &patch_verts);

void NexusAllocate(Crude &crude,
		   Nexus &nexus,
		   VFile<unsigned int> &face_remap,
		   vector<unsigned int> &patch_faces,
		   vector<unsigned int> &patch_verts);

void NexusFill(Crude &crude,
	       Nexus &nexus,
	       VertRemap &vert_remap,
	       VFile<RemapLink> &border_remap);

void NexusFixBorder(Nexus &nexus, 
		    VFile<RemapLink> &border_remap);

void NexusSplit(Nexus &nexus, VoronoiChain &vchain,
		unsigned int level,
		vector<Point3f> &newvert, 
		vector<unsigned int> &newface,
		vector<Link> &newbord,
		Nexus::Update &update, 
		float error);

float Decimate(unsigned int target_faces, 
	       vector<Point3f> &newvert, 
	       vector<unsigned int> &newface,
	       vector<Link> &newbord,
	       vector<int> &vert_remap);

void ReverseHistory(vector<Nexus::Update> &history);

int main(int argc, char *argv[]) {

  unsigned int patch_size = 1000;
  unsigned int patch_threshold = 200;
  unsigned int max_level = 0xffffffff;
  float scaling = 0.5;

  int option;
  while((option = getopt(argc, argv, "f:t:l:s:")) != EOF) {
    switch(option) {
    case 'f': patch_size = atoi(optarg);
      if(patch_size == 0 || patch_size > 32000) {
	cerr << "Invalid number of faces per patch: " << optarg << endl;
	return -1;
      }
      break;
    case 't': patch_threshold = atoi(optarg);
      if(patch_threshold == 0 || patch_threshold > patch_size) {
	cerr << "Invalid patch threshold: " << optarg << endl;
	return -1;
      }
      break;
    case 'l': max_level = atoi(optarg);
      if(max_level == 0) {
	cerr << "Invalid number of levels: " << optarg << endl;
	return -1;
      }
      break;
    case 's': scaling = atof(optarg);
      if(scaling <= 0 || scaling >= 1) {
	cerr << "Invalid scaling: " << optarg << endl;
	cerr << "Must be 0 < scaling < 1" << endl;
      }
      break;
    default: cerr << "Unknown option: " << (char)option << endl;
      return -1;
    }
  }

  //Test there are still 2 arguments
  if(optind != argc - 2) {
    cerr << "Usage: " << argv[0] << " <crude> <output> [options]\n";
    cerr << "  Options:\n";
    cerr << " -f N: use N faces per patch (default 1000, max 32000)\n";
    cerr << " -t N: mini faces per patch (default 200)\n";
    cerr << " -l N: number of levels\n";
    cerr << " -s F: scaling factor (0 < F < 1, default 0.5)\n\n";
    return -1;
  }

  Crude crude;
  if(!crude.Load(argv[optind])) {
    cerr << "Could not open crude input: " << argv[optind] << endl;
    return -1;
  }
  
  if(patch_size > crude.vert.Size()/2) {
    cerr << "Patch size too big: " << patch_size << " ~ " << crude.vert.Size()
	 << endl;
    return -1;
  }

  string output = argv[optind+1];

  Patch::Signature signature = Patch::DEFAULT;
  Nexus nexus;
  if(!nexus.Create(output, signature)) {
    cerr << "Could not create nexus output: " << output << endl;
    return -1;
  }

  VoronoiChain vchain;
  vchain.Initialize(patch_size, patch_threshold);
  vchain.scaling = scaling;

  //Now building level 0 of the Nexus

  VFile<unsigned int> face_remap;
  if(!face_remap.Create(output + ".rmf")) {
    cerr << "Could not create remap files: " << output << ".frm\n";
    return -1;
  }
  face_remap.Resize(crude.Faces());


  VertRemap vert_remap;
  if(!vert_remap.Create(output)) {
    cerr << "Could not create remap files: " << output << ".rmv and .rmb\n";
    return -1;
  }
  vert_remap.Resize(crude.Vertices());

  VFile<RemapLink> border_remap;
  if(!border_remap.Create(output + string(".tmp"))) {
    cerr << "Could not create temporary border remap file\n";
    return -1;
  }

  /* BUILDING FIRST LEVEL */

  //Remapping faces and vertices using level 0 and 1 of the chain

  vector<unsigned int> patch_faces;
  vchain.RemapFaces(crude, face_remap, patch_faces);
  
  vector<unsigned int> patch_verts;
  patch_verts.resize(patch_faces.size(), 0);
  RemapVertices(crude, vert_remap, face_remap, patch_verts);
  

  //allocate chunks for patches and copy faces (absoklute indexing) into them.
  NexusAllocate(crude, nexus, face_remap, patch_faces, patch_verts);

  //insert vertices and remap faces, prepare borders
  NexusFill(crude, nexus, vert_remap, border_remap);

  NexusFixBorder(nexus, border_remap);

  //filling history
  Nexus::Update update;
  for(unsigned int i = 0; i < nexus.index.size(); i++) 
    update.created.push_back(i);
  nexus.history.push_back(update);

  /* BUILDING OTHER LEVELS */
  unsigned int oldoffset = 0;

  for(unsigned int level = 1; level < max_level; level++) {
    cerr << "Level: " << level << endl;

    unsigned int newoffset = nexus.index.size();
    vchain.BuildLevel(nexus, oldoffset);
    
    map<unsigned int, set<unsigned int> >::iterator fragment;
    for(fragment = vchain.oldfragments.begin(); 
	fragment != vchain.oldfragments.end(); fragment++) {


      update.created.clear();
      update.erased.clear();

      cerr << "Joining: ";
      set<unsigned int> &fcells = (*fragment).second;
      set<unsigned int>::iterator s;
      for(s = fcells.begin(); s != fcells.end(); s++) {
	update.erased.push_back(*s);
	cerr << " " << (*s) << endl;
      }
      cerr << endl;
      
      vector<Point3f> newvert;
      vector<unsigned int> newface;
      vector<Link> newbord;
      
      nexus.Join((*fragment).second, newvert, newface, newbord);


      //simplyfy mesh
      vector<int> vert_remap;
      float error = Decimate(newface.size() * scaling/3, newvert, 
			     newface, newbord, vert_remap);


      NexusSplit(nexus, vchain, level, newvert, newface, newbord, 
		 update, error);


      nexus.history.push_back(update);
    }

    //if(vchain.levels.back().size() == 1) break;
    if(vchain.oldfragments.size() == 1) break;

    vchain.oldfragments = vchain.newfragments;
    oldoffset = newoffset;
  }
  //last level clean history:
  update.created.clear();
  update.erased.clear();
  map<unsigned int, set<unsigned int> >::iterator fragment;
  for(fragment = vchain.newfragments.begin(); 
      fragment != vchain.newfragments.end(); fragment++) {
    set<unsigned int> &fcells = (*fragment).second;
    set<unsigned int>::iterator s;
    for(s = fcells.begin(); s != fcells.end(); s++)
      update.erased.push_back(*s);
  }
  nexus.history.push_back(update);
  ReverseHistory(nexus.history);
  //debug:
  for(unsigned int i = 0; i < nexus.history.size(); i++) {
    Nexus::Update &update = nexus.history[i];
    cerr << "created:";
    for(unsigned int c = 0; c < update.created.size(); c++)
      cerr << " " << update.created[c];
    cerr << "\nerased:";
    for(unsigned int c = 0; c < update.erased.size(); c++)
      cerr << " " << update.erased[c];
    cerr << "\n\n";
  }

  //Clean up:
  nexus.Close();

  //TODO remove vert_remap, face_remap, border_remap
  return 0;
}

void RemapVertices(Crude &crude,
		   VertRemap &vert_remap,
		   VFile<unsigned int> &face_remap,	 
		   vector<unsigned int> &patch_verts) {

  for(unsigned int i = 0; i < crude.Faces(); i++) {
    Crude::Face &face = crude.GetFace(i);
    unsigned int patch = face_remap[i];
    for(int k = 0; k < 3; k++) {
      set<unsigned int> pp;
      vert_remap.GetValues(face[k], pp);
      if(!pp.count(patch)) {
	vert_remap.Insert(face[k], patch);      
	patch_verts[patch]++;
      }
    }
  }
}

void NexusAllocate(Crude &crude,
		   Nexus &nexus,
		   VFile<unsigned int> &face_remap,
		   vector<unsigned int> &patch_faces,
		   vector<unsigned int> &patch_verts) {
  
  
  nexus.index.resize(patch_faces.size());
  
  unsigned int totchunks = 0;
  //now that we know various sizes, lets allocate space
  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Nexus::Entry &entry = nexus.index[i];
    
    if(patch_faces[i] == 0 || patch_verts[i] == 0) 
      cerr << "Warning! Empty patch.\n";

    entry.patch_start = totchunks;
    entry.patch_size = Patch::ChunkSize(nexus.signature,
					patch_verts[i], patch_faces[i]);
    
    totchunks += entry.patch_size;
    entry.border_start = 0xffffffff;
    entry.nvert = patch_verts[i];
    entry.nface = 0;
    entry.error = 0;
  }

  nexus.patches.Resize(totchunks);


  //now we sort the faces into the patches (but still using absolute indexing
  //instead of relative indexing
  for(unsigned int i = 0; i < crude.face.Size(); i++) {
    Crude::Face &face = crude.face[i];
    unsigned int npatch = face_remap[i];
    
    Nexus::Entry &entry = nexus.index[npatch];
    Patch patch = nexus.GetPatch(npatch);
    
    Crude::Face *faces = (Crude::Face *)patch.VertBegin();
    faces[entry.nface] = face;
    entry.nface++;
  }
}


void NexusFill(Crude &crude,
	       Nexus &nexus,
	       VertRemap &vert_remap,
	       VFile<RemapLink> &border_remap) {
  

  //finally for every patch we collect the vertices
  //and fill the patch.
  //we need to remember start and size in border_remap;
  //  vector<unsigned int> border_start;
  //  vector<unsigned int> border_size;

  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Patch patch = nexus.GetPatch(i);
    Nexus::Entry &entry = nexus.index[i];

    //make a copy of faces (we need to write there :P)
    Crude::Face *faces = new Crude::Face[patch.FaceSize()];
    memcpy(faces, (Crude::Face *)patch.VertBegin(), 
	   patch.FaceSize() * sizeof(Crude::Face));

    //collect all vertices we need.
    //TODO an hash_map would be faster?
    unsigned int count = 0;
    map<unsigned int, unsigned short> remap;
    for(unsigned int k = 0; k < patch.FaceSize(); k++) {
      Crude::Face &face = faces[k];
      
      for(int j = 0; j < 3; j++) {
        if(!remap.count(face[j])) {          
	  assert(count < patch.VertSize());
	  Point3f &v = crude.vert[face[j]];
          patch.VertBegin()[remap.size()] = v;
	  entry.sphere.Add(v);
          remap[face[j]] = count++;
        }
	patch.FaceBegin()[k*3 + j] = remap[face[j]];
      }
    }
    assert(count == remap.size());
    assert(entry.nvert == remap.size());

    //record start of border:
    entry.border_start = border_remap.Size();

    //TODO hash_set?
    set<unsigned int> border_patches;
    map<unsigned int, unsigned short>::iterator m;
    for(m = remap.begin(); m != remap.end(); m++) {
      RemapLink link;
      link.abs_vert = (*m).first;
      link.rel_vert = (*m).second;

      vert_remap.GetValues(link.abs_vert, border_patches);
      assert(border_patches.size() >= 1);
      if(border_patches.size() == 1) continue; //its not a border

      set<unsigned int>::iterator s;
      for(s = border_patches.begin(); s != border_patches.end(); s++) {
	if((*s) == i) continue; 
	link.patch = *s;
	border_remap.PushBack(link);
      }
    }
    //and number of borders:
    entry.border_size = border_remap.Size() - entry.border_start;
    delete []faces;
  }

  //we can now update bounding sphere.
  for(unsigned int i = 0; i < nexus.index.size(); i++) 
    nexus.sphere.Add(nexus.index[i].sphere);
}

void NexusFixBorder(Nexus &nexus, 
		    VFile<RemapLink> &border_remap) {

  //and last convert RemapLinks into Links
  nexus.borders.Resize(border_remap.Size());

  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Nexus::Entry &local = nexus.index[i];

    // K is the main iterator (where we write to in nexus.borders)
    for(unsigned int k = local.border_start;
	k < local.border_start + local.border_size; k++) {
      
      RemapLink start_link = border_remap[k];
      assert(start_link.rel_vert < local.nvert);

      Nexus::Entry &remote = nexus.index[start_link.patch];

      bool found = false;
      for(unsigned int j = remote.border_start;
	  j < remote.border_start + remote.border_size; j++) {
	
	RemapLink end_link = border_remap[j];
	assert(end_link.rel_vert < remote.nvert);

	if(start_link.abs_vert == end_link.abs_vert &&
	   end_link.patch == i) { //found the match
	  assert(!found);
	  nexus.borders[k] = Link(start_link.rel_vert, 
				  end_link.rel_vert, start_link.patch);
	  found = true;
	}
      }
      assert(nexus.borders[k].start_vert < local.nvert);
      assert(found);
    }
  }
  nexus.borders.Flush();

  //Checking border consistency:
  /*  for(unsigned int i = 0; i < nexus.index.size(); i++) {
    Border border = nexus.GetBorder(i);
    Nexus::Entry &entry = nexus.index[i];
    for(unsigned int k = 0; k < border.Size(); k++) {
      Link &link = border[k];
      if(link.start_vert >= entry.nvert) {
	cerr << "K: " << k << endl;
	cerr << "patch: " << i << " nvert: " << entry.nvert << " startv: " 
	     << link.start_vert << endl;
	cerr << "bstart: " << entry.border_start 
	     << "bsize: " << entry.border_size << endl;
      }
      assert(link.end_patch < nexus.index.size());
      assert(link.start_vert < entry.nvert);
      Nexus::Entry &remote = nexus.index[link.end_patch];
      assert(link.end_vert < remote.nvert);
    }
    
    }*/
}


void NexusSplit(Nexus &nexus, VoronoiChain &vchain,
		unsigned int level,
		vector<Point3f> &newvert, 
		vector<unsigned int> &newface,
		vector<Link> &newbord, 
		Nexus::Update &update,
		float error) {

  //if != -1 remap global index to cell index
  map<unsigned int, vector<int> > vert_remap;
  map<unsigned int, unsigned int> vert_count;
  
  //simply collects faces
  map<unsigned int, vector<int> > face_remap;
  map<unsigned int, unsigned int> face_count;
  
  for(unsigned int f = 0; f < newface.size(); f += 3) {
    Point3f bari = (newvert[newface[f]] + 
		    newvert[newface[f+1]] + 
		    newvert[newface[f+2]])/3;
    
    unsigned int cell = vchain.Locate(level+1, bari);
    vector<int> &f_remap = face_remap[cell];
    f_remap.push_back(newface[f]);
    f_remap.push_back(newface[f+1]);
    f_remap.push_back(newface[f+2]);
    face_count[cell]++;
    
    if(!vert_remap.count(cell)) {
      vert_remap[cell].resize(newvert.size(), -1);
      vert_count[cell] = 0;
    }
    
    vector<int> &v_remap = vert_remap[cell];
    
    for(int i = 0; i < 3; i++)
      if(v_remap[newface[f+i]] == -1)
	v_remap[newface[f+i]] = vert_count[cell]++;
  }
  
  
  //TODO prune small count cells 
  
  //for every new cell
  map<unsigned int, unsigned int >::iterator c;
  for(c = vert_count.begin(); c != vert_count.end(); c++) {
    unsigned int cell = (*c).first;
    cerr << "Processing cell: " << cell << endl;
    
    vector<unsigned short> faces;
    vector<Point3f> verts;
    vector<Link> bords;
    
    vector<int> &v_remap = vert_remap[cell];
    vector<int> &f_remap = face_remap[cell];
    
    verts.resize(vert_count[cell]);
    for(unsigned int i = 0; i < newvert.size(); i++) {
      if(v_remap[i] != -1)
	verts[v_remap[i]] = newvert[i];
      //	    verts.push_back(newvert[v_remap[i]]);
    }
    
    assert(verts.size() == vert_count[cell]);
    assert(f_remap.size()/3 == face_count[cell]);
    
    faces.resize(face_count[cell]*3);
    
    for(unsigned int i = 0; i < f_remap.size(); i++) {
      assert(v_remap[f_remap[i]] != -1);
      faces[i] = v_remap[f_remap[i]];
    }
    //process external borders
    for(unsigned int i = 0; i < newbord.size(); i++) {
      Link link = newbord[i];
      if(v_remap[link.start_vert] == -1) continue;
      link.start_vert = v_remap[link.start_vert];
      bords.push_back(link);
    }
    
    //process internal borders;
    //TODO higly inefficient!!!
    map<unsigned int, unsigned int >::iterator t;  
    for(t = vert_count.begin(); t != vert_count.end(); t++) {
      if(cell == (*t).first) continue;
      vector<int> &vremapclose = vert_remap[(*t).first];
      for(unsigned int i = 0; i < newvert.size(); i++) {
	if(v_remap[i] != -1 && vremapclose[i] != -1) {
	  Link link;
	  link.end_patch = (*t).first;
	  link.start_vert = v_remap[i];
	  link.end_vert = vremapclose[i];
	  bords.push_back(link);
	}
      }
    }
    //create new nexus patch
    unsigned int patch_idx = nexus.AddPatch(verts.size(),
					    faces.size()/3,
					    bords.size());
    vchain.newfragments[cell].insert(patch_idx);
    update.created.push_back(patch_idx);

    Nexus::Entry &entry = nexus.index[patch_idx];
    //    entry.error = error;
    entry.error = level;
    
    Patch patch = nexus.GetPatch(patch_idx);
    memcpy(patch.FaceBegin(), &faces[0], 
	   faces.size() * sizeof(unsigned short));
    memcpy(patch.VertBegin(), &verts[0], verts.size() * sizeof(Point3f));
    
    
    for(int v = 0; v < verts.size(); v++) {
      entry.sphere.Add(verts[v]);
      nexus.sphere.Add(verts[v]);
    } 
    
    Border border = nexus.GetBorder(patch_idx);
    memcpy(&border[0], &bords[0], bords.size());
  }  
  cerr << "newfrag: " << vchain.newfragments.size() << endl;
}
 
void ReverseHistory(vector<Nexus::Update> &history) {
  reverse(history.begin(), history.end());
  vector<Nexus::Update>::iterator i;
  for(i = history.begin(); i != history.end(); i++)
    swap((*i).erased, (*i).created);
}
