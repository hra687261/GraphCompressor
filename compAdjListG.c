#include "compAdjListG.h"


/**
 * Frees the allocated memory for a CompAdjListG
 */
void free_CompAdjListG(CompAdjListG *a_l) 
{
    free(a_l->adj);
    free(a_l->cd);
    free(a_l);
}

/**
 * Reads a file containing an edgelist
 * And loads it into a CompAdjListG 
 */
CompAdjListG *load_CompAdjListG(char *path, uint8_t ign, 
    uint8_t *encoding_function(uint8_t *arr, uint64_t *u, uint64_t val), 
    uint64_t expl_function(uint64_t val), uint64_t *u)
{
   uint64_t 
        v1, v2, i, tmp, v, l=100,
        *d, // d[i] : the total size in number of bits of the coded values of i's neighbours
        *prec; // prec[i] : the last neighbour of node i that was added to the adjlist
        // since the adjlist needs to be sorted so for us to apply the gaps data compression technique
        // we suppose that the file was pre processed
        // if it wasn't then an assertion error will be raised during the execution
    CompAdjListG *alg=malloc(sizeof(CompAdjListG));
    alg->n=0;
    alg->e=0;

    // "line" will be used to ignore the (ign) first lines of the file containing the edgelist
    char *line = malloc (sizeof (char) * l);
    FILE *file=fopen(path,"r");
    for (i = 0; i < ign; i++)
		assert (fgets (line, l, file));

    while (fscanf(file,"%lu %lu", &v1, &v2)==2) {
		alg->n=max3(alg->n,v1,v2);
        alg->e++;
	}

    // cd[i+1]-cd[i]= size of i's neighbours in number of bits
    alg->cd=calloc((alg->n+2),sizeof(uint64_t));
    d=calloc((alg->n+1),sizeof(uint64_t));
    prec=calloc((alg->n+1),sizeof(uint64_t));
    
    uint64_t sum_alloc;
    sum_alloc=(alg->n*3+4)*8;
    assert(
        alg->cd != NULL && 
        d != NULL && 
        prec != NULL);
    
    rewind(file);
    for (i = 0; i < ign; i++)
		assert (fgets (line, l, file));

    while (fscanf(file,"%lu %lu", &v1, &v2)==2) {
        // counting the size of every node's list of neighbours
        // by summing the expected length of it's neighbours
    
        if(prec[v1]==0){
            v=func(v1,v2);
        }else{
            // this new neighbour of v1 needs to be bigger than it's previous one 
            // (the file needs to be sorted by the 1st and 2nd column) 
                assert(v2>prec[v1]);
            v=v2-prec[v1]-1;
        }
        prec[v1]=v2;
        //adding the expected length of v to the size of the neighbours of v1
        d[v1]+=expl_function(v);
       
    }
    alg->cd[0]=0;
	for (i=1;i<alg->n+2;i++) {
		alg->cd[i]=alg->cd[i-1]+d[i-1];
		d[i-1]=0;
        prec[i-1]=0;
    }

    alg->nbb=(alg->cd[alg->n+1] - 1)/8+1;
    alg->adj=calloc(alg->nbb,sizeof(uint8_t));
    assert(alg->adj != NULL);
    sum_alloc+=alg->cd[alg->n+1]/8+1;

    rewind(file);
    for (i = 0; i < ign; i++)
		assert (fgets (line, l, file));
    *u=0;
    while (fscanf(file,"%lu %lu", &v1, &v2)==2) {
        // counting the value of the neighbour of each node using the gaps technique
        // and encoding that value into the neighbours array 

        *u=alg->cd[v1]+d[v1];
        tmp=*u;
        if(prec[v1]==0){
            v=func(v1,v2);
        }else{
            v=v2-prec[v1]-1;
        }
        prec[v1]=v2;
        encoding_function(alg->adj,u,v);
        d[v1]+=*u-tmp;

    }

    fclose(file);
    free(line);
    free(prec);
    free(d);
    return alg;
}





/**
 * Prints a CompAdjListG
 */
