#ifndef SHORTESTPATH_HPP
#define SHORTESTPATH_HPP

#include "Graph.hpp"
#include <vector>
#include <string>
#include <queue>
#include<map>
class ShortestPath_Result {
public:
    int id;
    bool possible;
    double min_distance;    
    double min_time;        
    std::vector<int> path;  
};

class ShortestPath {
public:
    ShortestPath_Result findShortestPath(Graph& graph, int id, int source, int target, const std::string& mode, const std::vector<int>& forbidden_nodes,
        const std::vector<std::string>& forbidden_road_types);
    
    bool Is_Usable_Now(Node& destination,Edge& edge,std::vector<bool>& visited, std::unordered_map<std::string,bool>& fb_types);

    int Expected_time(Edge& edge,int start_time);
};

#endif