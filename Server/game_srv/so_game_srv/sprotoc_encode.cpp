#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>
#include "sproto.h"
#include "sprotoc_common.h"

int sprotoc_encode(const struct sproto_arg *args)
{
	int ret = 0;
	void **p = (void **)&(args->ud);

//	printf("%s tag[%d] type[%d] subtype[%p] value[%p] length[%d] index[%d] mainindex[%d]\n",
//		args->tagname, args->tagid, args->type, args->subtype, args->value, args->length,
//		args->index, args->mainindex);
	
//	char *p = args->ud;
	switch (args->type)
	{
		case SPROTO_TINTEGER:
		{
			void *data = args->ud;
			if (args->index > 0)
			{
				uint32_t size = *(uint32_t *)data;
				if (size < (uint32_t)args->index)
				{
					ret = SPROTO_CB_NIL;
					*p = (char *)(*p) + (sizeof(uint32_t) + sizeof(void *));
					break;
				}
				data = (char *)data + sizeof(uint32_t);
//				data = *(char **)data + sizeof(uint64_t) * (args->index - 1);
				*(uint64_t *)args->value = (*(uint64_t **)data)[args->index - 1];
			}
			else
			{
				*(uint64_t *)args->value = *(uint64_t *)data;
				*p = (char *)(*p) + sizeof(uint64_t);
//				printf("encode int value %lu\n", *(uint64_t *)args->value);
			}
			ret = 8;
			break;
		}
		case SPROTO_TBOOLEAN:
		{
			void *data = args->ud;
			if (args->index > 0)
			{
				uint32_t size = *(uint32_t *)data;
				if ((int)size < args->index)
				{
					ret = SPROTO_CB_NIL;
					*p = (char *)(*p) + (sizeof(uint32_t) + sizeof(void *));
					break;
				}
				data = (char *)data + sizeof(uint32_t);
				*(bool *)args->value = (*(bool **)data)[args->index - 1];				
			}
			else
			{
				*(int *)args->value = *(bool *)args->ud;
				*p = (char *)(*p) + sizeof(bool);			
				ret = 4;
			}
			break;
		}
		case SPROTO_TSTRING:
		{
			void *data = args->ud;
			if (args->index > 0)
			{
				uint32_t size = *(uint32_t *)data;
				if ((int)size < args->index)
				{
					ret = SPROTO_CB_NIL;
					*p = (char *)(*p) + (sizeof(uint32_t) + sizeof(void *));
					break;
				}
				data = (char *)data + sizeof(uint32_t);
				char *str = (*(char ***)data)[args->index - 1];
				ret = strlen(str) + 1;
				memcpy(args->value, str, ret);
			}
			else
			{
				ret = strlen(*(char **)(args->ud)) + 1;
				memcpy(args->value, *(char **)args->ud, ret);

//				printf("encode string value %s\n", *(char **)(args->ud));
				*p = (char *)(*p) + sizeof(void *);
			}
			break;
		}
		case SPROTO_TSTRUCT:
		{
			void *data = args->ud;
			if (args->index > 0)
			{
				uint32_t size = *(uint32_t *)data;
				if ((int)size < args->index)
				{
					ret = SPROTO_CB_NIL;
					*p = (char *)(*p) + (sizeof(uint32_t) + sizeof(void *));
					break;
				}
				data = (char *)data + sizeof(uint32_t);
				ret = sproto_encode(args->subtype, args->value, args->length, sprotoc_encode, (*(void ***)data)[args->index - 1]);
			}
			else
			{
				ret = sproto_encode(args->subtype, args->value, args->length, sprotoc_encode, *(char **)(args->ud));
			}
			break;
		}
	}
	return (ret);
}
