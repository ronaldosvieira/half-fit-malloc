#include <unistd.h>
//#include <string.h>

#define NALLOC 1024 /* minimum #units to request */

static Header base; /* empty list to get started */
static Header *freelistff = NULL;


/* morecorebf: ask system for more memory */
static Header *morecorebf(unsigned int number_of_units)
{
	char * previous_program_break;
	Header *new_slot;
	if (number_of_units < NALLOC)
		number_of_units = NALLOC;
	
	previous_program_break = sbrk(number_of_units * sizeof(Header));
	if (previous_program_break == (char *) -1) /* no space at all */
		return NULL;
	
	new_slot = (Header *) previous_program_break;
	new_slot->s.size = number_of_units;
	fffree((void *)(new_slot+1));
	return freelistff;
}

/* malloc: general-purpose storage allocator */
void *ffmalloc(size_t nbytes)
{
	Header *current_pointer, *previous_pointer;
	unsigned int nunits;

	/* Don't allocate blocks with zero bytes */
	if (nbytes == 0)
	{
		return NULL;
	}

	/* Use number magic to get good roundoff */
	nunits = (nbytes+sizeof(Header)-1)/sizeof(Header) + 1;


	/* First run, initialize the free list */	
	if (freelistff== NULL) 
	{
		base.s.next = freelistff = &base;
		base.s.size = 0;
	}

	previous_pointer = freelistff;
	current_pointer = previous_pointer->s.next;

	/* Go through all elements in the free list */
	for (;;)
	{
		/* Is this slot big enough? */
		if (current_pointer->s.size >= nunits) 
		{
			/* Is it exactly big enough? */
			if (current_pointer->s.size == nunits) 
			{
				/* Remove the slot */
				previous_pointer->s.next = current_pointer->s.next;
			}
			else 
			{
				/* Remove units from slot */
				current_pointer->s.size -= nunits;
				/* Create a new slot at the end of the big slot */ 
				current_pointer += current_pointer->s.size;
				current_pointer->s.size = nunits;
			}
			return (void *)(current_pointer+1);
		}
	
		/* Have we looked at all elements in the list? */
		if (current_pointer == freelistff)
		{
			/* Ask for more memory! */
			current_pointer = morecorebf(nunits);
			if (current_pointer == NULL)
			{
				/* Memory full */
				return NULL;
			}

		}
		
		/* Increment pointers */
		previous_pointer = current_pointer;
		current_pointer = current_pointer->s.next;
	}
}

/* free: put block in free list */
void fffree(void *target)
{
	if (target == NULL)
		return;
	
	Header *p;
	Header *target_head = (Header *)target - 1; /* Point to block header */
	
	/* Go through the free list in an attempt to find a block to merge with */
	for(p = freelistff; ;p = p->s.next)
	{
		/* Is the freë́'d block in between two free blocks? */
		if (p <= target_head && p->s.next >= target_head)
			break;
		
		/* Are we not counting upwards in memory? */
		/* Eqv. - Have we checked all slots? */
		if (p >= p->s.next)
		{
			/* Free'd block at start of area */
			if (target_head > p)
				break;
		
			/* Free'd block at end of area */
			if (target_head < p->s.next)
				break;
		}
	}
	/* After this loop, p should be set to a block next to the target block in memory */

	/* Join target to p.next */
	if (target_head + target_head->s.size == p->s.next)
	{ 
		target_head->s.size += p->s.next->s.size;
		target_head->s.next = p->s.next->s.next;
	}
	else
	{
		/* Merge up failed, just link them together */
		target_head->s.next = p->s.next;
	}
	
	/* Join target to p */
	if (p + p->s.size == target_head)
	{
		p->s.size += target_head->s.size;
		p->s.next = target_head->s.next;
	}
	else
	{
		/* Merge down failed, just link them together */
		p->s.next = target_head;
	}
}


void *ffrealloc(void * oldpointer, size_t new_size)
{
	/* If pointer is NULL, then the call is equivalent to malloc */
	if (oldpointer == NULL)
	{
		return ffmalloc(new_size);
	}
	/* If size is equal to zero then the call is equivalent to free */
	if (new_size == 0)
	{
		fffree(oldpointer);
		return NULL;
	}
	int old_size = (((Header*)oldpointer - 1)->s.size-1)*sizeof(Header);

	/* Allocate new space, copy contents and free old space */
	void * newpointer = ffmalloc(new_size);
	
	int copy_size = (old_size < new_size ? old_size : new_size);

	newpointer = memcpy(newpointer, oldpointer, copy_size);
	fffree(oldpointer);
	return newpointer;
}

