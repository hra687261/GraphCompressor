# PSTL-GraphCompression:
PSTL-GraphCompression is a project we realized in our second semester of Master 1 STL - Sorbonne University. It's a C project for handeling and compressing very large graphs, inspired by https://www.ics.uci.edu/~djp3/classes/2008_01_01_INF141/Materials/p595-boldi.pdf. 5 data structures were implemented, CompAdjListG, CompAdjListCL, CompAdjListCB, CompAdjListI, and the final one CompAdjList.

# Preprocessing:
Our codes expect as input filepaths to edge lists, where on every line of the file there is a pair of positive numbers separated by a space, the positive numbers are the nodes, and every line represents a link (or edge). They also need to be sorted by the 1st and the 2nd column, and they should have no header or comment in the beginning of the file.
For example if we take http://snap.stanford.edu/data/com-Amazon.html, we'll need to preprocess it. One way to do that is:

To get rid of the 4 first lines that start with #:
```bash
tail -n +5 path_to_the_file
```
To sort it by the first and second column:

```bash
sort -n -k1,1  -k2,2 -o path_to_the_file path_to_the_file
```
Then to get rid of the gaps between the values of the nodes, and have no missing nodes (if for a pair of nodes x and y, x>y+1, and there is no node y+1 in the graph then all the nodes >=x are decreased by 1), we renumber the graph with:
```bash
make renumbering 
renumbering path_to_the_file
```

# Runnables: 

renumbering.c: renumbers the edgelist.


in writeCAL.c: loads an edge list from a file, and stores it in a CompAdjList, then writes the CompAdjList to memory.  
Arguments:
* arg1: path to the (preprocessed) edge list.
* arg2: path to the output file where the CompAdjList will be stored.
* arg3: function id (cf. integerEncoding.c).
* arg4 (optional): the shrinking parameter k (cf. integerEncoding.c), add it only if arg3=4.
* arg5: window-size.
* arg6: max ref counting (max length of a reference list).
* arg7: the threshold of the length of an interval.
* arg8: length of the "jump".


in readCAL.c: takes a filepath to a stored CompAdjList, reads it and runs the Breadth-First Search of PageRank algorithm.
* arg1: path to the file containing a CompAdjList.
* arg2: algorithm to run, 1 for BFS, 2 to PageRank.
* arg3: 1 if you want to print the returned values of the executed algorithm, 0 otherwise.
* arg4: if arg3==1: the starting node of the BFS algorithm, else if arg3==2: then it's the number of iterations of the PageRank algorithm.
* arg5: 1-(the damping factor), needs to be between 0.1 & 0.2
* arg6 and+: the nodes who's pagerank you want to print (if none are given, just prints the node (one of the nodes) with the highest pagerank) 



in writeAL.c, readAL.c: do the same thing but to the other data structures (without the PageRank algorithm). an additional argument (in the 2nd position) is added, it's the datastructure id, 1 for CompAdjListG, 2 for CompAdjListCL 



# To compile:

```bash
make writeCAL 
make readCAL
make writeAL
make readAL
make
```
# To execute:
Execution example with http://snap.stanford.edu/data/com-Orkut.html, after preprocessing:

Writing:

```
<!-- language: lang-none -->
$ ./writeCAL ./../data/orkut cal_ork_44 4 4 10 10 2 5
CAL: n=3072441  e=117185083  size(cd)=4915912 B  size(adj)=221579106 B  sizesum=226495018 B
Loaded CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 614489 B
    size(adj) = 221579106 B
    sizesum = 226495018 B
Loading execution time = 0h1m8s
Writing execution time = 0h0m7s
Overall time = 0h1m15s
```

Reading with BFS:
```
<!-- language: lang-none -->
$ ./readCAL cal_ork_44 1 0 1 
>>> cmpfilepath : cal_ork_44
>>> p_res : 0

Read CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 4915912 B
    size(adj) = 221579106 B
    sizesum = 226495018 B
Reading execution time = 0h0m3s
BFS execution time = 0h0m23s
BFS length: 3072441
Overall time = 0h0m26s
Overall time = 0h0m26s
```

Reading with PageRank 1:
```
<!-- language: lang-none -->
./readCAL cal_ork_44 2 0 5 0.15  
>>> cmpfilepath : cal_ork_44
>>> p_res : 0

Read CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 4915912 B
    size(adj) = 221579106 B
    sizesum = 226495018 B
Reading execution time = 0h0m4s
PageRank execution time = 0h0m35s
Node with the highest PageRank 2745491: pr[2745491] = 0.00000788862073738412
Overall time = 0h0m39s
```
Reading with PageRank 2:

```
<!-- language: lang-none -->
$ ./readCAL cal_ork_44 2 1 5 0.15  1  3000000 654654
>>> cmpfilepath : cal_ork_44
>>> p_res : 1

Read CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 4915912 B
    size(adj) = 221579106 B
    sizesum = 226495018 B
Reading execution time = 0h0m5s
PageRank execution time = 0h0m36s
PageRank res: 
>>>> pr[1] = 0.00000021729203188709
>>>> pr[3000000] = 0.00000109128752359352
>>>> pr[654654] = 0.00000021729203188709

Node with the highest PageRank 2745491: pr[2745491] = 0.00000788862073738412
Overall time = 0h0m41s
```

# Code repartition:


 - adjList.c/.h: a simple adjacency list implementation, for comparaison.
 - bitArray.c/.h: a set of functions to do bitwise operations on an array of uint8_t.
 - integerEncoding.c/.h: a set of functions implementing some compression algorithm, allowing us to encode and decode uint64_t values into an array of uint8_t.
 - integerGeneration.c/.h: a set of functions to generate positive integers following some discrete distribution (Poisson, Binomial and Zipf's power law), they were used for testing.
 - exp.c/.h: some experimentations.
 - tools.c/.h: usefull functions.
 - CompAdjListG.c/.h: compressed adjacency list using the gaps technique.
 - CompAdjListCL.c/.h: compressed adjacency list using the copy lists technique, and gaps for residual values.
 - CompAdjListCB.c/.h: compressed adjacency list using the copy blocks technique, and gaps for residual values.
 - CompAdjListI.c/.h: compressed adjacency list using the copy blocks along with the intervals technique, and gaps for residual values.
 - CompAdjList.c/.h: compressed adjacency list using the copy blocks along with the intervals technique, and gaps for residual values, also stores the offset array using the "jump" parameter.

