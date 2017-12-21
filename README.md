FxLib
===

FxLib 是一款游戏服务器架构 网络底层实现了 tcp udp websocket 的链接方式 并实现了linux跟win跨平台的功能

内容
-------
Net:                封装的网络传输库
Database:           封装的数据库相关
RedisCon:           封装的redis相关的库
meta_header:        一些公用的函数
common:             公用函数的实现
LuaLib:             封装的lua库
gflags:             依赖的gflags(linux)相关
gflags_win:         依赖的gflags(win)相关
json:               依赖的json相关
lua:                依赖的lua相关
tolua:              依赖的tolua相关
redis-3.2.1:        依赖的redis(linux)相关(源码 库需要编译)
redis-win-3.2.100:  依赖的redis(win)相关(源码 库需要编译)
mysql:              mysql的头文件及运行时库
RerdisTest:         Redis连接的测试
TestServer:         server端的示例
TestClient:         client端的示例
ChatServerManager:  聊天管理服的简单实现
ChatServer:         聊天服的简单实现
GameServer:         配合聊天服的游戏服的简单实现
Debug/scripts:      一些lua的脚本
chat.sql:           聊天服用到的数据库脚本
h5.zip:             聊天服用到的websocket的客户端
gflags-master.zip:  gflags源码
jsoncpp-1.8.3.zip:  json源码
FxLib.sln:          win下工程文件
Makefile:           linux下makefile

编译
-------
windows下 vs2015 打开FxLib.sln 生成
linux下
```
make debug -j 3
```

用法
------
参照 TestServer与TestClient
网络连接有 SLT_CommonTcp,SLT_WebSocket,SLT_Udp 三种 在对应位置修改即可(main.cpp)
mysql数据库连接参考ChatServer(main.cpp)
redis连接参考RedisTest(main.cpp)

```
pListenSocket = pNet->Listen(CSessionFactory::Instance(), SLT_WebSocket, 0, FLAGS_port);
```

```
pListenSocket = pNet->Listen(CSessionFactory::Instance(), SLT_CommonTcp, 0, FLAGS_port);
...
pNet->TcpConnect(g_sSessions[i], dwIP, g_dwPort, true);
```

```
pListenSocket = pNet->Listen(CSessionFactory::Instance(), SLT_Udp, 0, FLAGS_port);
...
pNet->UdpConnect(g_sSessions[i], dwIP, g_dwPort, true);
```
### 运行参数:
TestServer:
gflags
[--port] 默认端口20000

TestClient:
lua
[--ip] 默认 "127.0.0.1"
[--port] 默认 20000

ChatServer:
gflags
[--chat_server_ip] 默认 "127.0.0.1", "Chat Server Ip"
[--chat_session_port] 默认 20000, "Chat Session Port"
[--chat_web_socket_session] 默认 20001, "Web Socket Session Port"
[--chat_server_session_port] 默认 20002, "Chat Server Session Port"
[--chat_server_manager_ip] 默认 "127.0.0.1", "Chat Server Manager Ip"
[--chat_server_manager_port] 默认 13000, "Chat Server Manager Port"
[--db_id] 默认 0, "DB Id"
[--db_port] 默认 3306, "DB Id"
[--character_set] 默认 "utf8", "Character Set"
[--db_name] 默认 "chat", "DB Name"
[--host_name] 默认 "127.0.0.1", "Host Name"
[--login_name] 默认 "test", "Login Name"
[--login_pwd] 默认 "test", "DB PassWord"

ChatServerManager:
gflags
[--chat_server_port] 默认 13000, "chat server listen port"
[--game_server_port] 默认 13001, "game server listen port"
[--gm_port], 默认 12000 "gm listen port"

### 运行示例:
    ./TestServer --port=20000
    ./TestClient --ip=127.0.0.1 --port=20000

### 聊天服运行示例:
bat文件
```
start ChatServerManager.exe

ping -n 5 127.0.0.1
start ChatServer.exe --chat_session_port=30001 --chat_web_socket_session=30002 --chat_server_session_port=30003

::ping -n 1 127.0.0.1
::start ChatServer.exe --chat_session_port=31001 --chat_web_socket_session=31002 --chat_server_session_port=31003

ping -n 1 127.0.0.1
start GameServer.exe
```

关于网络部分
----------
是整套架构较核心 并且写的较精彩的部分 win下用完成端口模型 linux用的epoll模型
tcp的实现较为中规中矩(mytcpsock.cpp)
websocket 是在tcp的上层多加了一次握手(mytcpsock.cpp)
udp靠模拟滑动窗口实现消息的包序列与完整性

