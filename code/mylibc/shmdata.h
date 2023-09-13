#ifndef _SHMDATA_H_HEADER
#define _SHMDATA_H_HEADER
#include<stdio.h>
#include<stdbool.h>
#include<sys/types.h>

#define TEXT_SZ 1024   //最大路径长度
#define MAX_HARD_LINK 5  //最多5个硬链接
#define LIST_LENGTH 1024

#define server_port 3333   //服务器端口

// #define DIR_LOAD "/home/liwei/桌面/code/Nodes/node3"     //设置挂载目录

// int kEY = 1234;   //内存挂载

typedef  void *(* fun_t)(const char * __filename, const char * __modes);
typedef void *(* fun_t_2)(FILE *__stream);
struct shared_use_st
{
    int written ; // 作为一个标志，非0：表示可读，0：表示可写
    char text[TEXT_SZ]; // 记录写入 和 读取 的文本
};
 struct redirect_addr
{
    char old_path[MAX_HARD_LINK][TEXT_SZ]; // 记录原本被删除的地址，最多5个硬链接
    int old_path_number;
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

struct alocate_info
{
    struct redirect_list * shared;
    void *shm;
    int shmid;
};


struct fopen_r_para
{
    fun_t fopenp;//函数指针
    char filename[TEXT_SZ];//文件绝对路径
    FILE * fback; //返回
    char open_type[4]; //文件的读写方式
    bool is_local;
};

struct file_link
{
    char filename[TEXT_SZ];//文件绝对路径
    int link;
    char md5_str[128];
    bool is_local;
};


#endif