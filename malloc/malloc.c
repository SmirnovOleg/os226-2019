#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <dlfcn.h>

#define CONCAT(...) __VA_ARGS__

#define FNX(_ret, _name, _decl, _call, ...) \
	static _ret (*_name ## _p)(_decl); \
	void _name ## _init(void) __attribute__((constructor)); \
	void _name ## _init(void) { \
		_name ## _p = dlsym(RTLD_NEXT, # _name); \
	} \
        _ret _name (_decl) { \
		__VA_ARGS__ \
        }

typedef struct block
{
	void *addr;
	struct block *next;
} memory_block;

#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))
#define MEMORY_SIZE (int) 1e9

char main_memory[MEMORY_SIZE];
size_t block_sizes[] = {10, 30, 120, 240, 500, 1024, 5000, 33000};
memory_block *free_lists[LEN(block_sizes)];
char was_initialized = 0;
void *base = main_memory;


static __attribute__((constructor)) void _free_lists_init()
{
	int region_size = MEMORY_SIZE / LEN(block_sizes);
	int index = 0;

	for (void *region_ptr = base;
			region_ptr < base + MEMORY_SIZE && index < LEN(block_sizes);
			region_ptr += region_size) 
	{
		size_t real_block_size = block_sizes[index] + sizeof(memory_block);
		free_lists[index++] = (memory_block *) region_ptr;

		for (void *block_ptr = region_ptr;
			block_ptr < region_ptr + region_size;
			block_ptr += real_block_size)
		{
			memory_block *new_block = (memory_block *) block_ptr;
			new_block->addr = block_ptr + sizeof(memory_block);
			if (block_ptr + real_block_size >= region_ptr + region_size)
				new_block->next = NULL;
			else
				new_block->next = (memory_block *) (block_ptr + real_block_size);
		}
	}
	was_initialized = 1;
}


size_t _index_of_approximate(size_t size) {
	size_t left_index = 0;
	size_t right_index = LEN(block_sizes) - 1;
	if (size > block_sizes[right_index]) 
	{
		fprintf(stderr, "Allocation error: not enough memory\n");
		exit(1);
	}
	if (size < block_sizes[left_index]) 
	{
		return left_index;
	}
	while (right_index > left_index + 1) 
	{
		int ind_avg = (right_index + left_index) >> 1;
		if (block_sizes[ind_avg] < size) 
			left_index = ind_avg;
		else 
			right_index = ind_avg;
	}
	if (block_sizes[left_index] == size)
		return left_index;
	else
		return right_index;	 
}


FNX(void *, malloc,
		size_t size,
		size,
{
	if (!was_initialized)
		_free_lists_init();
	//fprintf(stderr, "%s size %u\n", __func__, size);

	size_t index = _index_of_approximate(size);
	memory_block *free_block = free_lists[index];
	if (free_block == NULL) 
	{
		fprintf(stderr, "Allocation error: not enough memory\n");
		exit(1);
	}
	free_lists[index] = free_block->next;
	return free_block->addr;
})

FNX(void, free,
		void *ptr,
		ptr,
{
	if (!was_initialized)
		_free_lists_init();
	//fprintf(stderr, "%s ptr %p\n", __func__, ptr);

	if (ptr == NULL || 
			!(base + sizeof(memory_block) < ptr 
				&& ptr < base + MEMORY_SIZE))
		return;

	memory_block *current_block = (memory_block *) (ptr - sizeof(memory_block));
	size_t current_block_size = ((size_t) current_block->next) - ((size_t) current_block->addr);
	size_t index = _index_of_approximate(current_block_size);
	memory_block *head_block = free_lists[index];

	current_block->addr = ptr;
	current_block->next = head_block;
	free_lists[index] = current_block;
})

FNX(void *, calloc,
		CONCAT(size_t nmemb, size_t size),
		CONCAT(nmemb, size),
{
	if (!was_initialized)
		_free_lists_init();
	//fprintf(stderr, "%s nmemb %u size %u\n", __func__, nmemb, size);
	
	uint8_t block_size_index = _index_of_approximate(size);
	memory_block *free_block = free_lists[block_size_index];
	free_lists[block_size_index] = free_block->next;

	void *current_addr = free_block->addr;
	for (size_t *ptr = current_addr; ptr < (size_t *) (current_addr + size); ++ptr)
		*ptr = nmemb;

	return free_block->addr;
})

FNX(void *, realloc,
		CONCAT(void *ptr, size_t size),
		CONCAT(ptr, size),
{
	if (!was_initialized)
		_free_lists_init();
	//fprintf(stderr, "%s ptr %p size %u\n", __func__, ptr, size);

	free(ptr);
	return malloc(size);
})

FNX(void *, reallocarray,
		CONCAT(void *ptr, size_t nmemb, size_t size),
		CONCAT(ptr, nmemb, size),
{
	if (!was_initialized)
		_free_lists_init();
	//fprintf(stderr, "%s ptr %p nmemb %u size %u\n", __func__, ptr, nmemb, size);

	free(ptr);
	return calloc(nmemb, size);
})