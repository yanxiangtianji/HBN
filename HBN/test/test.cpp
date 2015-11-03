// test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../bayesian/Network.h"
#include "../bayesian/def.h"
#include "output.h"

using namespace std;
//using namespace boost;


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
	const string& f_struct,const bool with_md,const double md_ratio)
{
	cout<<"Loading structure..."<<endl;
	try{
		if(!net.initial(f_node,f_struct,true,with_md)){//have conditional distribution
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

void prepare_query_and_given(
	 map<node_name_t,size_t>& given_name_off,
	 map<node_name_t,size_t>& query_name_off,
	 Network& net,CSVreader& reader,const int predict_level)
{
	for(size_t i=0;i<net.get_n_nodes();i++){
		Network::node_pointer_t p=net.get_node(i);
		const node_name_t& name=net.get_node_name(p);
		size_t off=reader.find_position(name);
		if(p->get_level()<=predict_level)
			given_name_off[name]=off;
		else
			query_name_off[name]=off;
	}
}

void init_table_count(vector<vector<vector<size_t> > >& table_count,
	Network& net,map<node_name_t,size_t>& query_name_off,const bool miss_data)
{
	//dimesion=node,row=real_state,column=predict_state
	if(table_count.size()!=query_name_off.size()){
		table_count.clear();//might be used
		table_count.resize(query_name_off.size());
	}
	size_t offset_real=miss_data?1:0;
	//size_t offset_pred=miss_pred?1:0;
	for(map<node_name_t,size_t>::iterator it=query_name_off.begin();
		it!=query_name_off.end();it++)
	{
		size_t diff=std::distance(query_name_off.begin(),it);
		size_t n_states=net.get_node(it->first)->get_n_states();
		table_count[diff].resize(offset_real+n_states,vector<size_t>(n_states));
	}
}

//treat unkown state as an augment state by the Python script "miss_preprocess.py". (0->unkown, 1->clear)
inline state_t post_process(const state_t value){
	return value>0? value-1:value;
}

void verify_test(const string& f_node,const string& f_know,
	const string& f_data,const string& f_struct,
	const string& f_res_line,const string& f_res_table,const string& f_wrong_instance,
	const bool with_md,const double md_ratio,const int predict_level,const bool need_postprocess=false)
{
	//load:
	Network net;
	if(!init_net(net,f_node,f_know,f_struct,with_md,md_ratio))
		return;
	cout<<"Structure:"<<endl;
	try{
		net.output_structure(cout,false,false);
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return;
	}

	CSVreader reader(f_data);
	vector<string> header=reader.get_header();

	//prepare query and given name
	map<node_name_t,size_t> given_name_off;//given[name]=offset position in the file
	map<node_name_t,size_t> query_name_off;
	prepare_query_and_given(given_name_off,query_name_off,net,reader,predict_level);
	size_t N_GIVEN=given_name_off.size();
	size_t N_QUERY=query_name_off.size();

	//prepare statistics
	size_t pics=0,total=0,real_normal_case=0,real_slow_case=0,pred_normal_case=0,pred_slow_case=0;
	size_t right=0,right_normal=0,right_slow=0,miss_normal=0,miss_slow=0;
	vector<vector<vector<size_t> > > table_count(N_QUERY);
	//dimesion=node,row=real_state,column=predict_state
	init_table_count(table_count,net,query_name_off,false);

	//verify:
	ofstream f_wrong(f_wrong_instance.c_str());
	map<node_name_t,state_t> given;
	vector<node_name_t> query;
	//prepare input (query):
	for(map<node_name_t,size_t>::const_iterator it=query_name_off.begin();
			it!=query_name_off.end();it++)
		query.push_back(it->first);
	auto fun=[](const vector<string>& l){
		vector<state_t> res;
		for (const auto& s : l)
			res.push_back(stoi(s));
		return res;
	};
	//main loop:
	while (true){
		vector<state_t> line;
		bool success;
		tie(success,line) = reader.get_line<vector<state_t>>(fun);
		if (success == false)
			break;
		pics++;
		if(pics%1000==0)
			cout<<pics<<" lines tested."<<endl;
		//prepare input (given):
		for(map<node_name_t,size_t>::const_iterator it=given_name_off.begin();
			it!=given_name_off.end();it++)
		{
			given[it->first] = line[it->second];
		}
		//predict:
//		map<node_name_t,pair<state_t,double> > temp_res=net.predict_with_poss(given,query);
		map<node_name_t,state_t> res=net.predict(given,query);
		size_t node_id=0;
		//check result:
		for(map<node_name_t,size_t>::const_iterator it=query_name_off.begin();
			it!=query_name_off.end();it++)
		{
			state_t predict=res[it->first];
			state_t real=line[it->second];
			if(need_postprocess==true){
				predict=post_process(predict);
				real=post_process(real);
			}

			table_count[node_id][real][predict]++;
			total++;
			if(predict==0)
				pred_normal_case++;
			else
				pred_slow_case++;
			if(real==0)
				real_normal_case++;
			else
				real_slow_case++;

			if(predict==real){
				right++;
				if(predict==0){
					right_normal++;
				}else{
					right_slow++;
				}
			}else{
				f_wrong<<pics<<' '<<it->first<<' '<<real<<' '<<predict<<"\n";
				if(real==0)//predict!=0
					miss_normal++;
				else if(predict==0)//real!=0
					miss_slow++;
			}
			node_id++;
		}
		line.clear();
	}
	f_wrong.close();

	//show result:
	size_t n_states=net.get_node(query_name_off.begin()->first)->get_n_states();//assume all the future node has same number of state
	if(need_postprocess)
		n_states--;

	ofstream f_table(f_res_table.c_str());
	//summary
	output_result_summary(cout,pics,real_normal_case,real_slow_case,
		pred_normal_case,pred_slow_case,right_normal,right_slow,miss_normal,miss_slow);
	output_result_summary(f_table,pics,real_normal_case,real_slow_case,
		pred_normal_case,pred_slow_case,right_normal,right_slow,miss_normal,miss_slow);
	//confusion table
	output_result_table(cout,query_name_off,n_states,table_count);
	output_result_table(f_table,query_name_off,n_states,table_count);
	f_table.close();
	//lines (for plotting)
	ofstream f_line(f_res_line.c_str());
	output_result_line(f_line,query_name_off,n_states,table_count);
	f_line.close();
}

void verify_test_miss(const string& f_node,const string& f_know,
	const string& f_data,const string& f_struct,
	const string& f_res_line,const string& f_res_table,const string& f_wrong_instance,
	const bool with_md,const double md_ratio,const int predict_level)
{
	//load:
	Network net;
	if(!init_net(net,f_node,f_know,f_struct,with_md,md_ratio))
		return;
	cout<<"Structure:"<<endl;
	try{
		net.output_structure(cout,false,false);
	}catch(const std::exception& e){
		cout<<e.what()<<endl;
		return;
	}

	CSVreader reader(f_data);
	vector<string> header=reader.get_header();

	//prepare query and given name
	map<node_name_t,size_t> given_name_off;//given[name]=offset position in the file
	map<node_name_t,size_t> query_name_off;
	prepare_query_and_given(given_name_off,query_name_off,net,reader,predict_level);
	size_t N_GIVEN=given_name_off.size();
	size_t N_QUERY=query_name_off.size();

	//prepare statistics
	size_t pics=0,total=0,real_normal_case=0,real_slow_case=0,pred_normal_case=0,pred_slow_case=0;
	size_t right=0,right_normal=0,right_slow=0,miss_normal=0,miss_slow=0;
	vector<vector<vector<size_t> > > table_count(N_QUERY);
	//dimesion=node,row=real_state,column=predict_state
	init_table_count(table_count,net,query_name_off,true);

	//verify:
	ofstream f_wrong(f_wrong_instance.c_str());
	map<node_name_t,state_t> given;
	vector<node_name_t> query;
	vector<state_t> transed_invalid_st;
	//prepare input (query) and transed invalid state:
	for(map<node_name_t,size_t>::const_iterator it=query_name_off.begin();
			it!=query_name_off.end();it++)
	{
		query.push_back(it->first);
		transed_invalid_st.push_back(net.get_node(it->first)->get_n_states());
	}
	auto fun = [](const vector<string>& l){
		vector<state_t> res;
		for (const auto& s : l)
			res.push_back(stoi(s));
		return res;
	};
	//main loop:
	while (true){
		vector<state_t> line;
		bool success;
		tie(success, line) = reader.get_line<vector<state_t>>(fun);
		if (success == false)
			break;
		pics++;
		if(pics%1000==0)
			cout<<pics<<" lines tested."<<endl;
		//prepare input (given):
		for(map<node_name_t,size_t>::const_iterator it=given_name_off.begin();
			it!=given_name_off.end();it++)
		{
			state_t st = line[it->second];
			if(st!=INVALID_STATE)
				given[it->first]=st;
			else
				given.erase(it->first);
		}
		//predict:
//		map<node_name_t,pair<state_t,double> > temp_res=net.predict_with_poss(given,query);
		map<node_name_t,state_t> res=net.predict(given,query);
		size_t node_id=0;
		//check result:
		for(map<node_name_t,size_t>::const_iterator it=query_name_off.begin();
			it!=query_name_off.end();it++)
		{
			state_t predict=res[it->first];
			state_t real=line[it->second];
			table_count[node_id][real==INVALID_STATE?transed_invalid_st[node_id]:real][predict]++;
			total++;
			if(real==INVALID_STATE)
				continue;
			if(predict==0)
				pred_normal_case++;
			else
				pred_slow_case++;
			if(real==0)
				real_normal_case++;
			else
				real_slow_case++;

			if(predict==real){
				right++;
				if(predict==0){
					right_normal++;
				}else{
					right_slow++;
				}
			}else{
				f_wrong<<pics<<' '<<it->first<<' '<<real<<' '<<predict<<"\n";
				if(real==0)//predict!=0
					miss_normal++;
				else if(predict==0)//real!=0
					miss_slow++;
			}
			node_id++;
		}
		line.clear();
	}
	f_wrong.close();

	//show result:
	size_t n_states=net.get_node(query_name_off.begin()->first)->get_n_states();//assume all the future node has same number of state
	ofstream f_table(f_res_table.c_str());
	//summary
	output_result_summary(cout,pics,real_normal_case,real_slow_case,
		pred_normal_case,pred_slow_case,right_normal,right_slow,miss_normal,miss_slow);
	output_result_summary(f_table,pics,real_normal_case,real_slow_case,
		pred_normal_case,pred_slow_case,right_normal,right_slow,miss_normal,miss_slow);
	//confusion table
	output_result_table(cout,query_name_off,n_states,table_count,true);
	output_result_table(f_table,query_name_off,n_states,table_count,true);
	f_table.close();
	//lines (for plotting)
	ofstream f_line(f_res_line.c_str());
	output_result_line(f_line,query_name_off,n_states,table_count);
	f_line.close();
}

//treat unkown state as an augment state by a Python script. (0->unkown, 1->clear)

int main(){
	ios::sync_with_stdio(false);
	cout<<"start."<<endl;
#ifdef _DEBUG
	string base_dir("../Data/");
#else
	string base_dir("../Data/");
#endif
	string folder;
	string model_folder("");
	string test_data_folder("");
	ifstream fin((base_dir+"current_folder.txt").c_str());
	fin>>folder>>model_folder>>test_data_folder;
	fin.close();
	if(folder.empty()){
		cerr<<"Can not find which folder to use!"<<endl;
		return 1;
	}
	cout<<"Working in folder: "<<folder<<endl;
	cout<<"Model sub-folder: "<<model_folder<<endl;
	cout<<"Test data sub-folder: "<<test_data_folder<<endl;

	string base=base_dir+folder+"/";
	string f_node("node_traffic.txt");
	string f_know("knowledge_traffic.txt");
	string f_data("data.csv");
	string f_struct("structure_traffic.txt");
	string f_res_line("result_line.txt");
	string f_res_table("result_table.txt");
	string f_wrong_instance("wrong_instance.txt");
	string f_learn_param("learn_param.txt");

	//test_data sub-folder:
	f_data=test_data_folder+"/"+f_data;
	f_learn_param=model_folder+"/"+f_learn_param;
	//model and result sub-folder:
	f_struct=model_folder+"/"+f_struct;
	f_res_line=model_folder+"/"+test_data_folder+"_"+f_res_line;	//p2/test1_result_table.txt
	f_res_table=model_folder+"/"+test_data_folder+"_"+f_res_table;	//p2/test1_result_table.txt
	f_wrong_instance=model_folder+"/"+test_data_folder+"_"+f_wrong_instance;	//p2/test1_wrong_instance.txt

	bool with_md=false;
	double md_ratio;
	if(!load_learning_param(base+f_learn_param,with_md,md_ratio))
		return -1;

	string test_miss_method;
	{
		fin.clear();
		fin.open((base+test_data_folder+"/miss.conf").c_str());
		if(fin)
			fin>>test_miss_method;
		fin.close();
	}
	int predict_level;
	{
		fin.clear();
		fin.open((base+"future.conf").c_str());
		fin>>predict_level;//dummy
		fin>>predict_level;
		fin.close();
	}
	bool need_postprocess=true;//treat unkown state as an augment state, so result should be processed
	need_postprocess=false;
/*	if(test_miss_method!="mark"){
		verify_test(base+f_node,base+f_know,base+f_data,base+f_struct,
			base+f_res_line,base+f_res_table,base+f_wrong_instance,with_md,md_ratio,predict_level,need_postprocess);
	}else{
		verify_test_miss(base+f_node,base+f_know,base+f_data,base+f_struct,
			base+f_res_line,base+f_res_table,base+f_wrong_instance,with_md,md_ratio,predict_level);
	}*/
	verify_test(base+f_node,base+f_know,base+f_data,base+f_struct,
			base+f_res_line,base+f_res_table,base+f_wrong_instance,with_md,md_ratio,predict_level,need_postprocess);
	cout<<"end."<<endl;

	return 0;
}

