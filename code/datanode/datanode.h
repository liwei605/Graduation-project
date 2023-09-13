#ifndef DATANODE_H
#define DATANODE_H
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <threads.h>
#include <sys/stat.h>
#include <sys/wait.h>
#define _GNU_SOURCE
#include <search.h>
#include <pthread.h>
#include "datanode.h"
#include <sys/socket.h>
#include "filesearch.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include"shmdata.h"
#include <sys/shm.h>


// #define MYPORT  6666
// #define HEARTPORT  6665
#define BUFFER_SIZE 1024
#define META_RECV                   0
#define META_REDIRECT               1


//在ini文件中配置
int MYPORT;   //Meta端口
int MEM_KEY;   //内存key 
char MEtE_IP[100];  //Meta ip
char DATA_IP[100];  //本地 ip
char DIR_LOAD[101];  //挂载路径

typedef struct 
{
    int MSG;
    int count_ino;
    ino_t ino_arr_del[100];
    ino_t ino_arr_redir[100];
    char IP_datanode[100][50];
}Instruction;

//获取主机IP地址
void getHostIP(char ipbuf[20]);

//周期文件检测线程函数
void * clock_filesearch(void * argv);

//套接子发送索引表
void Socdket_Send(struct redirect_list *shared,void *shm);

//重删函数
void del_inode(const ino_t inode);

//内存共享函数
void store_memory(const Instruction *instr,struct redirect_list * shared,void *shm,int i);

//获取共享表的空位置索引
int get_index(int index_map[]);

//根据inode号获取文件路径,返回应链接个数
int GetPathByInode(const ino_t inode,char pathbuffer[][TEXT_SZ]);

//系统调用管道返回文件名字
void ExecCmd(char * in,char out[]);

//返回重定向表中绝对路径数组的空位置索引
int get_old_path_index(char pathbuffer[][TEXT_SZ]);
#endif
