/*
 * stream_yajl_get_key.c
 *
 *  Created on: 6 JAN 2015
 *      Author: Volodymyr Varchuk
 */

#include "stream_yajl_get_key.h"

static int cb_number(void * context, const char * stringVal, size_t stringLen)
{
	context_yajl *ctx = (context_yajl *) context;
	char *str = strndup((const char *) stringVal, stringLen);
	if (fs_next == ctx->find_status)
	{
		ctx->return_str = str;
		ctx->current_depth++;
		ctx->find_status = fs_ok;
		return YAJL_CALLBACK_RETURN_OK; //yajl_status_client_canceled;
	}
	return YAJL_CALLBACK_RETURN_OK;
}

static int cb_string(void * context, const unsigned char * stringVal,
		size_t stringLen)
{
	context_yajl *ctx = (context_yajl *) context;
	if (fs_next == ctx->find_status)
	{
		char *str = strndup((const char *) stringVal, stringLen);
		ctx->return_str = str;
		ctx->current_depth++;
		ctx->find_status = fs_ok;
		return YAJL_CALLBACK_RETURN_OK; //yajl_status_client_canceled;
	}
	return YAJL_CALLBACK_RETURN_OK;
}

static int cb_map_key(void * context, const unsigned char * stringVal,
		size_t stringLen)
{
	context_yajl *ctx = (context_yajl *) context;
	char *str = strndup((const char *) stringVal, stringLen);
	if (fs_ok != ctx->find_status)
	{
		if (fs_next == ctx->find_status && ctx->current_depth == ctx->max_depth)
		{
			ctx->current_depth--;
			ctx->find_status = fs_find;
		}
		if (fs_find == ctx->find_status
				&& strncasecmp(ctx->path[ctx->current_depth],
						(const char *) stringVal,
						strlen(ctx->path[ctx->current_depth])) == 0)
		{
			ctx->current_depth++;
		}
		if (ctx->max_depth == ctx->current_depth)
			ctx->find_status = fs_next;
	}
	free(str);
	return YAJL_CALLBACK_RETURN_OK;
}
static int cb_start_map(void * context)
{
	context_yajl *ctx = (context_yajl *) context;
	if (fs_next == ctx->find_status)
	{
		ctx->find_status = fs_skip;
		ctx->current_depth--;
	}
	return YAJL_CALLBACK_RETURN_OK;
}

static int cb_end_map(void * context)
{
	context_yajl *ctx = (context_yajl *) context;
	if (fs_skip == ctx->find_status)
		ctx->find_status = fs_find;
	return YAJL_CALLBACK_RETURN_OK;
}

static int cb_start_array(void * context)
{
	return YAJL_CALLBACK_RETURN_OK;
}

static int cb_end_array(void * context)
{
	return YAJL_CALLBACK_RETURN_OK;
}

const yajl_callbacks callbacks = {
NULL, /*cb NULL*/
NULL, /*cb boolean*/
NULL, /*cb integer*/
NULL, /*cb double*/
cb_number, /*cb number*/
cb_string, /*cb string*/
cb_start_map, /*cb start_map*/
cb_map_key, /*cb map_key*/
cb_end_map, /*cb end_map*/
cb_start_array, /*cb start_array*/
cb_end_array /*cb end_array*/
};

stream_get_key_handl *init_json_stream_get_key(const char **path)
{
	context_yajl *ctx = calloc(1, sizeof(context_yajl));
	stream_get_key_handl *s_handler = calloc(1, sizeof(stream_get_key_handl));
	int count_max_depth = 0;

	ctx->path = path;
	for (count_max_depth = 0; path[count_max_depth]; count_max_depth++)
		;
	ctx->current_depth = 0;
	ctx->max_depth = count_max_depth;
	ctx->find_status = fs_find;
	ctx->return_str = NULL;
	s_handler->ctx = ctx;
	s_handler->yajl_handle = yajl_alloc(&callbacks, NULL,
			(void *) s_handler->ctx);
	yajl_config(s_handler->yajl_handle, yajl_allow_comments, 1);
	return s_handler;
}

char *json_get_stream_key(stream_get_key_handl *s_handler,
		const char *json_buff, size_t json_buff_len)
{
	yajl_status y_status;
	y_status = yajl_parse(s_handler->yajl_handle,
			(const unsigned char *) json_buff, json_buff_len);
	if (y_status != yajl_status_ok)
		return NULL;

	if (s_handler->ctx->return_str != NULL)
	{
		char *str = s_handler->ctx->return_str;
		free_json_stream_get_key_((stream_get_key_handl *) s_handler);
		s_handler = NULL;
		return str;
	} else
		return NULL;
}

void free_json_stream_get_key_(stream_get_key_handl *handl)
{
	free(handl->ctx);
	yajl_free(handl->yajl_handle);
	free(handl);
}

