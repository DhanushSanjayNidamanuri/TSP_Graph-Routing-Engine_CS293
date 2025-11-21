
import json
import pandas as pd
import numpy as np
import os
from pathlib import Path
pwd = Path(__file__).parent.resolve()
os.chdir(pwd)
REAL_FILE = "output.json"
APPROX_FILE = "output_approx.json"
QUERIES_FILE = "Phase-2/testcases/queries_test1.json"
# ------------------------------------------------------------
# Parse REAL output
# ------------------------------------------------------------
def parse_real(data):
    rows = []
    for r in data["results"]:
        rid = r["id"]
        real_dist = r["minimum_time/minimum_distance"]
        possible = r["possible"]

        rows.append(( real_dist))
    return rows

# ------------------------------------------------------------
# Parse APPROX output
# ------------------------------------------------------------
def parse_approx(data):
    rows = []

    for r in data["results"]:
        for d in r["distances"]:
            approx = d["approx_shortest_distance"]
            rows.append(( approx))

    return rows

def parse_queries():
    rows=[]
    with open(QUERIES_FILE) as f:
        queries_data = json.load(f)
        for r in queries_data["events"]:
            threshold=r["acceptable_error_pct"]
            for q in r["queries"]:
                rows.append(threshold)
    return rows

# ------------------------------------------------------------
# Load
# ------------------------------------------------------------
with open(REAL_FILE) as f:
    real_data = json.load(f)

with open(APPROX_FILE) as f:
    approx_data = json.load(f)

df_real = pd.DataFrame(parse_real(real_data),
                       columns=["real"])

df_approx = pd.DataFrame(parse_approx(approx_data),
                         columns=["approx"])
df_queries=pd.DataFrame(parse_queries(),columns=["threshold"])
# Merge 
df = pd.concat([df_real, df_approx,df_queries], axis=1)

# ------------------------------------------------------------
# Compute errors
# ------------------------------------------------------------

df["abs_error_percentage"] =((df["approx"] - df["real"]) / df["real"]) * 100
df["approx paths given"]=((df["abs_error_percentage"] > 0) & (df["abs_error_percentage"] != 100) )
# ------------------------------------------------------------
# Summary
# ------------------------------------------------------------
summary = {
    "total_queries": len(df),
    "max_error": float(df["abs_error_percentage"].max()),
    "min_error": float(df["abs_error_percentage"].min()),
    "mean_error": float(df["abs_error_percentage"].mean()),
    "threshold_violations": int((df["abs_error_percentage"] > df["threshold"]).sum())+int(( df["abs_error_percentage"] < 0).sum()),
    "No of approx paths given":((df["abs_error_percentage"] > 0) & (df["abs_error_percentage"] != 100) ).sum(),
    "Incorretly_No_Path_given":((df["approx"] < 0) & (df["real"] >= 0 )).sum()
}

print("\n===== SUMMARY =====")
for k,v in summary.items():
    print(f"{k:28}: {v}")

df.to_csv("analysis_approx_output.csv", index=False)
print("\nSaved analysis_approx_output.csv")

