#include "mr_heaplet.h"
#include "stdint.h"
#include "string.h"
#include "stdio.h"

/*
 * Prints info about an heaplet.
 */
static void __attribute__ ((unused)) print_heaplet(const mr_heaplet_t* heaplet) {
	printf("Heaplet: %p, size = %zi, number_of_neighbours = %zi\n", heaplet, heaplet->size, heaplet->number_of_neighbours);
	printf("Neighbours = [");
	for (size_t i=0; i<heaplet->number_of_neighbours; i++) {
		printf("%p, ", heaplet->neighbours[i]);
	}
	printf("]\n");
}

/*
 * As the data in a heaplet is made of a t-v data, we can crawl through it to
 * find the next empty chunk.
 */
static char* next_intem_in_heaplet(char* data) {
	uint64_t item_size = *((uint64_t*) data);
	return data + sizeof(uint64_t) + item_size;
}

/*
 * Returns the next free space in an heaplet buffer, return NULL if there is
 * no more free place.
 */
static char* goto_empty_space(mr_heaplet_t* heaplet) {
	if (heaplet->size == 0) {
		return NULL;
	}
	char* next_free_space = heaplet->data;
	while (*((uint64_t*) next_free_space) != 0) {
		next_free_space = next_intem_in_heaplet(next_free_space);
		if ((size_t) (next_free_space - heaplet->data) >= heaplet->size) {
			return NULL;
		}
	}
	return next_free_space;
}

/*
 * Returns the empty space in a heaplet's buffer.
 */
static size_t empty_space(mr_heaplet_t* heaplet) {
	const char* next_free_space = goto_empty_space(heaplet);
	if (next_free_space == NULL) {
		return 0;
	}
	return heaplet->size - (next_free_space - heaplet->data);
}

/*
 * Assuming there is enough place in it, adds data into an heaplet's buffer
 */
static void add_data(mr_heaplet_t* heaplet, size_t size, const void* data) {
	char* target = goto_empty_space(heaplet);
	*((uint64_t*) target) = size;
	target += sizeof(uint64_t);
	memmove(target, data, size);
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
 * Randomly select a next heaplet to put data in. If a new heaplet must be
 * created, NULL is returned.
 * There is 1/3 chances of a new heaplet being made. If a neighbour is choosen,
 * all of the neighbours have equal chances.
 */
static mr_heaplet_t* choose_next_heaplet(const mr_heaplet_t* heaplet) {
	if (heaplet->number_of_neighbours == 0) {
		return NULL;
	}
	if (rand() % 3 == 0) {
		return NULL;
	}
	return heaplet->neighbours[
		rand() % heaplet->number_of_neighbours];
}

/*
 * Add a new heaplet to the list of neighbours of an other heaplet.
 */
static void new_neighbour(mr_heaplet_t* heaplet, mr_heaplet_t* neighbour) {
	mr_heaplet_t** new_buffer = malloc(sizeof(mr_heaplet_t) * (heaplet->number_of_neighbours + 1));
	for (size_t i=0; i<heaplet->number_of_neighbours; i++) {
		new_buffer[i] = heaplet->neighbours[i];
	}
	new_buffer[heaplet->number_of_neighbours] = neighbour;
	heaplet->number_of_neighbours++;
	free(heaplet->neighbours);
	heaplet->neighbours = new_buffer;
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

/*
 * Add data into a heaplet. If there is not enought space, a neighbour or a new
 * heaplet will be chosen. The heaplet choosen is returned.
 */
mr_heaplet_t* mr_add_data(mr_heaplet_t* heaplet, size_t size, const void* data) {
	if (size + sizeof(uint64_t) < empty_space(heaplet)) {
		add_data(heaplet, size, data);
		return heaplet;
	}
	mr_heaplet_t* next_heaplet = choose_next_heaplet(heaplet);
	if (next_heaplet == NULL) {
		next_heaplet = new_heaplet((size + sizeof(uint64_t)) * heaplet->number_of_neighbours, heaplet);
		new_neighbour(heaplet, next_heaplet);
	}
	return mr_add_data(next_heaplet, size, data);
}

