# aarch64-linux-gnu-g++ -static metadatanode.c metadatanode_main.c -o go_main_64.out -lpthread
# ./go_main_64.out

/usr/bin/gcc-9 -fdiagnostics-color=always -g metadatanode.c metadatanode_main.c  -o a.out -lpthread
./a.out