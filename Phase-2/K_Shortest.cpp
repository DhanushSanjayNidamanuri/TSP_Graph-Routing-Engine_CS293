#include "K_Shortest.hpp"
#include <queue>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <algorithm>
#include <limits>
#include <chrono>
#include <cmath>

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
//A* for finding the shortest path
std::pair<std::vector<int>, double> AstarShortestPath(Graph& graph, int source, int target, std::string mode){
    if(source == target){
        return {{source}, 0.0};
    }
    auto heuristic = [&](unsigned int node1,unsigned int node2) {
        if(node1 >= graph.node_list.size() || node2 >= graph.node_list.size()) return 0.0;
        double lat1 = graph.node_list[node1].lat, lon1 = graph.node_list[node1].lon;
        double lat2 = graph.node_list[node2].lat, lon2 = graph.node_list[node2].lon;
        return sqrt(pow(lat1-lat2, 2) + pow(lon1-lon2, 2)) * 111000.0; 
    };
    auto comp = [](const std::pair<double, int>& a, const std::pair<double, int>& b){
        return a.first > b.first;
    };
    
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, decltype(comp)> pq(comp);
    std::vector<double> g_score(graph.node_list.size(), std::numeric_limits<double>::max());
    std::vector<double> f_score(graph.node_list.size(), std::numeric_limits<double>::max());
    std::vector<int> prev(graph.node_list.size(), -1);    

    g_score[source] = 0.0;
    f_score[source] = heuristic(source, target);
    pq.push({f_score[source], source});

    while(!pq.empty()){
        auto [curr_f, node] = pq.top();
        pq.pop();
        //reached
        if(node == target) break;
        //was better earlier than now
        if(curr_f > f_score[node]) continue;

        for(Edge& edge : graph.adjacency_list[node]){
            //if not here, then the other is neighbor
            int v = (edge.u == node) ? edge.v : edge.u;
            //one-way, when the entrance the other side
            if(edge.oneway && edge.u != node) continue;

            double weight = (mode == "time") ? edge.average_time : edge.length;
            double dist_new = g_score[node] + weight;

            if(dist_new < g_score[v]){
                g_score[v] = dist_new;
                f_score[v] = dist_new + heuristic(v, target);
                prev[v] = node;
                pq.push({f_score[v], v});
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

    return {result, g_score[target]};
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

std::vector<A_path> paths_selection(std::vector<A_path> candidates, unsigned int k, int overlap_threshold){
    if(candidates.empty() || k<=0) return {};

    std::vector<A_path> result;
    result.push_back(candidates[0]);

    while(result.size() < k && result.size() <candidates.size()){
        double best_score = -1e9;
        int best_index = -1;

        for(size_t i=0; i< candidates.size(); i++){
            bool already_selected = false;
            for(const auto& x : result){
                if(samePaths(candidates[i].nodes, x.nodes)){
                    already_selected = true;
                    break;
                }
            }
            if(already_selected) continue;
            double min_overlap_penalty = 1.0;
            for(const auto& x : result){
                double overlap = overlap_amount(candidates[i], x);
                if(overlap > overlap_threshold){
                    min_overlap_penalty = 0.0;
                    break;
                }else{
                    double penalty = 1.0 - (overlap/overlap_threshold);
                    min_overlap_penalty = std::min(min_overlap_penalty, penalty);
                }
            }
            double distance_ratio = candidates[i].distance/result[0].distance;
            double distance_penalty = 1.0/(1.0 + distance_ratio);

            double total_score = min_overlap_penalty * distance_penalty;

            if(total_score > best_score){
                best_score = total_score;
                best_index = i;
            }
        }
        if(best_index != -1){
            result.push_back(candidates[best_index]);
        }else{
            break;
        }
    }
    return result;
}
//Yen's algorithm
std::vector<std::pair<std::vector<int>, int>> KShortestPaths::KShortest(Graph& graph, int source, int target, unsigned int k , std::string mode){
    k = std::min(k,(unsigned) 50);
    
    std::vector<std::pair<std::vector<int>, int>> result;
    std::vector<A_path> result_paths;

    auto first_shortest_path = AstarShortestPath(graph, source, target, mode);
    if(first_shortest_path.first.empty()){
        return result;
    }
    result.push_back({first_shortest_path.first, static_cast<int>(first_shortest_path.second)});
    
    auto start_time = std::chrono::steady_clock::now();
    //other paths
    for(unsigned int j=1;j<k;j++){
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        if(elapsed.count() > 12000){
            break;
        }
        auto& prev_path = result[j-1].first;
        //deviations
        for(size_t i=0; i<prev_path.size()-1; i++){
            int deviate_node = prev_path[i];
            std::vector<int> copy_path(prev_path.begin(), prev_path.begin() +i+1);

            if(result_paths.size() > k*5){
                continue;
            }
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
            auto deviate_path = AstarShortestPath(graph, deviate_node, target, mode);

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
std::vector<std::pair<std::vector<int>, int>> KShortestPaths::KShortest_heuristic(Graph& graph, int source, int target, unsigned int k, int overlap_threshold){
    k = std::min(k,(unsigned) 20);
    int paths_count = std::min(k*3,(unsigned) 30);
    auto all_paths = KShortest(graph, source, target, paths_count, "distance");
    if(all_paths.empty()) return {};

    std::vector<A_path> result_paths;
    for(auto& path : all_paths){
        result_paths.push_back(A_path(path.first, path.second));
    }

    std::vector<A_path> best_selection;

    if(k <= 5){
        double best_penalty = std::numeric_limits<double>::max();
    
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
            for(unsigned int i=start; i < result_paths.size(); i++){
                current.push_back(result_paths[i]);
                backtrack(i+1, current);
                current.pop_back();
            }
        };
        std::vector<A_path> current;
        backtrack(0, current);
    }else{
        best_selection = paths_selection(result_paths, k, overlap_threshold);
    }

    std::vector<std::pair<std::vector<int>, int>> result;
    for(auto& path : best_selection){
        result.push_back({path.nodes, static_cast<int>(path.distance)});
    }
    return result;
}

//type checking
KShortestPaths_Result KShortestPaths::findShortest(Graph& graph, std::string type, int id, int source, int target, int k, std::string mode, int overlap_threshold){
    if(k <=0 || source < 0 || target < 0 || source >= (int)graph.node_list.size() || target >= (int)graph.node_list.size()){
        return KShortestPaths_Result(id, {});
    }
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