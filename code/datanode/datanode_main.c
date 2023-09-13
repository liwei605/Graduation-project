#include "datanode.h"
#include "filesearch.h"
#include "md5.h"
#include <time.h>
#include <pthread.h> 
#include<dirent.h>
#include <fcntl.h>

#define FILE_DISC_PARTS_CHECK "node1"

// int stat(const char *path, struct stat *buf);

// 功能：
//   获取文件的信息
// 参数：
//   path：指定了文件的名字
//   buf：用来存储文件的属性的信息（元数据）
//   返回值：0 成功
//   -1 错误 errno被设置

//   第二个参数里包含了文件的许多信息，所以使用了结构体：
// struct stat：这个结构体中成员的信息：
// struct stat {
//   dev_t     st_dev;     /* ID of device containing file */    设备号
//   ino_t     st_ino;     /* inode number */      inode号（索引号）
//   mode_t    st_mode;    /* protection */        文件对应的类型，权限
//   nlink_t   st_nlink;   /* number of hard links */     硬链接数
//   uid_t     st_uid;     /* user ID of owner */      文件所有者
//   gid_t     st_gid;     /* group ID of owner */       文件所有者对应的组
//   dev_t     st_rdev;    /* device ID (if special file) */     特殊设备号
//   off_t     st_size;    /* total size, in bytes */     普通文件，对应的文件字节数
//   blksize_t st_blksize; /* blocksize for file system I/O */   文件内容对应的块大小
//   blkcnt_t  st_blocks;  /* number of 512B blocks allocated */ file内容对应的块数量
//   time_t    st_atime;   /* time of last access */      文件最后被访问的时间
//   time_t    st_mtime;   /* time of last modification */     文件最后被修改的时间
//   time_t    st_ctime;   /* time of last status change */     文件状态改变时间
// };
#define DEVICE_NAME "/dev/rwdev"
//配置读取函数
int GetIniKeyString(char *title,char *key,char *filename,char *buf);

int PutIniKeyString(char *title,char *key,char *val,char *filename);

/* 
    * 函数名：         GetIniKeyString 
    * 入口参数：        title 
    *                      配置文件中一组数据的标识 
    *                  key 
    *                      这组数据中要读出的值的标识 
    *                  filename 
    *                      要读取的文件路径 
    * 返回值：         找到需要查的值则返回正确结果 0 
    *                  否则返回-1 
    */  
int GetIniKeyString(char *title,char *key,char *filename,char *buf)  
{  
    FILE *fp;  
    int  flag = 0;  
    char sTitle[64], *wTmp;
    char sLine[1024];        
    sprintf(sTitle, "[%s]", title);
                     
    if(NULL == (fp = fopen(filename, "r"))) {  
        perror("fopen");  
        return -1;
    }
    while (NULL != fgets(sLine, 1024, fp)) {  
        // 这是注释行  
        if (0 == strncmp("//", sLine, 2)) continue;  
        if ('#' == sLine[0])              continue;        
        wTmp = strchr(sLine, '=');  
        if ((NULL != wTmp) && (1 == flag)) {  
            if (0 == strncmp(key, sLine, strlen(key))) { // 长度依文件读取的为准  
                sLine[strlen(sLine) - 1] = '\0';  
                fclose(fp);
                while(*(wTmp + 1) == ' '){
                    wTmp++;
                }
                strcpy(buf,wTmp + 1);
                return 0;  
            }  
        } else {  
            if (0 == strncmp(sTitle, sLine, strlen(sTitle))) { // 长度依文件读取的为准  
                flag = 1; // 找到标题位置  
            }  
        }  
    }  
    fclose(fp);  
    return -1;  
}        
      
/* 
    * 函数名：         PutIniKeyString 
    * 入口参数：        title 
    *                      配置文件中一组数据的标识 
    *                  key 
    *                      这组数据中要读出的值的标识 
    *                  val 
    *                      更改后的值 
    *                  filename 
    *                      要读取的文件路径 
    * 返回值：         成功返回  0 
    *                  否则返回 -1 
    */  
int PutIniKeyString(char *title,char *key,char *val,char *filename)  
{  
    FILE *fpr, *fpw;  
    int  flag = 0;  
    char sLine[1024], sTitle[32], *wTmp;        
    sprintf(sTitle, "[%s]", title);  
    if (NULL == (fpr = fopen(filename, "r")))  
        return -1;// 读取原文件  
    sprintf(sLine, "%s.tmp", filename);  
    if (NULL == (fpw = fopen(sLine,    "w")))  
        return -1;// 写入临时文件        
    while (NULL != fgets(sLine, 1024, fpr)) {  
        if (2 != flag) { // 如果找到要修改的那一行，则不会执行内部的操作  
            wTmp = strchr(sLine, '=');  
            if ((NULL != wTmp) && (1 == flag)) {  
                if (0 == strncmp(key, sLine, strlen(key))) { // 长度依文件读取的为准 
                    flag = 2;// 更改值，方便写入文件  
                    sprintf(wTmp + 1, " %s\n", val);
                }  
            } else {
                if (0 == strncmp(sTitle, sLine, strlen(sTitle))) { // 长度依文件读取的为准
                    flag = 1; // 找到标题位置  
                }  
            }  
        }        
        fputs(sLine, fpw); // 写入临时文件 
    }  
    fclose(fpr);  
    fclose(fpw);        
    sprintf(sLine, "%s.tmp", filename);  
    return rename(sLine, filename);// 将临时文件更新到原文件  
}   



// void ExecCmd(char * in,char out[])
// {
//     FILE *fp = NULL;
    
//     fp = popen(in, "r");
//     if(fp)
//     {
//         //int ret = fread(out,1,sizeof(out)-1,fp);
//         int ret = fread(out,sizeof(char),300,fp);
//         if(ret > 0) {
//             printf("系统调用执行成功\n");
//         }
//         pclose(fp);
//         //printf("\n");
//     }
    
