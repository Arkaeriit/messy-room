#include "mr_heaplet.h"
#include "stdlib.h"
#include "time.h"

#define GARBAGE_SIZE 100
#define LOOP_COUNT   10000

int main(void) {
	srand(time(NULL));
	// Testing starting from the same heaplet
	mr_heaplet_t* heaplet = mr_new();
	for (int i=0; i<LOOP_COUNT; i++) {
		char garbage[GARBAGE_SIZE];
		mr_add_data(heaplet, GARBAGE_SIZE, garbage);
	}
	// Testing changing the start heaplet
	for (int i=0; i<LOOP_COUNT; i++) {
		char garbage[GARBAGE_SIZE];
		heaplet = mr_add_data(heaplet, GARBAGE_SIZE, garbage);
	}
	mr_free(heaplet);
	return 0;
}

