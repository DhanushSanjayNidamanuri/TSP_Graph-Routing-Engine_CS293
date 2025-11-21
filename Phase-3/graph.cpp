#include "graph.hpp"
class K_Exact;
class K_Heuristic;
class Approx_Shortest;
void Graph::addNode(const Node& node) {
    node_list[node.id]=node;
}
 
void Graph::addEdge(const Edge& edge) {
    adjacency_list[edge.u].push_back(edge);
    adjacency_list[edge.v].push_back(edge);
}

nlohmann::json Graph::query_handler(const nlohmann::json& query){
    nlohmann::json out;
    TSP temp;
    std::vector<std::tuple<int,int,int>> orders_temp;
    for(auto& x : query["orders"]) {
        queries_temp.push_back(std::make_tuple(x["order_id"],x["pickup"],x["dropoff"]));
    }
    int no_deliv_guys,depot_node;
    no_deliv_guys=query["fleet"]["num_delivery_guys"];
    depot_node=query["fleet"]["depot_node"];
    TSP_Result out_res=temp.solve(*this,orders_temp,std::make_pair(no_deliv_guys,depot_node));
    std::vector<nlohmann::json> assignments;
    for(auto [x,y,z]:out_res.assignments){
        nlohmann::json inner_json;
        inner_json["driver_id"]=x;
        inner_json["route"]=y;
        inner_json["order_ids"]=z;
        tempdists.push_back(inner_json);
    }
    out["assignments"]=assignments;
    out["metrics"]["total_delivery_time_s"]=std::round(out_res.time* 1e6) / 1e6;
    return out;
}
void Graph::dijkstra(std::vector<float>& distances,int src){
    std::priority_queue<std::pair<float,int>,std::vector<std::pair<float,int>>,std::greater<std::pair<float,int>>> pq;
    pq.push(std::make_pair(0,src));
    std::vector<bool> visited(node_count,false);
    while(!pq.empty()){
        auto [dist,u]=pq.top();pq.pop();
        if(visited[u])continue;
        visited[u]=true;
        for(auto& e:adjacency_list[u]){
            int v =(e.u==u) ? e.v : e.u;
            if(((!e.oneway) || (e.u == u)) && !visited[v]){
                pq.push(std::make_pair(dist+e.length,v));
            }
        }
    }
}

void Graph::preprocess(){
    distances.resize(node_count);
    for(int i=0;i<node_count;i++){
        distances[i].resize(node_count);
    }
    for(int i=0;i<node_count;i++){
        dijkstra(distances[i],i);
    }
    
};
