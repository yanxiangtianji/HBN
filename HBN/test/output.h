#include "stdafx.h"
#include "../bayesian/def.h"

using namespace std;

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

void output_result_summary(ostream & os,const size_t pics,
	const size_t real_normal_case,const size_t real_slow_case,
	const size_t pred_normal_case,const size_t pred_slow_case,
	const size_t right_normal,const size_t right_slow,
	const size_t miss_normal,const size_t miss_slow);

//output result as confusion tables for each node;
//assume all query nodes have the same kinds of state;
void output_result_table(ostream & os,map<node_name_t,size_t>& query_name_off,
	const size_t n_states,const vector<vector<vector<size_t> > >& table_count,const bool miss_data=false);

//output result as lines for each nodes, with a header.
//assume all query nodes have the same kinds of state;
void output_result_line(ostream & os,map<node_name_t,size_t>& query_name_off,
	const size_t n_states,const vector<vector<vector<size_t> > >& table_count);
