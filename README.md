# Messy Room data structure

_Not disorganized, I have a system_

The base element is the item, an array of byte with a size.

Items are sored in small heaps. Each small heap got a memory buffer to copy the items and a list of pointers to neighboring small heap.

The data strucure does not have any encapsulating objects, any of the small heaps can do.

To insert an element, if our starting heaplet got enought place, put it here if not, you either randomly choose a neigbouring heaplet to try and find place or start a new heaplet.
The size of a new heaplet should be the size of the object we want to place multiplied by the number of neigouring heaplets. The probability to try and find find place on the neighoring heaplet is 2/3. In that case, all neigboring heaplet have the same probability to be choosen.
