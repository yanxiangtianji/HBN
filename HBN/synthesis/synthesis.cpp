// synthesis.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../bayesian/Network.h"
#include "../bayesian/Node.h"

using namespace std;
using namespace boost::random;

//output tool:
template<typename T>
void output(ostream& os,const vector<T>& vec,const string& col_sep,const string& line_sep){
	os<<vec.front();
	for(vector<T>::const_iterator it=vec.begin()+1;it!=vec.end();it++){
		os<<col_sep<<*it;
	}
	os<<line_sep;
}
template<typename T>
void output(ostream& os,const vector<vector<T> >& vec,const string& col_sep,const string& line_sep){
	for(vector<T>::const_iterator it=vec.begin();it!=vec.end();it++){
		output(os,*it,col_sep,line_sep);
	}
}

void prepare_query_and_given_names(const string& f_know,
	vector<node_name_t>& given,vector<node_name_t>& query)
{
	ifstream fin(f_know.c_str());
	string name;
	int level;
	while(fin>>name>>level){
		if(level==0)
			given.push_back(name);
		else
			query.push_back(name);
	}
	fin.close();
}
size_t prepare_name_n_states(const string& f_node,map<node_name_t,size_t>& name_nstates){
	ifstream fin(f_node.c_str());
	size_t num=0;
	fin>>num;
	for(size_t i=0;i<num;++i){
		string name;
		size_t n;
		fin>>name>>n;
		name_nstates[name]=n;
	}
	return num;
}

void prepare_name_off(map<node_name_t,size_t>& name_off,
	vector<node_name_t>& names,string &temp)
{
	names.clear();
	size_t plast = 0, p = temp.find(',');
	for (; p != string::npos;p=temp.find(',',plast)){
		names.push_back(temp.substr(plast, p - plast));
		plast = p + 1;
	}
	names.push_back(temp.substr(plast));
	for(size_t i=0;i<names.size();++i){
		name_off[names[i]]=i;
	}
}

void sep_cvt_csvline(vector<size_t>& temp,const string& str){
	size_t plast=0,p=str.find(',');
	while(p!=string::npos){
		temp.push_back(stoi(str.substr(plast,p-plast)));
		plast=p+1;
		p=str.find(',',plast);
	}
	temp.push_back(stoi(str.substr(plast)));
}

//order of dis is the same as given_names
void get_ref_distribution(ifstream& fin,const vector<node_name_t>& given_names,
	const vector<node_name_t>& names,map<node_name_t,size_t>& name_n_states,
	vector<vector<double> >&dis)
{
	vector<vector<size_t> > count;
	size_t total=0;
	vector<int> offset_mapping(names.size(),-1);	//offset in data->offset in given_names
	dis.clear();
	for(size_t i=0;i<given_names.size();++i){
		count.push_back(vector<size_t>(name_n_states[given_names[i]]));
		dis.push_back(vector<double>(name_n_states[given_names[i]]));
		vector<node_name_t>::const_iterator it=
			find(names.begin(),names.end(),given_names[i]);
		offset_mapping[it-names.begin()]=i;
	}
	string str;
	vector<size_t> temp;
	while(getline(fin,str)){
		if(str.empty())
			continue;
		temp.clear();
		sep_cvt_csvline(temp,str);
		total++;
		for(size_t i=0;i<temp.size();++i){
			int p=offset_mapping[i];
			if(p==-1)
				continue;
			count[p][temp[i]]++;
		}
	}
	for(size_t i=0;i<count.size();++i){
		for(size_t j=0;j<count[i].size();++j){
			dis[i][j]=static_cast<double>(count[i][j])/total;
		}
	}
	return;
}

void format_syn_data(vector<size_t>& res,const map<node_name_t,state_t>& given,
	const map<node_name_t,state_t>& query,map<node_name_t,size_t>& name_off)
{
	for(map<node_name_t,state_t>::const_iterator it=given.begin();
		it!=given.end();++it)
	{
		res[name_off[it->first]]=it->second;
	}
	for(map<node_name_t,state_t>::const_iterator it=query.begin();
		it!=query.end();++it)
	{
		res[name_off[it->first]]=it->second;
	}
}

