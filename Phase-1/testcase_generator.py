import requests
import json
import os
import math
import random
from pathlib import Path
import random
#Working directory
pwd = Path(__file__).parent.resolve()
os.chdir(pwd)

latDOWN=19.069276
latUP=19.076739
longLEFT=72.824315
longRIGHT=72.838874

query = f"""
[out:json][timeout:100];
(
  way[highway]({latDOWN},{longLEFT},{latUP},{longRIGHT});   // all roads
  node[amenity]({latDOWN},{longLEFT},{latUP},{longRIGHT});  // POIs like restaurants, schools, etc.
  node[shop]({latDOWN},{longLEFT},{latUP},{longRIGHT});     // shops
  node[tourism]({latDOWN},{longLEFT},{latUP},{longRIGHT});  // tourist POIs
);
(._;>;);
out body;
"""

print("🔍 Fetching data from Overpass API to generate a testcase...")
response = requests.get("https://overpass-api.de/api/interpreter", params={'data': query})
response.raise_for_status()
osm = response.json()

#Extract nodes
node_map = {}
node_id_map = {}
node_counter = 0
pois_available=set()
for element in osm["elements"]:
    if element["type"] == "node":
        tags = element.get("tags", {})
        pois = []
        if "amenity" in tags:
            pois.append(tags["amenity"].title())
            pois_available.update([tags["amenity"].title()])
        node_map[element["id"]] = {
            "id": node_counter,
            "lat": element["lat"],
            "lon": element["lon"],
            "pois": pois
        }
        node_id_map[element["id"]]=node_counter
        node_counter+=1

print(pois_available)
# Euclidean distance (meters)
def euclidean_meters(lat1, lon1, lat2, lon2):
    mean_lat = math.radians((lat1 + lat2) / 2)
    dx = (lon2 - lon1) * 111320 * math.cos(mean_lat)
    dy = (lat2 - lat1) * 111320
    return math.sqrt(dx * dx + dy * dy)

# Extract edges
edges = []
edge_id = 0
max_length=0
min_length=100000
def simplify_road_type(highway_tag: str) -> str:
    if highway_tag in ["motorway", "trunk"]:
        return "expressway"
    elif highway_tag in ["primary", "trunk_link"]:
        return "primary"
    else:
        return "secondary"

for el in osm["elements"]:
    if el["type"] == "way" and "highway" in el.get("tags", {}):
        road_type = el["tags"]["highway"]
        road_type=simplify_road_type(road_type)
        oneway = el["tags"].get("oneway", "no") == "yes"
        refs = el.get("nodes", [])

        for i in range(len(refs) - 1):
            u_ref, v_ref = refs[i], refs[i + 1]
            if u_ref in node_map and v_ref in node_map:
                u = node_map[u_ref]
                v = node_map[v_ref]

                # Euclidean distance (meters)
                length = euclidean_meters(u["lat"], u["lon"], v["lat"], v["lon"])
                max_length=max(length,max_length)
                min_length=min(length,min_length)
                # Approximate speed (m/s) from google
                base_speed = {
                    "motorway": 27.8,   
                    "primary": 22.2,  
                    "secondary": 16.6,  
                    "residential": 8.3, 
                    "service": 5.5
                }.get(road_type, 13.9)  # default 50 km/h

                # Time = distance / speed (seconds)
                avg_time = length / base_speed

                # Random 96-slot speed profile (m/s)
                speed_profile = [round(random.uniform(base_speed * 0.8, base_speed * 1.2), 2) for _ in range(96)]

                edges.append({
                    "id": edge_id,
                    "u": node_id_map[u_ref],
                    "v": node_id_map[v_ref],
                    "length": round(length, 2),
                    "average_time": round(avg_time, 2),
                    "speed_profile": speed_profile,
                    "oneway": oneway,
                    "road_type": road_type
                })
                edge_id += 1
edge_id-=1
#  Final structured JSON
custom_json = {
    "meta": {
        "id": "testcase_1",
        "nodes": len(node_map),
        "description": "OSM-based testcase (Euclidean distances)"
    },
    "nodes": list(node_map.values()),
    "edges": edges
}

# Save file
output_file = "testcases/test1.json"
with open(output_file, "w", encoding="utf-8") as f:
    json.dump(custom_json, f, indent=2)

print(f"✅ Saved {len(node_map)} nodes and {len(edges)} edges to {output_file}")

#query generating
queries=[]
queries.append(
        {
    "meta": { "id": "qset1" },
    "events": [ "query"]
    }
)
query_count=0
no_of_queries_per_type=max(int(node_counter/20),10)
for i in range(no_of_queries_per_type):
    source,destination,forbid=random.sample(range(node_counter),3)
    queries.append({
        "type":"shortest_path",
        "id":query_count,
        "source":f"{source}",
        "target": f"{destination}",
        "mode": "distance",
        "constraints": {
            "forbidden_nodes": [forbid],
            "forbidden_road_types": ["primary"] 
        }
    })
    query_count+=1

for i in range(no_of_queries_per_type):
    source,destination,forbid=random.sample(range(node_counter),3)
    queries.append({
        "type":"shortest_path",
        "id":query_count,
        "source":source,
        "target":destination,
        "mode": "time",
        "constraints": {
            "forbidden_nodes": [forbid],
            "forbidden_road_types": ["primary"] 
        }
    })
    query_count+=1
for i in range(no_of_queries_per_type):
    source=random.sample(range(node_counter),3)
    k=random.sample(range(int(no_of_queries_per_type/2)),1)
    lat=round(random.uniform(latDOWN,latUP),7)
    lon=round(random.uniform(longLEFT,longRIGHT),7)
    pois=random.sample(sorted(pois_available),1)
    metric=random.sample(["shortest_path","Euclidian Distance"],1)
    queries.append({
        "type": "knn",
        "id":query_count,
        "pois" : pois,
        "query_point": { "lat": lat, "lon": lon },
        "k": k,
        "metric": metric
    })
    query_count+=1
for i in range(no_of_queries_per_type):
    edge=random.sample(range(edge_id),1)
    queries.append({ 
        "type": "remove_edge",
        "edge_id": edge
    }
    )

for i in range(no_of_queries_per_type):
    edge=random.sample(range(edge_id),1)
    new_len=round(random.uniform(int(min_length),int(max_length)),2)
    queries.append({ 
        "type": "modify_edge",
        "edge_id": edge,
        "patch": { "length": new_len } 
    }
    )
random.shuffle(queries)
output_file = "testcases/queries_test1.json"
with open(output_file, "w", encoding="utf-8") as f:
    json.dump(queries, f, indent=2)

print(f"✅ Saved {query_count} queries  {output_file}")
