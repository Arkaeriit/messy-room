#include "messy-room.h"
#include "stdbool.h"
#include "string.h"

typedef bool (*mr_reader_function)(void* arg, char* c);
typedef void (*mr_writer_function)(void* arg, char c);

/*
 * Prints info about an heaplet.
 */
static void __attribute__ ((unused)) print_heaplet(const mr_heaplet_t* heaplet) {
	printf("Heaplet: %p, size = %zu, number_of_neighbours = %zu\n", heaplet, heaplet->size, heaplet->number_of_neighbours);
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
 * Writte a 64 bit number in a little endian fashion.
 */
static int serlial_64_le(void* arg, uint64_t n, mr_writer_function f) {
	for (unsigned int i=0; i<sizeof(uint64_t); i++) {
		f(arg, (n >> (8 * i)) & 0xFF);
	}
	return sizeof(uint64_t);
}

/*
 * Serialize a messy room by generating each char and putting using the result
 * in the given callback.
 * Return the number of char serialized.
 */
static size_t serialize_mr(void* arg, const mr_heaplet_t* heaplet, mr_writer_function f, bool first_call) {
	size_t ret = 0;
	ret += serlial_64_le(arg, heaplet->size, f);
	for (size_t i=0; i<heaplet->size; i++) {
		f(arg, heaplet->data[i]);
		ret++;
	}
	serlial_64_le(arg, first_call ? heaplet->number_of_neighbours : heaplet->number_of_neighbours - 1, f);
	for(size_t i = (first_call ? 0 : 1); i<heaplet->number_of_neighbours; i++) {
		ret += serialize_mr(arg, heaplet->neighbours[i], f, false);
	}
	return ret;
}

/*
 * Read a 64 bit number in little endian.
 * Return true if it can be done and false otherwize.
 */
static bool deserial_64_le(void* arg, uint64_t* n, mr_reader_function f) {
	*n = 0;
	for (unsigned int i=0; i<sizeof(uint64_t); i++) {
		char read;
		bool rc = f(arg, &read);
		if (!rc) {
			return false;
		}
		*n |= ((uint64_t) read) << (8 * i);
	}
	return true;
}

static mr_heaplet_t* deserialize_mr(void* arg, mr_reader_function f, mr_heaplet_t* previous_heaplet) {
	// Reading data
	uint64_t size;
	if (!deserial_64_le(arg, &size, f)) {
		return NULL;
	}
	mr_heaplet_t* ret = new_heaplet(size, NULL);
	for (uint64_t i=0; i<size; i++) {
		char read;
		if (!f(arg, &read)) {
			fprintf(stderr, "[MESSY ROOM] Error, unable to read needed char.\n");
			mr_free(ret);
			return NULL;
		}
		ret->data[i] = read;
	}
	// Reading neighbours
	if (!deserial_64_le(arg, &ret->number_of_neighbours, f)) {
		fprintf(stderr, "[MESSY ROOM] Error, unable to read number of neighbours.\n");
		mr_free(ret);
		return NULL;
	}
	if (previous_heaplet != NULL) {
		ret->number_of_neighbours++;
	}
	ret->neighbours = malloc(sizeof(mr_heaplet_t*) * ret->number_of_neighbours);
	for (size_t i=0; i<ret->number_of_neighbours; i++) {
		ret->neighbours[i] = NULL; // Ensure no segfault if the deserialization must be aborted.
	}
	uint64_t first_index = previous_heaplet == NULL ? 0 : 1;
	if (previous_heaplet != NULL) {
		ret->neighbours[0] = previous_heaplet;
	}
	for (uint64_t i=first_index; i<ret->number_of_neighbours; i++) {
		mr_heaplet_t* neighbour = deserialize_mr(arg, f, ret);
		if (neighbour == NULL) {
			fprintf(stderr, "[MESSY ROOM] Error, unable to neighbours.\n");
			mr_free(ret);
			return NULL;
		}
		ret->neighbours[i] = neighbour;
	}
	return ret;
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

	void _mr_free(mr_heaplet_t* heaplet, const mr_heaplet_t* last_freed) {
		for (size_t i=0; i<heaplet->number_of_neighbours; i++) {
			mr_heaplet_t* target = heaplet->neighbours[i];
			if (target != last_freed && target != NULL) {
				_mr_free(target, heaplet);
			}
		}
		free(heaplet->data);
		free(heaplet->neighbours);
		free(heaplet);
	}

	_mr_free(heaplet, NULL);
}

/*
 * Add data into a heaplet. If there is not enought space, a neighbour or a new
 * heaplet will be chosen. The heaplet choosen is returned.
 */
mr_heaplet_t* mr_add_data(mr_heaplet_t* heaplet, size_t size, const void* data) {
	if (size + sizeof(uint64_t) <= empty_space(heaplet)) {
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


/*
 * Execute a function on each element of the messy room. The function takes the
 * size and the content of each element and an extra pointer that can be used
 * to give arguments to the function.
 * If the function f returns 0, the next element is searched throught,
 * otherwize, mr_crawl returns the exit code of the function f.
 * If all function calls on all elements in the messy room have returned 0,
 * mr_crawl returns 0.
 */
int mr_crawl(mr_heaplet_t* heaplet, mr_crawler_function f, void* extra_args) {

	int _mr_crawl(mr_heaplet_t* heaplet, mr_crawler_function f, void* extra_args, const mr_heaplet_t* previous_heaplet) {
		char* end_of_data = goto_empty_space(heaplet);
		char* data = heaplet->data;
		if (end_of_data == NULL) {
			end_of_data = data + heaplet->size;
		}
		while(data < end_of_data) {
			int rc = f(*((uint64_t*) data), data + sizeof(uint64_t), extra_args);
			if (rc) {
				return rc;
			}
			data = next_intem_in_heaplet(data);
		}
		for (size_t i=0; i<heaplet->number_of_neighbours; i++) {
			mr_heaplet_t* neighbour = heaplet->neighbours[i];
			if (neighbour != previous_heaplet) {
				int rc = _mr_crawl(neighbour, f, extra_args, heaplet);
				if (rc) {
					return rc;
				}
			}
		}
		return 0;
	}

	return _mr_crawl(heaplet, f, extra_args, NULL);
}

/*
 * Write the content of a messy room to an array, it the given array is NULL,
 * nothing is written.
 * Return the number of char needed to serialize the messy room.
 */
size_t mr_write_to_array(mr_heaplet_t* heaplet, char* dest) {
	struct to_array_s {
		char* data;
		size_t index;
	};

	void write_to_array(void* arg, char c) {
		struct to_array_s* context = arg;
		context->data[context->index] = c;
		context->index++;
	}

	void do_nothing(void* _arg, char _c) {
		(void) _arg;
		(void) _c;
	}

	if (dest == NULL) {
		return serialize_mr(NULL, heaplet, do_nothing, true);
	} else {
		struct to_array_s context = {.data = dest, .index = 0};
		return serialize_mr(&context, heaplet, write_to_array, true);
	}
}

/*
 * Write the content of a messy room to a file.
 */
size_t mr_write_to_file(mr_heaplet_t* heaplet, FILE* f) {
	void write_to_file(void* arg, char c) {
		FILE* f = arg;
		fputc(c, f);	
	}
	
	return serialize_mr(f, heaplet, write_to_file, true);
}

/*
 * Read a messy room serialized in an array.
 */
mr_heaplet_t* mr_read_from_array(char* data, size_t size) {
	struct from_array_s {
		char* data;
		size_t size;
		size_t index;
	};

	bool read_byte(void* arg, char* c) {
		struct from_array_s* context = arg;
		if (context->index < context->size) {
			*c = context->data[context->index];
			context->index++;
			return true;
		}
		return false;
	}
	
	struct from_array_s context = {.data = data, .size = size, .index = 0};
	return deserialize_mr(&context, read_byte, NULL);
}

/*
 * Read a messy room serialized in a file.
 */
mr_heaplet_t* mr_read_from_file(FILE* f) {
	bool read_byte(void* arg, char* c) {
		int ch = fgetc((FILE*) arg);
		*c = ch;
		return ch != EOF;
	}
	
	return deserialize_mr(f, read_byte, NULL);
}

