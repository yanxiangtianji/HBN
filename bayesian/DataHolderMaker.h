#pragma once
#include "stdafx.h"
#include <CSVreader.h>
#include "def.h"
#include "DataHolder.h"

using namespace std;

//input process function
class DataHolderMaker
{
	typedef DataHolder::node_pointer_t node_pointer_t;
protected:
	static void parse_csv_line(data_t& res,const vector<string>& line);
public:
	DataHolderMaker() = default;
	~DataHolderMaker() = default;
public:
	shared_ptr<DataHolder> from_csv(const string& filename,const vector<node_pointer_t>& nodes);
};
