#ifndef GAME_EVENT_H__
#define GAME_EVENT_H__
#include <errno.h>
#include <stdlib.h>
#include "listen_node.h"
#include "event2/event.h"
#include "event2/event_compat.h"
//#include "event-internal.h"

extern "C" {
#include "log4c.h"
}

#ifdef LOG_FORMAT_CHECK  /*only for format check*/
#define LOG_ERR   printf
#define LOG_INFO   printf
#define LOG_DEBUG   printf
#else
#ifdef LOG_ERR
#undef LOG_ERR
#endif // LOG_ERR

#define LOG_ERR(fmt, arg...) 			log4c_category_log(mycat, LOG4C_PRIORITY_ERROR, fmt, ##arg);

#ifdef LOG_INFO
#undef LOG_INFO
#endif // LOG_INFO

#define LOG_INFO(fmt, arg...) 			log4c_category_log(mycat, LOG4C_PRIORITY_INFO, fmt, ##arg);

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif // LOG_DEBUG
#define LOG_DEBUG(fmt, arg...) 			log4c_category_log(mycat, LOG4C_PRIORITY_DEBUG, fmt, ##arg);
#endif

extern struct event_base *base;
extern log4c_category_t* mycat;

int game_event_init();
int game_add_listen_event(uint16_t port, listen_node_base *callback, const char *name);
int game_add_connect_event(struct sockaddr *sa, int socklen, conn_node_base *client);

void remove_listen_callback_event(conn_node_base *client);
int create_new_socket(int set_opt);
int add_timer(struct timeval t, struct event *event_timer, void *arg);
int add_signal(int signum, struct event *event, event_callback_fn callback);

void init_mycat();
void change_mycat();

#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

#endif
