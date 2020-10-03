# PSTL-GraphCompression:
PSTL-GraphCompression is a project we realized in our second semester of Master 1 STL - Sorbonne University. It's a C project for handeling and compressing very large graphs, inspired by https://www.ics.uci.edu/~djp3/classes/2008_01_01_INF141/Materials/p595-boldi.pdf. 5 data structures were implemented, CompAdjListG, CompAdjListCL, CompAdjListCB, CompAdjListI, and the final one CompAdjList. These data structures are compressed variants of an adjacency list, and they respectively represent the steps of compressing a graph described in the paper cited above.


# Preprocessing:
Our codes expect as input a path to a file containing an edge list, where on every line of the file there is a pair of positive numbers separated by a space, the positive numbers are the nodes, and every line represents a link from the node on the left to the node on the right. They also need to be sorted by the 1st and the 2nd column, and they should have no header or comment in the beginning of the file.
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

**renumbering:** renumbers the edgelist.

**writeCAL:** loads an edge list from an input file into a CompAdjList (Compressed adjacency list), and the stores the CompAdjList in an output file.  
Arguments:
* arg1: path to the (preprocessed) edge list file.
* arg2: path to the output file where the CompAdjList will be stored.
* arg3: window size.
* arg4: maximum reference count (max length of a reference chain).
* arg5: the minimum threshold of the length of an interval.


**readCAL_BFS:** loads a CompAdjList from the file in which it was stored, and runs the Breadth-First Search algorithm starting from values that must be written in the terminal.
* arg1: path to the file containing a CompAdjList.
* arg2: boolean value, 1 to print the all the traversed nodes, 0 to only print the number of traversed nodes.


**readCAL_PR:** loads a CompAdjList from the file in which it was stored, and runs the Pagerank algorithm, and prints the pagerank of the nodes that were given as arguments in the command line.
* arg1: path to the file containing a CompAdjList.
* arg2: the number of iterations of the PageRank algorithm.
* arg3: 1-the-damping-factor( should be between 0.1 & 0.2).
* arg4 and+:the nodes who's pagerank you want to print (if none are given, it just prints the first node that has the highest pagerank) 


**writeAL:**  loads an edge list from an input file into a one of the other compressed adjacency lists (CompAdjListG, CompAdjListCL, CompAdjListCB or CompAdjListI), and the stores it in an output file.  
* arg1: path to the (preprocessed) edge list file.
* arg2: path to the output file where the compressed adjacency list will be stored.

**readAL:** loads a compressed adjacency list from the file in which it was stored, and runs the Breadth-First Search algorithm starting from values that must be written in the terminal.
* arg1: path to the file containing a compressed adjacency list.
* arg2: boolean value, 1 to print the all the traversed nodes, 0 to only print the number of traversed nodes.

> :warning: when you run writeAL and readAL, a terminal prompt will ask you to choose a type of compressed adjacency list, make sure that when you store one in a file (with writeAL), you choose the same type when you read that file (with readAL).  



# To compile:
You can just:
```bash
make
```
Or choose one of the following:
```bash
make writeCAL 
make readCAL_BFS
make readCAL_PR
make writeAL
make readAL
make renumbering 
```


# To execute:
Execution examples with http://snap.stanford.edu/data/com-Orkut.html, after preprocessing:

**Writing with WriteCal:**  

```
<!-- language: lang-none -->
$ ./writeCAL ./../../data/orkut ./../../output/ork_cal 25 25 5 100
Choose the compression codes:
  Type 0 for the unary code.
  Type 1 for the gamma code.
  Type 2 for the delta code.
  Type 3 for the nibble code.
  Type 4 for the zeta-k code, followed by a the value of k (>0).

For the number of used bits by the compressed successors lists : 1

For the references : 2

For the block counts : 1

For the copy blocks : 3

For the intervals : 4
  Type a value that is > 0 for the shrinking parameter of the zeta code (k) : 3

For the residuals : 4
  Type a value that is > 0 for the shrinking parameter of the zeta code (k) : 2
Loaded CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 245800 B
    size(adj) = 235738885 B
    sizesum = 235984685 B
Loading execution time = 0h1m23s
Writing execution time = 0h0m5s
Overall time = 0h1m28s
```

**Reading with readCal_BFS:**   

