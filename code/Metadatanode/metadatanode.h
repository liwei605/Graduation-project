#ifndef METADATANODE_H
#define METADATANODE_H

#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <arpa/inet.h>
#include <sys/dir.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>  

#define HELLO_WORLD_SERVER_PORT    6666
// #define HEART_PORT    1111
#define LENGTH_OF_LISTEN_QUEUE     20
#define BUFFER_SIZE                1024
#define MAX_MDList_SIZE            2048
#define META_RECV                   0
#define META_REDIRECT               1
#define MAX_CLIENTS 100 // 最大并发连接数
typedef struct 
{
    int count_ino;
    char  file_ID[100][65];
    ino_t ino_arr[100];

}Datasend;


typedef struct 
{
    int MSG;
    int count_ino;
    ino_t ino_arr_del[100];
    ino_t ino_arr_redir[100];
    char IP_datanode[100][50];
}Instruction;

struct MetaData
{
   char ID_file[65];
   ino_t ino;
   char IP_client[50];
   int sharenum;
};
typedef struct 
{
    int index;
    char type;
}backindex;

static struct MetaData MDList[MAX_MDList_SIZE];  //列表，存放MetaData结构
static int index_MD_List[MAX_MDList_SIZE];   //列表索引
static int total_number=0;              //列表元素计数
pthread_mutex_t mutexglobel;           //列表上锁互斥访问

static char save_IP_client[LENGTH_OF_LISTEN_QUEUE][50];
static int total_client=0;
// 全局互斥锁
pthread_mutex_t mutex;

//接受来自datanode的元数据,并将数据存入缓冲区
void Socket_rev_meta();

//线程函数监听客户端的元数据请求
void * listening_client_meta(void * argv);

//传入指纹，判断指纹是否存在
backindex is_in_List(const char ID_file[],const char IP_client[],const ino_t ino);

//插入指纹表
void add_list(const char ID_file[],const ino_t ino,const char IP_client[]);

//打印MDlist
void print_MD_list();

//查找是否在接入表中
bool is_in_save_table(const char IPClient[50]);



//高并发模型

#endif