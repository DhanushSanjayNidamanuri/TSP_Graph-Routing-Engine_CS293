#include "K_Shortest.hpp"
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <limits>

class A_path{
public:
    std::vector<int> nodes;
    double distance;
    std::unordered_set<long long> edges;

    A_path(std::vector<int>& vec, double dist): nodes(vec), distance(dist){
        for(size_t i=0; i<nodes.size()-1; i++){
            long long id = nodes[i] * 10000 + nodes[i+1];
            edges.insert(id);
        }
    }
    bool operator>(const A_path& other) const{
        return distance > other.distance;
    }
};
//dijsktra's for finding the shortest path
std::pair<std::vector<int>, double> dijkstraShortestPath(Graph& graph, int source, int target, string mode){
    if(source == target){
        return {{source}, 0.0};
    }

    auto comp = [](const std::pair<double, int>& a, const std::pair<double, int>& b){
        return a.first > b.first;
    };
    
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, decltype(comp)> pq(comp);
    std::vector<double> sp(graph.node_list.size(), numeric_limits<double>::max());
    std::vector<int> prev(graph.node_list.size(), -1);    

    sp[source] = 0.0;
    pq.push({0.0, source});

    while(!pq.empty()){
        auto [curr_dist, node] = pq.top();
        pq.pop();
        //reached
        if(node == target) break;
        //was better earlier than now
        if(curr_dist > sp[node]) continue;

        for(Edge& edge : graph.adjacency_list[node]){
            //if not here, then the other is neighbor
            int v = (edge.u == node) ? edge.v : edge.u;
            //one-way, when the entrance the other side
            if(edge.oneway && edge.u != node) continue;

            double weight = (mode == "time") ? edge.average_time : edge.length;
            double dist_new = curr_dist + weight;

            if(dist_new < sp[v]){
                sp[v] = dist_new;
                prev[v] = node;
                pq.push({dist_new, v});
            }
        }
    }
    if(prev[target] == -1){
        return {{}, -1.0};
    }
    //returning path
    std::vector<int> result;
    for(int node = target; node!=-1; node = prev[node]){
        result.push_back(node);
    }
    std::reverse(result.begin(), result.end());

    return {result, sp[target]};
}

double path_distance(Graph& graph, std::vector<int>& path){
    double distance = 0.0;
    for(size_t i=0; i<path.size()-1; i++){
        int u = path[i], v = path[i+1];

        for( Edge& edge : graph.adjacency_list[u]){
            if((edge.u == u && edge.v == v) || (!edge.oneway && edge.u == v && edge.v == u)){
                distance += edge.length;
                break;
            }
        }
    }
    return distance;
}
//to check whether we get same path twice
bool samePaths(const std::vector<int>& path1, const std::vector<int>& path2){
    if(path1.size() != path2.size()) return false;
    for(size_t i=0; i<path1.size(); i++){
        if(path1[i] != path2[i]) return false;
    }
    return true;
}
//for checking does this already exist 
bool AlreadyFound(const std::vector<int>& path, const std::vector<std::pair<std::vector<int>, int>>& results){
    for(auto& x : results){
        if(samePaths(path, x.first)){
            return true;
        }
    }
    return false;
}

bool AlreadyInA(const std::vector<int>& path, const std::vector<A_path>& candidates) {
    for(const auto& candidate : candidates) {
        if(samePaths(path, candidate.nodes)) {
            return true;
        }
    }
    return false;
}

//for heuristic
double overlap_amount(const A_path& path1, const A_path& path2){
    int common = 0;

    for(long long edge : path1.edges){
        if(path2.edges.count(edge)){
            common++;
        }
    }

    int edges = std::min(path1.edges.size(), path2.edges.size());
    if(edges == 0) return 0.0;
    return (common * 100.0) / edges;
}

