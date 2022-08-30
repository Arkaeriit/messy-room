#include "kv-over-messy-room.h"
#include <stdint.h>
#include <string.h>

typedef struct {
	char key[K_SIZE + 1];
	char value[K_SIZE + 1];
} kv_t;

/*
 * Search for the kv element with the corresponding key.
 * Assumes that the key is not too big.
 * Returns it if found and NULL if not.
 */
static kv_t* get_from_key(mr_heaplet_t* heaplet, const char* k) {
	struct crawler_arg_s {
		const char* key;
		kv_t* element;
	};

	int key_searcher(uint64_t size, char* data, void* extra_args) {
		if (size != sizeof(kv_t)) {
			return 0;
		}
		struct crawler_arg_s* arg = extra_args;
		kv_t* element = (kv_t*) data;
		if (strcmp(element->key, arg->key)) {
			return 0;
		}
		arg->element = element;
		return 1;
	}

	struct crawler_arg_s arg = {.key = k};
	int rc = mr_crawl(heaplet, key_searcher, &arg);
	if (rc) {
		return arg.element;
	}
	return NULL;
}

/*
 * Count the number of elements in the db.
 */
static size_t elems_in_db(mr_heaplet_t* heaplet) {
	int elem_counter(uint64_t size, char* data, void* extra_args) {
		if (size == sizeof(kv_t)) {
			kv_t* element = (kv_t*) data;
			if (strcmp(element->key, "")) {
				size_t* arg = extra_args;
				*arg = *arg + 1;
			}
		}
		return 0;
	}

	size_t ret = 0;
	mr_crawl(heaplet, elem_counter, &ret);
	return ret;
}

/*
 * Write or overwrite an element to the kv store.
 * Assumes that both the key and the value are of the
 * right size.
 */
void kvomr_write(mr_heaplet_t* heaplet, const char* k, const char* v) {
	kv_t* element = get_from_key(heaplet, k);
	if (element == NULL) {
		kv_t new_element;
		memset(new_element.key, 0, K_SIZE+1);
		memset(new_element.value, 0, V_SIZE+1);
		strcpy(new_element.key, k);
		strcpy(new_element.value, v);
		mr_add_data(heaplet, sizeof(kv_t), &new_element);
		return;
	}
	memset(element->value, 0, V_SIZE+1);
	strcpy(element->value, v);
}

/*
 * Find a value from a key. Return NULL if not found.
 */
char* kvomr_read(mr_heaplet_t* heaplet, const char* k) {
	kv_t* element = get_from_key(heaplet, k);
	if (element == NULL) {
		return NULL;
	}
	return element->value;
}

/*
 * Delete the key from an element from the database. Return true if the element
 * is found and false if it is not.
 */
bool kvomr_delete(mr_heaplet_t* heaplet, const char* k) {
	kv_t* element = get_from_key(heaplet, k);
	if (element == NULL) {
		return false;
	}
	strcpy(element->key, "");
	return true;
}

/*
 * Return a NULL terminated list of all the element in the db.
 */
char** kvomr_list(mr_heaplet_t* heaplet) {
	int elem_lister(uint64_t size, char* data, void* extra_args) {
		if (size == sizeof(kv_t)) {
			kv_t* element = (kv_t*) data;
			if (strcmp(element->key, "")) {
				char** arg = extra_args;
				size_t index = 0;
				while(arg[index] != NULL) { // TODO: optimize
					index++;
				}
				arg[index] = element->key;
			}
		}
		return 0;
	}

	size_t n_elems = elems_in_db(heaplet);
	char** ret = malloc(sizeof(char*) * (n_elems + 1));
	for (size_t i=0; i<=n_elems; i++) {
		ret[i] = NULL;
	}
	mr_crawl(heaplet, elem_lister, ret);
	return ret;
}
