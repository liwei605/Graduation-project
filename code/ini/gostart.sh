gnome-terminal -t "node1_check 127.1.0.1 key:1234 " -x bash -c "cd /home/liwei/桌面/code/datanode;./start.sh;exec bash;"
gnome-terminal -t "node2_check 127.1.0.2 key:1235 " -x bash -c "cd /home/liwei/桌面/code/datanode;./start.sh;exec bash;"
gnome-terminal -t "node3_check 127.1.0.3 key:1236 " -x bash -c "cd /home/liwei/桌面/code/datanode;./start.sh;exec bash;"

gnome-terminal -t "node1_server 127.1.0.1 : 1111 " -x bash -c "cd /home/liwei/桌面/code/mylibtest;./start.sh;exec bash;"
gnome-terminal -t "node2_server 127.1.0.2 : 2222 " -x bash -c "cd /home/liwei/桌面/code/mylibtest;./start.sh;exec bash;"
gnome-terminal -t "node3_server 127.1.0.3 : 3333 " -x bash -c "cd /home/liwei/桌面/code/mylibtest;./start.sh;exec bash;"
