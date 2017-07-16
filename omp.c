#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#define MIN(a,b) \
	({ __typeof__(a) _a = (a); \
	__typeof__(b) _b = (b); \
	_a < _b ? _a : _b; })
#define SWAP(x, y) ({__typeof__(x) SWAP = x; x = y; y = SWAP;})

struct edge {	
	struct vertex* to;
	unsigned toi;
	long weight;
	struct edge* next;
};

struct vertex {
	long d;
	long h;
	unsigned priority;
	struct edge* E;
};

struct graph {
 	struct vertex** V;   
	unsigned* S; 
	unsigned order;
	unsigned srccnt;
	unsigned size;
};

struct vqueue {
	struct vertex** bheap;
	unsigned heapsize;
};

const long INF = LONG_MAX;

long ladd(long a, long b)
{ 
	return a == INF || b == INF ? INF : a + b;
}

long ladd3(long a, long b, long c)
{
	return ladd(a, ladd(b, c));
}

long ladd4(long a, long b, long c, long d)
{
	return ladd(a, ladd3(b, c, d));
}

struct vertex** get_vertex_array(unsigned order)
{
	struct vertex** V = malloc(sizeof(*V) * order + 1);
	V[order] = NULL;
	for (unsigned i = 0; i < order; ++i)
		V[i] = malloc(sizeof(*V[i]));
	return V;
}

unsigned* get_source_array(unsigned srccnt)
{
	unsigned* S = calloc(srccnt, sizeof(*S)); 
	for (unsigned s = 0; s < srccnt; ++s) {
		scanf("%u ", S + s);
		S[s]--; /* remove if array is 1-indexed */
	}
	return S;
}


void add_edge(struct vertex* V[], unsigned from, unsigned to, long weight)
{
	struct edge* e = malloc(sizeof(*e));
	e->weight = weight;
	e->to = V[to];
	e->toi = to;
	e->next = V[from]->E;
	V[from]->E = e;
}

void build_edges(struct vertex* V[], int straight)
{
	unsigned from, to;
	long weight;
	while (scanf("%u%*c%u%*c%ld\n", &from, &to, &weight) == 3) {
		--to; --from; /* from 1-index to 0-index */
		if (!straight)
			SWAP(from, to);
		add_edge(V, from, to, weight);
	}
}

struct graph* get_graph(int straight)
{
	struct graph* G = malloc(sizeof(*G));
	rewind(stdin);
	scanf("%u %u %u\n", &G->order, &G->srccnt, &G->size);
	G->V = get_vertex_array(G->order);
	G->S = get_source_array(G->srccnt);
	build_edges(G->V, straight);
	return G;
}

void swap(struct vqueue* Q, size_t i1, size_t i2)
{
	SWAP(Q->bheap[i1], Q->bheap[i2]);
	Q->bheap[i1]->priority = i1;
	Q->bheap[i2]->priority = i2;
}

struct vqueue* get_queue(struct vertex* const V[], unsigned order, unsigned source)
{
	static struct vqueue* Q = NULL;
	if (!Q) {
		Q = malloc(sizeof *Q);
		Q->bheap = calloc(order, sizeof(*Q->bheap));
	}
	for (unsigned i = 0; i < order; ++i) {
		Q->bheap[i] = V[i];
		Q->bheap[i]->priority = i;
	}
	Q->heapsize = order;
	swap(Q, 0, source);
	return Q;
}

/* QUEUE METHODS */

void min_heapify(struct vqueue* Q, unsigned parent)
{
        unsigned left   = 2 * parent + 1;
        unsigned right  = left + 1;
        unsigned lowest = parent;	

        if (left < Q->heapsize && Q->bheap[left]->d < Q->bheap[lowest]->d) {
                lowest = left;
	}
        if (right < Q->heapsize && Q->bheap[right]->d < Q->bheap[lowest]->d) {
                lowest = right;
	}
        if (parent != lowest) {
		swap(Q, parent, lowest);
                min_heapify(Q, lowest);
        }
}

