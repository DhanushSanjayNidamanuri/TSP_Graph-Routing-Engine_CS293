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
# #1000 nodes---------------->
# latDOWN=19.069276
# latUP=19.076739
# longLEFT=72.824315
# longRIGHT=72.838874
#-------------------------->


#5000 nodes---------------->
latDOWN = 19.0395
latUP = 19.0532
longLEFT = 72.8235
longRIGHT = 72.8404
#-------------------------->

query = f"""
[out:json][timeout:1000];
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
        if "shop" in tags:
            pois.append(tags["shop"].title())
            pois_available.update([tags["shop"].title()])
        if "tourism" in tags:
            pois.append(tags["tourism"].title())
            pois_available.update([tags["tourism"].title()])
        node_map[element["id"]] = {
            "id": node_counter,
            "lat": element["lat"],
            "lon": element["lon"],
            "pois": pois
        }
        node_id_map[element["id"]]=node_counter
        node_counter+=1

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
    if highway_tag in ["motorway", "trunk", "motorway_link"]:
        return "expressway"
    elif highway_tag in ["primary", "primary_link"]:
        return "primary"
    elif highway_tag in ["secondary", "secondary_link"]:
        return "secondary"
    elif highway_tag in ["tertiary", "tertiary_link"]:
        return "tertiary"
    else:
        return "local"

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
                    "expressway": 25.0,   # ~90 km/h
                    "primary": 16.7,      # ~60 km/h
                    "secondary": 13.9,    # ~50 km/h
                    "tertiary": 11.1,     # ~40 km/h
                    "local": 8.3,         # ~30 km/h
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
query_count=1
for i in range(1):
    source = random.randrange(node_counter) 
    no_deliv_guys = random.randint(1, 20)
    no_of_orders = random.randrange(node_counter // 50) 
    pickups = [random.randrange(node_counter) for _ in range(no_of_orders)]
    dropoffs = [random.randrange(node_counter) for _ in range(no_of_orders)]
    queries.append({
        "orders": [ {"order_id": j, "pickup": pickups[j], "dropoff": dropoffs[j]}for j in range(no_of_orders)],
        "fleet": {
            "num_delivery_guys": no_deliv_guys,
            "depot_node": source,
        },
    })
queries_json = {
    "meta": {"id": "testcase_1_queries"},
    "events":queries
}

output_file = "testcases/queries_test1.json"
with open(output_file, "w", encoding="utf-8") as f:
    json.dump(queries_json, f, indent=2)

print(f"✅ Saved {query_count} queries  {output_file}")
