#pragma once

#include "stdafx.h"
#include "Node.h"

using namespace std;

//a POD class
class Operation
{
/* type: */
public:
	enum class kind_t{OP_NONE=0,OP_ADD,OP_DELETE,OP_REVERSE};
/* variables: */
	kind_t kind;
	Node::pointer_t from,to;
public:

/* methods: */
//constructor & deconstructor:
public:
	Operation(const kind_t& k,const Node::pointer_t& f,const Node::pointer_t& t);
	Operation() :kind(kind_t::OP_NONE){}
	~Operation() = default;
//do operation:
public:
	bool do_it() const;
	bool undo_it() const;
	string get_kind_str() const;
};
