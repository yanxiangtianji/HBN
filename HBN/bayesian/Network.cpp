#include "stdafx.h"
#include "Network.h"

/*
free functions:
*/


/*
member functions:
*/

//nodes:
//Network::node_pointer_t& Network::get_node_uniform(const string& str){
//	pair<bool,size_t> t_res=Util::str2sizet(str);
//	return t_res.first?get_node(t_res.second):get_node(str);
//}

const Network::node_pointer_t& Network::get_node(const node_name_t& name) const{
	auto it=name_node_mapping.find(name);
	if(it==name_node_mapping.end()){
		throw invalid_argument(string("Unknown node name: ").append(name));
	}
	return it->second;
}
const node_name_t& Network::get_node_name(const node_pointer_t& p) const{
	return p->get_name();
}
void Network::clear_nodes(){
	nodes.clear();
	name_node_mapping.clear();
	nodes_in_level.clear();
}

//initialization:
bool Network::initial(const string& node_fname,const string& structure_fname,
	const bool with_cp,const bool with_md)
{
	//node-self
	ifstream fin(node_fname);
	if(!fin)
		return false;
	read_node(fin);
	size_t n_nodes=nodes.size();
	fin.close();
	fin.clear();
	//structure and cp
	fin.open(structure_fname.c_str());
	if(!fin)
		return false;
	read_structure_cp(fin,with_cp,with_md);
	fin.close();
	return true;
}

bool Network::initial_knowledge(const string& knowl_fname){
	ifstream fin(knowl_fname.c_str());
	if(!fin)
		return false;
	read_prior_knowledge(fin);
	fin.close();
	return true;
}

//inference:
void Network::_predict(map<node_pointer_t,Node::md_t> &given,map<node_pointer_t,Node::md_t>& query){
	for(vector<node_pointer_t>::iterator it=nodes.begin();it!=nodes.end();++it){
		(*it)->clear_md_mark();
	}
	//set given:
	for(map<node_pointer_t,Node::md_t>::iterator it=given.begin();it!=given.end();it++){
		it->first->set_md(it->second);
	}
	//predict all query:
	for(map<node_pointer_t,Node::md_t>::iterator it=query.begin();it!=query.end();it++){
//		it->first->clear_md_mark();
		it->second=it->first->get_md();
	}
}

void Network::trans_given(map<node_name_t,state_t>& org, map<node_pointer_t,Node::md_t> &given){
	Node::md_t temp_md;
	//transform given
	for(map<node_name_t,state_t>::iterator it=org.begin();it!=org.end();it++){
		temp_md.clear();
		node_pointer_t p=get_node(it->first);
		temp_md.resize(p->get_n_states(),0.0);
		temp_md[it->second]=1.0;
		given[p]=temp_md;
	}
}
void Network::trans_query(const vector<node_name_t> &org ,map<node_pointer_t,Node::md_t>& query){
	for(vector<node_name_t>::const_iterator it=org.begin();it!=org.end();it++){
		query[get_node(*it)];
	}
}
map<node_name_t,state_t> Network::predict(
	map<node_name_t,state_t>& given,const vector<node_name_t> &query)
{
	map<node_pointer_t,Node::md_t> _given,_query;
	//transform given:
	trans_given(given,_given);
	//transform query:
	vector<node_name_t> query_clear;
	vector<node_name_t> query_in_given;
	for(vector<node_name_t>::const_iterator it=query.begin();it!=query.end();it++){
		if(given.find(*it)==given.end())
			query_clear.push_back(*it);
		else
			query_in_given.push_back(*it);
	}
	trans_query(query_clear,_query);
	_predict(_given,_query);
	//prepare result:
	map<node_name_t,state_t> res;
	for(map<node_pointer_t,Node::md_t>::iterator it=_query.begin();it!=_query.end();it++){
		//use the static get_state function with parameter it->second for query level parallelization
		res[get_node_name(it->first)]=it->first->get_state();
		//Node::get_state_by_distribution(it->second).first;
		//res[get_node_name(it->first)]=it->first->get_state();
	}
	//add those in given
	for(vector<node_name_t>::iterator it=query_in_given.begin();it!=query_in_given.end();it++){
		res[*it]=given[*it];
	}
	return res;
}
map<node_name_t,pair<state_t,double> > Network::predict_with_poss(
	map<node_name_t,state_t>& given,const vector<node_name_t> &query)
{
	map<node_pointer_t,Node::md_t> _given,_query;
	trans_given(given,_given);//transform given
	vector<node_name_t> query_clear;
	vector<node_name_t> query_in_given;
	for(vector<node_name_t>::const_iterator it=query.begin();it!=query.end();it++){
		if(given.find(*it)==given.end())
			query_clear.push_back(*it);
		else
			query_in_given.push_back(*it);
	}
	trans_query(query_clear,_query);//transform query
	_predict(_given,_query);//work
	//prepare result:
	map<node_name_t,pair<state_t,double> > res;
	for(map<node_pointer_t,Node::md_t>::iterator it=_query.begin();it!=_query.end();it++){
		//use the static one with parameter it->second for query level parallelization
		res[get_node_name(it->first)]=it->first->get_state_with_poss();
		//Node::get_state_by_distribution(it->second);
		//res[get_node_name(it->first)]=it->first->get_state();
	}
	for(vector<node_name_t>::iterator it=query_in_given.begin();it!=query_in_given.end();it++){
		res[*it]=make_pair(given[*it],1.0);
	}
	return res;
}


