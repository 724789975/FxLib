protoc.exe --cpp_out=./ web_game.proto
cp *.h ../Server/WebGame/msg_proto
cp *.h ../Server/WebGameManager/msg_proto
cp *.cc ../Server/WebGame/msg_proto
cp *.cc ../Server/WebGameManager/msg_proto
protoc.exe --csharp_out=./ web_game.proto
cp *.cs ../Client_Unity/Test/Assets/Scripts/msg_proto
#pause
