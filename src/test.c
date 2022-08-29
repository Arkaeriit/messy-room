#include "mr_heaplet.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "time.h"

#define GARBAGE_SIZE 100
#define LOOP_COUNT   10000

const uint64_t special_data = 0xDECAFBAD;

static int search_special_data(uint64_t size, char* data, void* arg) {
	int* number_of_tries = arg;
	*number_of_tries = *number_of_tries + 1;
	if (size != sizeof(uint64_t)) {
		return 0;
	}
	uint64_t number = *((uint64_t*) data);
	return number == special_data;
}

static void basic_test(void) {
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
}

static void serialize_test(void) {
	mr_heaplet_t* heaplet = mr_new();
	const char* s1 = "Bobignou";
	mr_add_data(heaplet, strlen(s1), s1);
	const char* s2 = "Yay!";
	mr_add_data(heaplet, strlen(s2), s2);
	const char* s3 = "^.^";
	mr_add_data(heaplet, strlen(s3), s3);

	FILE* f = fopen("test1.mr", "w");
	mr_write_to_file(heaplet, f);
	fclose(f);

	f = fopen("test1.mr", "r");
	mr_heaplet_t* read_heaplet = mr_read_from_file(f);
	fclose(f);

	f = fopen("test2.mr", "w");
	mr_write_to_file(read_heaplet, f);
	fclose(f);

	mr_free(heaplet);
	mr_free(read_heaplet);
}

int main(void) {
	srand(time(NULL));
	basic_test();
	serialize_test();
	return 0;
}

