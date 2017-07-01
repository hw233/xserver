#include "conn_node_test.h"
#include "cmd_impl.h"
#include "server_proto.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include "../proto/test.pb-c.h"
#include "../proto/msgid.h"

static uint8_t send_buf[1024];
__attribute__((unused)) static PROTO_HEAD *head = (PROTO_HEAD *)&send_buf[0];
__attribute__((unused)) static uint8_t *send_data = &send_buf[sizeof(PROTO_HEAD)];

int cmd_impl::show_help()
{
	printf("help\n");
	return (0);
}

int cmd_impl::do_cmd_main(int argc, char *argv[])
{
	if (argc < 1)
		return (-1);
	if (strcmp(argv[0], "help") == 0)
		return show_help();
	else if (strcmp(argv[0], "move") == 0)
		return move_main(argc, argv);
	return (0);
}

int cmd_impl::move_main(int argc, char *argv[])
{
	if (argc < 3)
	{
		printf("move posx posy posx posy ...\n");
		return -1;
	}
	size_t size;
	MoveRequest req;
	move_request__init(&req);
	req.n_data = (argc - 1) / 2;
	PosData data[req.n_data];
	PosData *pdata[req.n_data];
	for (size_t i = 0; i < req.n_data; ++i)
	{
		pdata[i] = &data[i];
		pos_data__init(&data[i]);
		data[i].pos_x = atoi(argv[1 + i * 2]);
		data[i].pos_y = atoi(argv[2 + i * 2]);		
	}
	req.data = &pdata[0];

	size = move_request__pack(&req, send_data);
	head->len = ENDION_FUNC_4(sizeof(PROTO_HEAD) + size);
	head->msg_id = ENDION_FUNC_2(MSG_ID_MOVE_REQUEST);
	if (conn_node_test::conn_test)
		conn_node_test::conn_test->send_one_msg(head, 1);
	return (0);		
}
	
