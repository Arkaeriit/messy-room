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

static void help(const char* prg_name) {
	printf("kvomr: A CLI key-value database using messy-room as the back end.\n");
	printf("\n");
	printf("Usage:\n");
	printf("  %s --help       Show this help\n", prg_name);
	printf("  %s --list       List all keys stored\n", prg_name);
	printf("  %s <k> <v>      Store the message <v> at the key <k>\n", prg_name);
	printf("  %s <k>          Show the message at the key <k>\n", prg_name);
	printf("  %s --del <k>    Delete the value at the key <k>\n", prg_name);
	printf("\n");
}

#define CHECK_KEY(k) \
	if (strlen(k) > K_SIZE) { \
		fprintf(stderr, "Error: key in larger than %u bytes.\n", K_SIZE); \
		return 4; \
	} \
	if (strlen(k) == 0) { \
		fprintf(stderr, "Error: empty key.\n"); \
		return 8; \
	}

#define CHECK_VALUE(v) \
	if (strlen(v) > V_SIZE) { \
		fprintf(stderr, "Error: value in larger than %u bytes.\n", V_SIZE); \
		return 4; \
	}

int main(int argc, char** argv) {
	if (argc < 2) { // Getting help
		help(argv[0]);
		return 0;
	}

	if (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-help") || !strcmp(argv[1], "-h")) { // Getting help
		help(argv[0]);
		return 0;
	}

	if (!strcmp(argv[1], "--list")) { // Listing all saved keys
		if (argc != 2) {
			goto invalid_arg;
		}
		mr_heaplet_t* heaplet = read_db();
		char** key_list = kvomr_list(heaplet);
		size_t i = 0;
		while(key_list[i] != NULL) {
			printf("%s\n", key_list[i]);
			i++;
		}
		free(key_list);
		mr_free(heaplet);
		return 0;
	}

	if (!strcmp(argv[1], "--del")) { // Remove an element
		if (argc != 3) {
			goto invalid_arg;
		}
		CHECK_KEY(argv[2]);
		mr_heaplet_t* heaplet = read_db();
		if (kvomr_delete(heaplet, argv[2])) {
			printf("Successfully deleted element at key %s\n", argv[2]);
		} else {
			printf("No value indexed with the key \"%s\".\n", argv[1]);
		}
		save_db(heaplet);
		mr_free(heaplet);
		return 0;
	}

	if (argc == 2) { // Reading an entry
		CHECK_KEY(argv[1]);
		mr_heaplet_t* heaplet = read_db();
		char* ret = kvomr_read(heaplet, argv[1]);
		if (ret == NULL) {
			printf("No value indexed with the key \"%s\".\n", argv[1]);
		} else {
			printf("%s\n", ret);
		}
		mr_free(heaplet);
		return 0;
	}

	if (argc == 3) { // Writing an entry
		CHECK_KEY(argv[1]);
		CHECK_VALUE(argv[2]);
		mr_heaplet_t* heaplet = read_db();
		char* already_there = kvomr_read(heaplet, argv[1]);
		kvomr_write(heaplet, argv[1], argv[2]);
		if (already_there == NULL) {
			printf("Added value to key \"%s\".\n", argv[1]);
		} else {
			printf("Overwrote value to key \"%s\".\n", argv[1]);
		}
		save_db(heaplet);
		mr_free(heaplet);
		return 0;
	}

invalid_arg:
	fprintf(stderr, "Error: invalid arguments.\n");
	fprintf(stderr, "Use `%s --help` for more information.\n", argv[0]);
	return 1;
}

