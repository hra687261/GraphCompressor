#ifndef ADJLIST
#define ADJLIST

#include "tools.h"

typedef struct _adjlist
{
	uint64_t n;    //number of nodes +1
	uint64_t e;    //number of edges
	uint64_t *cd;  //cumulative degree cd[0]=0 length=n+1
	uint64_t *adj; //concatenated lists of neighbors of all nodes
}AdjList;

AdjList *make_AdjList();

void free_AdjList(AdjList *al);

AdjList *load_AdjListU(char *path);   

AdjList *load_AdjList(char *path);   

void print_AdjList(AdjList *al);

#endif