//calculate conditional distribution:
void Network::_dfs_cal_cd_with_prior(
	const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx)
{
	if(idx>=p->get_n_parents()){
		//double tot=math::tgamma_delta_ratio(get_alpha2(p,condition),offset_base.size());
		double tot=get_alpha2(p,condition)+offset_base.size();
		vector<double> cd_entry;
		for(size_t i=0;i<p->get_n_states();i++){
			//double t=math::tgamma_delta_ratio(get_alpha3(p,condition,i),_get_data_count(offset_base,p,i));
			double t=get_alpha3(p,condition,i)+pdata->_get_data_count(offset_base,p,i);
			//cd_entry.push_back(tot/t);
			cd_entry.push_back(t/tot);
		}
		p->set_cd_entry(condition,cd_entry);
		return;
	}
	node_pointer_t pparent=p->get_parent(idx);
	vector<size_t> offsets;
	for(size_t i=0;i<pparent->get_n_states();i++){
		//prepare the data offsets
		condition[pparent]=i;
		pdata->_get_data_offsets(offsets,offset_base,pparent,i);
		_dfs_cal_cd_with_prior(p,condition,offsets,idx+1);
		offsets.clear();
	}
}
void Network::_dfs_cal_cd_without_prior(
	const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx)
{
	if(idx>=p->get_n_parents()){
		size_t tot=offset_base.size();
		vector<double> cd_entry;
		if(tot==0){
			//not in data: assume evenly (1/n)
			cd_entry.resize(p->get_n_states(),1.0/p->get_n_states());
		}else
			for(size_t i=0;i<p->get_n_states();i++){
				double t=pdata->_get_data_count(offset_base,p,i);
				cd_entry.push_back(t/tot);
			}
		p->set_cd_entry(condition,cd_entry);
		return;
	}
	const node_pointer_t& pparent=p->get_parent(idx);
	vector<size_t> offsets;
	for(size_t i=0;i<pparent->get_n_states();i++){
		//prepare the data offsets
		condition[pparent]=i;
		pdata->_get_data_offsets(offsets,offset_base,pparent,i);
		_dfs_cal_cd_with_prior(p,condition,offsets,idx+1);
		offsets.clear();
	}
}
void Network::_dfs_cal_cd_without_prior_compact(
	const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx)
{
	if(offset_base.empty())
		return;
	if(idx>=p->get_n_parents()){
		size_t tot=0;//offset_base.size();
		vector<double> cd_entry;
		for(size_t i=0;i<p->get_n_states();i++){
			size_t t=pdata->_get_data_count(offset_base,p,i);
			tot+=t;//deal with miss;
			cd_entry.push_back(t);
		}
		if(tot==0)
			return;
		for(size_t i=0;i<p->get_n_states();++i)
			cd_entry[i]/=tot;
		p->set_cd_entry(condition,cd_entry);
		return;
	}
	const node_pointer_t& pparent=p->get_parent(idx);
	vector<size_t> offsets;
	for(size_t i=0;i<pparent->get_n_states();i++){
		//prepare the data offsets
		condition[pparent]=i;
		pdata->_get_data_offsets(offsets,offset_base,pparent,i);
		_dfs_cal_cd_without_prior_compact(p,condition,offsets,idx+1);
		offsets.clear();
	}
}

void Network::cal_cds(const bool with_md){
	condition_t cond;	//used as a temporal input holder
//	function<bool(const state_t&)> pred=(lambda::_1!=0);
	function<bool(const state_t&)> pred=[](const state_t& st){return st!=0;};
	vector<size_t> new_data_offset;
	for(vector<node_pointer_t>::iterator it=nodes.begin();it!=nodes.end();it++){
		if(with_md){
			size_t tot=0;//(*it)->get_n_states();
			Node::md_t def;
			for(size_t i=0;i<(*it)->get_n_states();i++){
				size_t t=pdata->_get_data_count(*it,i);
				tot+=t;
				def.push_back(t);
			}
			for(size_t i=0;i<(*it)->get_n_states();i++){
				def[i]/=tot;
			}
			(*it)->set_default_cd_entry(def);
		}
		if((*it)->get_n_parents()==0)
			continue;
		//_dfs_cal_cd_with_prior(*it,cond,all_offsets,0);
		_dfs_cal_cd_without_prior_compact(*it,cond,pdata->get_all_offsets(),0);
		cond.clear();
	}
}

//output:
void Network::output_structure(ostream &os,const bool with_md,const bool with_cd){
	for(map<node_name_t,node_pointer_t>::iterator it=name_node_mapping.begin();it!=name_node_mapping.end();++it){
		//node name & # of parents
		os<<it->first<<' '<<it->second->get_n_parents()<<'\n';
		const Node::parents_t& parents=it->second->get_parents();
		//marginal probability
		if(with_md){
			const Node::md_t md=it->second->get_default_cd_entry();
			for(size_t i=0;i<md.size();++i){
				if(i!=0)
					os<<' ';
				os<<md[i];
			}
			os<<'\n';
		}
		//parents
		for(Node::parents_t::const_iterator itp=parents.begin();itp!=parents.end();++itp){
			if(itp!=parents.begin())
				os<<' ';
			os<<get_node_name(*itp);
		}
		if(!parents.empty())
			os<<'\n';
		//Node class assume: nodes in condition are the same order with those in parents
		if(with_cd)
			it->second->output_cd_table(os);
	}
}

