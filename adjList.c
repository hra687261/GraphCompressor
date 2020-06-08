#include "adjList.h"

/**
 * Inits an AdjList
 */
AdjList *make_AdjList() 
{
  AdjList *al = malloc (sizeof (AdjList));
	al->n = 0;
	al->e = 0;
	al->cd = NULL;
	al->adj = NULL;
	return al;
}

/**
 * frees an AdjList and it's content
 */
void free_AdjList(AdjList *al) 
{
  free (al->cd);
	free (al->adj);
	free (al);
}

/**
 * Loads an undirected graph into an Adjlist.
 * from a  file containing a list of edges 
 * (pairs of positive number in a line separated by a space)
 */
AdjList *load_AdjListU(char *path) 
{
  uint64_t e1 = NB_NODES, i, v1, v2, *d;
	FILE *file;
	AdjList *g = make_AdjList ();
    file = fopen (path, "r");
    assert(file!=NULL);
	d = calloc (e1, sizeof (uint64_t));

  // counting the number of nodes, number of edges 
  // and the number of neighbours of every node
	while (fscanf (file, "%lu %lu", &v1, &v2) == 2)
		{
			g->n = max3 (g->n, v1, v2);
			g->e++;
			while (g->n >= e1)
				{
					d = realloc (d, (e1 + NB_NODES) * sizeof (uint64_t));
					assert (d != NULL);
					for (i = e1; i < e1 + NB_NODES; i++)
						d[i] = 0;
					e1 += NB_NODES;
				}
			d[v1]++;
			d[v2]++;
		}
  

	d = realloc (d, (g->n + 1) * sizeof (uint64_t));
	g->cd = malloc ((g->n + 2) * sizeof (uint64_t));
	g->cd[0] = 0;
	for (i = 1; i < g->n + 2; i++)
		{
			g->cd[i] = g->cd[i - 1] + d[i - 1];
			d[i - 1] = 0;
		}
	d[g->n] = 0;

	rewind (file);

	g->adj = malloc ((g->e * 2) * sizeof (uint64_t));
	while (fscanf (file, "%lu %lu", &v1, &v2) == 2)
		{
			g->adj[g->cd[v1] + d[v1]++] = v2;
			g->adj[g->cd[v2] + d[v2]++] = v1;
		}
	free (d);
	fclose (file);
	return g;
}

/**
 * Loads a directed graph into an Adjlist
 * from a  file containing a list of edges 
 * (pairs of positive number in a line separated by a space)
 */
AdjList *load_AdjList(char *path)
{
  uint64_t e1 = NB_NODES,  i, v1, v2, *d;
	FILE *file;
	AdjList *g = make_AdjList ();
    file = fopen (path, "r");
    assert(file!=NULL);
	

	d = calloc (e1, sizeof (uint64_t));
	while (fscanf (file, "%lu %lu", &v1, &v2) == 2)
		{
			g->n = max3 (g->n, v1, v2);
			g->e++;
			while (g->n >= e1)
				{
					d = realloc (d, (e1 + NB_NODES) * sizeof (uint64_t));
					assert (d != NULL);
					for (i = e1; i < e1 + NB_NODES; i++)
						d[i] = 0;
					e1 += NB_NODES;
				}
			d[v1]++;
		}
	d = realloc (d, (g->n + 1) * sizeof (uint64_t));

	g->cd = malloc ((g->n + 2) * sizeof (uint64_t));
	g->cd[0] = 0;
	for (i = 1; i < g->n + 2; i++)
		{
			g->cd[i] = g->cd[i - 1] + d[i - 1];
			d[i - 1] = 0;
		}
	d[g->n] = 0;

	rewind (file);

	g->adj = malloc ((g->e * 2) * sizeof (uint64_t));
	while (fscanf (file, "%lu %lu", &v1, &v2) == 2)
		g->adj[g->cd[v1] + d[v1]++] = v2;
	free (d);
	fclose (file);
	return g;
}


/**
 * prints an AdjList
 */
void print_AdjList(AdjList *al) 
{
	for (uint64_t i = 0; i < al->n+1; i++)
		{
			printf ("%lu |", i);
			for (uint64_t j = al->cd[i]; j < al->cd[i + 1]; j++)
				printf ("%lu |", al->adj[j]);
			printf ("\n");
		}    
}
