#include "kv-over-messy-room.h"

int main(int argc, char** argv) {
	if (argc != 4) {
		return -1;
	}

	FILE* f = fopen(argv[1], "r");
	mr_heaplet_t* heaplet;
	if (f == NULL) {
		heaplet = mr_new();
	} else {
		heaplet = mr_read_from_file(f);
		fclose(f);
	}

	kvomr_write(heaplet, argv[2], argv[3]);

	f = fopen(argv[1], "w");
	mr_write_to_file(heaplet, f);
	mr_free(heaplet);
	fclose(f);

	return 0;
}

