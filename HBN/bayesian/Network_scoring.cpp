#include "stdafx.h"
#include "Network.h"

//using namespace boost::math;
/*
free functions:
*/


/*
member functions:
*/
//structure learning:
//scoring:

//scoring--calculation
//return (num of negative infinites in famscores, sum of the rest famscores)
Network::score_t Network::cal_score(){
	double score=0.0;
	size_t n_inf=0;
	for(vector<node_pointer_t>::iterator it=nodes.begin();it!=nodes.end();it++){
		double t=get_famscore(*it);
		if(t==DOUBLE_NEG_INF)
			n_inf++;
		else
			score+=t;
	}
	return make_pair(n_inf,score);
}

//get the famscore with a cache mechanism
double Network::get_famscore(const node_pointer_t &p){
	cached_famscore_t::iterator it=cached_famscore.find(p);
	if(it->second.first==true)
		return it->second.second;
	double t=cal_famscore(p);
	it->second.second=t;
	it->second.first=true;	//thread safe
	return t;
}

inline double Network::cal_famscore(const node_pointer_t &p){
	condition_t temp;
//	return _dfs_cal_famscore_with_prior(p,temp,all_offsets,0);
	return _dfs_cal_famscore_without_prior(p,temp,pdata->get_all_offsets(),0);
	vector<size_t> offset=pdata->get_all_offsets();
	return _dfs_cal_famscore_without_prior(p,offset,0);
}
/*
double Network::_dfs_cal_famscore_with_prior(const node_pointer_t &p,condition_t &condition,const int idx){
	if(idx<0){
		vector<size_t> offsets;
		size_t total=_get_data_offsets(offsets,condition);
//		condition_t data_condition(condition);
		double res=log(math::tgamma_delta_ratio(get_alpha2(p,condition),total));
		for(size_t i=0;i<p->get_n_states() && res!=DOUBLE_NEG_INF;i++){
//			data_condition[p]=i;
//			res/=math::tgamma_delta_ratio(get_alpha3(p,condition,i),_get_data_count(data_condition));
			res-=log(math::tgamma_delta_ratio(get_alpha3(p,condition,i),_get_data_count(offsets,p,i)));
		}
		//return res*math::tgamma_delta_ratio(get_alpha2(p,condition),_get_data_count(condition));
		return res;
	}
	double res=0.0;
	node_pointer_t node=p->get_parent(idx);
	for(size_t i=0;i<node->get_n_states();i++){
		condition[node]=i;
		res+=_dfs_cal_famscore_with_prior(p,condition,idx-1);
	}
	return res;
}
double Network::_dfs_cal_famscore_with_prior(
	const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx)
{
	if(idx>=p->get_n_parents()){
		//more fast: divide each part together and log once.
		//more accurate: log each part and minus together.
		size_t total=offset_base.size();
//		double res=math::tgamma_delta_ratio(get_alpha2(p,condition),total);	//fast
		double res=log(math::tgamma_delta_ratio(get_alpha2(p,condition),total));	//accurate
		for(size_t i=0;i<p->get_n_states() && res!=DOUBLE_NEG_INF;i++){
//			res/=math::tgamma_delta_ratio(get_alpha3(p,condition,i),_get_data_count(offset_base,p,i));	//fast
			res-=log(math::tgamma_delta_ratio(get_alpha3(p,condition,i),_get_data_count(offset_base,p,i)));	//accurate
		}
//		return log(res);	//fast
		return res;	//accurate
	}
	double res=0.0;
	node_pointer_t node=p->get_parent(idx);
	vector<size_t> offsets;
	for(size_t i=0;i<node->get_n_states() && res!=DOUBLE_NEG_INF;i++){
		//prepare the data offsets
		condition[node]=i;
		_get_data_offsets(offsets,offset_base,node,i);
		res+=_dfs_cal_famscore_with_prior(p,condition,offsets,idx+1);
		offsets.clear();
	}
	return res;
}
*/
double Network::_dfs_cal_famscore_without_prior(
	const node_pointer_t &p,condition_t &condition,const vector<size_t> &offset_base,const size_t idx)
{
	if(offset_base.empty()){//shortcut
		static const double none_data_score=0.0;//log(gamma(1.0))==0.0
		return none_data_score;
	}
	if(idx>=p->get_n_parents()){
		//more fast: divide each part together and log once.
		//more accurate: log each part and minus together.
		size_t N=offset_base.size();
		double res=-lgamma(N+1.0);
		vector<size_t> offset_temp=offset_base;
		for(size_t i=0;i<p->get_n_states() && res!=DOUBLE_NEG_INF;i++){
//			res+=math::lgamma(pdata->_get_data_count(offset_base,p,i)+1.0);
			res+=lgamma(pdata->_get_data_count_and_exclude(offset_temp,p,i)+1.0);
		}
		return res;
	}
	double res=0.0;
	node_pointer_t node=p->get_parent(idx);
//	vector<size_t> offsets;
	vector<vector<size_t> > offsets(node->get_n_states());
	pdata->_get_data_offsets_split(offsets,offset_base,node);
	for(size_t i=0;i<node->get_n_states() && res!=DOUBLE_NEG_INF;i++){
		//prepare the data offsets
		condition[node]=i;
//		pdata->_get_data_offsets(offsets,offset_base,node,i);
		res+=_dfs_cal_famscore_without_prior(p,condition,offsets[i],idx+1);
//		offsets.clear();
	}
	return res;
}
double Network::_dfs_cal_famscore_without_prior(
	const node_pointer_t &p,vector<size_t> &offset_base,const size_t idx)
{
	if(offset_base.empty()){//shortcut
		static double none_data_score=0.0;//log(gamma(1.0))==0.0
		return none_data_score;
	}
	if(idx>=p->get_n_parents()){
		size_t N=offset_base.size();
		double res=-lgamma(N+1.0);//1.0 is assumed alpha value
		for(size_t i=0;i<p->get_n_states() && res!=DOUBLE_NEG_INF;i++){
			res+=lgamma(pdata->_get_data_count_and_exclude(offset_base,p,i)+1.0);
		}
		return res;
	}
	double res=0.0;
	node_pointer_t node=p->get_parent(idx);
	vector<size_t> offsets;
	for(size_t i=0;i<node->get_n_states() && res!=DOUBLE_NEG_INF;i++){
		//prepare the data offsets
		offsets.clear();
		pdata->_get_data_offsets_and_exclude(offsets,offset_base,node,i);
		res+=_dfs_cal_famscore_without_prior(p,offsets,idx+1);
	}
	return res;
}

/*
cached_famscose:
*/

//make every cache slot empty
void Network::clear_cached_famscore(){
	const pair<bool,double> dummy_famscore=make_pair(false,0.0);
	for(vector<node_pointer_t>::const_iterator it=nodes.begin();it!=nodes.end();it++)
		cached_famscore[*it]=dummy_famscore;
}
//calculate every famscores and cache them
void Network::initial_cached_famscore(){
	for(vector<node_pointer_t>::iterator it=nodes.begin();it!=nodes.end();it++){
		cached_famscore[*it]=make_pair(true,cal_famscore(*it));
	}
}