void sift_up(struct vqueue* Q, unsigned child)
{
	if (child && child < Q->heapsize) {
		unsigned parent = (child - 1) / 2;
		if (Q->bheap[parent]->d > Q->bheap[child]->d) {
			swap(Q, child, parent);
			sift_up(Q, parent);
		}
	}
}

struct vertex* extract_min(struct vqueue* Q)
{
	if (Q->heapsize == 0)
		return NULL;
	struct vertex* min = Q->bheap[0];
	swap(Q, 0, --Q->heapsize); 
	Q->bheap[Q->heapsize] = NULL;
        min_heapify(Q, 0);
        return min;
}

/* O(V) */
void initialize_single_source(struct graph* G, unsigned source)
{
	for (unsigned i = 0; i < G->order; ++i)
		G->V[i]->d = INF;
	G->V[source]->d = 0;
}

void reweight_edges(struct graph* G)
{
	for (unsigned i = 0; i < G->order; ++i) {
		struct vertex* v = G->V[i]; 
                for (struct edge* e = v->E; e; e = e->next)
                        e->weight += v->h - e->to->h;
 	}
}

/* O(VlgV + E) */
void dijkstra(struct graph* G, unsigned source)
{
	struct vertex* min;
	initialize_single_source(G, source);
	struct vqueue* Q = get_queue(G->V, G->order, source);
	/* TODO */ 
        while ((min = extract_min(Q))) {
                for (struct edge* e = min->E; e; e = e->next) {
                        e->to->d = MIN(e->to->d, ladd(min->d, e->weight));
                        sift_up(Q, e->to->priority);
		}
	}
}
	
/* O(VE) */
void dummy_bellman_ford(struct graph* G)
{
        for (unsigned v = 0; v < G->order; ++v)
		G->V[v]->h = 0;
        for (unsigned c = 0; c < G->order; ++c) {
        	for (unsigned i = 0; i < G->order; ++i) {
			struct vertex* v = G->V[i]; 
                        for (struct edge* e = v->E; e; e = e->next)
                       		e->to->h = MIN(e->to->h, v->h + e->weight);
		}
	}
}

unsigned find_omp(struct graph* G)
{
	dummy_bellman_ford(G);
	reweight_edges(G);
	long cost[G->order];
	memset(cost, 0, sizeof(cost));
        for (unsigned i = 0; i < G->srccnt; ++i) {
		unsigned s = G->S[i];
		dijkstra(G, s); 
                for (unsigned v = 0; v < G->order; ++v) 
                       cost[v] = ladd4(cost[v], G->V[v]->d, G->V[v]->h, -G->V[s]->h);
	}
	unsigned omp = 0;
        for (unsigned v = 1; v < G->order; ++v)
		if (cost[v] < cost[omp])
			omp = v;
        return omp;
}

/*\
 * Gt:           transposed graph
 * omp:          index for the OMP vertex
 * return:       sum of the costs of all source vertices to the OMP
 * side-effects: For each source with index 's', Gt->V[s]->d is written with
 *               the true distance from that vertex to the OMP.
\*/
long find_costs_to_omp(struct graph* Gt, unsigned ompi)
{
	dummy_bellman_ford(Gt);
	reweight_edges(Gt);
	dijkstra(Gt, ompi);
	long cost = 0;
	struct vertex* omp = Gt->V[ompi];
	for (unsigned i = 0; i < Gt->srccnt; ++i) {
		struct vertex* s = Gt->V[Gt->S[i]];
		s->d = ladd3(s->d, -omp->h, s->h); /* unreweighting the path */
		cost = ladd(cost, s->d);
	}
	return cost;
}

int main(void)
{
	struct graph* G  = get_graph(1); /* graph read from stdin */
	unsigned omp = find_omp(G);
	struct graph* Gt = get_graph(0);
	long total_cost = find_costs_to_omp(Gt, omp);
	if (total_cost == INF) {
                puts("N");
		return EXIT_FAILURE;
	} else {
		printf("%u %ld\n", omp + 1, total_cost); /* omp + 0 for 1-indexed vertices */
        	for (unsigned i = 0; i < Gt->srccnt; ++i) 
			printf("%ld ", Gt->V[Gt->S[i]]->d);
		putchar('\n');
		return EXIT_SUCCESS;
	}
}