// }



int main ()
{
    // char hostbuffer[256];
    // char *IPbuffer;
    // struct hostent *host_entry;
    // int hostname;
 
    // //接收主机名
    // hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    // checkHostName(hostname);
 
    // //接收主机信息
    // host_entry = gethostbyname(hostbuffer);
    // checkHostEntry(host_entry);
  
    //   //转换网络地址
    // IPbuffer = inet_ntoa(*(struct in_addr*)host_entry->h_addr_list[0]);
    // printf("Hostname: %s\n", hostbuffer);
    // printf("Host IP: %s\n", IPbuffer);
  
    // struct stat b;
    // struct stat c;
    // //获取文件的元数据
    // int sa = stat("/home/liwei/桌面/code/Nodes/node1/testnode/rr4.txt",&b);
    // if(sa == -1) {
    //     perror("stat");
    // }
    // //输出文件的元数据
    // printf("文件内容对应的块大小:%ld\n", b.st_blksize);
    // printf("inode号:%ld  : %d : %d : %d\n", b.st_ino,b.st_mtime,b.st_atime,b.st_ctime);//1677240261 : 1677240276 : 1677240261
    
    // sa = stat("node1/testnode2/dwa.txt",&c);
    // if(sa == -1) {
    //     perror("stat");
    // }
    // printf("inode号:%ld  : %d : %d : %d\n", c.st_ino,c.st_mtime,c.st_atime,c.st_ctime);//1677240261 : 1677240261 : 1677240261
    // // time_t timep;
    // // time(&timep);
    // // double dt= difftime(timep, (time_t)b.st_ctime);

    // char rminstr_1[100]="find . -inum ";
    // char rminstr_2[20]=" -exec rm -r {} \\;";
    // char buffer[15];
    // sprintf(buffer, "%d", b.st_ino);
    // strcat(rminstr_1,buffer);
    // strcat(rminstr_1,rminstr_2);
    // printf("%s\n",rminstr_1);
    // // printf("%d:%d:%lf\n",timep,b.st_ctime, dt);
    // system(rminstr_1);       /////////////////////////////////////////////////////////////////////////////////////////


    // DIR * dir;
    // struct dirent * ptr;
    // int i,error;
    // /******** 获取文件路径下的文件列表********/
    // dir =opendir(FILE_DISC_PARTS_CHECK);
    // char *ino_k[1]={"4464195"};
    //     while((ptr = readdir(dir))!=NULL)
    //     {
    //         char ino[25];
    //         printf("d_name: %s | d_type: %d | d_ino: %d \n",ptr->d_name,ptr->d_type,ptr->d_ino); //文件名  //4为文件夹，8为文件
    //         sprintf(ino,"%d",ptr->d_ino);
    //         if(strcmp(ino,ino_k[0])==0) printf("yes!\n");
    //     }
    // closedir(dir);   
    // char md5_str[64]={0};

    // calc_md5("/home/liwei/桌面/code/datanode/node2/hutao.jpg",md5_str);
    // printf("hutao图片%s",md5_str);


    ///////////============================================================================================================================================
    
    printf("数据节点扫描数据初始化加载中！........................................................\n");
    char buff[100];
    char laod_path[100];
    int ret;
    int go;  
    printf("bush datanode 输入配置文件：");
    scanf("%d",&go);
    if(go == 1)
    {
        strcpy(laod_path,"/home/liwei/桌面/code/ini/datanodeconfig_1.ini");
    }
    if(go == 2)
    {
        strcpy(laod_path,"/home/liwei/桌面/code/ini/datanodeconfig_2.ini");
    }
    if(go == 3)
    {
        strcpy(laod_path,"/home/liwei/桌面/code/ini/datanodeconfig_3.ini");
    }

    ret = GetIniKeyString("IP","IP_ADD",laod_path,DATA_IP);
    printf("ret:%d Datanode ip:%s\n",ret,DATA_IP);

    ret = GetIniKeyString("METE_IP","METE_IP_ADD",laod_path,MEtE_IP);
    printf("ret:%d Meta Datanode server ip:%s\n",ret,MEtE_IP);

    ret = GetIniKeyString("METE_PORT","METE_PORT",laod_path,buff);
    MYPORT = atoi(buff);
    printf("ret:%d Meta Datanode server port:%d\n",ret,MYPORT);

    ret = GetIniKeyString("KEY","MEM_KEY",laod_path,buff);
    MEM_KEY = atoi(buff);
    printf("ret:%d Mem key:%d\n",ret,MEM_KEY);

    ret = GetIniKeyString("LAOD_DIR","LAOD_DIR",laod_path,DIR_LOAD);
    printf("ret:%d Laod dir:%s\n",ret,DIR_LOAD);

    printf("数据节点扫描数据初始化加载完毕!........................................................\n");


    pthread_t tid;
    if((pthread_create(&tid,NULL,clock_filesearch,NULL))!=0){
        fprintf(stderr,"pthread create error\n");
        exit(EXIT_FAILURE);
    }
    // pthread_exit(NULL);
    sleep(36000);  //10h
    printf("end\n");
    ///////////============================================================================================================================================
    
    // char * shl="find /home/liwei/桌面/code/Nodes/node1 -inum 4490257"; 
    // char buf[300]={0};
    // ExecCmd(shl,buf);

    //4478410 
    // char pathbuffer[5][TEXT_SZ]={0};
    // int a = GetPathByInode(4490257 ,pathbuffer);
    // printf("%d ssss\n",a);
    // for (int i = 0; i < a; i++)
    // {
    //     printf("%s\t",pathbuffer[i]);
    // }
    
    return 0;
}

