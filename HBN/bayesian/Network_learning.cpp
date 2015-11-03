#include "stdafx.h"
#include "Network.h"

using namespace std::placeholders;
/*
free functions:
*/

//small learning functions:
inline bool dummy_1(const Node::pointer_t&){
	return true;
}
inline bool dummy_2(const Node::pointer_t&,const Node::pointer_t&){
	return true;
}
inline bool add_cons_inner(const Node::pointer_t& p_f,const Node::pointer_t& p_t){
//	return p_f!=p_t && p_f->check_parent_exist(p_t)==false;
	return p_f->get_level()>p_t->get_level() && p_f->check_parent_exist(p_t)==false;
}
inline bool delete_cons_inner(const Node::pointer_t& p_f,const Node::pointer_t& p_t){
//	return p_f!=p_t && p_f->check_parent_exist(p_t)==true;
	return p_f->get_level()>p_t->get_level() && p_f->check_parent_exist(p_t)==true;
}
inline bool reverse_cons_inner(const Node::pointer_t& p_f,const Node::pointer_t& p_t){
	return p_f!=p_t && p_f->check_parent_exist(p_t)==true;
}

//step functions:
void Network::_sf_all(score_t& b_value,Operation& b_op,
	 const Node::pointer_t& p_f,const Node::pointer_t& p_t)
{
	if(p_f!=p_t){
		if(p_f->check_parent_exist(p_t)==true){
			delete_fun(b_value,b_op,p_f,p_t);
			reverse_fun(b_value,b_op,p_f,p_t);
		}else
			add_fun(b_value,b_op,p_f,p_t);
	}
}
void Network::_sf_add_delete(score_t& b_value,Operation& b_op,
	const Node::pointer_t& p_f,const Node::pointer_t& p_t)
{
	if(p_f->get_level()>p_t->get_level()){
		if(p_f->check_parent_exist(p_t)==true){
			delete_fun(b_value,b_op,p_f,p_t);
		}else
			add_fun(b_value,b_op,p_f,p_t);
	}
}
void Network::_sf_add(score_t& b_value,Operation& b_op,
	const Node::pointer_t& p_f,const Node::pointer_t& p_t)
{
//	cout<<"_sf: "<<p_f.get()<<endl;

	if(p_f->get_level()>p_t->get_level()){
		if(p_f->check_parent_exist(p_t)==false)
			add_fun(b_value,b_op,p_f,p_t);
	}
}

/*
member functions:
*/

//learning:
bool Network::prepare_structure_learning(const string& f_date_name)
{
	//read data and other config:
	read_data(f_date_name);
	//initial:
	initial_cached_famscore();
	//working:
	cout.precision(4);
	return true;
}

void Network::do_operation(const Operation& op){
	switch (op.kind){//compressed expression
	case Operation::kind_t::OP_REVERSE:
		reset_cached_famscore(op.to);
	case Operation::kind_t::OP_ADD:
	case Operation::kind_t::OP_DELETE:
		reset_cached_famscore(op.from);
		break;
	default:
		break;
	}
	op.do_it();
}
inline void Network::undo_operation(const Operation& op,const double v_from,const double v_to){
	op.undo_it();
	set_cached_famscore(op.from,v_from);
	set_cached_famscore(op.to,v_to);
}

Network::score_t Network::test_op(const Operation& op){
	//TODO: just calculate the improvement by minus previous famscore with changed one
	//reset affected famscore flag
	double v_from,v_to;
	v_from=cached_famscore[op.from].second;//all famscores are ready before this method called
	v_to=cached_famscore[op.to].second;
	do_operation(op);
	score_t s=cal_score();
	undo_operation(op,v_from,v_to);
	return s;
}

void Network::relax(score_t& b_value,Operation& b_op,const Operation& op){
	score_t s=test_op(op);
	if(s.first<b_value.first || (s.first==b_value.first&&s.second>b_value.second)){
		b_value=s;
		b_op=op;
	}
}
void Network::do_on_enum_pair(score_t& b_value,Operation& b_op,cons1_fun_t cons_outer,cons2_fun_t cons_inner,
	void (Network::*fun)(score_t&,Operation&,const node_pointer_t&,const node_pointer_t&))
{
	for(vector<node_pointer_t>::const_iterator from=nodes.begin();from!=nodes.end();from++){
		if(!cons_outer(*from))
			continue;
		for(vector<node_pointer_t>::const_iterator to=nodes.begin();to!=nodes.end();to++){
			if(!cons_inner(*from,*to))
				continue;
//			cout<<get_node_name(*from)<<" "<<get_node_name(*to)<<endl;
			(this->*fun)(b_value,b_op,*from,*to);
		}
	}
}

