#include "utils/platform/platform.h"
#include "http/http.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include <stdio.h>
#include <string.h>

void test1(const HttpRequest* req, HttpResponse* res) {
    http_response_status_ok(res);
    http_response_set_text(res, "test1");
}

void test2(const HttpRequest* req, HttpResponse* res) {
    http_response_status_ok(res);
    http_response_set_text(res, "test2");
}

int main(void)
{
    net_init();

    NetSocket* server = net_tcp_listen("0.0.0.0", 7878);
    register_get_route("/test1", test1);
    register_get_route("/test2", test2);

    while (1) {
        NetSocket* client = net_accept(server);
        if (!client) continue;
        handle_client(server, client);
        net_close(client);
    }

    net_shutdown();
    return 0;
}