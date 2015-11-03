// train.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../bayesian/Network.h"

using namespace std;

void train_test(const string& f_node,const string& f_know,const string& f_data,
	const string& f_struct,const string& f_struct_brief,const string& f_learn_param)
{
	Network net;
	cout<<"Initializing"<<endl;
	try{
		if(!net.initial(f_node.c_str(),false,false,false))
			cerr<<"Error in initialization."<<endl;
		if(!net.initial_knowledge(f_know.c_str()))
			cerr<<"Error in getting knowledge."<<endl;
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return;
	}
	//learning:
	cout<<"Learnig structure."<<endl;
	double min_improve=5.0;	//default value
	size_t max_iteration=100;
	bool dynamic_data=false;
	double sand_ratio=1.0;
	bool with_md=false;
	double md_ratio=1.2;	//not used in training, just checking existence here.
	ifstream fin(f_learn_param.c_str());
	cout<<"\tTry loading learning configure from file."<<endl; 
	if(fin>>min_improve>>max_iteration){//try load learning 
		cout<<"\tLoading parameter successed."<<endl;
		if(fin>>dynamic_data>>sand_ratio){
			cout<<"\tLoading dynamic data option successed"<<endl;
		}else{
			dynamic_data=false;
			cout<<"\tAssume without dynamic data."<<endl;
		}
		if(fin>>with_md>>md_ratio){
			cout<<"\tLoading marginal probability option successed"<<endl;
		}else{
			with_md=false;
			cout<<"\tAssume without marginal probability."<<endl;
		}
	}else{
		cerr<<"\tNo learning parameter."<<endl;
		return;
	}
	fin.close();
	cout<<"min_improve="<<min_improve<<" , max_iteration="<<max_iteration<<"\n"
		<<"dynamic_data="<<dynamic_data<<" , sand_ratio="<<sand_ratio<<"\n"
		<<"with_md="<<with_md<<" , md_powerup="<<md_ratio<<endl;
	cout<<"Start training."<<endl;
	try{
		cout<<"\tPrepare data."<<endl;
		net.prepare_structure_learning(f_data);
		cout<<"\tGready learning."<<endl;
		//net.learning_gready(5.0,100);
		//double min_improve,size_t max_iterantion,bool dynamic_data 
		net.learning_gready_level(min_improve,max_iteration);
	}catch(const std::exception& e){
		cerr<<"Error in learning."<<endl;
		cout<<e.what()<<endl;
		return;
	}
	cout<<"Finish training."<<endl;
	/*
	net.read_data("data.csv");
	vector<size_t> off;
	Network::condition_t cond;
	cond[net.get_node(1)]=0;
	size_t t1=net._get_eligible_data_offsets(off,cond);
	for(int i=0;i<off.size();i++)
		cout<<off[i]<<' ';
	cout<<endl<<endl;
	off.clear();
	size_t t2=net._get_eligible_data_offsets(off,net.get_node(1),0);
	for(int i=0;i<off.size();i++)
		cout<<off[i]<<' ';
	cout<<endl<<endl;
	*/
	cout<<"calculating all conditional distributions."<<endl;
	net.cal_cds(with_md);
//	net.cal_cds(with_md);	//without daynamic data
	cout<<"outputing."<<endl;
	ofstream fout(f_struct.c_str());
	net.output_structure(fout,with_md,true);
	fout.close();
	fout.clear();
	fout.open(f_struct_brief.c_str());
	net.output_structure(fout,false,false);
	fout.close();
}


int main(int argc, char* argv[])
{
	ios::sync_with_stdio(false);
#ifdef _DEBUG
	string base_dir("../Data/");
#else
	string base_dir("../Data/");
#endif
	string folder;
	string model_folder("");
	//string test_data_folder("test/");//not used;
	ifstream fin((base_dir+"current_folder.txt").c_str());
	fin>>folder>>model_folder;//>>test_data_folder;
	fin.close();
	if(folder.empty()){
		cerr<<"Can not find which folder to use!"<<endl;
		return 1;
	}
	cout<<"Working in folder: "<<folder<<endl;
	cout<<"Model output sub-folder: "<<model_folder<<endl;
	string base=base_dir+folder+"/";
	string f_node("node_traffic.txt");
	string f_know("knowledge_traffic.txt");
	string f_data("data.csv");
	string f_struct("structure_traffic.txt");
	string f_struct_brief("structure_traffic_brief.txt");
	string f_learn_param("learn_param.txt");

	//model and result sub-folder:
	f_learn_param=model_folder+"/"+f_learn_param;
	f_struct=model_folder+"/"+f_struct;
	f_struct_brief=model_folder+"/"+f_struct_brief;

	train_test(base+f_node,base+f_know,base+f_data,base+f_struct,base+f_struct_brief,base+f_learn_param);
	return 0;
}