void print_CompAdjListG(CompAdjListG *alg, 
    uint64_t decoding_function(uint8_t *arr,uint64_t *s))
{
    uint64_t i,v,s=0, *prec=calloc(alg->n+1,sizeof(uint64_t));
    for(i=0;i<alg->n+1;i++){
        printf("%lu  |",i);
        while(s<alg->cd[i+1]){
            if(prec[i]==0){
                v=inv_func(i,decoding_function(alg->adj,&s));
                printf(" %lu |",v);
                prec[i]=v;
            }else{
                v=decoding_function(alg->adj,&s)+prec[i]+1;    
                printf(" %lu |",v);
                prec[i]=v;
            }
        }
        printf("\n");
    }
}

void write_CompAdjListG(CompAdjListG *alg, uint8_t id, char *path)
{
    FILE *file=fopen(path,"wb");
    uint64_t i;

    fwrite(&id,sizeof(char),1,file);
    if(id==4){
        uint8_t k=getK();
        fwrite(&k,sizeof(char),1,file);
    }

    fwrite(&alg->n,sizeof(uint64_t),1,file);
    fwrite(&alg->e,sizeof(uint64_t),1,file);
    fwrite(&alg->nbb,sizeof(uint64_t),1,file);

    for(i=0;i<alg->n+2;i++)
        fwrite(&alg->cd[i],sizeof(uint64_t),1,file);
    
    for(i=0;i<alg->nbb;i++)
        fwrite(&alg->adj[i],sizeof(uint8_t),1,file);

    fclose(file);
}

CompAdjListG *read_CompAdjListG(char *path, uint8_t *id)
{
    FILE *file=fopen(path,"rb");
    uint64_t i, k;
    CompAdjListG *alg=malloc(sizeof(CompAdjListG));
    
    assert( fread(id, sizeof(uint8_t), 1, file) == 1);
    if(*id==4){
        assert( fread(&k, sizeof(uint8_t), 1, file) == 1);
        setK(k);
    }
    
    assert( fread(&alg->n, sizeof(uint64_t), 1, file) == 1);
    assert( fread(&alg->e, sizeof(uint64_t), 1, file) == 1);
    assert( fread(&alg->nbb, sizeof(uint64_t), 1, file) == 1);

    alg->cd=malloc((alg->n+2)*sizeof(uint64_t));
    alg->adj=malloc(alg->nbb*sizeof(uint8_t));
    
    for(i=0;i<alg->n+2;i++)
        assert( fread(&alg->cd[i], sizeof(uint64_t), 1, file) == 1);
    
    for(i=0;i<alg->nbb;i++)
        assert( fread(&alg->adj[i], sizeof(uint8_t), 1, file) == 1);

    fclose(file);
    return alg;
}



uint64_t *bfs_CompAdjListG(CompAdjListG *alg,uint64_t curr,
    uint8_t *seen,uint64_t *nbvals,
    uint64_t decoding_function(uint8_t *arr,uint64_t *s))
{
    uint64_t i, v1, v2, u,
    s_file=0, e_file=0,
    *file=malloc((alg->n+1)*sizeof(uint64_t));
    uint8_t sv;
    *nbvals=0;

    if(seen==NULL){
        seen=calloc(alg->n+1,sizeof(uint8_t));
    } else
        if(seen[curr]==1)
            return NULL;
    
    file[e_file++]=curr;
    seen[curr]=1;

    for(i=s_file;i<e_file;i++)
    {   
        v1=file[i];
        seen[v1]=1;
        u=alg->cd[v1];
        v2=0;
        sv=1;
        while(u<alg->cd[v1+1]){
            if(sv){
                v2=inv_func(v1,decoding_function(alg->adj,&u));
                sv=0;
            }else
                v2=v2+1+decoding_function(alg->adj,&u);
            
            if(!seen[v2] && v2!=v1){
                file[e_file++]=v2;
                seen[v2]=1;
            }
        }
    }
    free(seen);
    file=realloc(file,e_file*sizeof(uint64_t));
    *nbvals=e_file;
    return file;
}




