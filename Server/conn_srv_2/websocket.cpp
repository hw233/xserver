#include "websocket.h"
#include "tea.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <openssl/sha.h>
#include <arpa/inet.h>

#define UNUSED(x) (void)(x)

char *get_websocket_request_key(char *s_req)
{
	const char *delim = "\r\n";
	char *p = NULL;
//	char *q = NULL;
//	UNUSED(q);

	p = strtok((char *)s_req, delim);
	if (p) {
		while ((p = strtok(NULL, delim)) != NULL) {
			if (strncasecmp(p, "Sec-WebSocket-Key:", 18) != 0) {
				continue;
			}
			p += 18;
			while (*++p == ' ');
//			q = p;
			return p; 
		}
	}
	return NULL;
}

int32_t parse_websocket_request(char *s_req, ws_req_t *ws_req)
{
	if (!s_req || !ws_req) {
		return -1;
	}
//	int len = strlen(s_req);
//	char tmp[len + 1];
//	tmp[len] = 0;
//	memcpy(tmp, s_req, len);

	const char *delim = "\r\n";
	char *p = NULL, *q = NULL;

	p = strtok((char *)s_req, delim);
	if (p) {
		//printf("%s\n", p);
		ws_req->req = p;
		while ((p = strtok(NULL, delim)) != NULL) {
			//printf("%s\n", p);
			if ((q = strstr(p, ":")) != NULL) {
				*q = '\0';
				if (strcasecmp(p, "Connection") == 0) {
					while (*++q == ' ');
					ws_req->connection = strdup(q);
				}
				if (strcasecmp(p, "Upgrade") == 0) {
					while (*++q == ' ');
					ws_req->upgrade = strdup(q);
				}
				if (strcasecmp(p, "Host") == 0) {
					while (*++q == ' ');
					ws_req->host = strdup(q);
				}
				if (strcasecmp(p, "Origin") == 0) {
					while (*++q == ' ');
					ws_req->origin = strdup(q);
				}
				if (strcasecmp(p, "Cookie") == 0) {
					while (*++q == ' ');
					ws_req->cookie = strdup(q);
				}
				if (strcasecmp(p, "Sec-WebSocket-Key") == 0) {
					while (*++q == ' ');
					ws_req->sec_websocket_key = strdup(q);
				}
				if (strcasecmp(p, "Sec-WebSocket-Version") == 0) {
					while (*++q == ' ');
					ws_req->sec_websocket_version = strdup(q);
				}
			}
		}
	}
	return 0;
}


void print_websocket_request(const ws_req_t *ws_req)
{
	if (ws_req) {
		if (ws_req->req) {
			fprintf(stdout, "%s\r\n", ws_req->req);
		}
		if (ws_req->connection) {
			fprintf(stdout, "Connection: %s\r\n", ws_req->connection);
		}
		if (ws_req->upgrade) {
			fprintf(stdout, "Upgrade: %s\r\n", ws_req->upgrade);
		}
		if (ws_req->host) {
			fprintf(stdout, "Host: %s\r\n", ws_req->host);
		}
		if (ws_req->origin) {
			fprintf(stdout, "Origin: %s\r\n", ws_req->origin);
		}
		if (ws_req->cookie) {
			fprintf(stdout, "Cookie: %s\r\n", ws_req->cookie);
		}
		if (ws_req->sec_websocket_key) {
			fprintf(stdout, "Sec-WebSocket-Key: %s\r\n", ws_req->sec_websocket_key);
		}
		if (ws_req->sec_websocket_version) {
			fprintf(stdout, "Sec-WebSocket-Version: %s\r\n", ws_req->sec_websocket_version);
		}
		fprintf(stdout, "\r\n");
	}
}

#define MAX_WEBSOCKET_04_KEY_LEN 128
static char *generate_key(const char *key, char *res)
{
	int len = strlen(key);
	if (len > MAX_WEBSOCKET_04_KEY_LEN)
		return NULL;
	//sha-1
	char tmp[256];
	memcpy(&tmp[0], key, len);
	memcpy(&tmp[len], "258EAFA5-E914-47DA-95CA-C5AB0DC85B11", 36);
//	string tmp = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	unsigned char md[20] = {0};
	SHA1((const unsigned char*)tmp, len + 36, md);

	//base64 encode
	len = sg_base64_encode(md, 20, res);
	res[len] = '\r';

	return res;
}

