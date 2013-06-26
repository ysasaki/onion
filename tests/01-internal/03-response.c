/*
	Onion HTTP server library
	Copyright (C) 2010 David Moreno Montero

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU Affero General Public License as
	published by the Free Software Foundation, either version 3 of the
	License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Affero General Public License for more details.

	You should have received a copy of the GNU Affero General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
	*/

#ifdef __APPLE__
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <string.h>

#include <onion/log.h>
#include <onion/dict.h>
#include <onion/request.h>
#include <onion/response.h>
#include <onion/types_internal.h>
#include <onion/onion.h>
#include <onion/http.h>

#include "../ctest.h"
#include "buffer_listen_point.h"

#define FILL(a,b) onion_request_write(a,b,strlen(b))


void t01_create_add_free(){
	INIT_LOCAL();
	
	onion_response *res;
	
	res=onion_response_new(NULL);
	FAIL_IF_NOT_EQUAL(res->code, 200);
	
	FAIL_IF_EQUAL(res,NULL);
	
	onion_response_set_length(res,1024);
	FAIL_IF_NOT_EQUAL_STR(onion_dict_get(res->headers,"Content-Length"),"1024");
	
	onion_response_free(res);
	
	END_LOCAL();
}

/// Just appends to the handler. Must be big enought or segfault.. Just for tests.
int write_append(onion_request *req, const char *data, unsigned int length){
	char *str=req->connection.user_data;
	ONION_DEBUG("char data %p", str);
	int p=strlen(str);
	strncat(str,data,length);
	str[p+length]=0;
	return length;
}


void t02_full_cycle_http10(){
	INIT_LOCAL();
	
	onion *server=onion_new(0);
	onion_add_listen_point(server,NULL,NULL,onion_buffer_listen_point_new());
	onion_request *request;
	char buffer[4096];
	memset(buffer,0,sizeof(buffer));
	
	request=onion_request_new(server->listen_points[0]);
	
	onion_response *response=onion_response_new(request);
	
	onion_response_set_length(response, 30);
	FAIL_IF_NOT_EQUAL(response->length,30);
	onion_response_write_headers(response);
	
	onion_response_write0(response,"123456789012345678901234567890");
	
	FAIL_IF_NOT_EQUAL(response->sent_bytes,30);
	
	onion_response_free(response);
	strncpy(buffer,onion_buffer_listen_point_get_buffer_data(request),sizeof(buffer));
	onion_request_free(request);
	onion_free(server);
	
	FAIL_IF_NOT_STRSTR(buffer, "HTTP/1.0 200 OK\r\n");
	FAIL_IF_NOT_STRSTR(buffer, "Connection: Keep-Alive\r\n");
	FAIL_IF_NOT_STRSTR(buffer, "Content-Length: 30\r\n");
	FAIL_IF_NOT_STRSTR(buffer, "Server: libonion");
	FAIL_IF_NOT_STRSTR(buffer, "coralbits");
	FAIL_IF_NOT_STRSTR(buffer, "\r\n\r\n123456789012345678901234567890");
	
	END_LOCAL();
}

void t03_full_cycle_http11(){
	INIT_LOCAL();
	
	onion *server=onion_new(0);
	onion_add_listen_point(server, NULL,NULL,onion_buffer_listen_point_new());
	onion_request *request;
	char buffer[4096];
	memset(buffer,0,sizeof(buffer));
	
	request=onion_request_new(server->listen_points[0]);
	FILL(request,"GET / HTTP/1.1\n");
	
	onion_response *response=onion_response_new(request);
	
	onion_response_set_length(response, 30);
	FAIL_IF_NOT_EQUAL(response->length,30);
	onion_response_write_headers(response);
	
	onion_response_write0(response,"123456789012345678901234567890");
	
	FAIL_IF_NOT_EQUAL(response->sent_bytes,30);
	
	onion_response_free(response);
	strncpy(buffer,onion_buffer_listen_point_get_buffer_data(request),sizeof(buffer));
	onion_request_free(request);
	onion_free(server);
	
	FAIL_IF_NOT_STRSTR(buffer, "HTTP/1.1 200 OK\r\n");
	FAIL_IF_STRSTR(buffer, "Connection: Keep-Alive\r\n");
	FAIL_IF_NOT_STRSTR(buffer, "Content-Length: 30\r\n");
	FAIL_IF_NOT_STRSTR(buffer, "Server: libonion");
	FAIL_IF_NOT_STRSTR(buffer, "coralbits");
	FAIL_IF_NOT_STRSTR(buffer, "\r\n\r\n123456789012345678901234567890");

	//FAIL_IF_NOT_EQUAL_STR(buffer, "HTTP/1.1 200 OK\r\nContent-Length: 30\r\nServer: libonion v0.1 - coralbits.com\r\n\r\n123456789012345678901234567890");
	
	END_LOCAL();
}

int main(int argc, char **argv){
	START();
	
	t01_create_add_free();
	t02_full_cycle_http10();
	t03_full_cycle_http11();
	
	END();
}

