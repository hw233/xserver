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

static int	print_field(FILE *fp, struct field *f)
{
	char c = ' ';
	int type = f->type;
	if (f->type & SPROTO_TARRAY)
	{
		c = '*';
		fprintf(fp, "	uint32_t n_%s; //%d\n", f->name, f->tag);
	}
	f->type &= ~SPROTO_TARRAY;
	switch (f->type)
	{
		case SPROTO_TDOUBLE:
			fprintf(fp, "	double %c%s; //%d\n", c, f->name, f->tag);			
			break;
		case SPROTO_TINTEGER:
			fprintf(fp, "	uint64_t %c%s; //%d\n", c, f->name, f->tag);
			break;
		case SPROTO_TBOOLEAN:
			fprintf(fp, "	bool %c%s; //%d\n", c, f->name, f->tag);			
			break;
		case SPROTO_TSTRING:
			fprintf(fp, "	char %c*%s; //%d\n", c, f->name, f->tag);						
			break;
		case SPROTO_TSTRUCT:
			fprintf(fp, "	struct %s %c*%s; //%d\n", f->st->name, c, f->name, f->tag);						
			break;			
	}
	f->type = type;
	return (0);
}

static void free_field(FILE *fp, struct field *f)
{
	int type = f->type;	
	bool array = false;
	if (f->type & SPROTO_TARRAY)
	{
		array = true;
	}
	f->type &= ~SPROTO_TARRAY;
	switch (f->type)
	{
		case SPROTO_TDOUBLE:
		case SPROTO_TINTEGER:
		case SPROTO_TBOOLEAN:
			if (array)
			{
				fprintf(fp, "    free(p->%s);\n", f->name);
			}
			break;
		case SPROTO_TSTRING:
			if (array)
			{
				fprintf(fp, "    for(size_t i = 0; i < p->n_%s; ++i)\n", f->name);
				fprintf(fp, "    {\n");
				fprintf(fp, "        free(p->%s[i]);\n", f->name);
				fprintf(fp, "    }\n");
				fprintf(fp, "    free(p->%s);\n", f->name);
				fprintf(fp, "    p->n_%s = 0;\n", f->name);
			}
			else
			{
				fprintf(fp, "    free(p->%s);\n", f->name);				
			}
			break;
		case SPROTO_TSTRUCT:
			if (array)
			{
				fprintf(fp, "    for(size_t i = 0; i < p->n_%s; ++i)\n", f->name);
				fprintf(fp, "    {\n");
				fprintf(fp, "        free_%s(p->%s[i]);\n", f->st->name, f->name);								
				fprintf(fp, "    }\n");
				fprintf(fp, "    free(p->%s);\n", f->name);
				fprintf(fp, "    p->n_%s = 0;\n", f->name);
			}
			else
			{
				fprintf(fp, "    free_%s(p->%s);\n", f->st->name, f->name);
				fprintf(fp, "    p->%s = NULL;\n", f->name);								
			}
			break;			
	}
	f->type = type;	
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
	FILE *fp_h = NULL;
	FILE *fp_cpp = NULL;	

	if (argc > 2)
	{
		char filename[128];
		sprintf(filename, "%s.h", argv[2]);
		fp_h = fopen(filename, "w");
		sprintf(filename, "%s.cpp", argv[2]);		
		fp_cpp = fopen(filename, "w");
	}
	else
	{
		return (0);
	}

	if (!sp)
		return (0);

	sort_index = (int *)malloc(sizeof(int) * sp->type_n);
	sort_sproto(sp, sort_index);

	char *name = strdup(argv[1]);
	escape_type_name(name);

	fprintf(fp_h, "#ifndef _%s_H__\n", name);
	fprintf(fp_h, "#define _%s_H__\n\n", name);
	fprintf(fp_h, "#include <stdlib.h>\n");
	fprintf(fp_h, "#include <stdint.h>\n\n");	

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
		fprintf(fp_h, "struct %s;\n", sp->type[index].name);
	}
	for (i=0;i<sp->type_n;i++) {
		int index = sort_index[i];
		escape_type_name((char *)sp->type[index].name);
		fprintf(fp_h, "void free_%s(struct %s *p);\n", sp->type[index].name, sp->type[index].name);
	}

	for (i=0;i<sp->type_n;i++) {	
		int index = sort_index[i];
		assert(index >= 0 && index < sp->type_n);
		
		fprintf(fp_h, "struct %s\n", sp->type[index].name);
		fprintf(fp_h, "{\n");
//		if (strcmp(type_name, sp->type[i].name) == 0) {
//			return &sp->type[i];
//		}
		int j;
		for (j = 0; j < sp->type[index].n; ++j) {
			print_field(fp_h, &sp->type[index].f[j]);
		}

		fprintf(fp_h, "}__attribute__ ((packed));\n\n");
	}

	fprintf(fp_h, "#endif\n");

	fclose(fp_h);

	fprintf(fp_cpp, "#include \"%s.h\"\n\n", argv[2]);

	for (i=0;i<sp->type_n;i++) {	
		int index = sort_index[i];
		assert(index >= 0 && index < sp->type_n);
		
		fprintf(fp_cpp, "void free_%s(struct %s *p)\n", sp->type[index].name, sp->type[index].name);
		fprintf(fp_cpp, "{\n");
		fprintf(fp_cpp, "    if (!p) return;\n");
		int j;
		for (j = 0; j < sp->type[index].n; ++j) {
			free_field(fp_cpp, &sp->type[index].f[j]);
		}
		fprintf(fp_cpp, "    free(p);\n");
		fprintf(fp_cpp, "};\n\n");
	}

	fclose(fp_cpp);
	
    return 0;
}