```
<!-- language: lang-none -->
$ ./readCAL_BFS ./../../output/ork_cal 0
Read CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 245800 B
    size(adj) = 235738885 B
    sizesum = 235984685 B
Reading execution time = 0h0m4s
Type 0 to exit.
Type 1 followed by a number < 3072442, to run the breadth first algorithm starting from that node.
1
1
BFS starting node: 1
    execution time = 0h0m57s
    length: 3072441
1
543637
BFS starting node: 543637
    execution time = 0h0m39s
    length: 2300924
0
Overall time = 0h2m5s
```

**Reading with readCal_PR:**   

```
<!-- language: lang-none -->
$ ./readCAL_PR ./../../output/ork_cal 5 0.15  1  3000000 654654
Read CompAdjList: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 245800 B
    size(adj) = 235738885 B
    sizesum = 235984685 B
Reading execution time = 0h0m4s
PageRank execution time = 0h0m56s
PageRank result:
    PageRank[1] = 0.00000021729203188709
    PageRank[3000000] = 0.00000109128752359352
    PageRank[654654] = 0.00000021729203188709
The (first) Node with the highest PageRank value 2745491:
    PageRank[2745491] = 0.00000788862073738412
Overall time = 0h1m0s
```

**Writing with WriteAL:**   

```
<!-- language: lang-none -->
$ ./writeAL ./../../data/orkut ./../../output/ork_al
Choose a compressed data structre:
  Type 0 for the CompAdjListG. (Compressed Adjacency list with the gaps technique)
  Type 1 for the CompAdjListCL. (Compressed Adjacency list with the copy list technique and gaps for the residuals)
  Type 2 for the CompAdjListCB. (Compressed Adjacency list with the copy blocks technique and gaps for the residualse)
  Type 3 for the CompAdjListI. (Compressed Adjacency list with the copy blocks and intervals technique and gaps for the residualse)
0

Choose a compression code:
  Type 0 for the unary code.
  Type 1 for the gamma code.
  Type 2 for the delta code.
  Type 3 for the nibble code.
  Type 4 for the zeta-k code, followed by a the value of k (>0).
4
  Type a value that is > 0 for the shrinking parameter of the zeta code (k): 2
Loaded CompAdjListG: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 24579544 B
    size(adj) = 231005849 B
    sizesum = 255585393 B
Loading execution time = 0h1m28s
Writing execution time = 0h0m6s
Overall time = 0h1m34s
```

**Reading with readAL:**   

```
<!-- language: lang-none -->
$ ./readAL ./../../output/ork_al 0
Choose a compressed data structre:
  Type 0 for the CompAdjListG. (Compressed Adjacency list with the gaps technique)
  Type 1 for the CompAdjListCL. (Compressed Adjacency list with the copy list technique and gaps for the residuals)
  Type 2 for the CompAdjListCB. (Compressed Adjacency list with the copy blocks technique and gaps for the residualse)
  Type 3 for the CompAdjListI. (Compressed Adjacency list with the copy blocks and intervals technique and gaps for the residualse)
0
Read CompAdjListG: 
    Number of nodes = 3072441
    Number of links = 117185083
    size(cd) = 24579544 B
    size(adj) = 231005849 B
    sizesum = 255585393 B
Reading execution time = 0h0m4s
Type 0 to exit.
Type 1 followed by a number < 3072442, to run the breadth first algorithm starting from that node.
1
1
BFS starting node: 1
    execution time = 0h0m6s
    length: 3072441
1
543637
BFS starting node: 543637
    execution time = 0h0m4s
    length: 2300924
0
Overall time = 0h0m24s
```

# Code repartition:
- integerEncoding.c/.h: a set of functions implementing some compression algorithm, allowing us to encode and decode uint64_t values into an array of uint8_t (Bytes).  
- tools.c/.h: some basic helpful functions.
- CompAdjListG.c/.h: compressed adjacency list using the gaps technique.
- CompAdjListCL.c/.h: compressed adjacency list using the copy lists technique, and gaps for residual values.
- CompAdjListCB.c/.h: compressed adjacency list using the copy blocks technique, and gaps for residual values.
- CompAdjListI.c/.h: compressed adjacency list using the copy blocks along with the intervals technique, and gaps for residual values.
- CompAdjList.c/.h: compressed adjacency list using the copy blocks along with the intervals technique, and gaps for residual values, also stores the offset array using the "jump", this is the final compression format.

> :warning: in all the CompAdjListxx files, the value UB_BITS_PER_LINKxx is the upper bound of the size in bits of a link in the file, it will be allocated for the compressed adjacency lists, then the allocated size will be adjusted to the real size, if it's too small that might cause heap memory corruption, if it's too big you might run out of memory, so it might be necessary to change the values.