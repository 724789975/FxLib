protoc --cpp_out=./ web_data.proto
protoc --cpp_out=./ web_game.proto
cp *.h ../Server/WebGame/msg_proto
cp *.h ../Server/WebGameCenter/msg_proto
cp *.h ../Server/WebGameLogin/msg_proto
cp *.h ../Server/WebGameTeam/msg_proto
cp *.h ../Server/WebGameManager/msg_proto
cp *.cc ../Server/WebGame/msg_proto
cp *.cc ../Server/WebGameCenter/msg_proto
cp *.cc ../Server/WebGameLogin/msg_proto
cp *.cc ../Server/WebGameTeam/msg_proto
cp *.cc ../Server/WebGameManager/msg_proto
protoc --csharp_out=./ web_data.proto
protoc --csharp_out=./ web_game.proto
cp *.cs ../Client_Unity/Test/Assets/Scripts/msg_proto
#pause
