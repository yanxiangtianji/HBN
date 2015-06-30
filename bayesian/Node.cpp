#include "stdafx.h"
#include "Node.h"

double Node::MD_JUDGE_RATIO=2.0;	//set on runtime

//free functions:
/*
bool operator<(const shared_ptr<Node> & l,const shared_ptr<Node> & r){
	return l->get_name()<r->get_name();
}
bool operator==(const shared_ptr<Node> & l,const shared_ptr<Node> & r){
	return l->get_name()==r->get_name();
}
*/

//member functions:
Node::Node(const node_name_t& nm,const size_t nstates,const int _level)
	:name(nm),n_states(nstates),is_md_ready(false),is_n_conditions_ready(false),
	is_def_cd_ready(false),n_conditions(0),level(_level)
{
}

//self:
size_t Node::cal_set_n_conditions(){
	n_conditions=(parents.size()==0?0:1);
	for(parents_t::iterator it=parents.begin();it!=parents.end();it++){
		n_conditions*=(*it)->get_n_states();
	}
	return n_conditions;
}

//parents:
Node::pointer_t Node::get_parent(const size_t offset) const{
	//return parents[offset];
	parents_t::iterator it = parents.begin();
	advance(it, offset);
	return *it;
}
//assume: nodes in condition are the same order with those in parents
void Node::set_parents(const parents_t& p){//set<pointer_t>
	is_md_ready = false;
	is_n_conditions_ready = false;
	parents = p;
}
void Node::set_parents(const vector<pointer_t>& p){
	is_md_ready = false;
	is_n_conditions_ready = false;
	parents.clear();
	parents.insert(p.begin(), p.end());
}
bool Node::add_parent(const pointer_t& p){
	pair<parents_t::iterator, bool> t = parents.insert(p);
	if (t.second == false)
		return false;
	is_md_ready = false;
	is_n_conditions_ready = false;
	return true;
}
bool Node::delete_parent(const pointer_t& p){
	size_t n = parents.erase(p);
	if (n == 0)
		return false;
	is_md_ready = false;
	is_n_conditions_ready = false;
	return true;
}



//possibility:
void Node::_dfs_add_md(const int p,	//how many parents' state is not setted
					   const double p_cdt,
					   condition_t& condition,
					   const vector<reference_wrapper<const md_t> >& par_dis
					   )
{
	//P(R=Rx,A=A1,B=B1,C=C1) = P(A=A1)*P(B=B1)*P(C=C1)*P(R=Rx|A=A1,B=B1,C=C1)
	//P(R=Rx) = SUM{ P(R=Rx|cond)*P(cond) } for all cond
	if (p_cdt == 0.0)
		return;
	else if(p<0){
		const md_t& cp=get_cd(condition);
		//n_state==md_vec.size()
		for(size_t i=0;n_states;i++)
			md_vec[i]+=p_cdt*cp[i];
		return;
	}
	pointer_t pt=get_parent(p);
	const md_t& p_dis = par_dis[p].get();
	for(size_t i=0;i<pt->get_n_states();i++){
		condition[pt]=i;
		_dfs_add_md(p - 1, p_cdt*p_dis[i], condition, par_dis);
	}
}

void Node::add_md(const vector<reference_wrapper<const md_t> >& par_dis){
	size_t p=get_n_parents();
	condition_t condition;
	//P(R=Rx,A=A1,B=B1,C=C1) = P(A=A1)*P(B=B1)*P(C=C1)*P(R=Rx|A=A1,B=B1,C=C1)
	//P(R=Rx) = SUM{ P(R=Rx|cond)*P(cond) } for all cond
	_dfs_add_md(p-1, 1.0, condition, par_dis);
}

void Node::cal_md(){
	//clear
	fill(md_vec.begin(),md_vec.end(),0.0);
	vector<reference_wrapper<const md_t> > par_dis;
	par_dis.reserve(get_n_parents());
	for(parents_t::iterator itp=parents.begin();itp!=parents.end();++itp){
		par_dis.push_back(ref((*itp)->get_md()));
	}
	add_md(par_dis);
}

void Node::set_md(const md_t& md){
	if(md.size()!=n_states){
		ostringstream oss;
		oss<<"new vector size ("<<md.size()<<") is not the same with n_states ("<<n_states<<")!";
		throw invalid_argument(oss.str());
	}
	md_vec=md;
	is_md_ready=true;
}
const Node::md_t& Node::get_md(){
	if(!is_md_ready){
		cal_md();
		is_md_ready=true;
	}
	return md_vec;
}

//cd:
void Node::set_default_cd_entry(const md_t& def_entry){
	default_cd_entry = def_entry;
	is_def_cd_ready = true;
	is_md_ready = false;
}
Node::md_t Node::cal_default_cd_entry(){
//	return _cal_default_cd_entry_uni();
	return _cal_default_cd_entry_avg();
}
Node::md_t Node::_cal_default_cd_entry_avg(){
	md_t def;
	def.resize(get_n_states());
	for(cd_table_t::const_iterator it=cd_table.begin();it!=cd_table.end();++it){
		const md_t& temp=it->second;
		for(size_t i=0;i<get_n_states();++i)
			def[i]+=temp[i];
	}
	for(size_t i=0;i<get_n_states();++i)
		def[i]/=cd_table.size();
	return def;
}
Node::md_t Node::_cal_default_cd_entry_uni(){
	return md_t(get_n_states(),1.0/get_n_states());
}

//static assert state
state_t Node::get_state_by_likely(const md_t& md){
	md_t::const_iterator it=max_element(md.begin(),md.end());
	return state_t(it-md.begin());
}

state_t Node::get_state_by_improve(const md_t& md,const md_t& ref,const double ratio){
	double largest=-1.0;
	size_t pos=-1;
	//try to set to biggest ratio one (above ratio)
	for(size_t i=0;i<md.size();i++){
		double t=md[i]/ref[i];
		if(t>ratio && t>largest){
			largest=t;
			pos=i;
		}
	}
	if(pos==-1){//if no one above the ratio, set to the most like one
		pos=0;
		for(size_t i=0;i<md.size();i++)
			if(md[i]>md[pos])
				pos=i;
	}
	return state_t(pos);
}
void Node::set_cd_entry(const condition_t& condition, const md_t& cd_entry){
	if (cd_entry.size() != get_n_states())
		throw invalid_argument("Entry size not fit.");
	is_md_ready = false;
	cd_table[condition] = cd_entry;
}


state_t Node::get_state(){
//	return get_state_by_likely(get_md());
	return get_state_by_improve(get_md(),get_default_cd_entry(),MD_JUDGE_RATIO);
}

//output:
void Node::output_condition(const condition_t& cond,ostream& os){
	//static
	//assume: nodes in condition are the same order with those in parents
	condition_t::const_iterator it=cond.begin();
	if(it!=cond.end())
		os<<it->second;
	for(++it;it!=cond.end();++it){
		os<<" "<<it->second;
	}
}
void Node::output_cd_table(ostream& os){
	for(cd_table_t::const_iterator it=cd_table.begin();it!=cd_table.end();++it){
		output_condition(it->first,os);
		for(md_t::const_iterator jt=it->second.begin();jt!=it->second.end();++jt){
			os<<' '<<*jt;
		}
		os<<'\n';
	}
}
