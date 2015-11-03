#include "stdafx.h"
#include "Operation.h"

Operation::Operation(const kind_t& k, const Node::pointer_t& f, const Node::pointer_t& t)
:kind(k), from(f), to(t)
{
}


string Operation::get_kind_str() const{
	string res;
	switch (kind){
	case kind_t::OP_NONE:
		res = "NONE";
		break;
	case kind_t::OP_ADD:
		res = "ADD";
		break;
	case kind_t::OP_DELETE:
		res = "DELETE";
		break;
	case kind_t::OP_REVERSE:
		res = "REVERSE";
		break;
	}
	return res;
}

//do operation:
bool Operation::do_it() const{
	bool res = true;
	switch (kind){
	case kind_t::OP_ADD:
		res &= from->add_parent(to);
		break;
	case kind_t::OP_DELETE:
		res &= from->delete_parent(to);
		break;
	case kind_t::OP_REVERSE:
		res &= from->delete_parent(to);
		res = res && (to->add_parent(from));//if first step failed second step should not execute
		break;
	default:
		res = false;
	}
	return res;
}

bool Operation::undo_it() const{
	bool res = true;
	switch (kind){
	case kind_t::OP_ADD:
		res &= from->delete_parent(to);
		break;
	case kind_t::OP_DELETE:
		res &= from->add_parent(to);
		break;
	case kind_t::OP_REVERSE:
		res &= to->delete_parent(from);
		res = res && (from->add_parent(to));//if first step failed second step should not execute
		break;
	default:
		res = false;
	}
	return res;
}
