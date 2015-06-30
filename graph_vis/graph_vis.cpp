// graph_vis.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

using namespace std;

void convert_to_gv(const string& fnode,const string& fknow,
				  const string &fstruct_b,const string &fgv)
{
	vector<string> nodes;//name->id
	int n_nodes;
	vector<vector<string> > node_level;
	//node::
	ifstream fin(fnode.c_str());
	if(!fin){
		cerr<<"Cannot open node file: "<<fnode<<endl;
		return;
	}
	cout<<"Reading node."<<endl;
	fin>>n_nodes;
	for(int i=0;i<n_nodes;i++){
		int temp;
		string name;
		fin>>name>>temp;
		nodes.push_back(name);
	}
	fin.close();
	fin.clear();
	//level:
	fin.open(fknow.c_str());
	if(!fin){
		cerr<<"Cannot open knowledge file: "<<fknow<<endl;
		return;
	}
	cout<<"Reading level."<<endl;
	for(int i=0;i<n_nodes;++i){
		size_t level;
		string name;
		fin>>name>>level;
		if(node_level.size()<=level)
			node_level.resize(level+1);
		node_level[level].push_back(name);
	}
	fin.close();
	fin.clear();
	//structure and output:
	fin.open(fstruct_b.c_str());
	if(!fin){
		cerr<<"Cannot open structure file: "<<fstruct_b<<endl;
		return;
	}
	ofstream fout(fgv.c_str());
	if(!fout){
		cerr<<"Cannot open output file: "<<fgv<<endl;
		return ;
	}
	cout<<"Outputting structure gv file."<<endl;

	fout<<"digraph traffic {\n\tsize=\"80,7\";\n";
	for(size_t i=0;i<node_level.size();++i){
		fout<<"\t{rank=same;";
		for(size_t j=0;j<node_level[i].size();++j){
			fout<<" "<<node_level[i][j];
		}
		fout<<";}\n";
	}
	for(int i=0;i<n_nodes;i++){
		string name,temp;
		int n;
		fin>>name>>n;
		if(n!=0)
			fout<<"\n";
		for(int j=0;j<n;j++){
			fin>>temp;
			fout<<"\t"<<temp<<"->"<<name<<";\n";
		}
	}
	fin.close();
	fout<<"}"<<endl;
	fout.close();
}

void draw(const string &f_gv,const string &f_format){
	char buf[1024];
	sprintf(buf,"dot -T%s \"%s\" -O",f_format.c_str(),f_gv.c_str());
	system(buf);
}

int _tmain(int argc, _TCHAR* argv[])
{
	ios::sync_with_stdio(false);
	cout<<"start."<<endl;
#ifdef _DEBUG
	string base_dir("../Data/");
#else
	string base_dir("../Data/");
#endif
	string folder;
	string model_folder("");
	ifstream fin((base_dir+"current_folder.txt").c_str());
	fin>>folder>>model_folder;
	fin.close();
	if(folder.empty()){
		cerr<<"Can not find which folder to use!"<<endl;
		return 1;
	}
	cout<<"Working in folder: "<<folder<<endl;
	cout<<"Model sub-folder: "<<model_folder<<endl;

	string base=base_dir+folder+"/";
	string f_node("node_traffic.txt");
	string f_know("knowledge_traffic.txt");
	string f_struct_brief("structure_traffic_brief.txt");
	string f_gv("structure.gv");
	string f_format("png");

	f_struct_brief=model_folder+"/"+f_struct_brief;
	f_gv=model_folder+"/"+f_gv;

	cout<<"start"<<endl;
	convert_to_gv(base+f_node,base+f_know,base+f_struct_brief,base+f_gv);
	cout<<"output picture"<<endl;
	draw(base+f_gv,f_format);
	cout<<"end"<<endl;

	return 0;
}

