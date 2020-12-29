#ifndef LIST
#define LIST

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

//----------LIST SETTINGS---------------------
typedef enum __list_result_code
{

	LIST_OK,
	LIST_NOMEM,
	LIST_SEGFAULT,
} list_result_code;

#define list_element int
//--------------------------------------------


//----------LIST_NODE-----------------------------------------------
typedef struct __list_node
{

	list_element value;

	size_t next,
		   previous;

	bool valid;
} list_node;

list_node new_list_node (int value, size_t next, size_t previous, bool valid)
{

	list_node temp;

	temp.value = value;
	temp.next = next;
	temp.previous = previous;

	temp.valid = valid;

	return temp;
}
//-----------------------------------------------------------------


//----------LIST---------------------------------------------------
typedef struct __list
{

	list_node *storage;

	size_t size,
		   capacity;

	size_t free_ind,
		   free_size;

	bool optimized;

	FILE *log_file_ptr;
} list;


list_result_code list_realloc (list *st, size_t new_capacity)
{

	assert(st);

	if (st->capacity >= new_capacity)
		return LIST_OK;

	st->storage = (list_node*) realloc(st->storage, (new_capacity + 1) * sizeof(list_node));
	if (!st->storage)
		return LIST_NOMEM;

	for (int i = new_capacity; i > st->capacity; --i)
	{

		st->storage[i].next = st->free_ind;
		st->storage[i].previous = 0;

		st->free_ind = i; 
	}

	st->free_size += new_capacity - st->capacity;
	st->capacity = new_capacity;

	return LIST_OK;
}


list new_list (FILE *log_file_ptr)
{

	list temp;

	temp.storage = (list_node*) calloc(1, sizeof(list_node));

	temp.storage[0] = new_list_node(0, 0, 0, false);
	temp.free_ind = 0;
	temp.free_size = 0;

	temp.size = 0;
	temp.capacity = 0;

	list_result_code list_realloc_rc = list_realloc(&temp, 10);

	assert(list_realloc_rc == LIST_OK);

	temp.optimized = true;

	temp.log_file_ptr = log_file_ptr;

	
	fprintf(log_file_ptr, "List created.\n");

	return temp;
}

list_result_code list_insert_after (list *st, size_t pos, int value)
{

	assert(st);

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "List insert after...\n");

	if (pos > st->capacity || (!st->storage[pos].valid && pos != 0))
	{

		if (st->log_file_ptr)
			fprintf(st->log_file_ptr, "EROR: List seagfault\n");

		return LIST_SEGFAULT;
	}

	if (st->free_size == 0)
	{

		list_result_code realloc_rc = list_realloc(st, 2*st->capacity);

		if (realloc_rc != LIST_OK)
		{

			if (st->log_file_ptr)
				fprintf(st->log_file_ptr, "EROR: Insert error\n");

			return realloc_rc;
		}
	}

	size_t next_free_ind = st->storage[st->free_ind].next,
		   old_pos_next  = st->storage[pos].next;

	st->storage[st->free_ind] = new_list_node(value, 0, pos, true);
	st->storage[pos].next = st->free_ind;

	if (st->storage[old_pos_next].valid)
	{

		st->storage[st->free_ind].next = old_pos_next;
		st->storage[old_pos_next].previous = st->free_ind;
		
		st->optimized = false;	
	}

	st->storage[0].previous = st->free_ind;

	st->free_ind = next_free_ind;
	st->free_size--;
	st->size++;

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "OK\n");

	return LIST_OK;
}

list_result_code list_insert_before (list *st, int pos, int value)
{

	assert(st);

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "List insert before...\n");

	if (pos > st->capacity || (!st->storage[pos].valid && pos != 0))
	{

		if (st->log_file_ptr)
			fprintf(st->log_file_ptr, "EROR: List seagfault\n");

		return LIST_SEGFAULT;
	}

	return list_insert_after(st, st->storage[pos].previous, value);
}

list_result_code list_push_back (list *st, int value)
{

	assert(st);	

	return list_insert_after(st, st->storage[0].previous, value);
}

list_result_code list_push_front (list *st, int value)
{

	assert(st);

	return list_insert_after(st, 0, value);
}

list_result_code list_erase (list *st, size_t pos)
{

	assert(st);

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "List erase...\n");

	if (pos > st->capacity || !st->storage[pos].valid)
	{

		if (st->log_file_ptr)
			fprintf(st->log_file_ptr, "EROR: List seagfault\n");

		return LIST_SEGFAULT;
	}

	if (st->storage[pos].next != 0 || st->storage[pos].previous != 0)
		st->optimized = false;

	st->storage[st->storage[pos].previous].next = st->storage[pos].next;
	st->storage[st->storage[pos].next].previous = st->storage[pos].previous;

	st->storage[pos].next = st->free_ind;
	st->storage[pos].previous = 0;
	st->storage[pos].valid = false;

	st->free_ind = pos;
	st->free_size++;
	st->size--;

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "OK\n");	

	return LIST_OK;
}