void Network::learning_gready_base(
	const double min_improvement,const size_t max_iteration,
	vector<node_pointer_t>& froms,vector<node_pointer_t>& tos,
	function<void(score_t&,Operation&,const Node::pointer_t&,const Node::pointer_t&)>& fun)
{
	score_t NONE_VALUE=make_pair(nodes.size(),-numeric_limits<double>::max());
	score_t b_value_last,b_value;
	b_value=NONE_VALUE;
//	Operation NONE_OP(Operation::OP_NONE,node_pointer_t(),node_pointer_t());
	Operation b_op;//=NONE_OP;
	//gready:
	for(size_t iteration=1;iteration<=max_iteration;++iteration){
		cout<<"Iteration "<<iteration<<":"<<ends;
		clock_t clk0=clock(),clk1;
		b_value_last=b_value;
		b_value=NONE_VALUE;
		for(vector<node_pointer_t>::iterator it=froms.begin();it!=froms.end();++it){
			for(vector<node_pointer_t>::iterator jt=tos.begin();jt!=tos.end();++jt){
//				cout<<"base: "<<it->get()<<endl;
//				_sf_add(b_value,b_op,*it,*jt,offset_base);
				fun(b_value,b_op,*it,*jt);
//				fun(ref(b_value),ref(b_op),cref(*it),cref(*jt),cref(offset_base));
			}
		}
		
		clk1=clock();
		cout<<double(clk1-clk0)/CLOCKS_PER_SEC<<" seconds used."<<endl;
		cout<<"\tNow : "<<b_value.first<<" nInfs+ "<<b_value.second;
		cout<<"\n\tImrpovement : "<<b_value_last.first-b_value.first<<" #Infs+ "<<b_value.second-b_value_last.second;
		cout<<"\n\tBest operation : "<<b_op.get_kind_str();
		cout<<" ( "<<get_node_name(b_op.from)<<" , "<<get_node_name(b_op.to)<<" )"<<endl;
		if(b_value.first<b_value_last.first
			|| (b_value.first==b_value_last.first && b_value.second-b_value_last.second>=min_improvement))
		{
			do_operation(b_op);
			set_cached_famscore(b_op.from,cal_famscore(b_op.from));
			set_cached_famscore(b_op.to,cal_famscore(b_op.to));
		}else{
			cout<<"\t(Abandoned!)."<<endl;
			break;
		}
	};
}

void Network::learning_gready(const double min_improvement,const size_t max_iteration){
	vector<node_pointer_t> froms=nodes;
	vector<node_pointer_t> tos=nodes;
	function<void(score_t&,Operation&,
		const Node::pointer_t&,const Node::pointer_t&)> fun;
	fun=bind(&Network::_sf_add_delete,this,_1,_2,_3,_4);
	learning_gready_base(min_improvement,max_iteration,froms,tos,fun);
}
/*
void Network::learning_gready(const double min_improvement,const size_t max_iteration){
	score_t NONE_VALUE=make_pair(nodes.size(),-numeric_limits<double>::max());
	score_t b_value_last,b_value;
	b_value=NONE_VALUE;
	Operation NONE_OP(Operation::OP_NONE,node_pointer_t(),node_pointer_t());
	size_t iteration=0;
	Operation b_op=NONE_OP;
	cout.precision(4);
	//gready:
	do{
		iteration++;
		cout<<"Iteration "<<iteration<<":"<<ends;
		clock_t clk0=clock(),clk1,clk2;
		b_value_last=b_value;
		b_value=NONE_VALUE;
		//using namespace boost::lambda;
		//add:
		do_on_enum_pair(b_value,b_op,dummy_1,add_cons_inner,&Network::add_fun);
		//delete:
		clk1=clock();
		do_on_enum_pair(b_value,b_op,dummy_1,delete_cons_inner,&Network::delete_fun);
		//reverse:
//		do_on_enum_pair(b_value,b_op,dummy_1,reverse_cons_inner,&Network::reverse_fun);
		
		clk2=clock();
		cout<<double(clk2-clk0)/CLOCKS_PER_SEC<<" seconds used (ADD: "<<double(clk1-clk0)/CLOCKS_PER_SEC
			<<", DELETE: "<<double(clk2-clk1)/CLOCKS_PER_SEC<<")"<<endl;
		cout<<"\tNow : "<<b_value_last.first<<" nInfs+ "<<b_value.second;
		cout<<"\n\tImrpovement : "<<b_value_last.first-b_value.first<<" nInfs+ "<<b_value.second-b_value_last.second;
		cout<<"\n\tBest operation : "<<b_op.get_kind_str();
		cout<<" ( "<<get_node_name(b_op.from)<<" , "<<get_node_name(b_op.to)<<" )"<<endl;
		if(b_value.first<b_value_last.first || (b_value.first==b_value_last.first&&b_value.second>b_value_last.second)){
			do_operation(b_op);
			set_cached_famscore(b_op.from,cal_famscore(b_op.from));
			set_cached_famscore(b_op.to,cal_famscore(b_op.to));
		}else{
			cout<<"\t(Abandoned!)."<<endl;
		}
	}while((b_value.first<b_value_last.first || (b_value.first==b_value_last.first&&b_value.second-b_value_last.second>=min_improvement))
		&& iteration<=max_iteration);
	output_structure(cout);
}
*/

void Network::learning_gready_level(
	const double min_improvement,const size_t max_iteration)
{
	function<void(score_t&,Operation&,
		const Node::pointer_t&,const Node::pointer_t&)> fun;
	//function<bool(const node_pointer_t&,const node_pointer_t&)> cons;
//	there is no boost::lambda::_4, just use boost::bind
	fun=bind(&Network::_sf_add,this,_1,_2,_3,_4);
//	fun=bind(&Network::_sf_add_delete,this,_1,_2,_3,_4);
//	using namespace boost::lambda;
	srand(static_cast<unsigned>(time(0)));
//	function<bool(const state_t&)> pred=(lambda::_1!=0);

	vector<size_t> new_data_offset;//dynamic data offset
	vector<node_pointer_t> froms,tos;
	tos=nodes_in_level[0];
	for(size_t i=1;i<nodes_in_level.size();i++){
		cout<<"Level : "<<i<<" :"<<endl;
		tos.insert(tos.end(),froms.begin(),froms.end());
		froms=nodes_in_level[i];
		for(size_t j=0;j<froms.size();j++){
			cout<<"Node : "<<get_node_name(froms[j])<<endl;
			vector<node_pointer_t> temp_from(1,froms[j]);
//			learning_gready_base(min_improvement,max_iteration,temp,tos,fun,all_offsets);
//			clear_cached_famscore();
			learning_gready_base(min_improvement,max_iteration,temp_from,tos,fun);
		}
	}
	output_structure(cout,false,false);
}

