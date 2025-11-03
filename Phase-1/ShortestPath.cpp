#include<ShortestPath.hpp>
ShortestPath_Result ShortestPath::findShortestPath(Graph& graph, int id, int source, int target,
     const std::string& mode, const std::vector<int>& forbidden_nodes,
        const std::vector<std::string>& forbidden_road_types){
            for(auto fb_id:forbidden_nodes){
                graph.node_list[fb_id].isValid=false;
            }
            if(mode=="time"){
                int node_count=graph.node_list.size();
                std::priority_queue<std::tuple<int,int,int>> pq;
                pq.push(std::make_tuple(0,source,source));
                std::vector<bool> visited(node_count,false);
                std::unordered_map<std::string,bool> fb_types;
                std::vector<int> parent(node_count,0);
                for(auto s:forbidden_road_types){
                    fb_types[s]=true;
                }
                while(!pq.empty()){
                    auto [neg_time,u,par]=pq.top();pq.pop();
                    if(visited[u]==true)continue;
                    visited[u]=true;
                    parent[u]=par;
                    if(u==target){
                        ShortestPath_Result Out;
                        Out.id=id;
                        Out.possible=true;
                        Out.min_time=-neg_time;
                        std::vector<int> path;
                        while(parent[u]!=u){
                            path.push_back(u);
                            u=parent[u];
                        }
                        path.push_back(u);
                        Out.path=path;
                        return Out;
                    }
                    for(auto p:graph.adjacency_list[u]){
                        if(p.second.isOpen==true && visited[p.second.v]==false && graph.node_list[p.second.v].isValid==true && fb_types[p.second.road_type]==false){
                            if(p.second.speed_profile.size()==0){
                                pq.push(std::make_tuple(neg_time-p.second.average_time,p.second.v,u));continue;
                            }
                            int present_time=-(neg_time);
                            int present_speed_profile_id=int(present_time/900)%96;
                            int travesal_time=0;
                            int distance=p.second.length;
                            while(distance>0){
                                if(900*p.second.speed_profile[present_speed_profile_id]>=distance){
                                    distance-=900*p.second.speed_profile[present_speed_profile_id];
                                    travesal_time+=900;present_speed_profile_id++;
                                    present_speed_profile_id%=96;continue;
                                }
                                else{
                                    travesal_time+=distance/p.second.speed_profile[present_speed_profile_id];break;
                                }
                            }
                            pq.push(std::make_tuple(neg_time-travesal_time,p.second.v,u));
                        }
                    }
                }
                ShortestPath_Result Out;
                Out.id=id;
                Out.possible=false;
                return Out;
            }
            else if(mode=="distance"){
                int node_count=graph.node_list.size();
                std::priority_queue<std::tuple<int,int,int>> pq;
                pq.push(std::make_tuple(0,source,source));
                std::vector<bool> visited(node_count,false);
                std::unordered_map<std::string,bool> fb_types;
                std::vector<int> parent(node_count,0);
                for(auto s:forbidden_road_types){
                    fb_types[s]=true;
                }
                while(!pq.empty()){
                    auto [neg_dist,u,par]=pq.top();pq.pop();
                    if(visited[u]==true)continue;
                    visited[u]=true;
                    parent[u]=par;
                    if(u==target){
                        ShortestPath_Result Out;
                        Out.id=id;
                        Out.possible=true;
                        Out.min_distance=-neg_dist;
                        std::vector<int> path;
                        while(parent[u]!=u){
                            path.push_back(u);
                            u=parent[u];
                        }
                        path.push_back(u);
                        Out.path=path;
                        return Out;
                    }
                    for(auto p:graph.adjacency_list[u]){
                        if(p.second.isOpen==true && visited[p.second.v]==false && graph.node_list[p.second.v].isValid==true && fb_types[p.second.road_type]==false){
                            pq.push(std::make_tuple(neg_dist-p.second.length,p.second.v,u));
                        }
                    }
                }
                ShortestPath_Result Out;
                Out.id=id;
                Out.possible=false;
                return Out;
            }
            for(auto fb_id:forbidden_nodes){
                graph.node_list[fb_id].isValid=true;
            }
        }