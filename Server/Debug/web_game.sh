find ./ -name '*log.txt' | xargs rm

./WebGameCenter --server_port 40000 &

sleep 1

./WebGameManager --server_id 30001 --server_port 30001 --server_port 3o002 --center_ip 127.0.0.1 --center_port 40000 &
sleep 1
./WebGameManager --server_id 30002 --server_port 30011 --server_port 30012 --center_ip 127.0.0.1 --center_port 40000 &

sleep 1

./WebGameLogin --server_id 10001 --player_port 11001 --center_ip 127.0.0.1 --center_port 40000 --logoin_port 11002 --team_port 11003 --game_manager_port 11004 &
sleep 1
./WebGameLogin --server_id 10002 --player_port 11011 --center_ip 127.0.0.1 --center_port 40000 --logoin_port 11012 --team_port 11013 --game_manager_port 11014 &