void output_syn_data(const string& f_node,const string& f_know,const string& f_struct,
	const string& f_struct_brief,const string& f_ref_data,const string& f_syn_data,
	const int amount,const bool with_ref_dis)
{
	cout<<"Synthesize data by existing distribution."<<endl;
	cout<<"Loading structure..."<<endl;
	Network net;
	try{
		bool flag;
		if(with_ref_dis)
			flag=net.initial(f_node,f_struct,true,false);
		else
			flag=net.initial(f_node,f_struct_brief,false,false);
		if(!flag){
			cerr<<"Error in initialization."<<endl;
			return;
		}
		if(!net.initial_knowledge(f_know)){
			cerr<<"Error in getting knowledge."<<endl;
			return;
		}
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return;
	}
	cout<<"finish initialization."<<endl;
	cout<<"Structure:"<<endl;
	try{
		net.output_structure(cout,false,false);
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return;
	}
//working:
//prepare:
	vector<node_name_t> given_names,query_names;
	map<node_name_t,size_t> name_off;//name_off[name]=offset position in the file
	vector<node_name_t> names;
	map<node_name_t,size_t> name_n_states;
	size_t n_given,n_query,n_total;
	string firstline;

	//set number of states for each name
	prepare_name_n_states(f_node,name_n_states);
	//seperate given and query
	prepare_query_and_given_names(f_know,given_names,query_names);
	ifstream fin(f_ref_data.c_str());
	if(!fin){
		cerr<<"Cannot open reference data file: "<<f_ref_data<<endl;
		return;
	}
	getline(fin,firstline);
	//set offset in data file for each name
	prepare_name_off(name_off,names,firstline);
	n_given=given_names.size();
	n_query=query_names.size();
	n_total=name_off.size();
	//distribution:
	//base_gen_t generator(time(0));
	mt19937 generator(42);
	vector<vector<double> > distribution;//same order with given_names
	typedef variate_generator<mt19937*,uniform_smallint<> > gen_uni_t;
	typedef variate_generator<mt19937*,discrete_distribution<> > gen_ref_t;
	map<node_name_t,shared_ptr<gen_uni_t> > gens_uni;
	map<node_name_t,shared_ptr<gen_ref_t> > gens_ref;
	if(with_ref_dis)
		get_ref_distribution(fin,given_names,names,name_n_states,distribution);
	for(size_t i=0;i<given_names.size();++i){
		if(with_ref_dis){
			gens_ref[given_names[i]]=make_shared<gen_ref_t>(&generator,
				discrete_distribution<>(distribution[i].begin(),distribution[i].end()) );
		}else{
			gens_uni[given_names[i]]=make_shared<gen_uni_t>(&generator,
				uniform_smallint<>(0,name_n_states[given_names[i]]-1) );
		}
	}
	
	//generate:
	ofstream fout(f_syn_data.c_str());
	if(!fout){
		cerr<<"Cannot establish output file!"<<endl;
		return;
	}
	fout<<firstline<<endl;
	map<node_name_t,state_t> given,pred_res;
	vector<node_name_t> query;
	vector<size_t> line(net.get_n_nodes());
	for(vector<node_name_t>::iterator it=given_names.begin();
			it!=given_names.end();++it)
		query.push_back(*it);
	for(int iteration=0;iteration<amount;++iteration){
		for(size_t i=0;i<given_names.size();++i){
			int t;
			if(with_ref_dis)
				t=gens_ref[given_names[i]]->operator ()();
			else
				t=gens_uni[given_names[i]]->operator ()();
			given[given_names[i]]=static_cast<state_t>(t);
		}
		pred_res=net.predict(given,query);
		format_syn_data(line,given,pred_res,name_off);
		output(fout,line,",","\n");
	}
}

int main(int argc,char* argv[])
{
	ios::sync_with_stdio(false);
	cout<<"start."<<endl;
#ifdef _DEBUG
	string base_dir("../");
#else
	string base_dir("../");
#endif
	string folder;
	string model_folder;
	{
		ifstream fin((base_dir+"current_folder.txt").c_str());
		fin>>folder>>model_folder;
		fin.close();
		if(folder.empty()){
			cerr<<"Can not find which folder to use!"<<endl;
			return 1;
		}
	}
	string syn_data_folder;
	int amount=0;
	bool with_dis;
	{
		string s;
		ifstream fin((base_dir+"synthesis.txt").c_str());
		fin>>syn_data_folder>>amount>>s;
		fin.close();
		if(syn_data_folder.empty()){
			cerr<<"Cannot find which folder to use!"<<endl;
			return 1;
		}
		if(amount==0){
			cerr<<"Cannot readin num of samples!"<<endl;
			return 1;
		}
		if(s!="dis" && s!="uni"){
			cerr<<"Cannot find whether to use referenced distribution!"<<endl;
			return 1;
		}
		with_dis=(s=="dis");
	}
	cout<<"Working in folder: "<<folder<<endl;
	cout<<"Model sub-folder: "<<model_folder<<endl;
	cout<<"Synthetic data sub-folder: "<<syn_data_folder<<endl;
	cout<<"Result folder: "<<syn_data_folder<<endl;
	cout<<"Amount of synthesis data: "<<amount<<endl;

	string base=base_dir+folder+"/";
	string f_node("node_traffic.txt");
	string f_know("knowledge_traffic.txt");
	string f_struct("structure_traffic.txt");
	string f_struct_brief("structure_traffic_brief.txt");
	string f_ref_data("data.csv");
	string f_syn_data("data.csv");

	//synthesis data sub-folder:
	syn_data_folder=syn_data_folder+"_"+folder+"_"+model_folder+"/";
	//model sub-folder:
	f_struct=model_folder+"/"+f_struct;
	f_struct_brief=model_folder+"/"+f_struct_brief;

	_mkdir((base_dir+syn_data_folder).c_str());

	output_syn_data(base+f_node,base+f_know,base+f_struct,base+f_struct_brief,
		base+f_ref_data,base_dir+syn_data_folder+f_syn_data,amount,with_dis);//true->with reference distribution

	return 0;
}

