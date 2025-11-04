#ifndef SHORTESTPATH_HPP
#define SHORTESTPATH_HPP

#include "graph.hpp"
#include <vector>
#include <string>
#include <queue>
#include <map>
#include <algorithm>
class Graph;
class Node;
class Edge;
class ShortestPath_Result {
public: 
    int id;
    bool possible;
    double min_dist_or_time;         
    std::vector<int> path; 
    ShortestPath_Result(int id,bool possible,double min=-1,std::vector<int> path={}) : id(id),possible(possible),min_dist_or_time(min),path(path){}; 
};

class ShortestPath {
public:
    ShortestPath_Result findShortestPath(Graph& graph, int id, int source, int target, const std::string& mode, const std::vector<int>& forbidden_nodes,
        const std::vector<std::string>& forbidden_road_types);
    
    bool Is_Usable_Now(Node& destination,Edge& edge,std::vector<bool>& visited, std::unordered_map<std::string,bool>& fb_types);

    int Expected_time(Edge& edge,int start_time);
    std::vector<int> Backtrack(int u,std::vector<int>& parent);
};

#endif