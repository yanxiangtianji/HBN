#include "StdAfx.h"
#include "DataHolderMaker.h"


void DataHolderMaker::parse_csv_line(data_t& res,const vector<string>& line){
	vector<state_t> sl;
	for(size_t i=0;i<line.size();i++){
//		sl.push_back(Util::str2int(line[i]).second);
		sl.push_back(stoi(line[i]));
	}
	res.push_back(sl);
}

shared_ptr<DataHolder> DataHolderMaker::from_csv(
	const string& filename,const vector<node_pointer_t>& nodes)
{
	CSVreader reader(filename);
	//read and store column index - node mapper
	vector<node_pointer_t> off;
	for (const string& head : reader.get_header()){
		size_t j;
		for (j = 0; j<nodes.size(); ++j){
			if (nodes[j]->get_name() == head){
				off.push_back(nodes[j]);
				break;
			}
		}
		if (j == nodes.size()){
			ostringstream oss;
			oss << j << "th node name: " << nodes[j]->get_name() << " is not a legal node name.";
			throw domain_error(oss.str());
		}
	}
	/*
	reader.read_header();
	for(size_t i=0;i<reader.header.size();i++){
		//node_pointer_t p=get_node(reader.header[i]);
		//offset_node_mapping.push_back(p);
		//node_offset_mapping[p]=i;
		size_t j;
		for(j=0;j<nodes.size();++j){
			if(nodes[j]->get_name()==reader.header[i]){
				off.push_back(nodes[j]);
				break;
			}
		}
		if(j==nodes.size()){
			ostringstream oss;
			oss<<j<<"th node name: "<<nodes[j]->get_name()<<" is not a legal node name.";
			throw domain_error(oss.str());
		}
	}*/
	//read and process data;
	//data_t* pdata=new data_t();
	//function<void(data_t&,const vector<string>&)> fun=bind(parse_csv_line,_1,_2);
	//size_t line_num=reader.read_data(*pdata,fun);
	auto fun=[](const vector<string>& line){
		vector<state_t> sl;
		sl.reserve(line.size());
		for (size_t i = 0; i<line.size(); i++){
			sl.push_back(stoi(line[i]));
		}
		sl.shrink_to_fit();
		return sl;
	};
	auto x=reader.get_data<vector<state_t>>(fun);
	return make_shared<DataHolder>(move(x), off);
	//set the global offsets;
}