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
void Graph::dijkstra_FarLM(std::vector<double>& distances,int src){
    std::priority_queue<std::pair<double,int>,std::vector<std::pair<double,int>>,std::greater<std::pair<double,int>>> pq;
    pq.push(std::make_pair(0,src));
    while(!pq.empty()){
        auto [dist,u]=pq.top();pq.pop();
        if(distances[u]<=dist)continue;
        distances[u]=dist;
        for(auto& e:adjacency_list[u]){
            int v =(e.u==u) ? e.v : e.u;
            if(((!e.oneway) || (e.u == u)) ){
                pq.push(std::make_pair(dist+e.length,v));
            }
        }
    }
}

void Graph::preprocess(){
    int no_of_landmarks=std::min(4096,node_count);
    nearest_into_landmark.resize(node_count,std::make_pair(-1,-1));
    nearest_outOf_landmark.resize(node_count,std::make_pair(-1,-1));
    std::vector<int> landmarkID_to_nodeID(no_of_landmarks);
    if(no_of_landmarks==node_count){
        for(unsigned int i=0;i<nearest_into_landmark.size();i++){
            nearest_into_landmark[i]=std::make_pair(i,0);
            nearest_outOf_landmark[i]=std::make_pair(i,0);
            landmarkID_to_nodeID[i]=i;
            node_list[i].is_landmark=true;
        }
        for(auto x:landmarkID_to_nodeID){
            for(auto y:landmarkID_to_nodeID){
                landmark_to_landmark[x][y]=-1;
            }
        }
        for(auto src:landmarkID_to_nodeID){
            std::vector<bool> visited(node_count,false);
            std::priority_queue<std::pair<double,int>,std::vector<std::pair<double,int>>,std::greater<std::pair<double,int>>> pq;
            pq.push(std::make_pair(0,src));
            while(!pq.empty()){
                auto [dist,u]=pq.top();pq.pop();
                if(visited[u]==true)continue;
                visited[u]=true;
                landmark_to_landmark[src][u]=dist;
                for(auto& e:adjacency_list[u]){
                    int v =(e.u==u) ? e.v : e.u;
                    if(!visited[v] && ((!e.oneway) || (e.u == u))){
                        pq.push(std::make_pair(dist+e.length,v));
                    }
                }
            }
        }
    }
    else{
        std::vector<double> dmin(node_count);
        std::vector<int> degrees(node_count);
        int maxi=0;
        for (int i = 0; i < node_count; i++) {
            dmin[i] =std::numeric_limits<double>::max()/200;
            degrees[i]=adjacency_list[i].size();maxi=std::max(degrees[i],maxi);
            if(degrees[i]==0)node_list[i].isolated=true;
        }
        landmarkID_to_nodeID[0] = 0;
        node_list[0].is_landmark = true;
        dijkstra_FarLM(dmin, 0);
        std::vector<double> score(node_count);
        for (int lm = 1; lm < no_of_landmarks; lm++) {
            // ---- compute selection score for every node ----
            for (int i = 0; i < node_count; i++) {
                score[i] = dmin[i] * degrees[i];        
            }
            // ---- pick node with maximum score ----
            int best = std::max_element(score.begin(), score.end()) - score.begin();
            landmarkID_to_nodeID[lm] = best;
            node_list[best].is_landmark = true;
            // ---- update dmin with Dijkstra from new LM ----
            dijkstra_FarLM(dmin, best);
        }

        for(auto x:landmarkID_to_nodeID){
            for(auto y:landmarkID_to_nodeID){
                landmark_to_landmark[x][y]=-1;
            }
        }
        for(int i=0;i<no_of_landmarks;i++){
            int count=0;
            std::priority_queue<std::pair<double,int>,std::vector<std::pair<double,int>>,std::greater<std::pair<double,int>>> pq;
            std::vector<bool> visited(node_count,false);
            pq.push(std::make_pair(0,landmarkID_to_nodeID[i]));
            while(!pq.empty()){
                auto [dist,u]=pq.top();pq.pop();
                if(visited[u])continue;
                visited[u]=true;
                if(node_list[u].is_landmark){
                    landmark_to_landmark[landmarkID_to_nodeID[i]][u]=dist;count++;
                    if(count==no_of_landmarks)break;
                }
                for(auto& e:adjacency_list[u]){
                    int v =(e.u==u) ? e.v : e.u;
                    if(!visited[v] && ((!e.oneway) || (e.u == u))){
                        pq.push(std::make_pair(dist+e.length,v));
                    }
                }
            }
        }
        multi_source_dijkstra_into(landmarkID_to_nodeID);
        multi_source_dijkstra_outOf(landmarkID_to_nodeID);
    }
    
};



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