//Yen's algorithm
std::vector<std::pair<std::vector<int>, int>> KShortestPaths::KShortest(Graph& graph, int source, int target, int k , string mode){
    std::vector<std::pair<std::vector<int>, int>> result;
    std::vector<A_path> result_paths;

    auto first_shortest_path = dijkstraShortestPath(graph, source, target, mode);
    if(first_shortest_path.first.empty()){
        return result;
    }
    result.push_back({first_shortest_path.first, static_cast<int>(first_shortest_path.second)});
    //other paths
    for(int j=1;j<k;j++){
        auto& prev_path = result[j-1].first;
        //deviations
        for(size_t i=0; i<prev_path.size()-1; i++){
            int deviate_node = prev_path[i];
            std::vector<int> copy_path(prev_path.begin(), prev_path.begin() +i+1);

            std::unordered_set<long long> removed_edges;
            for(auto& path_1 : result){
                auto& path = path_1.first;

                if(path.size() > i){
                    bool confirm = true;
                    for(size_t j=0; j<=i; j++){
                        if(j>=path.size() || path[j] != copy_path[j]){
                            confirm = false;
                            break;
                        }
                    }
                    if(confirm && i+1 < path.size()){
                        long long id = deviate_node*10000 + path[i+1];
                        removed_edges.insert(id);
                    }
                }
            }
            auto deviate_path = dijkstraShortestPath(graph, deviate_node, target, mode);

            if(!deviate_path.first.empty()){
                std::vector<int> complete_path = copy_path;
                complete_path.insert(complete_path.end(), deviate_path.first.begin()+1, deviate_path.first.end());
                double complete_dist = path_distance(graph, complete_path);

                if(!AlreadyFound(complete_path, result) && !AlreadyInA(complete_path, result_paths)){
                    result_paths.push_back(A_path(complete_path, complete_dist));
                }
            }
        }
        if(result_paths.empty()){
            break;
        }

        auto best_path = std::min_element(result_paths.begin(), result_paths.end(),[](const A_path& a, const A_path& b){
            return a.distance < b.distance;
        });
        result.push_back({best_path->nodes, static_cast<int>(best_path->distance)});
        result_paths.erase(best_path);
    }
    return result;
}

//Heuristic
std::vector<std::pair<std::vector<int>, int>> KShortestPaths::KShortest_heuristic(Graph& graph, int source, int target, int k, int overlap_threshold){
    int paths_count = std::min(k*3, 20);
    auto all_paths = KShortest(graph, source, target, paths_count, "distance");
    if(all_paths.empty()) return {};

    std::vector<A_path> result_paths;
    for(auto& path : all_paths){
        result_paths.push_back(A_path(path.first, path.second));
    }

    double best_penalty = std::numeric_limits<double>::max();
    std::vector<A_path> best_selection;

    std::function<void(int, std::vector<A_path>&)> backtrack = [&](int start, std::vector<A_path>& current){
        if(current.size() == k){
            double total_penalty = 0.0;
            double shortest_dist = current[0].distance;

            for(size_t i=0; i<current.size(); i++){
                int overlap_penalty = 0;
                for(size_t j=0; j<current.size(); j++){
                    if(i != j){
                        double overlap = overlap_amount(current[i], current[j]);
                        if(overlap > overlap_threshold){
                            overlap_penalty++;
                        }
                    }
                }
                double dist_diff = (current[i].distance - shortest_dist) / shortest_dist;
                double dist_penalty = dist_diff + 0.1;
                total_penalty += overlap_penalty*dist_penalty;
            }
            if(total_penalty < best_penalty){
                best_penalty = total_penalty;
                best_selection = current;
            }
            return;
        }
        for(int i=start; i < result_paths.size(); i++){
            current.push_back(result_paths[i]);
            backtrack(i+1, current);
            current.pop_back();
        }
    };
    std::vector<A_path> current;
    backtrack(0, current);

    std::vector<std::pair<std::vector<int>, int>> result;
    for(auto& path : best_selection){
        result.push_back({path.nodes, static_cast<int>(path.distance)});
    }
    return result;
}

//type checking
KShortestPaths_Result KShortestPaths::findShortest(Graph& graph, string type, int id, int source, int target, int k, string mode, int overlap_threshold){
    if(type == "k_shortest_paths"){
        auto paths = KShortest(graph, source, target, k , mode);
        return KShortestPaths_Result(id, paths);
    }else if(type == "k_shortest_paths_heuristic"){
        auto paths = KShortest_heuristic(graph, source, target, k , overlap_threshold);
        return KShortestPaths_Result(id, paths);
    }else{
        return KShortestPaths_Result(id, {});
    }
}