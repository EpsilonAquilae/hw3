/*
 * mm_alloc.c
 *
 * This allocator is based on an IBM design, memory fragmentation is still a problem.
 * 
 * Initially a memory start location is created at the absolute start of our managed memory, 
 * while the end is dynamically updated as we add more data chunks.
 * These data chucks contain a meta data block at the start, followed by a data segment
 * for the user to use.
 *
 * Each time the function is called, we iterate from the start location, over all the chunks
 * currently managed looking for a large enough free data segment. If a large enough free chunk
 * is found, we simply update the free condition and return the data segment start as a pointer.
 * If we find chunks that are free and adjacent, we merge them into one chunk if the new total
 * size will accomodate our request.
 * 
 * If there are no large enough free chunks we allocate more space, create the meta data block
 * at the start, move the last address pointer up to the new end, and return the data segment start
 * as the pointer.
 * 
 * Free simply reverses from the data segement to the block and updates the free condition making
 * the chunk available again for allcation.
 * 
 */

#include "mm_alloc.h"
#include <stdlib.h>

int initialized = 0;
void *mem_start;
void *last_address;

void initialize()
{
	last_address = sbrk(0); // get address at end of heap
	mem_start = last_address; // this represents the start of all our memory, we search from here
	initialized = 1; // done!
}

void* mm_malloc(size_t size)
{

	void *current_address;
	struct s_block *current_address_block;
	void *return_address;
	struct s_block *last_address_block;
	struct s_block *iterate_address_block;

	if (!initialized) initialize(); // setup our memory if we havent yet

	size += sizeof(struct s_block); // we are looking for size + our block's worth of space

	return_address = 0; // nothing to return yet

	current_address = mem_start;

	// we are iterating through all our memory blocks looking from a large enough free data segment
	// this will exit immediately the first time as the boundries are the same
	while (1){
		if (current_address == NULL || current_address == last_address) break;  // we have no chunks
		current_address_block = (struct s_block *)current_address; // the first address is the block
		last_address_block = (struct s_block *)current_address;
		if (current_address_block->free){
			if (current_address_block->size >= size){  // we have enough space right here
				current_address_block->free = 0;
				// we found a large enough free chunk, return now
				return_address = current_address;
				break;
			}else {
				// iterate through adjacent free chunks and tally the sizes
				// create a new chunk if the total size is enough
				iterate_address_block = (struct s_block *)current_address_block;
				int totalsize = 0;
				struct s_block *itr = iterate_address_block;
				while (itr->next != NULL && itr->free == 1 && size <= totalsize){
					totalsize += itr->size;
					itr = itr->next;
				}
				if(totalsize >= size){
					iterate_address_block->next = itr->prev;
					iterate_address_block->size = totalsize;
				}
			}
		}
		current_address = current_address_block->next;
	}

	// we reached the end so set the current to the last address
	if (current_address == NULL) current_address = last_address;

	// after iterating through all our allocated chunk's blocks we still havent found anything yet
	if (!return_address){
		sbrk(size);  // allocate some more space
		return_address = last_address; // move our returning ptr up to the current chunk
		last_address += size;  // move our memory end to the absolute end of the heap again

		current_address_block = current_address;  // create/setup the meta data block at the front
		current_address_block->free = 0;
		current_address_block->size = size;

		if (current_address != mem_start){
			// this is not the first chuck, so we have a prev and next to set
			struct s_block *last_address_block = current_address_block;
			current_address_block->prev = last_address-size; // broken
			current_address_block->next = NULL;
			last_address_block->next = current_address;
		}
	}

	return_address += sizeof(struct s_block);  // advance the ptr to return to after the meta block

	return return_address;

}

void* mm_realloc(void* ptr, size_t size)
{

	// get current chunk meta
	struct s_block *block;
	block = ptr - sizeof(struct s_block);

	// get malloc for new size
	void *new_ptr = mm_malloc(size);

	//move data to new address
	memcpy(ptr, new_ptr, block->size-sizeof(struct s_block));

	// free the last pointer
	mm_free(ptr);

	return new_ptr;

}

void mm_free(void* ptr)
{

	struct s_block *block;
	block = ptr - sizeof(struct s_block);  // we are at the data segment, reverse to the meta block
	block->free = 1;  // the magic of freeing memory
	return;

}
