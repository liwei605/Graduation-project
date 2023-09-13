#include "filesearch.h"
#include <string.h>
#include <stdio.h>  
#include <stdlib.h>
 


//判断是否为目录
bool isDir(const char* path)
{
    struct stat st;
    lstat(path, &st);

    return 0 != S_ISDIR(st.st_mode);
}

//遍历文件夹的驱动函数
void FindFiles(const char *path,double times,int epoch)
{
    unsigned long len;
    char temp[MAX_LEN];
    time_t timep;
    time(&timep);
    //去掉末尾的'/'
    len = strlen(path);
    strcpy(temp, path);
    if(temp[len - 1] == '/') temp[len -1] = '\0';
    
    if(access(path, F_OK) != 0)
    {
        perror("Error :");
        return ;
    }

    if(isDir(temp))
    {
        //处理目录
        int recursive = 1;
        findFiles(temp, recursive,times,epoch);
    }
    else   //输出文件
    {
        printf("======> %s\n", path);
        struct stat b;
        //获取文件的元数据
        int s = stat(path,&b);
        if(s == -1) {
            perror("stat");
        }

        //计算文件时间
        // time_t timep;
        // time(&timep);
        
        // //测试。用于计算指纹扫描时间,可删除
        // int filelen;
        // char md5_str[64]={0};
        // //计算指纹
        // filelen = calc_md5(temp,md5_str);
        
        // if(filelen<0)
        // {
        //     perror("ERROR : ");
        //     exit(0);
        // }


        //第一轮无差别扫描
        if(epoch == 1 )
        {
            int filelen;
            char md5_str[64]={0};
            //计算指纹
            filelen = calc_md5(temp,md5_str);
            if(filelen<0)
            {
                perror("ERROR : ");
            }

            if(is_in_hush(b.st_ino,md5_str)==false)  //如果不在哈希表中，就加入哈希表
            {
                myNode->ino_arr[myNode->count_ino] = b.st_ino;
                strcpy(myNode->file_ID[myNode->count_ino],md5_str);
                myNode->count_ino++;
                printf("第一轮扫描==> inode号:%d  |  指纹：%s \n ", b.st_ino,md5_str);
            }else
            {
                printf("%d : 硬链接或者指纹已经存在，无需加入哈希表！\n", b.st_ino);
            }

        }
        else if(difftime(timep, (time_t)b.st_mtime)<times) //创建时间在14内认为是新增文件
        {
            int filelen;
            char md5_str[64]={0};
            //计算指纹
            filelen = calc_md5(temp,md5_str);
         
            if(filelen<0)
            {
                perror("ERROR : ");
            }

            if(is_in_hush(b.st_ino,md5_str)==false)  //如果不在哈希表中，就加入哈希表
            {
                myNode->ino_arr[myNode->count_ino] = b.st_ino;
                strcpy(myNode->file_ID[myNode->count_ino],md5_str);
                myNode->count_ino++;
                printf("inode号:%d  |  指纹：%s | 时间差值%d \n ", b.st_ino,md5_str,timep-(time_t)b.st_mtime);
            }else
            {
                printf("%d : 硬链接或者指纹已经存在，无需加入哈希表！\n", b.st_ino);
            }
        }
    }
}
 
 
 
//遍历文件夹de递归函数
void findFiles(const char *path, int recursive,double times,int epoch)
{
    DIR *pdir;
    struct dirent *pdirent;
    char temp[MAX_LEN];
    pdir = opendir(path);
    if(pdir)
    {
        while((pdirent = readdir(pdir)))
        {
            //跳过"."和".."
            if(strcmp(pdirent->d_name, ".") == 0
               || strcmp(pdirent->d_name, "..") == 0)
                continue;
            sprintf(temp, "%s/%s", path, pdirent->d_name);
            
            //当temp为目录并且recursive为1的时候递归处理子目录
            if(isDir(temp)&& recursive)
            {
                findFiles(temp, recursive,times,epoch);
            }
            else
            {
                printf("======> %s\n", temp);   //目录下的文件在这里输出
                struct stat b;
                //获取文件的元数据
                int s = stat(temp,&b);
                if(s == -1) {
                    perror("stat");
                }

                // //测试。用于计算指纹扫描时间,可删除
                // int filelen;
                // char md5_str[64]={0};
                // //计算指纹
                // filelen = calc_md5(temp,md5_str);
                
                // if(filelen<0)
                // {
                //     perror("ERROR : ");
                //    // exit(0);
                // }

                //计算文件时间
                time_t timep;
                time(&timep);

                //第一轮无差别扫描
                if(epoch == 1 )
                {
                    int filelen;
                    char md5_str[64]={0};
                    //计算指纹
                    filelen = calc_md5(temp,md5_str);
                    if(filelen<0)
                    {
                        perror("ERROR : ");
                    }
                    
                    if(is_in_hush(b.st_ino,md5_str)==false)  //如果不在哈希表中，就加入哈希表
                    {
                        myNode->ino_arr[myNode->count_ino] = b.st_ino;
                        strcpy(myNode->file_ID[myNode->count_ino],md5_str);
                        myNode->count_ino++;
                        printf("第一轮扫描==> inode号:%d  |  指纹：%s \n ", b.st_ino,md5_str);
                    }else
                    {
                        printf("%d : 硬链接或者指纹已经存在，无需加入哈希表！\n", b.st_ino);
                    }
                }
                else if(difftime(timep, (time_t)b.st_mtime)<=times) //创建时间距离当前系统时间在14内认为是新增文件
                {
                    int filelen;
                    char md5_str[64]={0};
                    //计算指纹
                    filelen = calc_md5(temp,md5_str);
                
                    if(filelen<0)
                    {
                        perror("ERROR : ");
                        exit(0);
                    }

                    if(is_in_hush(b.st_ino,md5_str)==false)  //如果不在哈希表中，就加入哈希表
                    {
                        myNode->ino_arr[myNode->count_ino] = b.st_ino;
                        strcpy(myNode->file_ID[myNode->count_ino],md5_str);
                        myNode->count_ino++;
                        printf("inode号:%d  |  指纹：%s | 时间差值%d \n ", b.st_ino,md5_str,timep-(time_t)b.st_mtime);
                    }else
                    {
                         printf("%d : 硬链接或者指纹已经存在，无需加入哈希表！\n", b.st_ino);
                    }
                }
            }
        }
    }
    else
    {
        printf("opendir error:%s\n", path);
    }
    closedir(pdir);
}

 void creat_htab()
 {
    printf(".........创建哈希表\n");

    myNode=(Datasend*)malloc(sizeof(Datasend));

    myNode->count_ino=0;

    printf(".........内存分配完毕\n");
 }

void testhush()
{
    printf("================执行测试===================\n");

    for (int i = 0; i < myNode->count_ino; i++)
    {
        printf("%d : %s \n",myNode->ino_arr[i],myNode->file_ID[i]);
    }
    printf("================测试结束===================\n");
}

bool is_in_hush(ino_t ino_num,char md5_str[])
{
    for (int i = 0; i < myNode->count_ino; i++)
    {
        if(myNode->ino_arr[i]==ino_num)
        {
            return true;
        }
        if(strcmp( myNode->file_ID[i],md5_str)==0)
        {
            return true;
        }
    }
    return false;
}

 void destroy_htab()
 {
    printf(".........销毁哈希表\n");
   
    myNode->count_ino=0;
    free(myNode);
    printf(".........销毁完毕\n");
 }