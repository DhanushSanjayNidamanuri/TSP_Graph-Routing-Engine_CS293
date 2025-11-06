#ifndef APPROX_HPP
#define APPROX_HPP

#include "graph.hpp"
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <algorithm>
class ApproxShortest_Result {
public:
    int id;
    std::vector<std::tuple<int,int,int>> distances;
    ApproxShortest_Result(int id, std::vector<std::tuple<int,int,int>> distances) : id(id), distances(distances) {};
};

class ApproxShortest {
public:
    ApproxShortest_Result findApprox(Graph& graph, int id, std::vector<std::pair<int,int>> queries, int time_budget, double max_error);
};

#endif