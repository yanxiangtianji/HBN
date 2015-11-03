#include "StdAfx.h"
#include "DataHolder.h"


//offset:
void DataHolder::set_all_offsets(){
	all_offsets.clear();
	size_t n = data.size();
	for(size_t i=0;i<n;i++)
		all_offsets.push_back(i);
}


//mapping:
void DataHolder::set_offset_mapping(const vector<node_pointer_t>& off){
	offset_node_mapping=off;
	node_offset_mapping.clear();
	for(vector<node_pointer_t>::const_iterator it=off.begin();it!=off.end();++it){
		node_offset_mapping[*it]=it-off.begin();
	}
}

//count:

//find eligible offsets by (p,st) on whole set
//same as _get_data_offsets(offset_res,all_offsets,p,st);
size_t DataHolder::_get_data_offsets(vector<size_t>& offset_res,
	const node_pointer_t& p,const state_t& st)
{
	size_t pos=node_offset_mapping[p];
	size_t num=offset_res.size();
	data_t::const_iterator first=data.begin();
	for(data_t::const_iterator it=data.begin();it!=data.end();++it){
		if(it->at(pos)==st){
			offset_res.push_back(it-first);
		}
	}
	return offset_res.size()-num;
}

//find eligible offsets based on offset_base subset by (p,st)
size_t DataHolder::_get_data_offsets(vector<size_t>& offset_res,
	const vector<size_t>& offset_base,const node_pointer_t& p,const state_t& st)
{
	size_t pos=node_offset_mapping[p];
	size_t num=offset_res.size();
//	offset_res.clear();	//do it out off the function
	for(vector<size_t>::const_iterator it=offset_base.begin();it!=offset_base.end();++it){
		if(data[*it][pos]==st){
			//offset_res.push_back(it-offset_base.begin());
			offset_res.push_back(*it);
		}
	}
	return offset_res.size()-num;
}
//find eligible offsets by condition-set
size_t DataHolder::_get_data_offsets(vector<size_t>& res,const condition_t& condition)
{
	size_t num=res.size();
	size_t size=condition.size();
	vector<pair<size_t,state_t> > cond_p;
	for(condition_t::const_iterator it=condition.begin();it!=condition.end();++it){
		cond_p.push_back(make_pair(node_offset_mapping[it->first], it->second));
	}
	const data_t::const_iterator first=data.begin();
	auto end_cond_jt = cond_p.end();
	for(data_t::const_iterator it=data.begin();it!=data.end();++it){
		bool fit=true;
		for (auto jt = cond_p.begin(); fit && jt != end_cond_jt;++jt){
			fit = (it->at(jt->first) == jt->second);
		}
		if(fit==true){
			res.push_back(it-first);
		}
	}
	return res.size()-num;
}


//count number of eligible offsets by (p,st) on whole data set
//same as: _get_data_count(all_offsets,p,st)
size_t DataHolder::_get_data_count(const node_pointer_t& p,const state_t& st)
{
	size_t pos=node_offset_mapping[p];
	size_t count=0;
	for(data_t::const_iterator it=data.begin();it!=data.end();++it){
		if(it->at(pos)==st)
			count++;
	}
	return count;
}
//count number of eligible offsets by (p,st) on offsets subset
size_t DataHolder::_get_data_count(
	const vector<size_t>& offsets,const node_pointer_t& p,const state_t& st)
{
	size_t pos=node_offset_mapping[p];
	size_t count=0;
	for(vector<size_t>::const_iterator it=offsets.begin();it!=offsets.end();++it){
		if(data[*it][pos]==st)
			count++;
	}
	return count;
}
//count number of eligible offsets by condition-set on whole set
size_t DataHolder::_get_data_count(const condition_t &condition){
	size_t size=condition.size();
	size_t count=0;
	vector<pair<size_t,state_t> > cond_p;
	for (condition_t::const_iterator it = condition.begin(); it != condition.end(); ++it){
		cond_p.push_back(make_pair(node_offset_mapping[it->first], it->second));
	}
	const data_t::const_iterator first = data.begin();
	auto end_cond_jt = cond_p.end();
	for (data_t::const_iterator it = data.begin(); it != data.end(); ++it){
		bool fit = true;
		for (auto jt = cond_p.begin(); fit && jt != end_cond_jt; ++jt){
			fit = (it->at(jt->first) == jt->second);
		}
		if (fit == true){
			count++;
		}
	}
	return count;
}


//find eligible offsets base on offset_base subset by (p,st)
size_t DataHolder::_get_data_offsets_and_exclude(vector<size_t>& offset_res,
	vector<size_t>& offset_base,const node_pointer_t& p,const state_t& st)
{
	size_t pos=node_offset_mapping[p];
//	offset_res.clear();	//do it out off the function
	vector<size_t>::iterator first=offset_base.begin();
	for(vector<size_t>::iterator it=offset_base.begin();it!=offset_base.end();++it){
		if(data[*it][pos]==st){
			offset_res.push_back(*it);
		}else{
			*first++=*it;//move the rest to the beginning of "offset_base"
		}
	}
	offset_base.erase(first,offset_base.end());
	return offset_res.size();
}
//count number of eligible offsets by (p,st) on offsets subset
size_t DataHolder::_get_data_count_and_exclude(vector<size_t>& offsets,
	const node_pointer_t& p,const state_t& st)
{
	size_t pos=node_offset_mapping[p];
	size_t count=0;
	vector<size_t>::iterator first=offsets.begin();
	for(vector<size_t>::iterator it=offsets.begin();it!=offsets.end();++it){
		if(data[*it][pos]==st){
			count++;
//			*first++=*it;
		}else{
			*first++=*it;//move the rest to the beginning of "offset_base"
		}
	}
	offsets.resize(first-offsets.begin());
	return count;
}


void DataHolder::_get_data_offsets_split(vector<vector<size_t> >& offset_res,
	const vector<size_t>& offset_base,const node_pointer_t& p)
{
	size_t pos=node_offset_mapping[p];
	for(vector<size_t>::const_iterator it=offset_base.begin();it!=offset_base.end();++it){
		state_t st=data[*it][pos];
		if(st!=INVALID_STATE){
			offset_res[st].push_back(*it);
		}
	}
}