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
#include <atomic>
#include <cstdint>
#define _USE_MATH_DEFINES

static std::atomic<int> g_astar_max_ms(5000);

static inline uint64_t path_hash64(const std::vector<int>& nodes){
    uint64_t h = 1469598103934665603ull;
    for (int v : nodes) {
        h ^= static_cast<uint64_t>(v) + 0x9e3779b97f4a7c15ULL + (h<<6) + (h>>2);
        h *= 1099511628211ULL;
    }
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    h *= 0xc4ceb9fe1a85ec53ULL;
    h ^= h >> 33;
    return h;
}

class A_path{
public:
    std::vector<int> nodes;
    double distance;
    std::unordered_set<long long> edges;
    uint64_t h64;

    A_path(std::vector<int>& vec, double dist): nodes(vec), distance(dist){
        for(size_t i=0; i<nodes.size()-1; i++){
            long long id = nodes[i] * 10000 + nodes[i+1];
            edges.insert(id);
        }
        h64 = path_hash64(nodes);
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

    auto start_time = std::chrono::steady_clock::now();
    auto timeout_check = [&]() {
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
        return elapsed.count() > g_astar_max_ms.load(); 
    };

    auto is_forbidden = [&](int u, int v){
        for(auto& forbidden : removedEdges){
            if((forbidden.first == u && forbidden.second == v) || (forbidden.first == v && forbidden.second == u)){
                return true;
            }
        }
        return false;
    };
    
    auto comp = [](const std::pair<double, int>& a, const std::pair<double, int>& b){
        return a.first > b.first;
    };
    
    // DIJKSTRA: Only distance (no f_score)
    std::priority_queue<std::pair<double, int>, std::vector<std::pair<double, int>>, decltype(comp)> pq(comp);
    std::vector<double> dist(graph.node_list.size(), std::numeric_limits<double>::max());
    std::vector<int> prev(graph.node_list.size(), -1);    
    std::vector<bool> visited(graph.node_list.size(), false);

    dist[source] = 0.0;
    pq.push({dist[source], source});  // Push actual distance

    const int max_expansions = 200000;
    int expansions = 0;

    while(!pq.empty()){
        if(timeout_check()){
            return {{}, -1.0};
        }
        auto [curr_dist, node] = pq.top();  // This is actual distance now
        pq.pop();

        if(visited[node]) continue;
        visited[node] = true;

        if(++expansions > max_expansions){
            return {{}, -1.0};
        }
        
        if(node == target) break;
        
        if(curr_dist > dist[node]) continue;

        for(Edge& edge : graph.adjacency_list[node]){
            int v = (edge.u == node) ? edge.v : edge.u;
            
            if(edge.oneway && edge.u != node) continue;

            if(is_forbidden(node, v)){
                continue;
            }

            double weight = (mode == "time") ? edge.average_time : edge.length;
            double new_dist = dist[node] + weight;

            if(new_dist < dist[v]){
                dist[v] = new_dist;
                prev[v] = node;
                pq.push({dist[v], v});  // Push actual distance
            }
        }
    }
    
    if(prev[target] == -1){
        return {{}, -1.0};
    }
    
    std::vector<int> result;
    for(int node = target; node != -1; node = prev[node]){
        result.push_back(node);
    }
    std::reverse(result.begin(), result.end());

    return {result, dist[target]};

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
            //std::cerr<<"ERROR : NO EDGE FOUND";
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
bool AlreadyFound(const std::vector<int>& path, const std::vector<std::pair<std::vector<int>, double>>& results){
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
    result.reserve(k);
    result.push_back(candidates[0]);

    auto start_time = std::chrono::steady_clock::now();

    while(result.size() < k && result.size() <candidates.size()){
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
       
        if(elapsed.count() > 5000){
            break;
        }
       
        double best_penalty = 1e18;
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
            
            double total_penalty = 0.0;
            double shortest_distance = result[0].distance;
            
            for(size_t i=0; i<result.size(); i++){
                int overlap_penalty = 0;
                double overlap = overlap_amount(result[i], candidates[idx]);
                if(overlap > overlap_threshold) overlap_penalty++;
                double diff_per = ((result[i].distance - shortest_distance)/shortest_distance)*100.0;
                double distance_penalty = (diff_per/100.0) + 0.1;
                total_penalty += overlap_penalty * distance_penalty;
            }
            {
                double diff_per = ((candidates[idx].distance - shortest_distance)/shortest_distance)*100.0;
                double distance_penalty = (diff_per/100.0) + 0.1;
                int overlap_penalty = 0;
                for(const auto& rp : result){
                    double ov = overlap_amount(candidates[idx], rp);
                    if(ov > overlap_threshold) overlap_penalty++;
                }
                total_penalty += overlap_penalty * distance_penalty;
            }

            if(total_penalty < best_penalty){
                best_penalty = total_penalty;
                best_index = static_cast<int>(idx);
            }
        }
        if(best_index != -1){
            result.push_back(candidates[best_index]);
        }else{
            break;
        }
    }
    std::sort(result.begin(), result.end(),
    [](const A_path& a, const A_path& b) {
        return a.distance < b.distance;
    });
    return result;
}
//Yen's algorithm
std::vector<std::pair<std::vector<int>, double>> KShortestPaths::KShortest(Graph& graph, int source, int target, unsigned int k, std::string mode) {
    k = std::min(k, (unsigned)20);
    
    std::vector<std::pair<std::vector<int>, double>> result;
    std::priority_queue<A_path, std::vector<A_path>, std::greater<A_path>> candidates;

    auto start_time = std::chrono::steady_clock::now();
    auto timeout_check = [&](){
        auto current_time = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
        return elapsed.count() > 15000;
    };

    std::unordered_set<uint64_t> candidates_hashes;
    candidates_hashes.reserve(4096);
    std::unordered_set<uint64_t> result_hashes;
    result_hashes.reserve(64);
    g_astar_max_ms.store(4000);

    // Get first shortest path
    auto first_path = AstarShortestPath(graph, source, target, mode, {});
    if(first_path.first.empty()) {
        return result;
    }
    
    if(!is_simple(first_path.first)){
        //std::cerr<<"LOOP SPOTTED"<<std::endl;
    }
    double first_dist = path_distance(graph, first_path.first);
    A_path first_a_path(first_path.first, first_dist);
    candidates.push(first_a_path);
    candidates_hashes.insert(first_a_path.h64);
    result.push_back({first_path.first, first_dist});
    result_hashes.insert(path_hash64(first_path.first));

    
    while(result.size() < k && !candidates.empty()) {
        if(timeout_check()){
            std::cout<<"TIMEOUT IN MAIN LOOP"<<std::endl;
            break;
        }

        A_path current = candidates.top();
        candidates.pop();
        candidates_hashes.erase(current.h64);

        if(!is_simple(current.nodes)){
            continue;
        }
        
        uint64_t curh = current.h64;
        if(result_hashes.find(curh) == result_hashes.end()){
            result.push_back({current.nodes, static_cast<double>(current.distance)});
            result_hashes.insert(curh);
        }

        auto& path = current.nodes;
        auto now = std::chrono::steady_clock::now();
        int elapsed_ms = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count());
        int remaining_ms = 15000 - elapsed_ms;
        if(remaining_ms < 100) break; 

        
        int per_astar_ms = std::min(remaining_ms / 4, 3000);
        per_astar_ms = std::max(per_astar_ms, 200); 
        g_astar_max_ms.store(per_astar_ms);

        for(size_t i = 0; i < path.size() - 1; i++) {
            if(timeout_check()) break;
            int spurNode = path[i];
            std::vector<int> rootPath;
            rootPath.reserve(i+1);
            rootPath.insert(rootPath.end(), path.begin(), path.begin() + i + 1);

            if(!is_simple(rootPath)){
                continue;
            }
            std::vector<std::pair<int, int>> removedEdges;
            removedEdges.reserve(result.size());
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
                std::vector<int> totalPath;
                totalPath.reserve(rootPath.size() + tempPathResult.first.size());
                totalPath = rootPath;
                totalPath.insert(totalPath.end(), tempPathResult.first.begin()+1, tempPathResult.first.end());

                if(!is_simple(totalPath)){
                    continue;
                }
                double totalDist = path_distance(graph, totalPath);
                if(totalDist<0) continue;

                A_path new_candidate(totalPath, totalDist);
                if(result_hashes.find(new_candidate.h64) == result_hashes.end() &&
                   candidates_hashes.find(new_candidate.h64) == candidates_hashes.end()){
                    candidates.push(new_candidate);
                    candidates_hashes.insert(new_candidate.h64);
                }
            }
        }
    }
    std::sort(result.begin(), result.end(), 
        [](const std::pair<std::vector<int>, double>& a, const std::pair<std::vector<int>, double>& b) {
            return a.second < b.second;
    });
    return result;
}
//Heuristic
std::vector<std::pair<std::vector<int>, double>> KShortestPaths::KShortest_heuristic(Graph& graph, int source, int target, unsigned int k, int overlap_threshold){
    k = std::min(k,(unsigned) 7);
    int paths_count = std::min(k*2,(unsigned) 20);

    auto all_paths = KShortest(graph, source, target, static_cast<unsigned int>(paths_count), "distance");
    if(all_paths.empty()) return {};

    std::vector<std::pair<std::vector<int>, double>> simple_paths;
    for(auto& path : all_paths){
        if(is_simple(path.first)){
            simple_paths.push_back(path);
        }
    }
    if(simple_paths.empty()) return {};

    std::vector<A_path> result_paths;
    result_paths.reserve(simple_paths.size());
    for(auto& path : simple_paths){
        std::vector<int> tmp = path.first;
        result_paths.emplace_back(tmp, path.second);
    }

    std::vector<A_path> best_selection = paths_selection(result_paths, k , overlap_threshold);

    std::vector<std::pair<std::vector<int>, double>> result;
    result.reserve(best_selection.size());
    for(auto& path : best_selection){
        result.push_back({path.nodes, path.distance});
    }
    std::sort(result.begin(), result.end(), 
        [](const std::pair<std::vector<int>, double>& a, const std::pair<std::vector<int>, double>& b) {
            return a.second < b.second;
    });
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