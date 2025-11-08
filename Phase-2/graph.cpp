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
    if(query["type"]=="k_shortest_paths" || query["type"]=="k_shortest_paths_heuristic"){
        KShortestPaths temp;
        KShortestPaths_Result tempout=temp.findShortest(*this,query["type"],query["id"],query["source"],query["target"],query["k"],query["mode"],query["overlap_threshold"]);
        out["id"]=tempout.id;
        std::vector<nlohmann::json> tempdists;
        for(auto [x,y]:tempout.paths){
            nlohmann::json inner_json;
            inner_json["path"]=x;
            inner_json["length"]=y;
            tempdists.push_back(inner_json);
        }
        out["paths"]=tempdists;
        return out;
    }
    else if(query["type"]=="approx_shortest_path"){
        ApproxShortest temp;
        std::vector<std::pair<int,int>> queries_temp;
        for(auto x:query["queries"]){
            queries_temp.push_back(std::make_pair(x["source"],x["target"]));
        }
        ApproxShortest_Result tempout=temp.findApprox(*this,query["id"],queries_temp,query["time_budget_ms"],query["acceptable_error_pct"]);
        out["id"]=tempout.id;
        std::vector<nlohmann::json> tempdists;
        for(auto [x,y,z]:tempout.distances){
            nlohmann::json inner_json;
            inner_json["source"]=x;
            inner_json["target"]=y;
            inner_json["approx_shortest_distance"]=z;
            tempdists.push_back(inner_json);
        }
        out["distances"]=tempdists;
        return out;
    }
    else{
        return out;
    }
    
}
// double Graph::witness_search(int source,int target,int avoid,double dist_limit,int algo_limit=40){
//     const double INF_DOUBLE=std::numeric_limits<double>::infinity();
//     std::priority_queue<std::pair<double,int>,std::vector<std::pair<double,int>>,std::greater<std::pair<double,int>>> pq;
//     std::vector<double> distances(node_count,INF_DOUBLE);
//     distances[source]=0.0;
//     pq.push(std::make_pair(0.0,source));
//     int explored=0;
//     while(!pq.empty()){
//         auto [dist,u]=pq.top();pq.pop();
//         if(distances[u]!=INF_DOUBLE)continue;
//         distances[u]=dist;
//         if(u==target)return dist;
//         if(distances[u]>dist_limit)return INF_DOUBLE;
//         if(++explored>algo_limit)return INF_DOUBLE;
//         for (const auto& [v,edge]:processed_outgoing_edges[u]){
//             if (v==avoid) continue;
//             if(distances[v]!=INF_DOUBLE)continue;
//             pq.push(std::make_pair(dist+edge.length,v));         
//         }
//     }
//     return INF_DOUBLE;
// };
// void Graph::make_processed_adjacency_list(){
//     for(int i=0;i<node_count;i++){
//         for(const auto& edge:adjacency_list[i]){
//             int temp= (edge.u==i) ? edge.v : edge.u;
//             if(edge.oneway && temp==i){
//                 Shortcut_Edge se(temp,edge.length,-1,false);
//                 processed_incoming_edges[i][temp]=se;
//                 continue;
//             }
//             else if(edge.oneway){
//                 Shortcut_Edge se(temp,edge.length,-1,false);
//                 processed_outgoing_edges[i][temp]=se;
//                 continue;
//             }
//             Shortcut_Edge se(temp,edge.length,-1,false);
//             processed_outgoing_edges[i][temp]=se;
//             processed_incoming_edges[i][temp]=se;
//         }
//     }
// }
// void Graph::preprocessCH(int witness_limit=40){
//     const double INF_DOUBLE=std::numeric_limits<double>::infinity();
//     rank.assign(node_count,-1);upward_edges.resize(node_count);downward_edges.resize(node_count);
//     make_processed_adjacency_list();
//     std::vector<std::pair<int,int>> order;
//     for(int i=0;i<node_count;i++){
//         order.push_back(std::make_pair(processed_outgoing_edges[i].size()+processed_incoming_edges[i].size(),i));
//     }
//     std::sort(order.begin(),order.end());
//     int rank_counter=0;
//     for(const auto& x:order){
//         rank[x.second]=rank_counter++;
//     }
//     for(auto [degree,u]:order){
//         std::vector<std::pair<int,double>> incoming_neighbours;
//         std::vector<std::pair<int,double>> outgoing_neighbours;
//         for(const auto& [v,edge]:processed_outgoing_edges[u]){
//                 outgoing_neighbours.push_back(std::make_pair(v,edge.length));
//             }
//         for(const auto& [v,edge]:processed_incoming_edges[u]){
//                 incoming_neighbours.push_back(std::make_pair(v,edge.length));
//             }
//         if(incoming_neighbours.empty() && outgoing_neighbours.empty())continue;
//         for(int i=0;i<incoming_neighbours.size();i++){
//             for(int j=0;j<outgoing_neighbours.size();j++){
//                 int v=incoming_neighbours[i].first;
//                 int w=outgoing_neighbours[j].first;
//                 if(v==w)continue;
//                 double present_dist=incoming_neighbours[i].second+ outgoing_neighbours[j].second;
//                 double witness=witness_search(v,w,u,present_dist);
//                 if(witness>present_dist){
//                     if(rank[v]<rank[w]){
//                         Shortcut_Edge Se(w,present_dist,u);
//                         upward_edges[v].push_back(Se);
//                         Shortcut_Edge Se2(v,present_dist,u);
//                         downward_edges[w].push_back(Se2);
//                     }
//                     Shortcut_Edge Se(w,present_dist,u);
//                     processed_outgoing_edges[v][w]=Se;
//                     Shortcut_Edge Se2(v,present_dist,u);
//                     processed_incoming_edges[w][v]=Se2;
//                 }
//             }
//         }
//         for(const auto [v,edge] :processed_outgoing_edges[u]){
//             if(rank[u]<rank[v]){
//                 Shortcut_Edge Se(v,edge.length,-1,edge.is_Shortcut);
//                 upward_edges[u].push_back(Se);
//             }
//         }
//         for(const auto [v,edge] :processed_incoming_edges[u]){
//             if(rank[u]>rank[v]){
//                 Shortcut_Edge Se(v,edge.length,-1,edge.is_Shortcut);
//                 downward_edges[u].push_back(Se);
//             }
//         }
//     }   
// }