
#include <utility>
#include <map>
#include <list>

#include <string>
#include <iostream>
#include "SlotsNode.h"
#include "ClassesNode.h"
#include "InstancesNode.h"


using namespace std;



static char* XML_SCHEMA_NAME = "protegekb";


class FacetNode: public Node
{
virtual void printNode();
virtual int qualifyNode();
};



class FacetsNode: public Node
{
	NodeGroup facets;
	virtual void printNode();
	virtual int qualifyNode();
};



class MainNode: public Node
{
public:

	MainNode(void){node_type = MAIN_NODE;};
	int node_type;
	list<pair<const char*,const char*> > headers;

	void addHeaders(const char* str, const char*val);
	virtual void printNode();
	virtual int qualifyNode();
};

void MainNode::addHeaders(const char* str, const char*val)
{
	headers.push_back(pair<const char*, const char*>(str,val));
}
void MainNode::printNode()
{

	cout<<"MainNode: node_type is "<<node_type<<"\n";
	list<pair<const char*,const char*> >::iterator it;
	for(it = headers.begin(); it!= headers.end(); ++it)
	{
		cout<<"MainNode: First element is "<< it->first<<"\n";
		cout<<"MainNode: Second element is "<<it->second<<"\n";
	}
}

int MainNode::qualifyNode()
{return node_type;}


class XMLTree
{
public:
	XMLTree(void){};
	~XMLTree(void){};
	NodeGroup root;
	NodeGroup ng;	
	SlotNode sn;
	

	// methods
	void initializeMain();
	void finalizeMain();
	void addHeaders(const char* str, const char*val);

	void addSlots(SlotNode* sn);
//	void addFacets();
	void addClasses(ClassNode* cn);
	void addInstances(InstanceNode* in);
	void addNode(const char* s, int value_type, const char* name);

	void printXMLTree();

};

void XMLTree::initializeMain()
{
	
	MainNode*	mn = new MainNode;
	//NodeGroup* ng = new NodeGroup;
	
	mn->headers.push_back(pair<const char*,const char*>("protegekb",""));

	char* s1 = "http://www.w3.org/2001/XMLSchema-instance";
	char* s2 = new(char[25]);
	sprintf(s2,"\"%s\"",s1);
	mn->addHeaders(" xmlns:xsi=", s2);

	
	s1 = "http://protege.stanford.edu/plugins/xmlbackend/protege_xml_backend.xsd";
	s2 = new(char[100]);
	sprintf(s2,"\"%s\"",s1);
	mn->addHeaders(" xsi:noNamespaceSchemaLocation=", s2);
	mn->addHeaders(" xmlns:xsi=", s2);
	root.Sons.push_back(mn);


}

void XMLTree::finalizeMain()
{


	addSlots(&sn);

	
	OwnSlotsNode* ossn = new OwnSlotsNode;
	ossn->addOwnSlot(&ng);
	ClassNode* cn = new ClassNode;
	cn->addOwnSlots(ossn);

	addClasses(cn);

	InstanceNode* in = new InstanceNode;
	in->addOwnSlots(ossn);

	addInstances(in);
	MainNode*	mn = new MainNode;
	
	mn->headers.push_back(pair<const char*,const char*>("/",XML_SCHEMA_NAME));
	root.Sons.push_back(mn);

}


void XMLTree::addHeaders(const char* str, const char*val)
{
	MainNode* mn = (MainNode*) root.Sons.front();
	mn->headers.push_back(pair<const char*,const char*>(str,val));
}

void XMLTree::addSlots(SlotNode* sn)
{
	SlotsNode* sn0 = new SlotsNode; // 1 solo

	sn0->addSlot(sn);
	root.Sons.push_back(sn0);
	

}

void XMLTree::addClasses(ClassNode* cn)
{
	ClassesNode* cn0 = new ClassesNode; // 1 solo

	cn0->addClass(cn);
	root.Sons.push_back(cn0);
}

void XMLTree::addNode(const char* s, int value_type, const char* name)
{
	
	ValueNode* vn = new ValueNode;
	EntryNode* en = new EntryNode;
	OwnSlotNode* osn = new OwnSlotNode;

	


	switch(value_type)
	{
case VALUE_INTEGER:
	en->type = "Integer";
		break;
case VALUE_FLOAT:
	en->type = "Float";
			
case VALUE_BOOL:
	en->type = "Bool";
		break;
case VALUE_STRING:
	en->type = "String";
		break;

	}

	
	vn->setValue(s);
	en->addValue(*vn);
	osn->setName(name);
	osn->addEntry(*en);
	sn.addOwnSlot(osn);
	ng.addNode(osn);

}
void XMLTree::addInstances(InstanceNode* in)
{
	InstancesNode* in0 = new InstancesNode; // 1 solo

	in0->addInstance(in);
	root.Sons.push_back(in0);
}

