#ifndef SHORTESTPATH_HPP
#define SHORTESTPATH_HPP

#include "Graph.hpp"
#include <vector>
#include <string>

class ShoretestPath_Result {
public:
    int id;
    bool possible;
    double min_distance;    
    double min_time;        
    std::vector<int> path;  
};

class ShortestPath {
public:
    Result findShortestPath(Graph& graph, int id, int source, int target, const std::string& mode, const std::vector<int>& forbidden_nodes,
        const std::vector<std::string>& forbidden_road_types);
};

#endif