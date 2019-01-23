#include "heap.h"
#include "trustmon.h"
#include "tag.h"
#include "heap.c"

void *malloc(size_t size)
{
#ifndef HEAP_INTERLEAVING
	return pvPortMalloc(size);
#else
	/* align size */
	size += sizeof(uword_t) + sizeof(size_t);
	size &= ~((size_t)MASK_REGBYTES);
	char *p = pvPortMalloc(size);
	if (p) {
		/* align pointer */
		if (((uintptr_t)p) & MASK_REGBYTES) {
			sysExit(-6);
		}
		/* Claim all memory */
		if (ato_check_set_tag_zero_range(p, ((char*)p) + size, T_NORMAL, T_NORMAL) != TM_SUCCESS) {
			sysExit(-4);
		}
		/* Store size at beginning of block */
		size_t *ps = (size_t*)p;
		*ps = size;
		return ps + 1;
	}
	return p;
#endif
}

void free(void *ptr)
{
#ifndef HEAP_INTERLEAVING
	vPortFree(ptr);
#else
	if (ptr) {
		size_t *ps = ptr;
		ps--;
		size_t size = *ps;
		/* Unclaim all memory */
		if (ato_check_set_tag_zero_range(ps, ((char*)ps) + size, T_NORMAL, T_NORMAL) != TM_SUCCESS) {
			sysExit(-5);
		}
		vPortFree(ps);
	}
#endif
}

void *calloc(size_t nmemb, size_t size)
{
	if (nmemb == 0 || size > SIZE_MAX / nmemb) {
		return NULL;
	}
	void *p = malloc(nmemb * size);
#ifndef HEAP_INTERLEAVING
	// For heap interleaving, malloc already clears the allocated buffer
	if (p) {
		memset(p, 0, nmemb * size);
	}
#endif
	return p;
}

void *realloc(void *ptr, size_t size)
{
	void *p = malloc(size);
	if (p) {
		memcpy(p, ptr, size);
		free(ptr);
	}
	return p;
}

void vApplicationMallocFailedHook(void) {
	sysExit(-3);
	while(1);
}
