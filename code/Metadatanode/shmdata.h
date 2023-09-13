#include <stdlib.h>
#ifndef _SHMDATA_H_HEADER
#define _SHMDATA_H_HEADER
 
#define TEXT_SZ 2048
#define LIST_LENGTH 1024

struct shared_use_st
{
    int written ; // 作为一个标志，非0：表示可读，0：表示可写
    char text[TEXT_SZ]; // 记录写入 和 读取 的文本
};
 struct redirect_addr
{
    char old_path[TEXT_SZ]; // 记录原本被删除的地址
    ino_t inonumber;
    char IP_addr[50];
};

struct redirect_list
{
    int mutex;   //1表示内存可用，0表示内存不可用
    struct redirect_addr list[LIST_LENGTH];
    int index_map[LIST_LENGTH];
    int count;
};

#endif