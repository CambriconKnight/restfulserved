<p align="center">
    <a href="https://gitee.com/cambriconknight/restfulserved">
        <img alt="RESTfulServed" src="./res/restful.jpeg" height="140" />
        <h1 align="center">RESTFUL服务组件</h1>
    </a>
</p>

[![Build Status](./res/passing.png)](https://gitee.com/cambriconknight/restfulserved)

# 概述

RESTfulServed 是一个用于构建高性能REST Web服务器的C++库。建立在[Boost.ASIO](http://www.boost.org)为开发人员提供一个简单的API，以便在 C++ 程序中创建HTTP服务。

**特点:**

* [x] HTTP 1.1 compatible request parser
* [x] Middleware / plug-ins
* [x] Flexible handler API
* [x] Cross-platform compatible

# 安装

## 环境要求

名称          | 详细信息       | 备注
------------ | ------------- | -------------
Linux        | GCC 4.8       |
OSX          | Clang 3.5     |
Boost        | 1.53 or newer | http://www.boost.org/
Ragel        | --            | http://www.complang.org/ragel

## 下载编译

```bash
git clone https://gitee.com/cambriconknight/restfulserved
cd restfulserved && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=./restfulserved_install
make -j4
make install
```

**编译选项**

Option                 | Purpose
---------------------- | -----------------------------------
SERVED_BUILD_SHARED    | Build shared library
SERVED_BUILD_STATIC    | Build static library
SERVED_BUILD_TESTS     | Build unit test suite
SERVED_BUILD_EXAMPLES  | Build bundled examples
SERVED_BUILD_DEB       | Build DEB package (note: you must also have dpkg installed)
SERVED_BUILD_RPM       | Build RPM package (note: you must also have rpmbuild installed)

## 交叉编译

```bash
git clone https://gitee.com/cambriconknight/restfulserved
cd restfulserved && mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/ToolChainEdge.cmake \
        -DCMAKE_INSTALL_PREFIX=./restfulserved_install
make -j4
make install
```

## 快速上手

### 开发

The most basic example of creating a server and handling a `HTTP GET` for the path `/hello`:
```cpp
#include <served/served.hpp>

int main(int argc, char const* argv[]) {
	// Create a multiplexer for handling requests
	served::multiplexer mux;

	// GET /hello
	mux.handle("/hello")
		.get([](served::response & res, const served::request & req) {
			res << "Hello world!";
		});

	// Create the server and run with 10 handler threads.
	served::net::server server("0.0.0.0", "8080", mux);
	server.run(10);

	return (EXIT_SUCCESS);
}
```

### 测试

To test the above example, you could run the following command from a terminal:
```bash
$ curl http://localhost:8080/hello -ivh
```

### 其他

You can also use named path variables for REST parameters:
```cpp
mux.handle("/users/{id}")
	.get([](served::response & res, const served::request & req) {
		res << "User: " << req.params["id"];
	});
```

To test the above example, you could run the following command from a terminal:
```bash
$ curl http://localhost:8080/users/dave -ivh
```

If you need to be more specific, you can specify a pattern to use to validate the parameter:
```cpp
mux.handle("/users/{id:\\d+}")
	.get([](served::response & res, const served::request & req) {
		res << "id: " << req.params["id"];
	});
```

To test the above example, you could run the following command from a terminal:
```bash
$ curl http://localhost:8080/users/1 -ivh
```

Method handlers can have arbitrary complexity:
```cpp
mux.handle("/users/{id:\\d+}/{property}/{value:[a-zA-Z]+")
	.get([](served::response & res, const served::request & req) {
		// handler logic
	});
```

If you want to automatically log requests, you could use a plugin (or make your own):
```cpp
#include <served/plugins.hpp>
// ...
mux.use_after(served::plugin::access_log);
```

You can also access the other elements of the request, including headers and
components of the URI:
```cpp
mux.handle("/posts/{id:\\d+}")
	.post([](served::response & res, const served::request & req) {
		if (req.header("Content-Type") != "application/json") {
			served::response::stock_reply(400, res);
			return;
		}
		res << req.url().fragment();
	});
```
