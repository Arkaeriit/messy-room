#ifndef KV_OVER_MESSY_ROOM
#define KV_OVER_MESSY_ROOM

#include "messy-room.h"
#include <stdbool.h>

#define K_SIZE 255
#define V_SIZE 255

void kvomr_write(mr_heaplet_t* heaplet, const char* k, const char* v);
char* kvomr_read(mr_heaplet_t* heaplet, const char* k);
bool kvomr_delete(mr_heaplet_t* heaplet, const char* k);

#endif

