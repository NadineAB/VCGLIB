#include <iostream>
#include "nexus.h"

using namespace std;
using namespace vcg;
using namespace nxs;

Nexus::Nexus(): index_file(NULL) {}
Nexus::~Nexus() {
  Close();
}

bool Nexus::Create(const string &file, Patch::Signature sig) {
  index_file = fopen((file + ".nxs").c_str(), "wb+");
  if(!index_file) {
    cerr << "Could not create file: " << file << ".nxs\n";
    return false;
  }

  signature = sig;
  totvert = 0;
  totface = 0;
  sphere = Sphere3f();
  
  index.clear();

  //Important: chunk_size must be 1 so that i can use Region in VFile.
  if(!patches.Create(file + ".nxp", 1)) {
    cerr << "Could not create file: " << file << ".nxp" << endl;
    return false;
  }
  if(!borders.Create(file + ".nxb", 1)) {
    cerr << "Could not create file: " << file << ".nxb" << endl;
    return false;
  }
  history.clear();
  return true;
}

bool Nexus::Load(const string &file) {
  index_file = fopen((file + ".nxs").c_str(), "rb+");
  if(!index_file) return false;
  
  unsigned int readed;
  readed = fread(&signature, sizeof(unsigned int), 1, index_file);
  if(!readed) return false;
  readed = fread(&totvert, sizeof(unsigned int), 1, index_file);
  if(!readed) return false;
  readed = fread(&totface, sizeof(unsigned int), 1, index_file);
  if(!readed) return false;
  readed = fread(&sphere, sizeof(Sphere3f), 1, index_file);
  if(!readed) return false;
  
  unsigned int size; //size of index
  readed = fread(&size, sizeof(unsigned int), 1, index_file);
  if(!readed) return false;

  index.resize(size);
  readed = fread(&index[0], sizeof(Entry), size, index_file);
  if(readed != size) return false;

  //history size;
  fread(&size, sizeof(unsigned int), 1, index_file);
  vector<unsigned int> buffer;
  buffer.resize(size);
  fread(&(buffer[0]), sizeof(unsigned int), size, index_file);

  //number of history updates
  size = buffer[0];
  history.resize(size);

  unsigned int pos = 1;
  for(unsigned int i = 0; i < size; i++) {
    unsigned int erased = buffer[pos++];
    unsigned int created = buffer[pos++];
    history[i].erased.resize(erased);
    history[i].created.resize(created);
    for(unsigned int e = 0; e < erased; e++) 
      history[i].erased[e] = buffer[pos++];
    for(unsigned int e = 0; e < created; e++) 
      history[i].created[e] = buffer[pos++];
  }

  if(!patches.Load(file + ".nxp", 1)) return false;
  if(!borders.Load(file + ".nxb", 1)) return false;
  return true;
}

void Nexus::Close() {
  if(!index_file) return;
  rewind(index_file);

  fwrite(&signature, sizeof(unsigned int), 1, index_file);  
  fwrite(&totvert, sizeof(unsigned int), 1, index_file);
  fwrite(&totface, sizeof(unsigned int), 1, index_file);
  fwrite(&sphere, sizeof(Sphere3f), 1, index_file);
  
  unsigned int size = index.size(); //size of index
  fwrite(&size, sizeof(unsigned int), 1, index_file);
  fwrite(&(index[0]), sizeof(Entry), size, index_file);

  vector<unsigned int> buffer;
  buffer.push_back(history.size());
  for(unsigned int i = 0; i < history.size(); i++) {
    Update &update = history[i];
    buffer.push_back(update.erased.size());
    buffer.push_back(update.created.size());
    for(unsigned int e = 0; e < update.erased.size(); e++)
      buffer.push_back(update.erased[e]);
    for(unsigned int e = 0; e < update.created.size(); e++)
      buffer.push_back(update.created[e]);
  }

  size = buffer.size();
  fwrite(&size, sizeof(unsigned int), 1, index_file);
  fwrite(&(buffer[0]), sizeof(unsigned int), size, index_file);


  fclose(index_file);
  index_file = NULL;

  patches.Close();
  borders.Close();
}

