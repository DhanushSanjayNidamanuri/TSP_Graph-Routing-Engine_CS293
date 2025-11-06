#include "ShortestPath.hpp"

int ShortestPath::heuristic_distance(const Node& a, const Node& b) {
    if(heuristic_distance_cache[a.id].find(b.id)!=heuristic_distance_cache[a.id].end())return heuristic_distance_cache[a.id][b.id];
    double mean_lat = (a.lat + b.lat) * 0.5 *  3.14159265358979323846 / 180.0;
    double dx = (b.lon - a.lon) * 111320.0 * std::cos(mean_lat);
    double dy = (b.lat - a.lat)*111320.0;
    double ans=std::sqrt(dx * dx + dy * dy);
    heuristic_distance_cache[a.id][b.id]=int(ans);
    heuristic_distance_cache[b.id][a.id]=int(ans);
    return int(ans);
    }
bool ShortestPath::Is_Usable_Now(Node& destination,Edge& edge,std::vector<bool>& visited, std::unordered_map<std::string,bool>& fb_types){
    if(!destination.isValid)return false;
    if(!edge.isOpen)return false;
    if(edge.oneway && destination.id==edge.u)return false;
    if(visited[destination.id])return false;
    if(fb_types[edge.road_type])return false;
    return true;
}

int ShortestPath::Expected_time(Edge& edge,int start_time){
    if(edge.speed_profile.size()==0){
        return start_time+edge.average_time;
    }
    int present_speed_profile_id=int(start_time/900)%96;
    int travesal_time=0;
    int distance=edge.length;
    while(distance>0){
        if(900*edge.speed_profile[present_speed_profile_id]>=distance){
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
            std::priority_queue<std::tuple<int,int,int>> pq;

            int node_count=graph.node_list.size();
            std::vector<bool> visited(node_count,false);
            std::vector<int> parent(node_count,0);
            /////--------------------------------/////

            if(mode=="time"){
                pq.push(std::make_tuple(0,source,source));
                while(!pq.empty()){
                    auto [neg_time,u,par]=pq.top();pq.pop();
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
                            int expected_time_to_travel=Expected_time(p.second,-neg_time);
                            pq.push(std::make_tuple(-expected_time_to_travel,v,u));
                        }
                    }
                }
            }
            else if(mode=="distance"){
                pq.push(std::make_tuple(-heuristic_distance(graph.node_list[source],graph.node_list[target]),source,source));
                while(!pq.empty()){
                    auto [neg_dist,u,par]=pq.top();pq.pop();
                    neg_dist=neg_dist+heuristic_distance_cache[u][target];
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