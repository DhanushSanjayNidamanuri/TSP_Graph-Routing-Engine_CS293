#include "KNN.hpp"

Result_KNN KNN::findKNN(const Graph& graph, int id, double lat, double lon, const std::string& poi, int k, const std::string& metric) {
    if(metric=="shortest_path") {
        return Result_KNN(id,findKNN_ShortestPath(graph, lat, lon, poi, k));
    }
    else {
        return Result_KNN(id,findKNN_Euclidean(graph, lat, lon, poi, k));
    }
}

double euclidean(double lat_a, double lon_a, double lat_b, double lon_b) {
    return (lat_a-lat_b)*(lat_a-lat_b) + (lon_a-lon_b)*(lon_a-lon_b);
}

std::vector<int> KNN::findKNN_Euclidean(const Graph& graph, double lat, double lon, const std::string& poi, int k) {
    auto cmp=[lat, lon](const Node& u, const Node& v) {
        return euclidean(u.lat,u.lon,lat,lon) > euclidean(v.lat,v.lon,lat,lon);
    };
    std::vector<int> K_nearest;
    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
    for(Node u:graph.node_list) {
        for(std::string poi_u:u.pois) {
            if(poi_u==poi) {
                pq.push(u);
                break;
            }
        }
    }
    while(!pq.empty() && k--) {
        K_nearest.push_back(pq.top().id);
        pq.pop();
    }
    return K_nearest;
}

std::vector<int> KNN::findKNN_ShortestPath(const Graph& graph, double lat, double lon, const std::string& poi, int k) {
    if(graph.node_list.empty()||k==0) return {};
    Node infinity=Node(-1,1e9,1e9,{poi});
    Node closest=infinity;
    for(Node u:graph.node_list) {
        if(euclidean(u.lat,u.lon,lat,lon) < euclidean(closest.lat,closest.lon,lat,lon) && std::find(u.pois.begin(),u.pois.end(),poi)!=u.pois.end()) closest=u;
    }
    if(closest.id==-1) return {};
    std::vector<int> K_nearest;
    std::priority_queue<std::pair<double,int>, std::vector<std::pair<double,int>>, std::greater<std::pair<double,int>>> pq;
    std::vector<bool> visited(graph.node_count);
    std::vector<double> SP(graph.node_count,1e18);
    pq.push({0,closest.id});
    SP[closest.id]=0;
    int count=0;
    while(!pq.empty() && count<k) {
        auto u=pq.top();
        pq.pop();
        if(visited[u.second]) continue;
        visited[u.second]=true;
        for(auto poi_u:graph.node_list[u.second].pois) {
            if(poi_u==poi) {
                K_nearest.push_back(u.second);
                count++;
            }
        }
        for(auto [id,edge]:graph.adjacency_list[u.second]) {
            if(!edge.isOpen) continue;
            if(!visited[edge.v]&&SP[u.second]+edge.length<SP[edge.v]) {
                SP[edge.v]=SP[u.second]+edge.length;
                pq.push({SP[edge.v],edge.v});
            }
        }
    }
    return K_nearest;
}