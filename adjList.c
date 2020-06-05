#include "adjList.h"

AdjList *make_AdjList() 
{
  AdjList *al = malloc (sizeof (AdjList));
	al->n = 0;
	al->e = 0;
	al->cd = NULL;
	al->adj = NULL;
	return al;
}

void free_AdjList(AdjList *al) 
{
  free (al->cd);
	free (al->adj);
	free (al);
}

AdjList *load_AdjListU(char *path, uint8_t ign) 
{
  uint64_t e1 = NB_NODES, l = 100, i, v1, v2, *d;
	FILE *file;
	char *line = malloc (sizeof (char) * l);
	AdjList *g = make_AdjList ();
    file = fopen (path, "r");
    assert(file!=NULL);
	
  for (i = 0; i < ign; i++)
		assert (fgets (line, l, file) != NULL);

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
	for (i = 0; i < ign; i++)
		assert (fgets (line, l, file) != NULL);

	g->adj = malloc ((g->e * 2) * sizeof (uint64_t));
	while (fscanf (file, "%lu %lu", &v1, &v2) == 2)
		{
			g->adj[g->cd[v1] + d[v1]++] = v2;
			g->adj[g->cd[v2] + d[v2]++] = v1;
		}
	free (d);
	free (line);
	fclose (file);
	return g;
}


AdjList *load_AdjList(char *path, uint8_t ign)
{
  uint64_t e1 = NB_NODES, l = 100, i, v1, v2, *d;
	FILE *file;
	char *line = malloc (sizeof (char) * l);
	AdjList *g = make_AdjList ();
    file = fopen (path, "r");
    assert(file!=NULL);
	
  for (i = 0; i < ign; i++)
		assert (fgets (line, l, file) != NULL);

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
	for (i = 0; i < ign; i++)
		assert (fgets (line, l, file) != NULL);

	g->adj = malloc ((g->e * 2) * sizeof (uint64_t));
	while (fscanf (file, "%lu %lu", &v1, &v2) == 2)
		g->adj[g->cd[v1] + d[v1]++] = v2;
	free (d);
	free (line);
	fclose (file);
	return g;
}

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
