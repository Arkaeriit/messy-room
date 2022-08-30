# Messy Room data structure

_I am not disorganized, I have a system_

Messy Room is a data structure inspired by a messy bedroom.

The data are stored on small heap of memory, called heaplets. If there is not enough room in an heaplet to add more memory, just start a new heaplet next to it.

## Data structure description

The base element of the Messy Room is the heaplet. Each heaplet have a specific size that is chosen upon creation and an arrays of byte of this size. The arrays of byte is allocated at the creation of the heaplet and is modified each time we want to put new data in the heaplet.

Upon creation, all the bytes in the heaplet's array are set to 0. To add some data in the heaplet, the size of the data in written on 64 bits, followed by the actual data. Thus, various elements in the heaplet are stored as a chain of size-value elements.

Each heaplet also got a list of all neighboring heaplets which evolves as the Messy Room grows. All the heaplets of the Messy Room are connected in a network of neighbors. Thus, any heaplet can be used to manipulate the whole Messy Room.

To insert an element, if our starting heaplet got enough place, put it here if not, you either randomly choose a neighboring heaplet to try and find place or start a new heaplet.
The size of a new heaplet should be the size of the object we want to place multiplied by the number of neighboring heaplets. The probability to try and find find place on the neighboring heaplet is 2/3. In that case, all neighboring heaplet have the same probability to be chosen. This lead to an insertion time of O(1) as the number of elements in the Messy Room does not impact the insertion time.

When a new heaplet neighbor heaplet is created, it's list of neighbor is initialized with the heaplet we came from. New neighbor heaplets are appended at the end of the list.

Even if adding object in your messy room is easy, finding them in such a mess is hard. To find a element, you must crawl through all elements, in all the heaplets, until you find it. This lead to a fetch time of O(N).

Data can be added and modified in the Messy Room but you can't remove it because, let's be honest, who have the time to clean their room?

## API

### Messy room manipulation

`mr_heaplet_t* mr_new(void)`: Create a new, empty messy room.

`void mr_free(mr_heaplet_t* heaplet)`: Free all the memory used by a messy room.

`mr_heaplet_t* mr_add_data(mr_heaplet_t* heaplet, size_t size, const void* data)`: Add the element `data` of size `size` to the messy room. The heaplet where the data ends up being put on is returned. Thus `mr_add_data(heaplet, 5, "test");` lets you add element stating always from the same heaplet and `heaplet = mr_add_data(heaplet, 5 "test");` lets you change the starting heaplet. Doing the first method let to Messy Rooms that are somewhat more compact but the second method make it easier to fetch recently added elements.

`int mr_crawl(mr_heaplet_t* heaplet, mr_crawler_function f, void* extra_args)`: given a function of prototype `int (*mr_crawler_function)(uint64_t size, char* data, void* extra_args)`, crawls through the Messy Room until the function returns a value that is not 0. In that case, this value will be the return value of `mr_crawl`. If all the elements of the Messy Room have been checked and the crawler function always returns 0, 0 will be the return value of `mr_crawl`.

### Serialization

If you want to use Messy Rooms to store non-volatile data, you will want to store it to a file or something similar. The following functions can be used to do so:

`size_t mr_write_to_array(mr_heaplet_t* heaplet, char* dest)`: Serialize the Messy Room into the given array and returns the number of bytes written. If `NULL` is given as the destination, nothing will be written but the number of bytes needed is still returned.

`size_t mr_write_to_file(mr_heaplet_t* heaplet, FILE* f)`: Serialize the Messy Room into the given open file.

`mr_heaplet_t* mr_read_from_array(char* data, size_t size)`: Deserialize a Messy Room from the given array of the given size.

`mr_heaplet_t* mr_read_from_file(FILE* f)`: Deserialze a Messy Room from an open file.

## Example

In the example directory, there a small key-value store CLI app using Messy Room as its backend. It is meant to be an example of how to use Messy Room in a real project (not that you should do it).

