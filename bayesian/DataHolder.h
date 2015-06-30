#pragma once
#include "stdafx.h"
#include "def.h"
#include "Node.h"

using namespace std;

class DataHolder
{
//typedef:
public:
	typedef Node::pointer_t node_pointer_t;
	typedef map<node_pointer_t,state_t> condition_t;
//variable:
protected:
	//data:
	data_t data;
//	data_t& data;
	vector<size_t> all_offsets;	//global offsets containing all indexes setted once in read_data

	vector<node_pointer_t> offset_node_mapping;
	map<node_pointer_t,size_t> node_offset_mapping;//the position of node in data
//	map<node_name_t,node_pointer_t> name_node_mapping;
//	map<node_pointer_t,node_name_t> node_name_mapping;
public:
/*
	method:
*/
//self:
public:
	size_t size() const {
		return data.size();
	}
	const vector<size_t>& get_all_offsets() const {
		return all_offsets;
	}
//initialize:
public:
	DataHolder() = default;
	DataHolder(data_t&& pd,const vector<node_pointer_t>& off)
		:data(move(pd))
	{
		set_all_offsets();
		set_offset_mapping(off);
	}
private:
	void set_all_offsets();

	//mapping:
public:
	void set_offset_mapping(const vector<node_pointer_t>& off);

//count:
	//find eligible offsets by (p,st) on whole set
	size_t _get_data_offsets(vector<size_t>& offset_res,const node_pointer_t& p,const state_t& st);
	//find eligible offsets based on offset_base subset by (p,st)
	size_t _get_data_offsets(vector<size_t>& offset_res,
		const vector<size_t>& offset_base,const node_pointer_t& p,const state_t& st);
	//find eligible offsets by condition-set on whole set
	size_t _get_data_offsets(vector<size_t>& offset_res,const condition_t& condition);

	//count number of eligible offsets by (p,st) on whole data set
	size_t _get_data_count(const node_pointer_t& p,const state_t& st);
	//count number of eligible offsets by (p,st) on offsets subset
	size_t _get_data_count(const vector<size_t>& offsets,const node_pointer_t& p,const state_t& st);
	//count number of eligible offsets by condition-set on whole set
	size_t _get_data_count(const condition_t &condition);

	//find eligible offsets by (p,st) on whole set and exclude found
	size_t _get_data_offsets_and_exclude(vector<size_t>& offset_res,
		vector<size_t>& offset_base,const node_pointer_t& p,const state_t& st);
	//count number of eligible offsets by (p,st) on offsets subset
	size_t _get_data_count_and_exclude(vector<size_t>& offsets,const node_pointer_t& p,const state_t& st);


	void _get_data_offsets_split(vector<vector<size_t> >& offset_res,
		const vector<size_t>& offset_base,const node_pointer_t& p);

};
