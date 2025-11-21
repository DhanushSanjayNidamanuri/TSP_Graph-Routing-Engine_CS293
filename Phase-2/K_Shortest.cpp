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
#include <iostream>

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
std::pair<std::vector<int>, double> AstarShortestPath(Graph& graph, int source, int target, std::string mode, const std::vector<std::pair<int, int>>& removedEdges){
    if(source == target){
        return {{source}, 0.0};
    }

    auto is_forbidden = [&](int u, int v){
        for(auto& forbidden : removedEdges){
            if((forbidden.first == u && forbidden.second == v) || (forbidden.first == v && forbidden.second == u)){
                return true;
            }
        }
        return false;
    };
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
    std::vector<bool> visited(graph.node_list.size(), false);

    g_score[source] = 0.0;
    f_score[source] = heuristic(source, target);
    pq.push({f_score[source], source});

    while(!pq.empty()){
        auto [curr_f, node] = pq.top();
        pq.pop();

        if(visited[node]) continue;
        visited[node]=true;
        //reached
        if(node == target) break;
        //was better earlier than now
        if(curr_f > f_score[node]) continue;

        for(Edge& edge : graph.adjacency_list[node]){
            //if not here, then the other is neighbor
            int v = (edge.u == node) ? edge.v : edge.u;
            //one-way, when the entrance the other side
            if(edge.oneway && edge.u != node) continue;

            if(is_forbidden(node, v)){
                continue;
            }

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
    if(path.size() <= 1) return 0.0;
    for(size_t i = 0; i < path.size()-1; i++){
        int u = path[i], v = path[i+1];
        bool edge_found = false;
        
        for(const Edge& edge : graph.adjacency_list[u]){
            if((edge.u == u && edge.v == v) || 
               (!edge.oneway && edge.u == v && edge.v == u)){
                distance += edge.length;
                edge_found = true;
                break;
            }
        }
        
        if(!edge_found){
            for(const Edge& edge : graph.adjacency_list[v]){
                if((edge.u == v && edge.v == u) || 
                   (!edge.oneway && edge.u == u && edge.v == v)){
                    distance += edge.length;
                    edge_found = true;
                    break;
                }
            }
        }
        if(!edge_found){
            std::cerr<<"ERROR : NO EDGE FOUND";
            return -1.0;
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

bool AlreadyInA(const std::vector<int>& path, std::priority_queue<A_path, std::vector<A_path>, std::greater<A_path>>& candidates) {
    auto temp = candidates;
    while(!temp.empty()) {
        if(samePaths(path, temp.top().nodes)) return true;
        temp.pop();
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

bool is_simple(const std::vector<int>& path){
    std::unordered_set<int> visited;
    for(int node:path){
        if(visited.count(node)){
            return false;
        }
        visited.insert(node);
    }
    return true;
}
std::vector<A_path> paths_selection(std::vector<A_path> candidates, unsigned int k, int overlap_threshold){
    if(candidates.empty() || k<=0) return {};

    std::vector<A_path> result;
    result.push_back(candidates[0]);

    auto start_time = std::chrono::steady_clock::now();

    while(result.size() < k && result.size() <candidates.size()){
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
       
        if(elapsed.count() > 5000){
            break;
        }
       
        double best_penalty = 1e9;
        int best_index = -1;

        for(size_t idx=0; idx< candidates.size(); idx++){
            bool already_selected = false;
            for(const auto& x : result){
                if(samePaths(candidates[idx].nodes, x.nodes)){
                    already_selected = true;
                    break;
                }
            }
            if(already_selected) continue;

            std::vector<A_path> temp_result = result;
            temp_result.push_back(candidates[idx]);
            
            double total_penalty = 0.0;
            double shortest_distance = temp_result[0].distance;
            
            for(size_t i=0; i<temp_result.size(); i++){
                int overlap_penalty = 0;
                for(size_t j=0; j<temp_result.size(); j++){
                    if(i != j){
                        double overlap = overlap_amount(temp_result[i], temp_result[j]);
                        if(overlap > overlap_threshold){
                            overlap_penalty++;
                        }
                    }
                }
                double diff_per = ((temp_result[i].distance - shortest_distance)/shortest_distance)*100.0;
                double distance_penalty = (diff_per/100.0) + 0.1;

                double path_penalty = overlap_penalty * distance_penalty;
                total_penalty += path_penalty;
            }
            if(total_penalty < best_penalty){
                best_penalty = total_penalty;
                best_index = idx;
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
std::vector<std::pair<std::vector<int>, int>> KShortestPaths::KShortest(Graph& graph, int source, int target, unsigned int k, std::string mode) {
    k = std::min(k, (unsigned)20);
    
    std::vector<std::pair<std::vector<int>, int>> result;
    std::priority_queue<A_path, std::vector<A_path>, std::greater<A_path>> candidates;

    auto start_time = std::chrono::steady_clock::now();

    // Get first shortest path
    auto first_path = AstarShortestPath(graph, source, target, mode, {});
    if(first_path.first.empty()) {
        return result;
    }
    
    if(!is_simple(first_path.first)){
        std::cerr<<"LOOP SPOTTED"<<std::endl;
    }
    double first_dist = path_distance(graph, first_path.first);
    A_path first_a_path(first_path.first, first_dist);
    candidates.push(first_a_path);
    result.push_back({first_path.first, static_cast<int>(first_dist)});

    
    while(result.size() < k && !candidates.empty()) {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        if(elapsed.count() > 15000){
            std::cout<<"TIMEOUT"<<std::endl;
            break;
        }

        A_path current = candidates.top();
        candidates.pop();

        if(!is_simple(current.nodes)){
            continue;
        }
        
        if(!AlreadyFound(current.nodes, result)) {
            result.push_back({current.nodes, static_cast<int>(current.distance)});
        }

        auto& path = current.nodes;
        for(size_t i = 0; i < path.size() - 1; i++) {
            int spurNode = path[i];
            std::vector<int> rootPath(path.begin(), path.begin() + i + 1);

            if(!is_simple(rootPath)){
                continue;
            }
            std::vector<std::pair<int, int>> removedEdges;
            for(auto& res_path : result){
                if(i < res_path.first.size()-1){
                    bool matching_pre = true;
                    for(size_t j=0; j<=i; j++){
                        if(rootPath[j] != res_path.first[j]){
                            matching_pre = false;
                            break;
                        }
                    }
                    if(matching_pre){
                        removedEdges.push_back({res_path.first[i], res_path.first[i+1]});
                    }
                }
            }

            auto tempPathResult = AstarShortestPath(graph, spurNode, target, mode, removedEdges);

            if(!tempPathResult.first.empty() && tempPathResult.second>=0){
                std::vector<int> totalPath = rootPath;
                totalPath.insert(totalPath.end(), tempPathResult.first.begin()+1, tempPathResult.first.end());

                if(!is_simple(totalPath)){
                    continue;
                }
                double totalDist = path_distance(graph, totalPath);
                if(totalDist<0) continue;

                A_path new_candidate(totalPath, totalDist);
                if(!AlreadyInA(totalPath, candidates) && !AlreadyFound(totalPath, result)){
                    candidates.push(new_candidate);
                }
            }
        }
    }
    
    return result;
}
//Heuristic
std::vector<std::pair<std::vector<int>, int>> KShortestPaths::KShortest_heuristic(Graph& graph, int source, int target, unsigned int k, int overlap_threshold){
    k = std::min(k,(unsigned) 7);
    int paths_count = std::min(k*2,(unsigned) 20);

    auto all_paths = KShortest(graph, source, target, paths_count, "distance");
    if(all_paths.empty()) return {};

    std::vector<std::pair<std::vector<int>, int>> simple_paths;
    for(auto& path : all_paths){
        if(is_simple(path.first)){
            simple_paths.push_back(path);
        }
    }
    if(simple_paths.empty()) return {};

    std::vector<A_path> result_paths;
    for(auto& path : simple_paths){
        result_paths.push_back(A_path(path.first, path.second));
    }

    std::vector<A_path> best_selection;

    best_selection = paths_selection(result_paths, k , overlap_threshold);

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