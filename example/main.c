#include "kv-over-messy-room.h"

int main(int argc, char** argv) {
	if (argc != 4 && argc != 3) {
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

	if (argc == 4) {
		kvomr_write(heaplet, argv[2], argv[3]);
		f = fopen(argv[1], "w");
		mr_write_to_file(heaplet, f);
		fclose(f);
	} else {
		printf("%s\n", kvomr_read(heaplet, argv[2]) == NULL ? "(null)" : kvomr_read(heaplet, argv[2]));
	}

	mr_free(heaplet);

	return 0;
}