char *generate_websocket_response(const char *key, int *len)
{
	static char resp[] = "HTTP/1.1 101 WebSocket Protocol HandShake\r\nConnection: Upgrade\r\nUpgrade: WebSocket\r\nServer: WebChat Demo Server\r\nSec-WebSocket-Accept: 1234567890123456789012345678\r\n\r\n";
	
	if (!generate_key(key, &resp[135]))
		return NULL;
//		resp = (char *)"HTTP/1.1 101 WebSocket Protocol HandShake\r\nConnection: Upgrade\r\nUpgrade: WebSocket\r\nServer: WebChat Demo Server\r\nSec-WebSocket-Accept: 258EAFA5-E914-47DA-95CA-C5AB0DC85B11\r\n\r\n";
		// resp += "HTTP/1.1 101 WebSocket Protocol HandShake\r\n";
		// resp += "Connection: Upgrade\r\n";
		// resp += "Upgrade: WebSocket\r\n";
		// resp += "Server: WebChat Demo Server\r\n";
		// resp += "Sec-WebSocket-Accept: " + generate_key(ws_req->sec_websocket_key) + "\r\n";
		// resp += "\r\n";
	*len = 167;
	return &resp[0];
}

int32_t unmask_payload_data(char *data, int len, unsigned char masking_key[4])
{
	for (int32_t i = 0; i < len; ++i) {
		data[i] = data[i] ^ masking_key[i % 4];
	}
	return 0;
}


bool is_frame_valid(const frame_t *frame)
{
	if (frame && frame->fin <= 1 && frame->opcode <= 0xf && frame->mask == 1) {
		return true;
	}
	return false;
}

int set_frame_head(uint8_t fin, uint8_t opcode, uint64_t payload_len, char *send_data)
{
	assert(send_data);
	if (fin > 1 || opcode > 0xf) {
		return -1;
	}

	char *p = send_data; //buffer
	uint64_t len = 0; //buffer length

	unsigned char c1 = 0x00;
	unsigned char c2 = 0x00;
	c1 = c1 | (fin << 7); //set fin
	c1 = c1 | opcode; //set opcode
//	c2 = c2 | (mask << 7); //set mask

	if (payload_len == 0) {
		*p = c1;
		*(p + 1) = c2;
		len = 2;
	} else if (payload_len <= 125) {
		*p = c1;
		*(p + 1) = c2 + payload_len;
		len = 2;
	} else if (payload_len >= 126 && payload_len <= 65535) {
		*p = c1;
		*(p + 1) = c2 + 126;
		uint16_t tmplen = htons((uint16_t)payload_len);
		memcpy(p + 2, &tmplen, 2);
		len = 4;
	} else if (payload_len >= 65536) {
		*p = c1;
		*(p + 1) = c2 + 127;
		uint64_t tmplen = htonl(payload_len);
		memcpy(p + 2, &tmplen, 8);
		len = 10;
	}

	return len;
}

int32_t parse_frame_header(const char *buf, frame_t *frame)
{
	if (!buf || !frame)
		return -1;

	unsigned char c1 = *buf;
	unsigned char c2 = *(buf + 1);
	frame->fin = (c1 >> 7) & 0xff;
	frame->opcode = c1 & 0x0f;
	frame->mask = (c2 >> 7) & 0xff;
	frame->payload_len = c2 & 0x7f;
	return 0;
}


// int32_t unmask_payload_data(frame_t *frame) {
// 	if (frame && frame->payload_data && frame->payload_len > 0) {
// 		for (int32_t i = 0; i < frame->payload_len; ++i) {
// 			*(frame->payload_data + i) = *(frame->payload_data + i) ^ *(frame->masking_key + i % 4);
// 		}
// 		return 0;
// 	}
// 	return -1;
// }


int32_t unmask_payload_data(const char *masking_key, char *payload_data, uint32_t payload_len) {
	if (!masking_key || !payload_data || payload_len == 0) {
		return -1;
	}
	for (uint32_t i = 0; i < payload_len; ++i) {
		*(payload_data + i) = *(payload_data + i) ^ *(masking_key + i % 4);
	}
	return 0;
}
