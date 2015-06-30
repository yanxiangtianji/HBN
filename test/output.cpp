#include "stdafx.h"
#include "output.h"

using namespace std;

void output_result_summary(ostream & os,const size_t pics,
	const size_t real_normal_case,const size_t real_slow_case,
	const size_t pred_normal_case,const size_t pred_slow_case,
	const size_t right_normal,const size_t right_slow,
	const size_t miss_normal,const size_t miss_slow)
{
	//overall result:
	size_t total=real_normal_case+real_slow_case;
	size_t right=right_normal+right_slow;
	size_t miss=miss_normal+miss_slow;
	os<<"Pieces of data="<<pics<<". "
		<<"Accuracy= "<<right<<"/"<<total<<"="<<(double)right/total*100.0<<"%\n"
		<<"Miss= "<<miss<<"/"<<total<<"="<<(double)miss/total*100.0<<"%"<<endl;
	os<<"Accuracy on normal= "<<right_normal<<"/"<<pred_normal_case<<"="
		<<(double)right_normal/pred_normal_case*100.0<<"%\n";
	os<<"Accuracy on slow= "<<right_slow<<"/"<<pred_slow_case<<"="
		<<(double)right_slow/pred_slow_case*100.0<<"%\n";
	os<<"Missed on normal= "<<miss_normal<<"/"<<pred_slow_case<<"="
		<<(double)miss_normal/real_slow_case*100<<"%"<<endl;
	os<<"Missed on slow= "<<miss_slow<<"/"<<real_slow_case<<"="
		<<(double)miss_slow/real_slow_case*100<<"%"<<endl;
}
//output result as confusion tables for each node;
//assume all query nodes have the same kinds of state;
void output_result_table(ostream & os,map<node_name_t,size_t>& query_name_off,
	const size_t n_states,const vector<vector<vector<size_t> > >& table_count,const bool miss_data)
{
	//node result;
	for(map<node_name_t,size_t>::iterator it=query_name_off.begin();
			it!=query_name_off.end();it++)
	{
		//node name:
		os<<"\nNode "<<it->first<<":"<<endl;
		size_t diff=distance(query_name_off.begin(),it);
		//header
		for(size_t i=0;i<n_states;i++)
			os<<"\tPred_"<<i;
		os<<"\n";
		//confusion table:
		vector<size_t> real(n_states+ (miss_data?1:0) ),pred(n_states);
		for(size_t i=0;i<n_states;i++){
			os<<"Real_"<<i;
			for(size_t j=0;j<n_states;j++){
				os<<"\t"<<table_count[diff][i][j];
				real[i]+=table_count[diff][i][j];
				pred[j]+=table_count[diff][i][j];
			}
			os<<"\t\t"<<real[i]<<"\n";
		}
		if(miss_data==true){
			os<<"Real_m";
			for(size_t j=0;j<n_states;j++){
				os<<"\t"<<table_count[diff][n_states][j];
				real[n_states]+=table_count[diff][n_states][j];
				pred[j]+=table_count[diff][n_states][j];
			}
			os<<"\t\t"<<real[n_states]<<"\n";
		}
		for(size_t i=0;i<n_states;i++)
			os<<"\t"<<pred[i];
		os<<endl;
	}
}
//output result as lines for each nodes, with a header.
//assume all query nodes have the same kinds of state;
void output_result_line(ostream & os,map<node_name_t,size_t>& query_name_off,
	const size_t n_states,const vector<vector<vector<size_t> > >& table_count)
{
	//header of table
	os<<"#Node_Name";
	for(size_t i=0;i<n_states;i++)
		for(size_t j=0;j<n_states;j++)
			os<<"\tR"<<i<<"_P"<<j;
	for(size_t i=0;i<n_states;i++)
		os<<"\t#R"<<i;
	for(size_t i=0;i<n_states;i++)
		os<<"\t#P"<<i;
	os<<"\ttotal"<<endl;
	for(map<node_name_t,size_t>::iterator it=query_name_off.begin();
			it!=query_name_off.end();it++)
	{
		os<<it->first;
		if(it->first.size()<=8)
			os<<"\t";
		size_t diff=distance(query_name_off.begin(),it);
		vector<size_t> real(n_states),pred(n_states);
		for(size_t i=0;i<n_states;i++){
			for(size_t j=0;j<n_states;j++){
				os<<"\t"<<table_count[diff][i][j];
				real[i]+=table_count[diff][i][j];
				pred[j]+=table_count[diff][i][j];
			}
		}
		size_t total=0;
		for(size_t i=0;i<n_states;i++){
			os<<"\t"<<real[i];
			total+=real[i];
		}
		for(size_t i=0;i<n_states;i++)
			os<<"\t"<<pred[i];
		os<<"\t"<<total<<"\n";
	}
}
