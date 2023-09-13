#ifndef FILESEARCH_H
#define FILESEARCH_H
#include <stdio.h>
#include <sys/dir.h>
#include <string.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <unistd.h>
#include <time.h>
#include "md5.h"
#include <search.h>
#include <stdlib.h>

#define MAX_LEN 512 * 256 

typedef struct 
{
    int count_ino;
    char file_ID[100][65];
    ino_t ino_arr[100];

}Datasend;

Datasend *myNode;

//创建键值表
 void creat_htab();
//销毁键值表
 void destroy_htab();

//判断是否为文件夹
bool isDir(const char* path);
 
//遍历文件夹的驱动函数
void FindFiles(const char *path,double times,int epoch);
 
//遍历文件夹de递归函数
void findFiles(const char *path, int recursive,double times,int epoch);
 
//确认哈希表
bool is_in_hush(ino_t ino_num,char md5_str[]);

//哈希测试函数
void testhush();

#endif 