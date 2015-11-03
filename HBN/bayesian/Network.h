#pragma once

#include "stdafx.h"
#include "Node.h"
#include "Operation.h"
#include "def.h"
#include "DataHolder.h"

using namespace std;
//using namespace boost;

class Network
{
/* typedef: */
public:
	typedef Node::pointer_t node_pointer_t;

	typedef pair<size_t,double> score_t;

	typedef map<node_pointer_t,state_t> condition_t;
	typedef map<node_pointer_t,map<condition_t,map<state_t,double> > > alpha_table3_t;
	typedef map<node_pointer_t,map<condition_t,double> > alpha_table2_t;
	typedef vector<vector<state_t> > data_t;
protected:
	//for learning: (just calculate famscores of changed nodes)
//	typedef bool (*cons1_fun_t)(const node_pointer_t&);
	typedef function<bool(const node_pointer_t&)> cons1_fun_t;
//	typedef bool (*cons2_fun_t)(const node_pointer_t&,const node_pointer_t&);
	typedef function<bool(const node_pointer_t&,const node_pointer_t&)> cons2_fun_t;
	typedef map<node_pointer_t,pair<bool,double> > cached_famscore_t;
//	typedef void SFun_t(score_t& b_value,Operation& b_op,const Node::pointer_t& p_f,const Node::pointer_t& p_t);

/* variable: */
//nodes:
protected:
	map<node_name_t,node_pointer_t> name_node_mapping;
	vector<node_pointer_t> nodes;
	vector<vector<node_pointer_t> > nodes_in_level;
//training
protected:
	//data:
	shared_ptr<DataHolder> pdata;
	alpha_table3_t alpha_table3;
	alpha_table2_t alpha_table2;
	//score:
	cached_famscore_t cached_famscore;	//bool indicates whether the value is ready; initialed at structure_learning method

/* methods: */

// basic functions:
public:
	Network() = default;
	~Network() = default;
public:
	size_t get_n_nodes() const {
		return nodes.size();
	}
//	node_pointer_t& get_node_uniform(const string& str);
	const node_pointer_t& get_node(const node_name_t& name) const;
	const node_pointer_t& get_node(const size_t id) const{
		return nodes[id];
	}
	const node_name_t& get_node_name(const node_pointer_t& p) const;
	void clear_nodes();

//initialization:
public:
	//two seperated files for node and structure
	bool initial(const string& node_fname,const string& structure_fname,
		const bool with_cp,const bool with_md);
	bool initial_knowledge(const string& knowl_fname);
protected:
	int read_node(ifstream &fin);
	void read_md(ifstream& fin,node_pointer_t& pnode);
	void read_cp(ifstream& fin,node_pointer_t& pnode,const vector<node_pointer_t>& parents);
	void read_structure_cp(ifstream &fin,const bool with_cp,const bool with_md);
	int read_prior_knowledge(ifstream &fin);
	size_t read_data(const string &name);

//inference:
public:
	//domain methods
	map<node_name_t,state_t> 
		predict(map<node_name_t,state_t>& given,const vector<node_name_t> &query);
	map<node_name_t,pair<state_t,double> > 
		predict_with_poss(map<node_name_t,state_t>& given,const vector<node_name_t> &query);
protected:
	//working methods
	void trans_given(map<node_name_t,state_t>& org, map<node_pointer_t,Node::md_t> &given);
	void trans_query(const vector<node_name_t> &org ,map<node_pointer_t,Node::md_t>& query);
	void _predict(map<node_pointer_t,Node::md_t> &given,map<node_pointer_t,Node::md_t>& query);

//structure learning:
public:
	bool prepare_structure_learning(const string& f_date_name);

//learning
	void learning_gready(const double min_improvement,const size_t max_iteration=0xffffffff);
	void learning_gready_level(const double min_improvement,const size_t max_iteration=0xffffffff);

protected:
//scoring:

	//scan data late:
	double _dfs_cal_famscore_with_prior(
		const node_pointer_t &p,condition_t &condition,const int left);
	//scan data each time:
	double _dfs_cal_famscore_with_prior(
		const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx);
	double _dfs_cal_famscore_without_prior(
		const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx);
	double _dfs_cal_famscore_without_prior(
		const node_pointer_t &p,vector<size_t> &offset_base,const size_t idx);

