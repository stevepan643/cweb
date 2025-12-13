#include "utils/platform/platform.h"
#include "http/http.h"
#include "http/http_request.h"
#include "http/http_response.h"
#include <stdio.h>
#include <string.h>

#define PAGE(x) void x(const HttpRequest* req, HttpResponse* res)

// -------- 接口实现 --------

// 返回 HTML 页面
PAGE(index_page) {
    http_response_status_ok(res);
    http_response_set_file(res, "../test/index.html");  // HTML 文件路径，因爲CMake的可執行文件在build中
}

// GET 接口
PAGE(test_get) {
    http_response_status_ok(res);
    http_response_set_text(res, "GET success");
}

// POST 接口
PAGE(test_post) {
    http_response_status_ok(res);
    size_t len;
    const char* body = http_request_get_body(req, &len);
    http_response_set_text(res, body ? body : "POST success");
}

// PUT 接口
PAGE(test_put) {
    http_response_status_ok(res);
    size_t len;
    const char* body = http_request_get_body(req, &len);
    http_response_set_text(res, body ? body : "PUT success");
}

// DELETE 接口
PAGE(test_delete) {
    http_response_status_ok(res);
    http_response_set_text(res, "DELETE success");
}

int main(void)
{
    net_init();

    NetSocket* server = net_tcp_listen("0.0.0.0", 7878);

    // 注册路由
    register_get_route("/", index_page);            // 访问根目录返回 HTML
    register_get_route("/test_get", test_get);
    register_post_route("/test_post", test_post);
    register_put_route("/test_put", test_put);
    register_delete_route("/test_delete", test_delete);

    while (1) {
        NetSocket* client = net_accept(server);
        if (!client) continue;
        handle_client(server, client);
        net_close(client);
    }

    net_shutdown();
    return 0;
}
