#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include "sproto.h"
#include "sprotoc_common.h"

static int escape_type_name(char *name)
{
	while(*name)
	{
		if (*name == '.')
			*name = '_';
		++name;
	}
	return (0);
}

static int	print_field(struct field *f)
{
	char c = ' ';
	if (f->type & SPROTO_TARRAY)
	{
		c = '*';
		printf("	uint32_t n_%s; //%d\n", f->name, f->tag);
	}
	f->type &= ~SPROTO_TARRAY;
	switch (f->type)
	{
		case SPROTO_TDOUBLE:
			printf("	double %c%s; //%d\n", c, f->name, f->tag);			
			break;
		case SPROTO_TINTEGER:
			printf("	uint64_t %c%s; //%d\n", c, f->name, f->tag);
			break;
		case SPROTO_TBOOLEAN:
			printf("	bool %c%s; //%d\n", c, f->name, f->tag);			
			break;
		case SPROTO_TSTRING:
			printf("	char %c*%s; //%d\n", c, f->name, f->tag);						
			break;
		case SPROTO_TSTRUCT:
			printf("	struct %s %c*%s; //%d\n", f->st->name, c, f->name, f->tag);						
			break;			
	}
	return (0);
}

/* static void swap_sproto_type(struct sproto *sp, uint32_t src, uint32_t dst) */
/* { */
/* 	assert(src < sp->type_n); */
/* 	assert(dst < sp->type_n); */
/* 	struct sproto_type type; */
/* 	memcpy(&type, &sp->type[dst], sizeof(type)); */
/* 	memcpy(&sp->type[dst], &sp->type[src], sizeof(type)); */
/* 	memcpy(&sp->type[src], &type, sizeof(type)); */
/* } */

static bool already_in_sort_index(int i, int *sort_index, int size)
{
	for (int n = 0; n < size; ++n)
	{
		if (sort_index[n] < 0)
			return false;
		if (sort_index[n] == i)
			return true;
	}
	return false;
}

static void sort_sproto(struct sproto *sp, int *sort_index)
{
	int i, j, k;

	for (j = 0; j < sp->type_n; ++j)
		sort_index[j] = -1;

	for (j = 0; j < sp->type_n; ++j)
	{
		int min_index = -1;
		for (i=0; i<sp->type_n; i++)
		{
			if (already_in_sort_index(i, sort_index, sp->type_n))
				continue;

			if (min_index < 0)
			{
				min_index = i;
				continue;
			}
			
			if (strcmp(sp->type[i].name, sp->type[min_index].name) < 0)
			{
				min_index = i;
			}
		}
		
		sort_index[j] = min_index;
//			swap_sproto_type(sp, j, min_index);
	}
	
}

int main(int argc, char *argv[])
{
	int *sort_index;
	static char buf[4096000];	
	int fd = open(argv[1], O_RDONLY);
	size_t size =  read(fd, buf, sizeof(buf));
	struct sproto *sp = sproto_create(&buf[0], size);
	close(fd);

	if (!sp)
		return (0);

	sort_index = (int *)malloc(sizeof(int) * sp->type_n);
	sort_sproto(sp, sort_index);

	char *name = strdup(argv[1]);
	escape_type_name(name);

	printf("#ifndef _%s_H__\n", name);
	printf("#define _%s_H__\n\n", name);

	int i;

	/* for (i=0;i<sp->type_n;i++) { */
	/* 	int index = sort_index[i]; */
	/* 	printf("%d ", index); */
	/* } */

	/* printf("\n=================\n"); */

	/* for (i=0;i<sp->type_n;i++) { */
	/* 	escape_type_name((char *)sp->type[i].name); */
	/* 	printf("[%d] struct %s;\n", i, sp->type[i].name); */
	/* } */

	/* printf("=================\n"); */
	
	for (i=0;i<sp->type_n;i++) {
		int index = sort_index[i];
		escape_type_name((char *)sp->type[index].name);
		printf("struct %s;\n", sp->type[index].name);
	}

	for (i=0;i<sp->type_n;i++) {	
		int index = sort_index[i];
		assert(index >= 0 && index < sp->type_n);
		
		printf("struct %s\n", sp->type[index].name);
		printf("{\n");
//		if (strcmp(type_name, sp->type[i].name) == 0) {
//			return &sp->type[i];
//		}
		int j;
		for (j = 0; j < sp->type[index].n; ++j) {
			print_field(&sp->type[index].f[j]);
		}

		printf("}__attribute__ ((packed));\n\n");
	}

	printf("#endif\n");
	
    return 0;
}
