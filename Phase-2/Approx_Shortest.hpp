#ifndef APPROX_HPP
#define APPROX_HPP

#include "graph.hpp"
#include <vector>
#include <string>
#include <queue> 
#include <map>
#include <algorithm>
#define _USE_MATH_DEFINES
#include <cmath>
class Graph;
class Node;
class ApproxShortest_Result {
public:
    int id;
    std::vector<std::tuple<int,int,double>> distances;
    ApproxShortest_Result(int id, std::vector<std::tuple<int,int,double>> distances) : id(id), distances(distances) {};
};

class ApproxShortest {
public:
    ApproxShortest_Result findApprox(Graph& graph, int id, std::vector<std::pair<int,int>>& queries,double time_budget, double max_error);
    double Hybrid_A_Star(Graph& graph,double time_limit,int source,int target,double upper_bound);
    double heuristic_distance(const Node& a, const Node& b);
};

#endif