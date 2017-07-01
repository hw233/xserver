#include "game_event.h"
#include "conn_node_test.h"
#include "../proto/msgid.h"
#include "../proto/test.pb-c.h"
#include <assert.h>
#include <errno.h>

conn_node_test::conn_node_test()
{
	max_buf_len = 1024 * 1024;
	buf = (uint8_t *)malloc(max_buf_len + sizeof(EXTERN_DATA));
	assert(buf);
}

conn_node_test::~conn_node_test()
{
	conn_node_test::conn_test = NULL;
}

int conn_node_test::recv_func(evutil_socket_t fd)
{
	__attribute__((unused))	PROTO_HEAD *head;
	__attribute__((unused))	MoveAnswer *move_answer;
	__attribute__((unused))	MoveNotify *move_notify;		
	for (;;) {
		int ret = get_one_buf();
		if (ret == 0) {
				//todo
			head = (PROTO_HEAD *)buf_head();
			int cmd = get_cmd();
			printf("get cmd %d\n", cmd);

			switch (cmd)
			{
				case MSG_ID_MOVE_ANSWER:
					move_answer = move_answer__unpack(NULL, (head->len) - sizeof(PROTO_HEAD), get_data());
					printf("move answer %p\n", move_answer);
					move_answer__free_unpacked(move_answer, NULL);
					break;
				case MSG_ID_MOVE_NOTIFY:
					move_notify = move_notify__unpack(NULL, (head->len) - sizeof(PROTO_HEAD), get_data());
					printf("move notify %lu: ", move_notify->playerid);
					for (size_t i = 0; i < move_notify->n_data; ++i)
					{
						printf(", x[%d] y[%d]", move_notify->data[i]->pos_x, move_notify->data[i]->pos_y);
					}
					printf("\n");
					move_notify__free_unpacked(move_notify, NULL);
					break;
					
			}

		}
		
		if (ret < 0) {
			LOG_INFO("%s %d: connect closed from fd %u, err = %d", __FUNCTION__, __LINE__, fd, errno);
			return (-1);		
		} else if (ret > 0) {
			break;
		}
		
		ret = remove_one_buf();
	}
	return (0);
}

