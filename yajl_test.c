/*
 * yajl_test.c
 *
 *  Created on: 24 груд. 2014
 *      Author: Volodymyr Varchuk
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "stream_yajl_get_key.h"


int main(int argc, char ** argv)

{
	static unsigned char fileData[500];
	size_t rd;
	int retval = 0;
	const char* tenant_xpath[] = { "access", "token", "tenant", "id", (const char*) 0 };

	stream_get_key_handl *ten_handler = init_json_stream_get_key(tenant_xpath);

	FILE *f = fopen("/home/volodymyr/format_auth.json", "r");
	printf("start parsing\n");
	int i = 0;

	while ((rd = fread((void *) fileData, 1, sizeof(fileData) - 1, f)) > 0)
	{
		char *str2 = NULL;
		fileData[rd] = 0;
		if (ten_handler
				&& (str2 = json_get_stream_key(ten_handler,
						(const char*) fileData, rd)) != NULL)
		{
			ten_handler = NULL;
			printf("%s\n", str2);
			free(str2);
		}
		if (!ten_handler)
			break;
		i++;
	}
	printf("eng parsing. Total reads count %d\n", i);

	return retval;
}

