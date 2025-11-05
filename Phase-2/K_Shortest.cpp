#include "K_Shortest.hpp"
std::vector<std::pair<std::vector<int>,int>> KShortestPaths::KShortest_heuristic(Graph& graph, int source, int target, int k, int overlap_threshold){
    std::vector<std::pair<std::vector<int>,int>> paths;
    std::vector<bool> visited(graph.node_list.size());
    std::vector<int> parent(graph.node_list.size(),-1);
    std::vector<bool> in_a_path(graph.node_list.size(),false);
    std::priority_queue<std::pair<int,int>> pq;
    
}