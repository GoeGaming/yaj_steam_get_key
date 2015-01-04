/*
 * yajl_test.c
 *
 *  Created on: 24 груд. 2014
 *      Author: Volodymyr Varchuk
 */
#include <yajl/yajl_gen.h>
#include <yajl/yajl_parse.h>
#include <yajl/yajl_tree.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


typedef struct {
	const char **path;
	char *return_str;
	int current_depth;
	int max_depth;
	int find_flag;
} context_t;
//typedef struct context_t *context;

struct stream_handler_t{
	yajl_handle yajl_handle;
	context_t *ctx;
};
typedef struct stream_handler_t *stream_handler;


static int handle_number(void * ctx, const char * stringVal,
                           size_t stringLen
						   )
{
	context_t *c = (context_t *) ctx;
    char *str = strndup( (const char *)stringVal, stringLen );
//    printf ("handle number: %s compare %s, also compare %d with %d\n", str, c->path[c->current_depth], c->current_depth, c->max_depth);
    if ( !c->find_flag && c->current_depth == c->max_depth )
    {
    	c->return_str = str;
    	c->find_flag = 1;
    	return yajl_status_client_canceled;
    }
	return 1;//yajl_status_ok;
}


static int handle_string(void * ctx, const unsigned char * stringVal,
                           size_t stringLen
						   )
{
	context_t *c = (context_t *) ctx;
    char *str = strndup( (const char *)stringVal, stringLen );
//    printf ("handle string: %s compare %s, also compare %d with %d\n", str, c->path[c->current_depth], c->current_depth, c->max_depth);
    if ( !c->find_flag && c->current_depth == c->max_depth )
    {
    	c->return_str = str;
    	c->find_flag = 1;
    	return yajl_status_client_canceled;
    }
	return 1;
}

static int handle_map_key(void * ctx, const unsigned char * stringVal,
                            size_t stringLen)
{
	context_t *c = (context_t *)  ctx;
	char *str = strndup( (const char *)stringVal, stringLen );
	printf ("handle map key\n");
	if (!c->find_flag)
	{
		if (c->current_depth <= c->max_depth &&  strncasecmp( c->path[c->current_depth], str, strlen (c->path[c->current_depth]) ) == 0)
		{
			c->current_depth++;
		}
	}
    free (str);
    return 1;
}

static char *json_get_stream_key (stream_handler s_hadler, const char *json_buff, size_t json_buff_len)
{
	yajl_status y_status;
	y_status= yajl_parse(s_hadler->yajl_handle, (const unsigned char *) json_buff, json_buff_len);
	if ( y_status != yajl_status_ok )
		printf ("Error\n");
	if (s_hadler->ctx->return_str != NULL)
	{
		char *str = s_hadler->ctx->return_str;
		free_json_stream_get_key (s_hadler);
		s_hadler = NULL;
		return str;
	}
	else
		return NULL;
}
const yajl_callbacks callbacks = {
	NULL,
	NULL,
    NULL,
    NULL,
	handle_number,
    handle_string,
	NULL,
	handle_map_key,
    NULL,
    NULL,
	NULL
};

stream_handler init_json_stream_get_key (const char **path)
{
	context_t *ctx = malloc( sizeof (context_t) );
	printf ("%zu, %zu\n", sizeof (context_t), sizeof (struct stream_handler_t));
	stream_handler s_hadler = malloc( sizeof (struct  stream_handler_t) );
	int i = 0;

	ctx->path = path;
	for (i = 0; path[i]; i++)
		;
	ctx->find_flag = 0;
	ctx->current_depth = 0;
	ctx->max_depth = i;
	ctx->return_str = NULL;
	s_hadler->ctx = ctx;
	s_hadler->yajl_handle = yajl_alloc(&callbacks, NULL, (void *) s_hadler->ctx );
    yajl_config( s_hadler->yajl_handle, yajl_allow_comments, 1);
	return s_hadler;
}
void free_json_stream_get_key(stream_handler hanld)
{
	free ( hanld->ctx );
	free (hanld);
}

int
main(int argc, char ** argv)

{
    static unsigned char fileData[200];
    yajl_status y_status;
    size_t rd;
    int retval = 0;
	const char* token_xpath[] = {"access", "token", "id", (const char* )0};
	const char* tenant_xpath[] = {"access", "token", "tenant", "id", (const char* )0};

	stream_handler tok_handler = init_json_stream_get_key(token_xpath);
	stream_handler ten_handler = init_json_stream_get_key(tenant_xpath);

    FILE *f = fopen ("/home/volodymyr/auth.json", "r");
    printf ( "%s find flag = %d\n", tok_handler->ctx->path[0], tok_handler->ctx->find_flag );
    printf ("start parsing\n");
    int i = 0;

    while ((rd = fread((void *) fileData, 1, sizeof(fileData) - 1, f)) > 0)
    {
        fileData[rd] = 0;
        char *str1 = NULL;
        char *str2 = NULL;

        if ( tok_handler && ( str1 = json_get_stream_key (tok_handler, fileData, rd)) != NULL )
        {
        	tok_handler = NULL;
        	printf ( "_%s_%p\n", str1, tok_handler );
        }
        if ( ten_handler && ( str2 = json_get_stream_key (ten_handler, fileData, rd)) != NULL )
        {
        	ten_handler = NULL;
        	printf ( "__%s__%p\n", str2, ten_handler);
        }
        if ( !ten_handler && !tok_handler )
        	break;
        i++;
        if (i == 10 )
        	break;
        printf ("read %d\n", i);
    }
    printf ("eng parsing. reads count %d\n", i);

    return retval;
}

