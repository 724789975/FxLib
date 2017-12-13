del *log.txt

start ChatServerManager.exe

ping -n 5 127.0.0.1
start ChatServer.exe --chat_session_port=30001 --chat_web_socket_session=30002 --chat_server_session_port=30003

::ping -n 1 127.0.0.1
::start ChatServer.exe --chat_session_port=31001 --chat_web_socket_session=31002 --chat_server_session_port=31003

ping -n 1 127.0.0.1
start GameServer.exe
