# CWeb - A Lightweight C Web Framework

CWeb 是一個輕量級的跨平臺的 C 語言 Web 框架，旨在提供簡單、直觀、可擴展的 HTTP 服務能力，方便學習與實驗。

CWeb is a lightweight C web framework designed to provide simple, intuitive, and extensible HTTP server functionality, 
suitable for learning and experimental projects.

## 功能 Features

### 已實現 ✅

- 支持基本 HTTP 方法：GET / POST / PUT / DELETE
- 返回靜態 HTML 文件、JSON 和純文本響應
- 簡單路由系統，支持不同接口函數綁定
- TCP 網絡封裝，跨平台接口初步設計
- 日誌系統，支持請求、響應與錯誤記錄

### Implemented ✅

- Supports basic HTTP methods: GET / POST / PUT / DELETE
- Serve static HTML files, JSON and plain text responses
- Simple routing system with handler function binding
- Basic TCP networking abstraction for cross-platform use
- Logging system for requests, responses, and errors

### 不久將來計劃 🎯

- 支持多種文件類型（HTML / CSS / JS / JSON / images 等）
- 更智能的資源查找，支持自定義路徑和相對路徑
- 後臺終端命令接口，方便服務運維
- 多線程優化，提高並發處理能力
- 內存管理優化，避免內存泄漏和重複分配
- 配置文件支持，方便定義端口、路徑和參數

### Near-future Roadmap 🎯

- Support multiple file types (HTML / CSS / JS / JSON / images, etc.)
- Smarter resource locating, with custom and relative paths
- Background terminal command interface for server management
- Multithreading support for better concurrency
- Memory management optimization to prevent leaks and redundant allocation
- Configuration file support for defining port, paths, and parameters

### 遠期規劃 🔮

- 數據庫支持（SQLite / MySQL / PostgreSQL 等）
- RESTful API 與 JSONP 支持
- 模板渲染引擎，方便動態頁面生成
- 安全性增強（HTTPS / 請求驗證 / CSRF 保護）
- 高級緩存和資源管理，提高性能
- 模塊化插件系統，便於第三方功能擴展

### Long-term Vision 🔮

- Database integration (SQLite / MySQL / PostgreSQL, etc.)
- RESTful API with JSONP support
- Template rendering engine for dynamic page generation
- Security enhancements (HTTPS / authentication / CSRF protection)
- Advanced caching and resource management for better performance
- Modular plugin system for third-party extensions

## 快速嘗試 Quick to Try 🚀

1. 克隆項目 Clone the repository:
```bash
git clone https://github.com/yourusername/cweb.git
cd cweb
```

2. 用 CMake 構建測試項目 Build the test using CMake:
```bash
mkdir cweb_test_build
cd cweb_test_build
cmake ../test
cmake --build .
```

3. 運行服務器 Run the server
```bash
./cweb_test
```

1. 訪問：http://127.0.0.1:7878 Access: http://127.0.0.1:7878

## 使用方法 / Usage 📦

1. 定義路由和接口函數 Define routes and handler functions
```c
register_get_route("/", index_page);
register_post_route("/test_post", test_post);
```

2. 實現接口函數 Implement handler functions
```c
PAGE(test_get) {
    http_response_status_ok(res);
    http_response_set_text(res, "GET success");
}
```

3. 啟動服務器，接受請求 Start the server and handle requests
```c
while (1) {
    NetSocket* client = net_accept(server);
    handle_client(server, client);
    net_close(client);
}
```

## 目標與願景 Goals & Vision ✨

CWeb 希望成為一個 輕量、靈活、可擴展 的 C 語言 Web 框架，既能作為學習和實驗平台，也可以逐步支持小型生產環境。

CWeb aims to be a lightweight, flexible, and extensible C web framework suitable for learning, experimentation, and gradually small-scale production use.

## 许可证 License 📄

MIT License © 2025
