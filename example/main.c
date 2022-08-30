#include "kv-over-messy-room.h"
#include <stdlib.h>
#include <string.h>


/*
 * Get the filename of the messy-room file. It is either $KVOMR_PATH, or
 * $XDG_DATA_DIR/kvomr.mr, or $HOME/.kvomr.mr
 */
static char* get_db_path(void) {
	if (getenv("KVOMR_PATH") != NULL) {
		return strdup(getenv("KVOMR_PATH"));
	}
	if (getenv("XDG_DATA_HOME") != NULL) {
		char* ret = malloc(strlen(getenv("XDG_DATA_HOME")) + strlen("/kvomr.mr") + 1);
		strcpy(ret, getenv("XDG_DATA_HOME"));
		strcat(ret, "/kvomr.mr");
		return ret;
	}
	if (getenv("HOME") == NULL) {
		fprintf(stderr, "Error, specify at least one of the following environement variables: HOME, XDG_DATA_DIR, or KVOMR_PATH");
		exit(1);
	}
	char* ret = malloc(strlen(getenv("HOME")) + strlen("/.kvomr.mr") + 1);
	strcpy(ret, getenv("HOME"));
	strcat(ret, "/.kvomr.mr");
	return ret;
}

/*
 * Open in the desired mode the db file.
 */
static FILE* open_db(const char* mode) {
	char* path = get_db_path();
	FILE* ret = fopen(path, mode);
	if (ret == NULL && !strcmp(mode, "r")) { // Try to make an empty file
		FILE* f = fopen(path, "w");
		mr_heaplet_t* heaplet = mr_new();
		mr_write_to_file(heaplet, f);
		mr_free(heaplet);
		fclose(f);
		ret = fopen(path, mode);
	}
	if (ret == NULL) {
		fprintf(stderr, "Error, unable to open %s\n", path);
		exit(2);
	}
	free(path);
	return ret;
}

/*
 * Open the data base.
 */
static mr_heaplet_t* read_db(void) {
	FILE* f = open_db("r");
	mr_heaplet_t* ret = mr_read_from_file(f);
	fclose(f);
	return ret;
}

/*
 * Save the data base.
 */
static void save_db(mr_heaplet_t* heaplet) {
	FILE* f = open_db("w");
	mr_write_to_file(heaplet, f);
	fclose(f);
}

int main(int argc, char** argv) {
	if (argc == 2) { // Reading an entry
		mr_heaplet_t* heaplet = read_db();
		char* ret = kvomr_read(heaplet, argv[1]);
		if (ret == NULL) {
			printf("No value indexed with the key \"%s\".\n", argv[1]);
		} else {
			printf("%s", ret);
		}
		mr_free(heaplet);
		return 0;
	}

	if (argc == 3) { // Writing an entry
		mr_heaplet_t* heaplet = read_db();
		char* already_there = kvomr_read(heaplet, argv[1]);
		kvomr_write(heaplet, argv[1], argv[2]);
		if (already_there == NULL) {
			printf("Added value to key \"%s\"\n.", argv[1]);
		} else {
			printf("Overwrote value to key \"%s\"\n.", argv[1]);
		}
		save_db(heaplet);
		mr_free(heaplet);
		return 0;
	}

	printf("OnO\n");
	return 1;

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

