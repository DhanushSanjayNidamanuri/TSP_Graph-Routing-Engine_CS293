#ifndef KSHORTESTPATH_HPP
#define KSHORTESTPATH_HPP

#include "graph.hpp"
#include <vector>
#include <string>

class path{
public:
    std::vector<int> path;
    double length;
};

class Kshorstestpath_result{
public:
    int id;
    std::vector<path> paths;
};

class Kshortestpath{
public:
    Kshorstestpath_result kshortestpath_exact(Graph& graph, int id, int source, int target, int k, const std::string& mode);
    Kshorstestpath_result kshortestpath_heuristics(Graph& graph, int id, int source, int target, int k, double overlap_threshold);
}