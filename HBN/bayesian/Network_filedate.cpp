#include "stdafx.h"
#include "Network.h"
#include "DataHolderMaker.h"

/*
Implementation for Network class.
file and data access part.
*/

/*
free functions:
*/

/*
member functions:
*/

int Network::read_node(ifstream &fin){
	int n_nodes;
	fin>>n_nodes;
	for(int n=0;n<n_nodes;n++){
		int n_states;
		string name;
		fin>>name>>n_states;//name as node name
		if (n_states <= 0){
			ostringstream oss;
			oss<< "Node: " << name << " only has "<<n_states<<" (less than 1) states!";
			throw logic_error(oss.str());
		}
		node_pointer_t p=make_shared<Node>(name,n_states);
		nodes.push_back(p);//add to container
		name_node_mapping[name]=p;
	}
	return n_nodes;
}
void Network::read_structure_cp(ifstream &fin,const bool with_cp,const bool with_md){
	//structure and cp
	for(size_t n=0;n<get_n_nodes();n++){
		size_t n_parents;
		string name;//multi-used
		vector<node_pointer_t> parents;
		fin>>name>>n_parents;//current node name
		node_pointer_t pnode=get_node(name);
		//marginal distribution:
		if(with_md){
			read_md(fin,pnode);
		}
		//structure (parents)
		for(size_t i=0;i<n_parents;i++){
			fin>>name;//each parent name/index
			node_pointer_t t=get_node(name);
			parents.push_back(t);
		}//parents
		pnode->set_parents(parents);
		//conditional possiblities:
		if(with_cp)
			read_cp(fin,pnode,parents);
	}
}
void Network::read_md(ifstream& fin,node_pointer_t& pnode){
	Node::md_t md(pnode->get_n_states());
	for(size_t i=0;i<pnode->get_n_states();i++){
		fin>>md[i];
	}
	pnode->set_default_cd_entry(md);
}
void Network::read_cp(ifstream& fin,node_pointer_t& pnode,const vector<node_pointer_t>& parents){
	size_t n_conditions=pnode->get_n_conditions();
	size_t n_states=pnode->get_n_states();
	map<node_pointer_t,state_t> condition;
	vector<double> cps;
	cps.reserve(n_states);
	for(size_t i=0;i<n_conditions;i++){
		condition.clear();
		for(size_t j=0;j<parents.size();j++){
			state_t st;
			fin>>st;//each state name/index
			condition[parents[j]]=st;
		}
		if(!fin.good()){//compacted cp table
			fin.clear();
			return;
		}
		cps.clear();
		for(size_t j=0;j<n_states;j++){
			double cp;
			fin>>cp;
			cps.push_back(cp);
		}
		pnode->set_cd_entry(condition,cps);
	}//conditional possibilities
}
int Network::read_prior_knowledge(ifstream &fin){
//level:
	int max_level=numeric_limits<int>::min();
	for(size_t i=0;i<get_n_nodes();i++){
		string node_name;
		int level;
		fin>>node_name>>level;
		//level information stored in nodes:
		node_pointer_t p=get_node(node_name);
		p->set_level(level);
		max_level=max(max_level,level);
//		nodes_in_level.resize(max(level,nodes_in_level.size()));
//		nodes_in_level[level].push_back(p);
	}
	//level information stores in network:
	nodes_in_level.resize(max_level+1);
	for(vector<node_pointer_t>::iterator it=nodes.begin();it!=nodes.end();++it){
		nodes_in_level[(*it)->get_level()].push_back(*it);
	}
	//setted alpha:
	//TODO:
	/*
	while(!fin.eof()){
		fin>>node;
		p=get_node(node);
		map<node_pointer_t,state_t> condition;
		//
	}
	*/
	return 0;
}

size_t Network::read_data(const string &name){
	DataHolderMaker dhm;
	pdata=dhm.from_csv(name,nodes);
	return pdata->size();
}

