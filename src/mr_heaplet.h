#ifndef MR_HEAPLET_H
#define MR_HEAPLET_H

#include "stdlib.h"

typedef struct mr_heaplet_s {
	size_t size;
	char* data;
	size_t number_of_neighbours;
	struct mr_heaplet_s** neighbours;
} mr_heaplet_t;

mr_heaplet_t* mr_new(void);
void mr_free(mr_heaplet_t* heaplet);

#endif

