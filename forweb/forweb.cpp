// forweb.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../bayesian/Node.h"
#include "../bayesian/Network.h"

void show_vec(const vector<double>& vec){
	for(size_t i=0;i<vec.size();i++){
		cout<<vec[i]<<' ';
	}
	cout<<endl;
}

int main(int argc, char* argv[])
{
	string f_node("../1/node_traffic.txt");
	string f_know("../1/knowledge_traffic.txt");
	string f_data("../1/data.csv");
	string f_struct("../1/structure_traffic.txt");

	Network net;
	cout<<"Loading structure..."<<endl;
	try{
		if(!net.initial(f_node,f_struct,true,false))
			cerr<<"Error in initialization."<<endl;
		if(!net.initial_knowledge(f_know))
			cerr<<"Error in getting knowledge."<<endl;
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return 0;
	}
	cout<<"finish initialization."<<endl;
	cout<<"Structure:"<<endl;
	try{
		net.output_structure(cout,true,false);
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return 0;
	}

	Node::pointer_t p=net.get_node("future_state_1");
	Node::cd_table_t cd_table=p->get_cd();
	Node::cd_table_t::const_iterator it=cd_table.begin();
	vector<Node::condition_t> conds;

	for(int i=0;i<10;i++){
		conds.push_back(it->first);
		cout<<"it->second:\t";
		show_vec(it->second);
		cout<<"get first:\t";
		show_vec(p->get_cd(it->first));
		cout<<"get stored:\t";
		show_vec(p->get_cd(conds[i]));
		++it;
	}

	cout<<"after"<<endl;
	for(int i=0;i<10;i++){
		cout<<"search stored:\t";
		show_vec(p->get_cd(conds[i]));
	}

	return 0;
}