/*
//scoring:
//scoring--data:
size_t Network::_get_data_count(const data_t& data,const condition_t &condition){
	size_t count=0;
	vector<size_t> offset;
	for(condition_t::const_iterator it=condition.begin();it!=condition.end();++it){
		offset.push_back(node_offset_mapping[it->first]);
	}
	for(vector<vector<state_t> >::const_iterator it=data.begin();it!=data.end();++it){
		size_t i=0;
		for(condition_t::const_iterator itc=condition.begin();itc!=condition.end();++itc,i++){
			if(it->at(offset[i])!=itc->second)
				break;
		}
		if(i==offset.size()){
			count++;
		}
	}
	return count;
}
//find eligible offsets by condition-set
size_t Network::_get_data_offsets(const data_t& data,
	vector<size_t>& res,const condition_t& condition)
{
	vector<size_t> offset;
	for(condition_t::const_iterator it=condition.begin();it!=condition.end();++it){
		offset.push_back(node_offset_mapping[it->first]);
	}
	size_t count=0;
	for(data_t::const_iterator it=data.begin();it!=data.end();++it){
		bool fit=true;
		size_t i=0;
		for(condition_t::const_iterator itc=condition.begin();fit && itc!=condition.end();++itc,i++){
			if(it->at(offset[i])!=itc->second)
				fit=false;
		}
		if(fit==true){
			res.push_back(it-data.begin());
			count++;
		}
	}
	return count;
}
//find eligible offsets by (p,st) on whole set
size_t Network::_get_data_offsets(const data_t& data,vector<size_t>& offset_res,
								  const node_pointer_t& p,const state_t& st)
{
	size_t off=node_offset_mapping[p];
	size_t count=0;
	for(data_t::const_iterator it=data.begin();it!=data.end();++it){
		if(it->at(off)==st){
			offset_res.push_back(it-data.begin());
			count++;
		}
	}
	return count;
}
//find eligible offsets base on offset_base subset by (p,st)
size_t Network::_get_data_offsets(const data_t& data,vector<size_t>& offset_res,
	const vector<size_t>& offset_base,const node_pointer_t& p,const state_t& st)
{
	size_t off=node_offset_mapping[p];
	size_t count=0;
//	offset_res.clear();	//do it out off the function
	for(vector<size_t>::const_iterator it=offset_base.begin();it!=offset_base.end();++it){
		if(data[*it][off]==st){
			//offset_res.push_back(it-offset_base.begin());
			offset_res.push_back(*it);
			count++;
		}
	}
	return count;
}
//find eligible offsets base on offset_base subset by (p,st)
size_t Network::_get_data_offsets_and_exclude(const data_t& data,vector<size_t>& offset_res,
	vector<size_t>& offset_base,const node_pointer_t& p,const state_t& st)
{
	size_t off=node_offset_mapping[p];
//	offset_res.clear();	//do it out off the function
	vector<size_t>::iterator first=offset_base.begin();
	for(vector<size_t>::iterator it=offset_base.begin();it!=offset_base.end();++it){
		if(data[*it][off]==st){
			//offset_res.push_back(it-offset_base.begin());
			offset_res.push_back(*it);
			*first++=*it;
		}
	}
	offset_base.resize(first-offset_base.begin());
	return offset_res.size();
}

size_t Network::_get_data_count(const data_t& data,
	const vector<size_t>& offsets,const node_pointer_t& p,const state_t& st)
{
	size_t off=node_offset_mapping[p];
	size_t count=0;
	for(vector<size_t>::const_iterator it=offsets.begin();it!=offsets.end();++it){
		if(data[*it][off]==st)
			count++;
	}
	return count;
}
size_t Network::_get_data_count_and_exclude(const data_t& data,vector<size_t>& offsets,
	const node_pointer_t& p,const state_t& st)
{
	size_t off=node_offset_mapping[p];
	size_t count=0;
	vector<size_t>::iterator first=offsets.begin();
	for(vector<size_t>::iterator it=offsets.begin();it!=offsets.end();++it){
		if(data[*it][off]==st){
			count++;
			*first++=*it;
		}
	}
	offsets.resize(count);
	return count;
}
*/

//scoring--prior knowledge:
double Network::get_alpha3(const node_pointer_t& p,const condition_t& condition,const state_t st){
	return 1.0;
}
double Network::get_alpha2(const node_pointer_t& p,const condition_t& condition){
//	return 1.0;
	return p->get_n_states();
}

