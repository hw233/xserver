#include "ai_srv.h"

typedef int (*install_func)(int argc, char **argv);
typedef int (*reload_func)();
typedef int (*uninstall_func)();
typedef void (*http_request_func)(struct evhttp_request *req, void *arg);
typedef void (*cb_timer_func)();

listen_node_aisrv        server_listener;
static http_request_func g_http_request_func;
static cb_timer_func     g_cb_timer_func;
ai_recv_func             g_ai_recv_func;

extern int init_signals();

void *report_callback(void *lparam);

static int install_so(int argc, char **argv);
static int  reload_so();
static void uninstall_so();
static void generic_request_handler(struct evhttp_request *req, void *arg)
{
    //	std::map<std::string, std::string> param_map;
    //	char *param = strchr(req->uri, '?');

    if (strcmp(req->uri, "/reload") == 0)
    {
        uninstall_so();
        reload_so();
        struct evbuffer *returnbuffer = evbuffer_new();

        evbuffer_add_printf(returnbuffer, "reload aisrv so success!\n");
        evhttp_send_reply(req, HTTP_OK, "Client", returnbuffer);
        evbuffer_free(returnbuffer);
        return;
    }

    if (g_http_request_func)
        g_http_request_func(req, arg);
}

extern struct event_base *event_global_current_base_;
void init_http_server(uint32_t port)
{
    if (port <= 0)
        return;
    short          http_port   = port;
    const char *   http_addr   = "0.0.0.0";
    struct evhttp *http_server = NULL;

    event_global_current_base_ = base;
    http_server                = evhttp_start(http_addr, http_port);
    assert(http_server);
    evhttp_set_gencb(http_server, generic_request_handler, NULL);
}

static void *so_aisrv;
static void  set_so_funcs()
{
    g_http_request_func = (http_request_func)dlsym(so_aisrv, "on_http_request");
    g_cb_timer_func     = (cb_timer_func)dlsym(so_aisrv, "cb_aisrv_timer");
    g_ai_recv_func      = (ai_recv_func)dlsym(so_aisrv, "ai_recv_func");

    if (!g_http_request_func)
        LOG_ERR("%s: %d", __FUNCTION__, __LINE__);
    if (!g_cb_timer_func)
        LOG_ERR("%s: %d", __FUNCTION__, __LINE__);
    if (!g_ai_recv_func)
        LOG_ERR("%s: %d", __FUNCTION__, __LINE__);
}
static int install_so(int argc, char **argv)
{
    so_aisrv = dlopen("./so_ai_srv/libaisrv.so", RTLD_NOW | RTLD_GLOBAL);

    if (!so_aisrv)
    {
        LOG_ERR("dlerror = %s", dlerror());
        return (-1);
    }
    install_func t = (install_func)dlsym(so_aisrv, "install");
    if (!t)
    {
        LOG_ERR("dlsym install failed");
        return (-1);
    }
    set_so_funcs();
    return t(argc, argv);
}

static int reload_so()
{
    so_aisrv = dlopen("./so_ai_srv/libaisrv.so", RTLD_NOW | RTLD_GLOBAL);
    if (!so_aisrv)
    {
        LOG_ERR("dlerror = %s", dlerror());
        return (-1);
    }
    reload_func t = (reload_func)dlsym(so_aisrv, "reload");
    if (!t)
    {
        LOG_ERR("dlsym install failed");
        return (-1);
    }
    set_so_funcs();
    return t();
}

static void uninstall_so()
{
    assert(so_aisrv);
    uninstall_func t = (uninstall_func)dlsym(so_aisrv, "uninstall");
    if (!t)
    {
        LOG_ERR("err = %s\n", dlerror());
    }
    else
    {
        t();
    }

    g_http_request_func = NULL;
    g_cb_timer_func     = NULL;
    g_ai_recv_func      = NULL;

    dlclose(so_aisrv);
}

static struct event aisrv_event_timer;
struct timeval      aisrv_timeout;
uint64_t            timer_loop_count;
void                cb_aisrv_timer(evutil_socket_t, short, void * /*arg*/)
{
    if (g_cb_timer_func)
        g_cb_timer_func();
    add_timer(aisrv_timeout, &aisrv_event_timer, NULL);
}

int main(int argc, char **argv)
{
    srandom(time(NULL));
    srand(time(NULL));

    int ret = log4c_init();
    if (ret != 0)
    {
        printf("log4c_init failed[%d]\n", ret);
        return (ret);
    }

    init_mycat();
    if (!mycat)
    {
        printf("log4c_category_get(\"six13log.log.app.application1\"); failed\n");
        return (0);
    }

    if (install_so(argc, argv) != 0)
    {
        LOG_ERR("install so failed");
        return (0);
    }

    aisrv_event_timer.ev_callback = cb_aisrv_timer;
    add_timer(aisrv_timeout, &aisrv_event_timer, NULL);

    ret = event_base_loop(base, 0);
    LOG_INFO("event_base_loop stoped[%d]", ret);

    struct timeval tv;
    event_base_gettimeofday_cached(base, &tv);

    LOG_INFO("ai_srv stoped[%d]", ret);
    return (ret);
}
