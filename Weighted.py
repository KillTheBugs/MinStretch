## Importing Libraries

import networkx as nx
import time
import random
from networkx.algorithms.centrality import edge_betweenness_centrality as EBC
from networkx.algorithms.tree.mst import minimum_spanning_edges as MST
from networkx.algorithms.shortest_paths.unweighted import single_source_shortest_path as SSSP
from networkx.algorithms.shortest_paths.generic import average_shortest_path_length as calculate_stretch
from networkx.algorithms.components import is_connected as is_conn
from networkx.algorithms.components import number_connected_components as conn_comp_no

def CalculateStretch(G):
    return calculate_stretch(G);
#     for i in range (0,n):
#         shortest_paths = []

## Initializing Values & Generating Random Graphs

n = 1000
p = 0.7

file_writer = open("Timings.txt","a+")
file_writer.write("n="+str(n)+" p="+str(p))
print("n="+str(n)+" p="+str(p))

G = nx.generators.random_graphs.fast_gnp_random_graph(n,p);

# for (u,v,w) in G.edges(data=True):
#     w['weight'] = random.randint(1,10)



EC_MST = nx.generators.random_graphs.fast_gnp_random_graph(n,0);

Dijkstra_MST = nx.generators.random_graphs.fast_gnp_random_graph(n,0);

Dummy_G=G.copy()

## Calculating Edge Centrality
EC_start = time.process_time()
EC = EBC(G)
EC_end = time.process_time()
file_writer.write("\nEdge Centrality calculation Time = "+str(EC_end - EC_start))
print("Edge Centrality calculation Time = "+str(EC_end - EC_start))
#EC

### Sorting the centralities

EC_sorted = sorted(EC.items(), key=lambda x: x[1], reverse=True)

# for x in EC_sorted:
#     print(x[0], ":", x[1])

#  for x in EC:
#     print(x[0],x[1])

## Generating MST

#mst = MST(G)



random_MST = nx.minimum_spanning_tree(G)

#random_MST.edges()



### Generating Max Centrality Spanning Tree

disjointArray = [x for x in range(0,len(EC_sorted))]

# disjoint_set = [{x} for x in range(0,len(EC_sorted))]

# disjoint_set

#disjointArray

EC_MST_start = time.process_time()

edges = 0
EC_edges_to_add = []
for i in range(0,len(EC_sorted)):
    if(edges>n-2):
        break
    u = EC_sorted[i][0][0]
    v = EC_sorted[i][0][1]
    u_set = disjointArray[u]
    v_set = disjointArray[v]
    if(u_set == v_set):
        continue
    EC_edges_to_add.append((u,v))
    EC_MST.add_edge(u,v)
    edges = edges+1
    #Could be optimized as u is always smaller than v
    for j in range(0,len(EC_sorted)):
        if(disjointArray[j]==v_set):
            disjointArray[j] = u_set

EC_MST_End = time.process_time()
file_writer.write("\nEdge Centrality Spanning Tree calculation Time = "+str(EC_MST_End - EC_MST_start))
print("Edge Centrality Spanning Tree calculation Time = "+str(EC_MST_End - EC_MST_start))

#print(len(EC_MST.edges))
#EC_MST.edges

# set1 = {2, 4, 5, 6} 
# set2 = {7, 8, 9, 10} 
# set3 = {1, 2} 
# set4 = set1.union(set2)

# set1.union(set2)

# Calculating Dijkstra Spanning Tree

Dummy_G.add_node(n+1)

max_edge_centrality_u = EC_sorted[0][0][0]
max_edge_centrality_v = EC_sorted[0][0][1]
EC_sorted[0][0]

Dummy_G.add_edge(EC_sorted[0][0][0],n+1,weight=0)

Dummy_G.add_edge(EC_sorted[0][0][1],n+1,weight=0)

#Dummy_G.edges(data=True)

all_shortest_paths = []
all_shortest_paths = SSSP(Dummy_G,n+1)

#all_shortest_paths

#Testing

Dij_MST_start = time.process_time()
dijkstra_edges_to_add = [(max_edge_centrality_u,max_edge_centrality_v)]
for i in range(0,n):
    if(i==max_edge_centrality_u or i==max_edge_centrality_v):
        continue
    dijkstra_edges_to_add.append(tuple(sorted((all_shortest_paths[i][-1],all_shortest_paths[i][-2]))))
Dij_MST_end = time.process_time()

file_writer.write("\nDijkstra Spanning Tree calculation Time = "+str(Dij_MST_end - Dij_MST_start))
print("Dijkstra Spanning Tree calculation Time = "+str(Dij_MST_end - Dij_MST_start))

# print(len(dijkstra_edges_to_add))

for (u,v) in dijkstra_edges_to_add:
    Dijkstra_MST.add_edge(u,v)

#Dijkstra_MST.edges

print("Dijktra : " + str(calculate_stretch(Dijkstra_MST)))
print("EC : " + str(calculate_stretch(EC_MST)))
print("Rand_MST : " + str(calculate_stretch(random_MST)))
print("")

file_writer.write("\nDijktra : " + str(calculate_stretch(Dijkstra_MST)))
file_writer.write("\nEC : " + str(calculate_stretch(EC_MST)))
file_writer.write("\nRand_MST : " + str(calculate_stretch(random_MST)))
file_writer.write("\n\n")

G.clear()
EC_MST.clear()
Dijkstra_MST.clear()
Dummy_G.clear()
random_MST.clear()