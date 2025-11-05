#ifndef APPROXIMATESHORTESTPATH_HPP
#define APPROXIMATESHORTESTPATH_HPP

#include "graph.hpp"
#include <vector>
#include <unordered_map>
#include <chrono>

class start_end{
public:
    int source;
    int target;
};

class start_end_result{
public:
    int source;
    int target;
    double approx_shortest_distance;
};

class ApproxShortestPath_result{
public:
    int id;
    std::vector<start_end_result> distances;
};

class ApproxShortestPath{
public:
    ApproxShortestPath_result approxmateshortestpath(Graph& graph, int id, std::vector<start_end>& queries, double time_budget_ms, double acceptable_error_pct);
};