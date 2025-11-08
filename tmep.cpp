
#include <nlohmann/json.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
/*
    Add other includes that you require, only write code wherever indicated
*/
#include "graph.hpp"
using json = nlohmann::json;

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: " << argv[0] << " <graph.json> <queries.json> <output.json>" << std::endl;
        return 1;
    }

    // Read graph from first file
    /*
        Add your graph reading and processing code here
        Initialize any classes and data structures needed for query processing
        Close the file after reading it
    */
    ///////-------> MODIFIED FROM HERE <------------//////

    std::ifstream input_graph_file(argv[1]);
    if (!input_graph_file.is_open()) {
        std::cerr << "Failed to open " << argv[1] << std::endl;
        return 1;
    }
    json graph_data;
    input_graph_file>>graph_data;
    input_graph_file.close();
    //reading nodes
    Graph Graph_internal(graph_data["nodes"].size());
    for(auto x:graph_data["nodes"]){
        std::vector<std::string> pois=x["pois"].get<std::vector<std::string>>();
        Node temp(x["id"],x["lat"],x["lon"],pois);
        Graph_internal.addNode(temp);
    }

    for(auto x:graph_data["edges"]){
        std::vector<double> speed_profile=x["speed_profile"].get<std::vector<double>>();
        Edge temp(x["id"],x["u"],x["v"],x["length"],x["average_time"],speed_profile,x["oneway"],x["road_type"]);
        Graph_internal.addEdge(temp);
    }
    ////////-------> UPTO HERE <-----------///////////////

    // Read queries from second file
    std::ifstream queries_file(argv[2]);
    if (!queries_file.is_open()) {
        std::cerr << "Failed to open " << argv[2] << std::endl;
        return 1;
    }

    json queries_json;
    queries_file >> queries_json;
    queries_file.close();

    json meta = queries_json["meta"];
    std::vector<json> results;

    for (const auto& query : queries_json["events"]) {
        auto start_time = std::chrono::high_resolution_clock::now();

        /*
            Add your query processing code here
            Each query should return a json object which should be printed to sample.json
        */

        // Answer each query replacing the function process_query using 
        // whatever function or class methods that you have implemented
        json result = Graph_internal.query_handler(query);

        auto end_time = std::chrono::high_resolution_clock::now();
        result["processing_time"] = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        results.push_back(result);
    }

    std::ofstream output_file(argv[3]);
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output.json for writing" << std::endl;
        return 1;
    }

    json output;
    output["meta"] = meta;
    output["results"] = results;
    output_file << output.dump(4) << std::endl;

    output_file.close();
    return 0;
}