#include "ShortestPath.hpp"

double ShortestPath::heuristic_time(const Node& a, const Node& b) {
    double time_seconds = heuristic_distance(a,b)/ 60;
    return (time_seconds);
}

double ShortestPath::heuristic_distance(const Node& a, const Node& b) {
    double lat1=a.lat,lon1=a.lon,lat2=a.lat,lon2=a.lon;
    lat1 *= M_PI / 180.0;
    lon1 *= M_PI / 180.0;
    lat2 *= M_PI / 180.0;
    lon2 *= M_PI / 180.0;
    double dlat = lat2 - lat1;
    double dlon = lon2 - lon1;
    double d= std::sin(dlat / 2) * std::sin(dlat / 2) + std::cos(lat1) * std::cos(lat2) * std::sin(dlon / 2) * std::sin(dlon / 2);
    double c = 2 * std::atan2(std::sqrt(d), std::sqrt(1 - d));
    return 6371.0*c;

    }
bool ShortestPath::Is_Usable_Now(Node& destination,Edge& edge,std::vector<bool>& visited, std::unordered_map<std::string,bool>& fb_types){
    if(!destination.isValid)return false;
    if(!edge.isOpen)return false;
    if(edge.oneway && destination.id==edge.u)return false;
    if(visited[destination.id])return false;
    if(fb_types[edge.road_type])return false;
    return true;
}

double ShortestPath::Expected_time(Edge& edge,double start_time){
    if(edge.speed_profile.size()==0){
        return start_time+edge.average_time;
    }
    int present_speed_profile_id=int(start_time/900)%96;
    double travesal_time=0;
    double distance=edge.length;
    double delta=((present_speed_profile_id+1))*900-start_time%86400;
    if(edge.length<=delta*edge.speed_profile[present_speed_profile_id]){
        return start_time+edge.length/edge.speed_profile[present_speed_profile_id];
    }
    travesal_time+=delta;
    distance-=delta*edge.speed_profile[present_speed_profile_id];
    present_speed_profile_id+=1;
    present_speed_profile_id%=96;

    while(distance>0){
        if(900*edge.speed_profile[present_speed_profile_id]<=distance){
            distance-=900*edge.speed_profile[present_speed_profile_id];
            travesal_time+=900;present_speed_profile_id++;
            present_speed_profile_id%=96;continue;
        }
        else{
            travesal_time+=distance/edge.speed_profile[present_speed_profile_id];break;
        }
    }
    return start_time+travesal_time;
}

std::vector<int> ShortestPath::Backtrack(int u,std::vector<int>& parent){
    std::vector<int> path;
    while(parent[u]!=u){
        path.push_back(u);
        u=parent[u];
    }
    path.push_back(u);
    std::reverse(path.begin(),path.end());
    return path;
};

ShortestPath_Result ShortestPath::findShortestPath(Graph& graph, int id, int source, int target,
     const std::string& mode, const std::vector<int>& forbidden_nodes,
        const std::vector<std::string>& forbidden_road_types){

            /////------->  INITIALISATION  <------/////
            for(auto fb_id:forbidden_nodes){
                graph.node_list[fb_id].isValid=false;
            }
            std::unordered_map<std::string,bool> fb_types;
            for(auto s:forbidden_road_types){
                    fb_types[s]=true;
            }
            

            int node_count=graph.node_list.size();
            std::vector<bool> visited(node_count,false);
            std::vector<int> parent(node_count,0);
            /////--------------------------------/////

            if(mode=="time"){
                std::priority_queue<std::tuple<double,int,int>> pq;
                pq.push(std::make_tuple(-heuristic_time(graph.node_list[source],graph.node_list[target]),source,source));
                while(!pq.empty()){
                    auto [neg_time,u,par]=pq.top();pq.pop();
                    neg_time=neg_time+heuristic_time(graph.node_list[u], graph.node_list[target]);
                    if(visited[u]==true)continue;
                    visited[u]=true;
                    parent[u]=par;
                    if(u==target){
                        ShortestPath_Result Out(id,true,-neg_time,Backtrack(u,parent));
                        for(auto fb_id:forbidden_nodes){
                            graph.node_list[fb_id].isValid=true;
                        }
                        return Out;
                    }
                    for(auto& p:graph.adjacency_list[u]){
                        int v =(p.second.u==u) ? p.second.v : p.second.u;
                        if(Is_Usable_Now(graph.node_list[v],p.second,visited,fb_types)){
                            double expected_time_to_reach=Expected_time(p.second,-neg_time);
                            pq.push(std::make_tuple(-expected_time_to_reach-heuristic_time(graph.node_list[v], graph.node_list[target]),v,u));
                        }
                    }
                }
            }
            else if(mode=="distance"){
                std::priority_queue<std::tuple<double,int,int>> pq;
                pq.push(std::make_tuple(-heuristic_distance(graph.node_list[source],graph.node_list[target]),source,source));
                while(!pq.empty()){
                    auto [neg_dist,u,par]=pq.top();pq.pop();
                    neg_dist=neg_dist+heuristic_distance(graph.node_list[u], graph.node_list[target]);
                    if(visited[u]==true)continue;
                    visited[u]=true;
                    parent[u]=par;
                    if(u==target){
                        ShortestPath_Result Out(id,true,-neg_dist,Backtrack(u,parent));
                        for(auto fb_id:forbidden_nodes){
                            graph.node_list[fb_id].isValid=true;
                        }
                        return Out;
                    }
                    for(auto p:graph.adjacency_list[u]){
                        int v =(p.second.u==u) ? p.second.v : p.second.u;
                        if(Is_Usable_Now(graph.node_list[v],p.second,visited,fb_types)){
                            pq.push(std::make_tuple(neg_dist-p.second.length-heuristic_distance(graph.node_list[v],graph.node_list[target]),v,u));
                        }
                    }
                }
            }
            for(auto fb_id:forbidden_nodes){
                graph.node_list[fb_id].isValid=true;
            }
            ShortestPath_Result Out(id,false);
            return Out;
        }