	double cal_famscore(const node_pointer_t &p);
	double get_famscore(const node_pointer_t &p);

	//calculate every famscores and cache them
	void initial_cached_famscore();
	//make every cache slot empty
	void clear_cached_famscore();
	void reset_cached_famscore(const node_pointer_t &p){
		cached_famscore[p].first=false;
	}
	void set_cached_famscore(const node_pointer_t &p,const double famscore){
		cached_famscore_t::iterator it=cached_famscore.find(p);
		it->second.first=true;
		it->second.second=famscore;
	}

//scoring-interface:
	double get_alpha3(const node_pointer_t& p,const condition_t& condition,const state_t st);
	double get_alpha2(const node_pointer_t& p,const condition_t& condition);
	score_t cal_score();

protected:
//learning:
//	learning-helper:
	void add_fun(score_t& b_value,Operation& b_op,
		const Node::pointer_t& p_f,const Node::pointer_t& p_t){
		Operation op(Operation::kind_t::OP_ADD, p_f, p_t);
		relax(b_value,b_op,op);
	}
	void delete_fun(score_t& b_value,Operation& b_op,
		const Node::pointer_t& p_f,const Node::pointer_t& p_t){
		Operation op(Operation::kind_t::OP_DELETE, p_f, p_t);
		relax(b_value,b_op,op);
	}
	void reverse_fun(score_t& b_value,Operation& b_op,
		const Node::pointer_t& p_f,const Node::pointer_t& p_t){
		Operation op(Operation::kind_t::OP_REVERSE, p_f, p_t);
		relax(b_value,b_op,op);
	}
//	learning-step-function:
//	SFun_t _sf_all;
//	SFun_t _sf_add_delete;
//	SFun_t _sf_add;
	void _sf_all(score_t& b_value,Operation& b_op,
		const Node::pointer_t& p_f,const Node::pointer_t& p_t);
	void _sf_add_delete(score_t& b_value,Operation& b_op,
		const Node::pointer_t& p_f,const Node::pointer_t& p_t);
	void _sf_add(score_t& b_value,Operation& b_op,
		const Node::pointer_t& p_f,const Node::pointer_t& p_t);

//learning-interface:
	void do_operation(const Operation& op);
	void undo_operation(const Operation& op,const double v_from,const double v_to);
	score_t test_op(const Operation& op);
	void relax(score_t& b_value,Operation& b_op,const Operation& op);
	void do_on_enum_pair(score_t& b_value,Operation& b_op,cons1_fun_t cons_outer,cons2_fun_t cons_inner,
		void (Network::*fun)(score_t&,Operation&,const node_pointer_t&,const node_pointer_t&));
	void learning_gready_base(const double min_improvement,const size_t max_iteration,
		vector<node_pointer_t>& froms,vector<node_pointer_t>& tos,
		function<void(score_t&,Operation&,const Node::pointer_t&,const Node::pointer_t&)>& fun);

//conditional distribution:
public:
	void cal_cds(const bool with_md=false);
protected:
	//assume the conditions without data support is evenly distruted:
	void _dfs_cal_cd_with_prior(
		const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx);
	void _dfs_cal_cd_without_prior(
		const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx);
	//just set those conditions with data support
	void _dfs_cal_cd_without_prior_compact(
		const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx);

//output:
public:
	//whether with marginal distribution; whether with conditional distribution table
	void output_structure(ostream &os,const bool with_md,const bool with_cd);
	
//test:
public:

};

//small learning functions:
bool dummy_1(const Node::pointer_t&);
bool dummy_2(const Node::pointer_t&,const Node::pointer_t&);
bool add_cons_inner(const Node::pointer_t& p_f,const Node::pointer_t& p_t);
bool delete_cons_inner(const Node::pointer_t& p_f,const Node::pointer_t& p_t);
bool reverse_cons_inner(const Node::pointer_t& p_f,const Node::pointer_t& p_t);

const double DOUBLE_NEG_INF=log(0.0);
const double DOUBLE_POS_INF=-DOUBLE_NEG_INF;
