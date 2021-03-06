find ./ -name '*log.txt' | xargs rm -rf

#删除共享内存
ipcs -m | awk '$2 ~/[0-9]+/{print $2}' | while read s; do ipcrm -m $s; done
#删除进程锁
ipcs -s | awk '$2 ~/[0-9]+/{print $2}' | while read s; do ipcrm -s $s; done

./WebGameCenter --server_port 40000 &

sleep 1

./WebGameManager --server_id 30001 --server_ip 127.0.0.1 --server_port 30001 --player_port 30002 --center_ip 127.0.0.1 --center_port 40000 &

#sleep 1
#./WebGameManager --server_id 30002 --server_ip 127.0.0.1 --server_port 30011 --player_port 30012 --center_ip 127.0.0.1 --center_port 40000 &

sleep 1
./WebGameTeam --server_id 20001 --center_ip 127.0.0.1 --center_port 40000 -- game_manager_port 20001 &

#sleep 1
#./WebGameTeam --server_id 20002 --center_ip 127.0.0.1 --center_port 40000 -- game_manager_port 20011 &

sleep 1
./WebGameLogin --server_id 10001 --player_port 11001 --center_ip 127.0.0.1 --center_port 40000 --login_port 11002 --team_port 11003 --game_manager_port 11004 &

#sleep 1
#./WebGameLogin --server_id 10002 --player_port 11011 --center_ip 127.0.0.1 --center_port 40000 --login_port 11012 --team_port 11013 --game_manager_port 11014 &
