/*
 * stream_yajl_get_key.h
 *
 *  Created on: 6 JAN 2015
 *      Author: Volodymyr Varchuk
 */

#ifndef STREAM_YAJL_GET_KEY_H_
#define STREAM_YAJL_GET_KEY_H_

#include <yajl/yajl_parse.h>
#include <string.h>
#include <stdlib.h>

#define YAJL_CALLBACK_RETURN_OK 1

typedef enum find_status_t {
	fs_ok, fs_next, fs_find, fs_skip, fs_err
} find_status;

typedef struct context_yajl_t {
	const char **path;
	char *return_str;
	int current_depth;
	int max_depth;
	find_status find_status;
} context_yajl;

typedef struct stream_get_key_handl_t {
	yajl_handle yajl_handle;
	context_yajl *ctx;
} stream_get_key_handl;

void free_json_stream_get_key_(stream_get_key_handl *);
char *json_get_stream_key(stream_get_key_handl *, const char *, size_t);
stream_get_key_handl *init_json_stream_get_key(const char **);

#endif /* STREAM_YAJL_GET_KEY_H_ */
