#include "mr_heaplet.h"
#include "stdlib.h"
#include "stdio.h"
#include "time.h"

#define GARBAGE_SIZE 100
#define LOOP_COUNT   10000

const uint64_t special_data = 0xDECAFBAD;

int search_special_data(uint64_t size, char* data, void* arg) {
	int* number_of_tries = arg;
	*number_of_tries = *number_of_tries + 1;
	if (size != sizeof(uint64_t)) {
		return 0;
	}
	uint64_t number = *((uint64_t*) data);
	return number == special_data;
}

int main(void) {
	srand(time(NULL));
	// Testing starting from the same heaplet
	mr_heaplet_t* heaplet = mr_new();
	for (int i=0; i<LOOP_COUNT; i++) {
		char* garbage = malloc(GARBAGE_SIZE);
		mr_add_data(heaplet, GARBAGE_SIZE, garbage);
		free(garbage);
	}
	// Adding a specific element
	mr_add_data(heaplet, sizeof(uint64_t), &special_data);
	// Testing changing the start heaplet
	for (int i=0; i<LOOP_COUNT; i++) {
		char* garbage = malloc(GARBAGE_SIZE);
		heaplet = mr_add_data(heaplet, GARBAGE_SIZE, garbage);
		free(garbage);
	}
	// Find the special data back
	int number_of_tries = 0;
	int rc = mr_crawl(heaplet, search_special_data, &number_of_tries);
	printf("%s in %i tries\n", rc ? "Found special data" : "Special data not found" , number_of_tries);

	mr_free(heaplet);
	return 0;
}

