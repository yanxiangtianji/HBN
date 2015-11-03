// generate.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../bayesian/Network.h"
#include "../bayesian/def.h"

using namespace std;

bool load_learning_param(const string& f_learn_param,bool& with_md,double& md_ratio){
	int max_iteration=100;
	bool dynamic_data=false;
	double min_improve=5.0,sand_ratio=1.0;

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
		return false;
	}
	fin.close();
	cout<<"Learning parameter:\n"
		<<"\tmin_improve="<<min_improve<<" , max_iteration="<<max_iteration<<"\n"
		<<"\tdynamic_data="<<dynamic_data<<" , sand_ratio="<<sand_ratio<<"\n"
		<<"\twith_md="<<with_md<<" , md_powerup="<<md_ratio<<endl;
	return true;
}

bool init_net(Network& net,const string& f_node,const string& f_know,
	const string& f_struct_brief,const bool with_md,const double md_ratio)
{
	cout<<"Loading structure..."<<endl;
	try{
		if(!net.initial(f_node,f_struct_brief,false,false)){//do not have conditional distribution
			cerr<<"Error in initialization."<<endl;
			return false;
		}
		if(!net.initial_knowledge(f_know)){
			cerr<<"Error in getting knowledge."<<endl;
			return false;
		}
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return false;
	}
	if(with_md){
		cout<<"Setting md_ratio"<<endl;
		Node::set_judge_ratio(md_ratio);
	}
	cout<<"finish initialization."<<endl;
	return true;
}

void generate_distribution(const string& f_node,const string& f_know,
	const string& f_data,const string& f_struct_brief,const string& f_struct,
	const bool with_md,const double md_ratio)
{
	Network net;
	if(!init_net(net,f_node,f_know,f_struct_brief,with_md,md_ratio)){
		cerr<<"failded"<<endl;
		return;
	}

	cout<<"\tPrepare data."<<endl;
	net.prepare_structure_learning(f_data);

	cout<<"calculating all conditional distributions."<<endl;
	net.cal_cds(with_md);
	cout<<"outputing."<<endl;
	ofstream fout(f_struct.c_str());
	net.output_structure(fout,with_md,true);
	fout.close();
}

int main()
{
	ios::sync_with_stdio(false);
	cout<<"start."<<endl;
#ifdef _DEBUG
	string base_dir("../Data/");
#else
	string base_dir("../Data/");
#endif
	string folder;
	string model_folder;
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
	string f_data("data.csv");
	string f_struct("structure_traffic.txt");
	string f_struct_brief("structure_traffic_brief.txt");
	string f_learn_param("learn_param.txt");

	//model and result sub-folder:
	f_learn_param=model_folder+"/"+f_learn_param;
	f_struct=model_folder+"/"+f_struct;
	f_struct_brief=model_folder+"/"+f_struct_brief;

	bool with_md=false;
	double md_ratio;
	if(!load_learning_param(base+f_learn_param,with_md,md_ratio))
		return -1;

	generate_distribution(base+f_node,base+f_know,base+f_data,
		base+f_struct_brief,base+f_struct,with_md,md_ratio);

	cout<<"end"<<endl;
	return 0;
}

