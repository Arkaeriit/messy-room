#ifndef MR_HEAPLET_H
#define MR_HEAPLET_H

#include "stdlib.h"
#include "stdint.h"
#include "stdio.h"

typedef struct mr_heaplet_s {
	size_t size;
	char* data;
	size_t number_of_neighbours;
	struct mr_heaplet_s** neighbours;
} mr_heaplet_t;

typedef int (*mr_crawler_function)(uint64_t size, char* data, void* extra_args);

mr_heaplet_t* mr_new(void);
void mr_free(mr_heaplet_t* heaplet);
mr_heaplet_t* mr_add_data(mr_heaplet_t* heaplet, size_t size, const void* data);
int mr_crawl(mr_heaplet_t* heaplet, mr_crawler_function f, void* extra_args);

size_t mr_write_to_array(mr_heaplet_t* heaplet, char* dest);
size_t mr_write_to_file(mr_heaplet_t* heaplet, FILE* f);
mr_heaplet_t* mr_read_from_array(char* data, size_t size);
mr_heaplet_t* mr_read_from_file(FILE* f);

#endif

