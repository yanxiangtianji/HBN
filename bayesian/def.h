#pragma once
#include <vector>
#include <string>

typedef int state_t;
static const state_t INVALID_STATE=-1;

typedef std::vector<std::vector<state_t> > data_t;

typedef std::string node_name_t;