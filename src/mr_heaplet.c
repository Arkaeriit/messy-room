#include "mr_heaplet.h"
#include "stdint.h"

/*
 * As the data in a heaplet is made of a t-v data, we can crawl through it to
 * find the next empty chunk.
 */
static const char* next_intem_in_heaplet(const char* data) {
	uint64_t item_size = *((const uint64_t*) data);
	return data + sizeof(uint64_t) + item_size;
}

/*
 * Returns the empty space in a heaplet's buffer.
 */
static size_t empty_space(const mr_heaplet_t* heaplet) {
	const char* next_free_space = heaplet->data;
	while (next_intem_in_heaplet(next_free_space) != next_free_space) {
		next_free_space = next_intem_in_heaplet(next_free_space);
		if ((size_t) (next_free_space - heaplet->data) >= heaplet->size) {
			return 0;
		}
	}
	return heaplet->size - (next_free_space - heaplet->data);
}

/*
 * Create a new heaplet, if the neighbor is set to NULL, the list of neighbour
 * will be left empty.
 */
static mr_heaplet_t* new_heaplet(size_t size, mr_heaplet_t* neighbour) {
	mr_heaplet_t* ret = malloc(sizeof(mr_heaplet_t));
	ret->size = size;
	ret->data = calloc(size, 1);
	if (neighbour == NULL) {
		ret->number_of_neighbours = 0;
		ret->neighbours = NULL;
	} else {
		ret->number_of_neighbours = 1;
		ret->neighbours = malloc(sizeof(neighbour));
		ret->neighbours[0] = neighbour;
	}
	return ret;
}

/*
 * Create a new empty heaplet with no neighbour.
 */
mr_heaplet_t* mr_new(void) {
	return new_heaplet(0, NULL);
}

/*
 * Free a heaplet and all its neighbours, recursively.
 */
void mr_free(mr_heaplet_t* heaplet) {
	for (size_t i=1; i<heaplet->number_of_neighbours; i++) { // Start at 1 in order not to stay stuck between the two same neighbours.
		mr_free(heaplet->neighbours[i]);
	}
	free(heaplet->data);
	free(heaplet->neighbours);
	free(heaplet);
}