Patch Nexus::GetPatch(unsigned int patch) {
  Entry &entry = index[patch];
  Chunk *start = patches.GetRegion(entry.patch_start, entry.patch_size);
  return Patch(signature, start, entry.nvert, entry.nface);
}

Border Nexus::GetBorder(unsigned int patch) {
  Entry &entry = index[patch];
  Link *start = borders.GetRegion(entry.border_start, entry.border_size);
  return Border(start, entry.border_size);
}


unsigned int Nexus::AddPatch(unsigned int nvert, unsigned int nface,
			     unsigned int nbord) {
  Entry entry;
  entry.patch_start = patches.Size();
  entry.patch_size = Patch::ChunkSize(signature, nvert, nface);
  entry.border_start = borders.Size();
  entry.border_size = nbord;
  entry.nvert = nvert;
  entry.nface = nface;
  
  patches.Resize(patches.Size() + entry.patch_size);
  borders.Resize(borders.Size() + nbord);
  index.push_back(entry);
  totvert += nvert;
  totface += nface;
  return index.size() -1;
}

void Nexus::Join(const std::set<unsigned int> &patches,
		 std::vector<Point3f> &newvert,
		 std::vector<unsigned int> &newface,
		 std::vector<Link> &newbord) {

  map<unsigned int, vector<unsigned int> > remap;

  set<unsigned int>::const_iterator i;
  for(i = patches.begin(); i != patches.end(); i++) {
    unsigned int patch = *i;
    Nexus::Entry &entry = index[patch];
    remap[*i].resize(entry.nvert, 0xffffffff);
  }
  unsigned int vcount = 0;
  unsigned int fcount = 0;
  unsigned int bcount = 0;
  for(i = patches.begin(); i != patches.end(); i++) {
    unsigned int patch = *i;
    Nexus::Entry &entry = index[patch];
    fcount += entry.nface;
    assert(fcount < 0xffff);
    for(unsigned int k = 0; k < entry.nvert; k++) {
      if(remap[patch][k] == 0xffffffff) { //first time
	remap[patch][k] = vcount++;
      }
    }

    Border border = GetBorder(patch);
    for(unsigned int k = 0; k < border.Size(); k++) {
      Link &link = border[k];
      if(!remap.count(link.end_patch)) {
	bcount++;
	continue;
      }
      if(remap[link.end_patch][link.end_vert] == 0xffffffff) { //first time
	remap[link.end_patch][link.end_vert] = remap[patch][link.start_vert];
      }
    }
  }



  newvert.resize(vcount);
  newface.resize(fcount*3);
  newbord.resize(bcount);

  fcount = 0;
  bcount = 0;
  for(i = patches.begin(); i != patches.end(); i++) {
    Patch patch = GetPatch(*i);
    Border border = GetBorder(*i);
    vector<unsigned int> &vmap = remap[*i];

    for(unsigned int i = 0; i < patch.VertSize(); i++)
      newvert[vmap[i]] = patch.Vert(i);

    for(unsigned int i = 0; i < patch.FaceSize(); i++) {
      for(int k = 0; k < 3; k++) {
	newface[3*fcount + k] = vmap[patch.Face(i)[k]];
      }
      fcount++;
    }
    for(unsigned int i = 0; i < border.Size(); i++) {
      Link &link = border[i];
      if(remap.count(link.end_patch)) continue;
      Link newlink = link;
      newlink.start_vert = vmap[link.start_vert];
      newbord[bcount++] = newlink;
    }
  }  
 /* unsigned int newentry = AddPatch(vcount, fcount, bcount);
  Patch newpatch = GetPatch(newentry);
  Border newborder = GetBorder(newentry);

  memcpy(newpatch.VertBegin(), &(newvert)[0],
	 newvert.size() * sizeof(Point3f));

  memcpy(newpatch.FaceBegin(), &(newface)[0],
	 newface.size() * sizeof(unsigned short));

  memcpy(&(newborder[0]), &(newbord[0]),
	 newbord.size() * sizeof(Link));*/
  return;
}
