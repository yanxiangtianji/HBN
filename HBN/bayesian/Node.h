#pragma once

#include "stdafx.h"
#include "def.h"

using namespace std;
//using namespace boost;

class Node
{
/*
typedef:
*/
public:
	typedef shared_ptr<Node> pointer_t;
//	typedef size_t state_t;
	typedef vector<double> md_t;
	typedef set<pointer_t> parents_t;	//in the same order with that in contidion
	//typedef vector<pointer> parents_t;
	typedef map<pointer_t,state_t> condition_t;	//in the same order with that in parent
	typedef map<condition_t,md_t> cd_table_t;
	//typedef vector<vector<double> > cd_t;//need additional hash function to access
/*
variable:
*/
//self:
protected:
	bool is_n_conditions_ready;
	size_t n_states;
	size_t n_conditions;
	int level;
	node_name_t name;
//structure:
protected:
	parents_t parents;	//in the same order with that in contidion
//possibility:
protected:
	bool is_md_ready;
	md_t md_vec;	//marginal distribution
	cd_table_t cd_table;	//conditional distribution
	bool is_def_cd_ready;
	md_t default_cd_entry;	//default conditional distribution entry(those not setted in cd_table)
	static double MD_JUDGE_RATIO;	//used for judge state (cd/md > RATIO)
/*
method:
*/
//constructor & deconstructor:
public:
	Node():Node("", 0, 0){}
	Node(const node_name_t& nm):Node(nm, 0, 0){}
	Node(const node_name_t& nm,const size_t nstates,const int level=0);
//self:
protected:
	size_t cal_set_n_conditions();
public:
	const node_name_t& get_name() const;
	void set_n_state(const size_t nstate);
	size_t get_n_states() const;
	void set_level(const int lv);
	int get_level() const;
	size_t get_n_conditions(){
		if(!is_n_conditions_ready){
			cal_set_n_conditions();
			is_n_conditions_ready=true;
		}
		return n_conditions;
	}
//parent:
public:
	size_t get_n_parents() const;
	pointer_t get_parent(const size_t offset) const;
	const parents_t& get_parents() const;
	//assume: nodes in condition are the same order with those in parents
	void set_parents(const parents_t& p);//set<pointer_t>
	void set_parents(const vector<pointer_t>& p);
	bool check_parent_exist(const pointer_t& p) const;
	bool add_parent(const pointer_t& p);
	bool delete_parent(const pointer_t& p);

//possibility:
//marginal distribution:
protected:
	//P(R=Rx,A=A1,B=B1,C=C1) = P(A=A1)*P(B=B1)*P(C=C1)*P(R=Rx|A=A1,B=B1,C=C1)
	//P(R=Rx) = SUM{ P(R=Rx|cond)*P(cond) } for all cond
	void _dfs_add_md(const int p,const double p_cdt,condition_t& condition,
		const vector<reference_wrapper<const md_t> >& local_mds);
	//agent for _dfs_add_md
	void add_md(const vector<reference_wrapper<const md_t> >& local_mds);
	void cal_md();
public:
	void set_md(const md_t& md);
	const md_t& get_md();
	double get_md(const size_t state_idx);
	void clear_md_mark();
//state from md:
public:
	//assert current state by current marginal distribution
	static void set_judge_ratio(const double ratio){
		MD_JUDGE_RATIO=ratio;
	}
	static double get_judeg_ratio(){
		return MD_JUDGE_RATIO;
	}
	static state_t get_state_by_likely(const md_t& md);
	static state_t get_state_by_improve(const md_t& md,const md_t& ref,const double ratio);
	state_t get_state();	//get a state from current marginal distribution with configured method
	pair<state_t,double> get_state_with_poss(){
		state_t p=get_state();
		return make_pair(p, get_md()[p]);
	}

//conditional distribution:
protected:
	md_t cal_default_cd_entry();
	md_t _cal_default_cd_entry_avg();	//set default cd by averaging existing cd
	md_t _cal_default_cd_entry_uni();	//set default cd by uniform distribution
public:
	void set_default_cd_entry(const md_t& def_entry);
	const md_t& get_default_cd_entry(){
		//if nothing is preset, just calculated one
		if(is_def_cd_ready==false){
			set_default_cd_entry(cal_default_cd_entry());
			is_def_cd_ready=true;
		}
		return default_cd_entry;
	}
	void set_cd(cd_table_t&& cdt);
	void set_cd(const cd_table_t& cdt);
	const cd_table_t& get_cd() const;
	void set_cd_entry(const condition_t& condition, const md_t& cd_entry);
	const md_t& get_cd(const condition_t& condition){
		//Assume all the conditional distribution not it the condition table is even distribution
		cd_table_t::iterator it=cd_table.find(condition);
		if(it!=cd_table.end())
			return it->second;
		return get_default_cd_entry();
	}
	bool check_cd_entry(const condition_t& condition) const;	//check whether an entry with the specific condition exists

//output:
protected:
	
public:
	static void output_condition(const condition_t& cond,ostream& os);
	//assume: nodes in condition are the same order with those in parents
	void output_cd_table(ostream& os);
};

//self:
inline const node_name_t& Node::get_name() const{
	return name;
}
inline void Node::set_n_state(const size_t nstate){
	n_states = nstate;
	md_vec.clear();
	md_vec.resize(n_states,0.0);
	is_md_ready = false;
}
inline size_t Node::get_n_states() const{
	return n_states;
}
inline void Node::set_level(const int lv){
	level = lv;
}
inline int Node::get_level() const{
	return level;
}


//parent:
inline const Node::parents_t& Node::get_parents() const{
	return parents;
}
inline size_t Node::get_n_parents() const{
	return parents.size();
}
inline bool Node::check_parent_exist(const pointer_t& p) const{
	return parents.find(p) != parents.end();
}

//probability:
//md:
inline double Node::get_md(const size_t state_idx){
	return get_md()[state_idx];
}
inline void Node::clear_md_mark(){
	is_md_ready = false;
}
//cd:
inline void Node::set_cd(cd_table_t&& cdt){
	is_md_ready = false;
	cd_table = move(cdt);
}
inline void Node::set_cd(const cd_table_t& cdt){
	is_md_ready = false;
	cd_table = cdt;
}
inline const Node::cd_table_t& Node::get_cd() const{
	return cd_table;
}
inline bool Node::check_cd_entry(const condition_t& condition) const{
	return cd_table.find(condition) != cd_table.end();
}



//bool operator<(const shared_ptr<Node> & l,const shared_ptr<Node> & r);
//bool operator==(const shared_ptr<Node> & l,const shared_ptr<Node> & r);
