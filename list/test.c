#include "list.c"

int main ()
{

	FILE *list_dump_file = fopen("list_dump_file", "w");

	FILE *list_log_file = fopen("list_log_file", "w");

	list example = new_list(list_log_file);

	for (int i = 0; i < 10; ++i)
		assert(list_push_back(&example, i + 10) == LIST_OK);

	list_erase(&example, 5);
	list_erase(&example, 7);

	int *buf = NULL;
	size_t buf_size = 0;

	//list_optimize(&example, &buf, &buf_size);

	list_insert_after(&example, 0, 1000);

	list_dump(&example, list_dump_file);


	int x = 0;
	list_get(&example, 4, &x);
	printf("%d\n", x);

	list_check(&example);

	system("dot -Tpng list_dump_file -o list_dump_picture.png");
	system("feh list_dump_picture.png");

	list_clear(&example);

	if (buf)
		free(buf);

	return 0;
}
