#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <stdint.h>

//WebSocket request
typedef struct WebsocketRequest {
	char *req;
	char *connection;
	char *upgrade;
	char *host;
	char *origin;
	char *cookie;
	char *sec_websocket_key;
	char *sec_websocket_version;
} ws_req_t;

typedef struct Frame {
	uint8_t fin;
	uint8_t opcode;
	uint8_t mask;
	uint64_t payload_len;
	unsigned char masking_key[4];
//	char *payload_data;
} frame_t;

int32_t unmask_payload_data(char *data, int len, unsigned char masking_key[4]);
int set_frame_head(uint8_t fin, uint8_t opcode, uint64_t payload_len, char *send_data);
char *get_websocket_request_key(char *s_req);
int32_t parse_websocket_request(char *s_req, ws_req_t *ws_req);
void print_websocket_request(const ws_req_t *ws_req);
char *generate_websocket_response(const char *key, int *len);
int32_t parse_frame_header(const char *buf, frame_t *frame);
bool is_frame_valid(const frame_t *frame);
#endif /* WEBSOCKET_H */
