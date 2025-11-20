#include "KNN.hpp"

Result_KNN KNN::findKNN(const Graph& graph, int id, double lat, double lon, const std::string& poi, int k, const std::string& metric) {
    if(metric=="shortest_path") {
        return Result_KNN(id,findKNN_ShortestPath(graph, lat, lon, poi, k));
    }
    else {
        return Result_KNN(id,findKNN_Euclidean(graph, lat, lon, poi, k));
    }
}

double euclidean(double lat1, double lon1, double lat2, double lon2) {
    const double DEG_TO_RAD = 3.14159265358979323846 / 180.0;
    double mean_lat = (lat1 + lat2) * 0.5 * DEG_TO_RAD;
    double dx = (lon2 - lon1) * 111320.0 * std::cos(mean_lat);
    double dy = (lat2 - lat1) * 111320.0;
    return std::sqrt(dx * dx + dy * dy);
}

std::vector<int> KNN::findKNN_Euclidean(const Graph& graph, double lat, double lon, const std::string& poi, int k) {
    auto cmp=[lat, lon](const Node& u, const Node& v) {
        return euclidean(u.lat,u.lon,lat,lon) > euclidean(v.lat,v.lon,lat,lon);
    };
    std::vector<int> K_nearest;
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
    for(int u:graph.pois[poi]) {
        pq.push(u);
    }
    while(!pq.empty() && k--) {
        K_nearest.push_back(pq.top().id);
        pq.pop();
    }
    return K_nearest;
}

std::vector<int> KNN::findKNN_ShortestPath(const Graph& graph, double lat, double lon, const std::string& poi, int k) {
    if(graph.node_list.empty()||k==0) return {};
    int closest_id=0;
    double best_d=euclidean(graph.node_list[closest_id].lat,graph.node_list[closest_id].lon,lat,lon);
    for(const Node& u:graph.node_list) {
        double temp_distance=euclidean(u.lat,u.lon,lat,lon);
        if(temp_distance<best_d){
            closest_id=u.id;best_d=temp_distance;
        }
    }
    std::vector<int> K_nearest;
    std::priority_queue<std::pair<double,int>, std::vector<std::pair<double,int>>, std::greater<std::pair<double,int>>> pq;
    std::vector<bool> visited(graph.node_count,false);
    pq.push(std::make_pair(0,closest_id));
    int count=0;

    auto normalize = [](std::string s) {
    s.erase(0, s.find_first_not_of(" \t\r\n"));
    s.erase(s.find_last_not_of(" \t\r\n") + 1);
    s.erase(std::remove_if(s.begin(), s.end(),
    [](unsigned char c){ return c < 32 || c > 126; }),
    s.end());
    return s;};

    while(!pq.empty() && count<k) {
        auto [dist,u]=pq.top();
        pq.pop();
        if(visited[u]) continue;
        visited[u]=true;
        for(auto poi_u:graph.node_list[u].pois) {
            if(normalize(poi_u) == normalize(poi)) {
                count++;
                K_nearest.push_back(u);
                break;
            }
        }
        for(auto [id,edge]:graph.adjacency_list[u]) {
            if(!edge.isOpen) continue;
            int v =(edge.u==u) ? edge.v : edge.u;
            if(!visited[v]) {
                pq.push({dist+edge.length,v});
            }
        }
    }
    return K_nearest;
}