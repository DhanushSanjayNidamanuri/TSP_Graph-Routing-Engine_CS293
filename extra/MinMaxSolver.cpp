#include "MinMaxSolver.hpp"
#include <algorithm>
#include <limits>
#include <iostream>

double MinMaxSolver::calculate_time(Graph& graph, std::vector<int>& route){
    double total_time = 0.0;
    for(size_t i=1; i<route.size(); i++){
        total_time += graph.apsp_times[route[i-1]][route[i]];
    }
    return total_time;
}

std::vector<std::tuple<int, std::vector<int>, std::vector<int>>> MinMaxSolver::solve_min_max(Graph& graph, std::vector<std::tuple<int, int, int>>& orders, int num_drivers, int depot){
    std::vector<std::tuple<int, std::vector<int>, std::vector<int>>> assignments;

    //All drivers start at depot
    for(int i=0; i<num_drivers; i++){
        assignments.push_back(std::make_tuple(i, std::vector<int>{depot}, std::vector<int>{}));
    }

    //allotment
    for(auto& order : orders){
        int order_id = std::get<0>(order);
        int pickup = std::get<1>(order);
        int dropoff = std::get<2>(order);

        double best_max_time = std::numeric_limits<double>::max();
        int best_driver = -1;
        std::vector<int> best_route;
        std::vector<int> best_order_ids;

        //trying all combinations and permutations
        for(int d_id = 0; d_id < num_drivers; d_id++){
            auto current_route = std::get<1>(assignments[d_id]);
            auto current_order_ids = std::get<2>(assignments[d_id]);

            //new route
            std::vector<int> new_route = current_route;
            new_route.push_back(pickup);
            new_route.push_back(dropoff);

            std::vector<int> new_order_ids = current_order_ids;
            new_order_ids.push_back(order_id);

            //new max delivery time
            double new_max_time = 0.0;
            for(int d=0; d < num_drivers; d++){
                double driver_time;
                if(d == d_id){
                    driver_time = calculate_time(graph, new_route);
                }else{
                    driver_time = calculate_time(graph, std::get<1>(assignments[d]));
                }
                new_max_time = std::max(new_max_time, driver_time);
            }

            if(new_max_time < best_max_time){
                best_max_time = new_max_time;
                best_driver = d_id;
                best_route = new_route;
                best_order_ids = new_order_ids;
            }
        }
        if(best_driver != -1){
            std::get<1>(assignments[best_driver]) = best_route;
            std::get<2>(assignments[best_driver]) = best_order_ids;
        }
    }
    return assignments;
}

nlohmann::json MinMaxSolver::solve(Graph& graph, const nlohmann::json& query){
    std::vector<std::tuple<int, int, int>> orders;
    for(auto& order_json : query["orders"]){
       orders.push_back(std::make_tuple(order_json["order_id"], order_json["pickup"], order_json["dropoff"])); 
    }

    int num_drivers = query["fleet"]["num_delivery_guys"];
    int depot = query["fleet"]["depot_node"];

    //solve
    auto assignments = solve_min_max(graph, orders, num_drivers, depot);

    //total time
    double total_delivery_time = 0.0;
    for(auto& assignment : assignments){
        total_delivery_time += calculate_time(graph, std::get<1>(assignment));
    }

    nlohmann::json output;
    nlohmann::json assignments_json = nlohmann::json::array();
    
    for (const auto& assignment : assignments) {
        nlohmann::json driver_json;
        driver_json["driver_id"] = std::get<0>(assignment);
        driver_json["route"] = std::get<1>(assignment);
        driver_json["order_ids"] = std::get<2>(assignment);
        assignments_json.push_back(driver_json);
    }
    
    output["assignments"] = assignments_json;
    output["metrics"]["total_delivery_time_s"] = total_delivery_time;
    
    return output;
}