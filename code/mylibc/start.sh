rm mymalloc.so
gcc -DRUNTIME -shared -fpic -o mymalloc.so mymalloc.c md5.c -lssl -lcrypto -ldl -lpthread
gcc main.c -o main
LD_PRELOAD="./mymalloc.so" ./main