void XMLTree::printXMLTree()
{	
	string ext,s("XMLFile");
	cout<<"\t Preparing to create XML file"<<"\n";
	cout<<"\t enter the name for the file \n \t ";
	cin >> ext;


	s.append(ext);
	s.append(".xml");

	const char* filename = s.c_str();
	FILE* fp = fopen(filename, "w");

	list<Node*>::iterator it;
	list<Node*>::iterator it2;
	list<Node*>::iterator it3;
	list<pair<const char*,const char*> >::iterator lit;
	MainNode* mn;
	SlotsNode* sns;
	SlotNode* sn;
	OwnSlotNode* osn;	
	ClassesNode* csn;	
	ClassNode* cn;	
	InstancesNode* isn;	
	InstanceNode* in;	
	int nn = 0;
	for(it = root.Sons.begin(); it!=root.Sons.end(); ++it){
		cout<<"Creating Node #"<< nn<<"\n";
		cout<<"Node Type is "<< (*it)->qualifyNode()<<"\n";
		switch((*it)->qualifyNode())
		{
		case MAIN_NODE:
			mn = (MainNode*)(*it);
			fprintf(fp,"<");
			for(lit = mn->headers.begin(); lit!= mn->headers.end(); ++lit)
					fprintf(fp,"%s%s", lit->first,lit->second );
				fprintf(fp,"> \n");
			
			break;
		
		case SLOTS_NODE:
			sns = (SlotsNode*)(*it);
			fprintf(fp,"<slots> \n");

			for(it2 = sns->slot.Sons.begin(); it2!=sns->slot.Sons.end(); ++it2)
			{
				sn = (SlotNode*) (*it2);
				fprintf(fp,"\t<slot>");
				for(it3 = sn->own_slot.Sons.begin(); it3!=sn->own_slot.Sons.end(); ++it3)
				{
					osn = (OwnSlotNode*) (*it3);
					fprintf(fp,"<own-slot slot-name=\":%s\">\n",osn->name);
					fprintf(fp,"\t\t<entry type=\"%s\">\n",osn->entry.type);
					fprintf(fp,"\t\t\t<value>\n");
					fprintf(fp,"\t\t\t%s\n",osn->entry.value.value);
					fprintf(fp,"\t\t\t</value>\n");
					fprintf(fp,"\t\t</entry>\n");
					fprintf(fp,"\t</own-slot>");
				}
				fprintf(fp,"</slot>\n");
			}
			fprintf(fp,"</slots>\n");

			break;
	case CLASSES_NODE:
			csn = (ClassesNode*)(*it);
			fprintf(fp,"<classes> \n");

			for(it2 = csn->classn.Sons.begin(); it2!=csn->classn.Sons.end(); ++it2)
			{
				cn = (ClassNode*) (*it2);
				fprintf(fp,"\t<class>");
				fprintf(fp,"<own-slots>\n");
				for(it3 = cn->own_slots.own_slot.Sons.begin(); it3!=cn->own_slots.own_slot.Sons.end(); ++it3)
				{
					osn = (OwnSlotNode*) (*it3);
					fprintf(fp,"\t\t<own-slot slot-name=\":%s\">\n",osn->name);
					fprintf(fp,"\t\t\t<entry type=\"%s\">\n",osn->entry.type);
					fprintf(fp,"\t\t\t\t<value>\n");
					fprintf(fp,"\t\t\t%s\n",osn->entry.value.value);
					fprintf(fp,"\t\t\t\t</value>\n");
					fprintf(fp,"\t\t\t</entry>\n");
					fprintf(fp,"\t\t</own-slot>\n");
				}
				fprintf(fp,"\t</own-slots>\n");
				fprintf(fp,"</class>\n");
			}
			fprintf(fp,"</classes>\n");

			break;
	case INSTANCES_NODE:
			isn = (InstancesNode*)(*it);
			fprintf(fp,"<instances> \n");

			for(it2 = isn->instances.Sons.begin(); it2!=isn->instances.Sons.end(); ++it2)
			{
				in = (InstanceNode*) (*it2);
				fprintf(fp,"\t<instance>\n");
				fprintf(fp,"\t\t<id>\n");
				fprintf(fp,"\t\t%s\n", in->id);
				fprintf(fp,"\t\t</id>\n");
				fprintf(fp,"\t\t<type>\n");
				fprintf(fp,"\t\t%s\n", in->type);
				fprintf(fp,"\t\t</type>\n");
				fprintf(fp,"\t\t<own-slots>\n");
				for(it3 = in->own_slots.own_slot.Sons.begin(); it3!=in->own_slots.own_slot.Sons.end(); ++it3)
				{
					osn = (OwnSlotNode*) (*it3);
					fprintf(fp,"\t\t\t<own-slot slot-name=\":%s\">\n",osn->name);
					fprintf(fp,"\t\t\t\t<entry type=\"%s\">\n",osn->entry.type);
					fprintf(fp,"\t\t\t\t<value>\n");
					fprintf(fp,"\t\t\t\t%s\n",osn->entry.value.value);
					fprintf(fp,"\t\t\t\t</value>\n");
					fprintf(fp,"\t\t\t\t</entry>\n");
					fprintf(fp,"\t\t\t</own-slot>\n");
				}
				fprintf(fp,"\t\t</own-slots>\n");
				fprintf(fp,"\t</instance>\n");
			}
			fprintf(fp,"</instances>\n");

			break;
		}
		++nn;
	}
	fclose(fp);
}