list_result_code list_dump (list *st, FILE *graphvis_file_ptr)
{

	assert(st);

	fprintf(graphvis_file_ptr, "digraph LIST_DUMP {\n"
							   		"\tnode [shape=box, style=\"filled\", fillcolor=\"white\", fontcolor=\"black\", fontsize=\"9\"];\n");


	fprintf(graphvis_file_ptr, "label = \"List valid nodes\";\n");

	for (int i = 0, j = 0; ; i = st->storage[i].next, ++j)
	{

		if (i != 0)
			fprintf(graphvis_file_ptr, "\"%d\" [shape=\"record\", label=\"physycal number: %d |"
																         "logic number: %d |"
																         "value: %d\"];\n",

									   i, i, j, st->storage[i].value);

		if (st->storage[i].next == 0)
			break;
	}

	for (int i = 0; ; i = st->storage[i].next)
	{


		if (i != 0 && st->storage[i].next != 0)
		{

			fprintf(graphvis_file_ptr, "\"%d\"->\"%ld\" [constraint = \"false\"];\n", i, st->storage[i].next);
			fprintf(graphvis_file_ptr, "\"%ld\"->\"%d\";\n", st->storage[i].next, i);
		}

		if (st->storage[i].next == 0)
			break;
	}

	fprintf(graphvis_file_ptr, "}\n");
	fflush(graphvis_file_ptr);
}

list_result_code list_clear (list *st)
{

	assert(st);

	free(st->storage);

	st->free_size = 0;
	st->free_ind = 0;
	st->size = 0;
	st->capacity = 0;
	st->optimized = false;
	st->log_file_ptr = NULL;

	return LIST_OK;
}


list_result_code list_optimize (list *st, int **buf, size_t *buf_size)
{

	assert(st);

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "List optimize...\n");	

	if (*buf_size < st->size)
	{

		*buf = realloc(*buf, (st->capacity * 2) * sizeof(int));
		*buf_size = st->capacity * 2;
	}

	for (int i = st->storage[0].next, j = 0; ; i = st->storage[i].next, ++j)
	{

		(*buf)[j] = st->storage[i].value;

		if (st->storage[i].next == 0)
			break;
	}

	for (int i = 1; i <= st->size; ++i)
	{


		st->storage[i].value = (*buf)[i - 1];
		st->storage[i].next = i + 1;
		st->storage[i].previous = i - 1;

		st->storage[i].valid = true;
	}

	for (int i = st->size + 1; i <= st->capacity; ++i)
		st->storage[i].valid = false;

	st->storage[st->size].next = 0;
	st->storage[0].previous = st->size;

	st->free_ind = 0;

	for (int i = st->capacity; !st->storage[i].valid; --i)
	{

		st->storage[i].next = st->free_ind;
		st->storage[i].previous = 0;

		st->free_ind = i;
	}

	st->optimized = true;

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "OK\n");
}

list_result_code list_check (list *st)
{

	assert(st);

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "List check...\n");

	for (int i = 0, j = 0; ; i = st->storage[i].next, ++j)
	{

		if (j > st->size || i > st->capacity || (st->storage[i].next == 0 && j != st->size))
		{

			if (st->log_file_ptr)
				fprintf(st->log_file_ptr, "ERROR: valid nodes incorrect\n");

			return LIST_SEGFAULT;			
		}

		if (st->storage[i].next == 0)
		{

			if (st->log_file_ptr)
				fprintf(st->log_file_ptr, "Valid nodes OK\n");

			break;
		}
	}

	for (int i = st->free_ind, j = 1; ; i = st->storage[i].next, ++j)
	{

		if (j > st->free_size || i > st->capacity || (st->storage[i].next == 0 && j != st->free_size))
		{

			if (st->log_file_ptr)
				fprintf(st->log_file_ptr, "ERROR: free nodes incorrect\n");

			return LIST_SEGFAULT;			
		}

		if (st->storage[i].next == 0)
			break;
	}

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "free nodes OK\n");

	return LIST_OK;
}

list_result_code list_get (list *st, size_t pos, int *value)
{

	assert(st && value);

	if (st->log_file_ptr)
		fprintf(st->log_file_ptr, "List get...\n");

	if (pos > st->capacity)
	{

		if (st->log_file_ptr)
			fprintf(st->log_file_ptr, "ERROR: capacity overflow\n");

		return LIST_SEGFAULT;
	}

	if (!st->storage[pos].valid)
	{

		if (st->log_file_ptr)
			fprintf(st->log_file_ptr, "ERROR: node not valid\n");

		return LIST_SEGFAULT;		
	}

	*value = st->storage[pos].value;

	return LIST_OK;
}
//-----------------------------------------------------------------

#endif //LIST