#ifdef RUNTIME
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>
#include <stddef.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <netinet/in.h>  
#include <arpa/inet.h> 
#include <sys/types.h>  
#include <sys/socket.h>  
#include <pthread.h>  
#include<errno.h>
#include<fcntl.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <sys/stat.h>
#include "shmdata.h"
#include "md5.h"

#define MAX_OPNE_LIST_SIZE 5  //进程最大打开文件数量
//进程文件指针表
static struct fopen_r_para fd_list[MAX_OPNE_LIST_SIZE];
static int fd_list_index[MAX_OPNE_LIST_SIZE]={0};
static int fd_list_count = 0;
static int fd_mutex =1;   //上锁

//进程打开表
static struct file_link open_list[MAX_OPNE_LIST_SIZE];
static int open_list_index[MAX_OPNE_LIST_SIZE]={0};
static int open_list_count = 0;
static int open_file_mutex =1;   //上锁


int kEY = 1234;   //内存挂载

//只读打开
void * fopen_r(void *arg);
//设置共享内存

//查重定向表,文件名字仅限绝对路径
int is_in_list(const char *filename,struct redirect_list * shared , struct redirect_addr * read_addr);

//分配共享内存
void alocate_memory( struct alocate_info * shmshare);

//释放共享内存
void free_memory(struct alocate_info * shmshare);

//删除共享内存!!!!!
void delt_memory(struct alocate_info * shmshare);

//只读连接打开文件
void Socket_Client_Fopen_r(struct redirect_addr * read_addr,char filename[]);

//只读下载文件
void download_file(char filename [],SSL * ssl,ino_t inode);

//只读退出ssl链接
void quit(SSL * ssl);

//存入进程文件指针表
void store_fd_list(struct fopen_r_para * pstru);

//返回进程文件指针表空位置
int get_index_fd_list();

//根据文件名字在文件打开表中找
int find_file_list_by_name(const char *name);

//存入文件打开表
void store_open_list(struct fopen_r_para * pstru,FILE * fd);

//首次存入文件打开表
void first_store_open_list(const char *name,FILE * fd,bool local);

//获取文件打开表的空位置
int get_index_open_file_list();

//根据索引修改文件打开表
void change_open_list(int index);


//===================================================================================================================
int find_fd_list_by_stream(FILE * stream);

//删除fd list表相
void del_fd_list_by_index(int index);

//关闭进程fd_list
int close_fd_list(FILE * stream,fun_t_2 fclosep);


/*
*******************************************************************************************************************************************************************************
控制读写权限的字符串（必须指明）
打开方式	说明
"r"	        以“只读”方式打开文件。只允许读取，不允许写入。文件必须存在，否则打开失败。
"w"	        以“写入”方式打开文件。如果文件不存在，那么创建一个新文件；如果文件存在，那么清空文件内容（相当于删除原文件，再创建一个新文件）。
"a"	        以“追加”方式打开文件。如果文件不存在，那么创建一个新文件；如果文件存在，那么将写入的数据追加到文件的末尾（文件原有的内容保留）。
"r+"	    以“读写”方式打开文件。既可以读取也可以写入，也就是随意更新文件。文件必须存在，否则打开失败。
"w+"	    以“写入/更新”方式打开文件，相当于w和r+叠加的效果。既可以读取也可以写入，也就是随意更新文件。如果文件不存在，那么创建一个新文件；如果文件存在，那么清空文件内容（相当于删除原文件，再创建一个新文件）。
"a+"	    以“追加/更新”方式打开文件，相当于a和r+叠加的效果。既可以读取也可以写入，也就是随意更新文件。如果文件不存在，那么创建一个新文件；如果文件存在，那么将写入的数据追加到文件的末尾（文件原有的内容保留）。
===============================================================================================================================================================================
    整体来说，文件打开方式由 r、w、a、t、b、+ 六个字符拼成，各字符的含义是：
    r(read)：读
    w(write)：写
    a(append)：追加
    t(text)：文本文件
    b(binary)：二进制文件
    +：读和写
*******************************************************************************************************************************************************************************
*/



//以“只读”方式打开文件。只允许读取，不允许写入。文件必须存在，否则打开失败。
//本地存在则直接返回，否则将文件从远程传输到本地然后返回,注意，当文件从远端回传后会将对应旧地址在重定向表中删除！
/*
void * fopen_r(void *arg)
{
    printf("Enter fopen_r().................\n");
    //======================================================
    //检验文件路径是否有效
    int fd = access(pstru->filename,F_OK);
    if(fd == 0) //存在
    {
        pstru->fback = pstru->fopenp(pstru->filename,pstru->open_type);
    }
    else //本地不存在则查重定向表
    {
        //分配共享内存
        alocate_memory(shmshare);
        //这个需要在远程传输完毕后释放
        struct redirect_addr * read_addr =(struct redirect_addr * ) malloc(sizeof(struct redirect_addr));
        int index = is_in_list(pstru->filename,shmshare->shared,read_addr);
        if(index!=-1) //在重定向表中
        {
            //远程传输
            Socket_Client_Fopen_r(read_addr,pstru->filename);
            pstru->fback = pstru->fopenp(pstru->filename,pstru->open_type);
        }   
        else //不在重定向表中
        {
            pstru->fback =pstru->fopenp(pstru->filename,pstru->open_type);  //NULL
        }
        //线程分离共享内存
        free_memory(shmshare);
        //释放资源
        free(read_addr);
        free(shmshare);
    }
}
*/
void * fopen_r(void *arg)
{   
    printf("Enter fopen_r().................\n");
    struct fopen_r_para * pstru;
    pstru = (struct fopen_r_para *) arg;
    //本地直接打开,如果本地存在则直接返回文件指针
    //FILE * f =  pstru->fopenp(pstru->filename,"r");

    /*
        0 （F_OK） 只判断是否存在
        2 （R_OK） 判断写入权限
        4 （W_OK） 判断读取权限
        6 （X_OK） 判断执行权限
    */
    printf("路径 %s \n",pstru->filename);
    int fd = access(pstru->filename,F_OK);

    if(fd == 0) //存在
    {
        pstru->fback = pstru->fopenp(pstru->filename,pstru->open_type);
        pstru->is_local = true;
        printf("%s 本地文件 %s ====> fd:%X\n",pstru->open_type,pstru->filename,pstru->fback);
    }
    else //本地不存在则查重定向表
    {
        //这个需要在open_r结束后释放
        struct alocate_info* shmshare =(struct alocate_info* ) malloc(sizeof(struct alocate_info));
        shmshare->shm=NULL;
        //分配共享内存
        alocate_memory(shmshare);
        //这个需要在远程传输完毕后释放
        struct redirect_addr * read_addr =(struct redirect_addr * ) malloc(sizeof(struct redirect_addr));
        int index = is_in_list(pstru->filename,shmshare->shared,read_addr);
        printf("index = %d\n",index);
        if(index!=-1) //在重定向表中
        {
            //TOCHECK远程传输
            Socket_Client_Fopen_r(read_addr,pstru->filename);
            pstru->fback = pstru->fopenp(pstru->filename,pstru->open_type);
            pstru->is_local = false;
            printf("%s 远程文件 %s ====> fd:%X\n",pstru->open_type,pstru->filename,pstru->fback);
        }   
        else //不在重定向表中
        {
            printf("%s 文件不在重定向表中,新文件创建或者返回NULL!\n",pstru->filename);
            pstru->is_local = true;
            pstru->fback =pstru->fopenp(pstru->filename,pstru->open_type);  //NULL
        }
       
        //线程分离共享内存
        free_memory(shmshare);
        //释放资源
        free(read_addr);
        free(shmshare);
    }

    //线程退出
    pthread_exit(NULL);  
}


/*
FILE *fopen(const char *__restrict __filename, const char *__restrict __modes) 
{
   // Enter fopen()................. 
   //=============================================================
   //检测是否超过文件打开上限
    if(fd_list_count == MAX_OPNE_LIST_SIZE)
    {
        printf("超过进程最大文件指针数！进程结束\n");
        exit(EXIT_FAILURE);
    }
    //============================================================
    //设置库打桩替换原函数接口
    struct fopen_r_para *pstru =( struct fopen_r_para *) malloc(sizeof(struct fopen_r_para)) ;
    void *(* fopenp)(const char *__restrict __filename, const char *__restrict __modes);
    fopenp = dlsym(RTLD_NEXT, "fopen");
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    //============================================================
    //fopen创建子线程托管函数fopen_r
    if(pthread_create(&thread,NULL,fopen_r,pstru)!=0)//创建子线程  
    {  
        perror("pthread_create:");  
    }else
    {
        printf("线程创建成功\n");
    }  
    //存文件指针表和文件表
    f1 = pstru->fback;
    //============================================================
    //存入到进程文件指针表
    store_fd_list(pstru);
    //============================================================
    //存入到进程文件打开表
    store_open_list(pstru,f1);
    //===========================================================
    //返回文件指针
    return f1;
}
*/
FILE *fopen(const char *__restrict __filename, const char *__restrict __modes) 
{
    printf("Enter fopen()................. \n");

    if(fd_list_count == MAX_OPNE_LIST_SIZE)
    {
        printf("超过进程最大文件指针数！进程结束\n");
        exit(EXIT_FAILURE);
    }
    if(open_list_count == MAX_OPNE_LIST_SIZE)
    {
        printf("超过进程最大打开文件数！进程结束\n");
        exit(EXIT_FAILURE);
    }

    void *(* fopenp)(const char *__restrict __filename, const char *__restrict __modes);
    char * error;
    fopenp = dlsym(RTLD_NEXT, "fopen");
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }
    FILE * f1;

    pthread_t thread;
    //Instruction *instr=(Instruction*)malloc(sizeof(Instruction));

    //这个需要fopen执行结束后释放
    struct fopen_r_para *pstru =( struct fopen_r_para *) malloc(sizeof(struct fopen_r_para)) ;
    pstru->fopenp = fopenp;
    strcpy(pstru->filename,__filename);
    strcpy(pstru->open_type,__modes);
    if(pthread_create(&thread,NULL,fopen_r,pstru)!=0)//创建子线程  
    {  
        perror("pthread_create:");  
    }else
    {
        printf("线程创建成功\n");
    }  

    void * thread_result;
    //阻塞主线程，等待 thread 线程执行结束
    int res = pthread_join(thread, &thread_result);
    if (res != 0) {
        printf("等待线程失败\n");
    }
    printf("文件存表调用栈================================================================\n");
    //返回文件指针到进程
    f1 = pstru->fback;
    //============================================================
    //存入到进程文件指针表
    store_fd_list(pstru);
    //============================================================
    //存入到进程文件打开表
    store_open_list(pstru,f1);
    //===========================================================
    //释放指针
    free(pstru);
    return f1;
}

int is_in_list(const char *filename,struct redirect_list * shared , struct redirect_addr * read_addr)
{
    printf("Enter is_in_list().......\n");
    while (shared->mutex == 0)
    {
        sleep(1);
        printf("Waiting to read...\n");
    }
    //获取锁读数据
    shared->mutex=0; //获取锁
    for (int index = 0; index < LIST_LENGTH; index++) //遍历map
    {
        if(shared->index_map[index] == 1 )
        {
            printf("在共享内存中找到有效位置,有效数量为: %d 重定向目标表项为:%s : %d\n",shared->count,shared->list[index].IP_addr,shared->list[index].inonumber);
            for (int j = 0; j < MAX_HARD_LINK; j++)  //遍历硬链接5
            {
                printf("%s 硬链接数为 : %d : %s\n",filename,shared->list[index].old_path_number,shared->list[index].old_path[j]);
                
                if(strcmp(filename,shared->list[index].old_path[j]) == 0)
                {
                    //返回值
                    read_addr->inonumber = shared->list[index].inonumber;
                    strcpy(read_addr->IP_addr,shared->list[index].IP_addr);
                    // read_addr->old_path_number = shared->list[index].old_path_number-1;

                    // //修改重定向表,将已经读取的文件从表中删除//??
                    // strcpy(shared->list[index].old_path[j],"");

                    // //TODO 不存储应链接，只会存储一个
                    // shared->index_map[index] =0 ; //位置为0表示在重删后再次读取，然后将重定向表删除
                    // strcpy(shared->list[index].IP_addr,"");
                    // shared->list[index].inonumber =0 ;
                    // shared->list[index].old_path_number =0 ;
                    // shared->count-- ;

                    //释放锁
                    shared->mutex = 1; 
                
                    return index;
                }
            }
            // read_addr->inonumber = shared->list[index].inonumber;
            // strcpy(read_addr->IP_addr,shared->list[index].IP_addr);
            // read_addr->old_path_number = shared->list[index].old_path_number-1;
            //  //释放锁
            // shared->mutex = 1; 
            // return index;
        }
    }
    shared->mutex = 1; //释放锁
    return -1;
}

void alocate_memory( struct alocate_info * shmshare)
{
    printf("Enter alocatce_memory().......\n");
    // 创建共享内存
    shmshare->shmid = shmget((key_t)kEY, sizeof(struct redirect_list), 0666|IPC_CREAT);
    if (shmshare->shmid == -1)
    {
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }

    // 将共享内存连接到当前进程的地址空间
    shmshare->shm= shmat(shmshare->shmid, 0, 0);
    if (shmshare->shm == (void *)-1)
    {
        fprintf(stderr, "shmat failed\n");
        exit(EXIT_FAILURE);
    }
    
    printf("\nMemory attached at %X  shmid is %d\n", (int)shmshare->shm,shmshare->shmid);

    // 设置共享内存
    shmshare->shared = (struct redirect_list*)shmshare->shm;
}

void free_memory(struct alocate_info * shmshare)
{
    printf("Enter free_memory().......\n");
    // 把共享内存从当前进程中分离
    if (shmdt(shmshare->shm) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("内存分离成功！\n");
    }
}

void delt_memory(struct alocate_info * shmshare)
{
     // 删除共享内存
    if (shmctl(shmshare ->shmid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("内存删除成功！");
    }
}

void Socket_Client_Fopen_r(struct redirect_addr * read_addr,char filename[])
{
    printf("Enter Socket_Client_Fopen_r().......\n");
    //参数声明
    char ipaddr[50];
    int sockfd;
    struct sockaddr_in sockaddr;
    SSL_CTX *ctx;//SSL套接字
    SSL *ssl;
    printf("参数声明结束\n");
    //初始化SSl
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    ctx = SSL_CTX_new(SSLv23_client_method());//创建SSL套接字，参数表明支持版本和客户机
    if (ctx == NULL)
    {
        printf("Creat CTX error!!!\n");
    }

    //创建socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
		_exit(0);
	}

    //连接
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(server_port);  //3333
	sockaddr.sin_addr.s_addr = inet_addr(read_addr->IP_addr); //目标IP地址
	if (connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1)
	{
		perror("connect");
		_exit(0);
	}else
    {
        printf("连接成功\n");
    }

    ssl = SSL_new(ctx);
	SSL_set_fd(ssl, sockfd);
	if (SSL_connect(ssl) <0 )
	{
        printf("SSL connect error!\n");
        _exit(0);
	}
	else
	{
		printf("SSL connect successful!\n");
	}
    //下载文件
    download_file(filename,ssl,read_addr->inonumber);
    //关闭链接
    quit(ssl);
    //结尾操作
	close(sockfd);
	//释放CTX
	SSL_CTX_free(ctx);
}

void download_file(char filename [],SSL * ssl,ino_t inode)
{
    printf("Enter download_file().......\n");
    int fd;
	char cmd = 'D';
	char buf[1024];
    ino_t inodenumber = inode;
	int FileNameSize = strlen(filename);
	int filesize = 0, count = 0, totalrecv = 0;
 
	//发送命令
	SSL_write(ssl, &cmd, 1);
    
	//发送文件inodenumber
    SSL_write(ssl,&inodenumber,sizeof(ino_t));
	// SSL_write(ssl, (void *)&FileNameSize, 4);
	// SSL_write(ssl, filename, FileNameSize);
 
	//打开并创建文件
	if ((fd = open(filename, O_RDWR | O_CREAT)) == -1)
	{
		perror("open:");
		_exit(0);
	}
 
	//接收数据
	SSL_read(ssl, &filesize, 4);
	while ((count = SSL_read(ssl, (void *)buf, 1024)) > 0)
	{
		write(fd, buf, count);
		totalrecv += count;
		if (totalrecv == filesize) break;
	}
 
	//关闭文件
	close(fd);

}

void quit(SSL * ssl)
{
    printf("Enter quit().......\n");
	char cmd = 'Q';
	//发送命令
	SSL_write(ssl, (void *)&cmd, 1);
	//关闭及释放SSL连接
	SSL_shutdown(ssl);
	SSL_free(ssl);
	//清屏
	system("clear");
	//退出
	//_exit(0);
}

void store_fd_list(struct fopen_r_para * pstru)
{   
    printf("Enter store_fd_list().......\n");
    while (fd_mutex == 0)
    {
        sleep(1);
        printf("Waiting to use fd_list...\n");
    }
    fd_mutex = 0;
//     struct fopen_r_para
    // {
    //     fun_t fopenp;//函数指针
    //     char filename[TEXT_SZ];//文件绝对路径
    //     FILE * fback; //返回
    //     char open_type[4]; //文件的读写方式
    // };

    // struct file_link
    // {
    //     char filename[TEXT_SZ];//文件绝对路径
    //     int link;
    // };
    int index = get_index_fd_list();
    fd_list[index].fopenp = pstru->fopenp;
    strcpy(fd_list[index].filename,pstru->filename);
    fd_list[index].fback = pstru->fback;
    strcpy(fd_list[index].open_type,pstru->open_type);
    fd_list_index[index] = 1; //修改占位
    fd_list_count++;
    fd_mutex = 1;
}

int get_index_fd_list()
{
    printf("Enter get_index_fd_list().......\n");
    static int index = 0;
    int back =0;

    if(fd_list_index[index] == 0)
    {
        back = index;
        index = (index +1)%MAX_OPNE_LIST_SIZE;
        return back;
    }
    index = (index +1)%MAX_OPNE_LIST_SIZE;

    while (fd_list_index[index] !=0)
    {
        index = (index +1)%MAX_OPNE_LIST_SIZE;
    }
    back = index;
    index = (index +1)%MAX_OPNE_LIST_SIZE;
    return back;
}

int find_file_list_by_name(const char *name)
{
    printf("Enter find_file_list_by_name().......\n");
    for (int i = 0; i < MAX_OPNE_LIST_SIZE; i++)
    {
       if(open_list_index[i] == 1 && strcmp(open_list[i].filename,name) == 0 )
       {
            return i;
       }
    }
    return -1;
}

void store_open_list(struct fopen_r_para * pstru,FILE * fd)
{
    printf("Enter store_open_list().......\n");
    //获取锁
    while (open_file_mutex == 0)
    {
        sleep(1);
        printf("Waiting to use open_file_list...\n");
    }
    open_file_mutex = 0;
    //存入到文件打开表
    int index_of_file_list = find_file_list_by_name(pstru->filename);
    if(index_of_file_list == -1 )  //表示没有存储过该打开的文件
    {
        first_store_open_list(pstru->filename,fd,pstru->is_local);
    }   
    else
    {
        change_open_list(index_of_file_list);
    }
    //释放锁
    open_file_mutex = 1;
}

void first_store_open_list(const char *name,FILE * fd,bool local)
{
    printf("Enter first_store_open_list().......\n");
    //文件存在计算指纹
    int index = get_index_open_file_list();
    strcpy(open_list[index].filename,name);
    open_list[index].link = 1;
    open_list[index].is_local = local;
    if(fd ==NULL) 
    {
        strcpy(open_list[index].md5_str,""); //文件不存在指纹赋值为空
    }else
    {
        calc_md5(name,open_list[index].md5_str);
        printf("md5 :%s\n",open_list[index].md5_str);
    }

    open_list_index[index] = 1; //占位
    open_list_count++;

}

int get_index_open_file_list()
{
    printf("Enter get_index_open_file_list().......\n");
    static int index = 0;
    int back =0;

    if(open_list_index[index] == 0)
    {
        back = index;
        index = (index +1)%MAX_OPNE_LIST_SIZE;
        return back;
    }
    index = (index +1)%MAX_OPNE_LIST_SIZE;

    while (open_list_index[index] !=0)
    {
        index = (index +1)%MAX_OPNE_LIST_SIZE;
    }
    back = index;
    index = (index +1)%MAX_OPNE_LIST_SIZE;
    return back;
}

void change_open_list(int index)
{
    printf("Enter change_open_list().......\n");
    //只会修改link数，不会修改指纹
    open_list[index].link++;
}





/*
*******************************************************************************************************************************************************************************
关闭文件说明:
    当进程的文件打开表中文件link数为0时才可进行文件彻底关闭!   当文件的指纹未发生变化时文件会被删除，不会长期保留在磁盘上；否则将会在磁盘上存储 
*******************************************************************************************************************************************************************************
#define MAX_OPNE_LIST_SIZE 5  //进程最大打开文件数量
//进程文件指针表
static struct fopen_r_para fd_list[MAX_OPNE_LIST_SIZE];
static int fd_list_index[MAX_OPNE_LIST_SIZE]={0};
static int fd_list_count = 0;
static int fd_mutex =1;   //上锁

//进程打开表
static struct file_link open_list[MAX_OPNE_LIST_SIZE];
static int open_list_index[MAX_OPNE_LIST_SIZE]={0};
static int open_list_count = 0;
static int open_file_mutex =1;   //上锁
*/

int find_fd_list_by_stream(FILE * stream)
{
   printf("find_fd_list_by_stream.......\n");
   static int index = 0;
   int back = 0;
   if(fd_list[index].fback == stream)
   {
        back = index;
        index = (index + 1)%MAX_OPNE_LIST_SIZE;
        return back;
   }    
    index = (index + 1)%MAX_OPNE_LIST_SIZE;
    while (fd_list[index].fback != stream)
    {
        index = (index + 1)%MAX_OPNE_LIST_SIZE;
    }
    back = index;
    index = (index + 1)%MAX_OPNE_LIST_SIZE;
    printf("find_fd_list_by_stream down.......\n");
    return back;
}

void del_fd_list_by_index(int index)
{
    printf("del_fd_list_by_index.......\n");
    fd_list_index[index]=0; //修改占位符
    fd_list_count--; 
}  

int close_fd_list(FILE * stream,fun_t_2 fclosep)
{
    printf("Enter close_fd_list.......\n");
    int back;
    while (fd_mutex == 0)
    {
        sleep(1);
        printf("Waiting to use fd_list...\n");
    }
    fd_mutex = 0;
    //只要传进去就一定能找到
    int index_fd = find_fd_list_by_stream(stream);
    del_fd_list_by_index(index_fd);

    fd_mutex = 1;

    while (open_file_mutex == 0)
    {
        sleep(1);
        printf("Waiting to use open_file_list...\n");
    }
    open_file_mutex == 0;
    int index_open = find_file_list_by_name(fd_list[index_fd].filename);
    int link = --open_list[index_open].link ;
    //这个link只是该进程的link不是全局的link
    printf("fd %X : link %d\n",stream,link);
    if(link == 0)
    {
        open_list_index[index_open] =0;  //修改占位
        open_list_count --;

        if(stream != NULL )//文件是否存在
        {
            printf("指针存在！\n");
            //如果为远程文件且指纹变化就删除 ，做测试，所有本地文件均被设置为远程文件
            printf("fclosep关闭！\n");
            back = fclosep(stream);
            if(open_list[index_open].is_local == false)
            {
                printf("检查为远程文件\n");
                char md5name [TEXT_SZ];
                calc_md5(open_list[index_open].filename,md5name);
                if(strcmp(md5name,open_list[index_open].md5_str) == 0)
                {
                    printf("指纹没变 删除 %s : %s   fd %X\n",open_list[index_open].md5_str,md5name,stream);
                    open_file_mutex == 1;
                    //删除？remove可能需要修改？？
                    remove(open_list[index_open].filename);
                    return back;
                }else
                {
                    printf("指纹改变 不删除 %s : %s   fd %X\n",open_list[index_open].md5_str,md5name,stream);
                    open_file_mutex == 1;
                    return back;
                }
            }
        }  
    }
    open_file_mutex == 1;
    printf(" close_fd_list down.......\n");
    return  fclosep(stream);
}

extern int fclose (FILE *__stream)
{
    printf("Enter fclose.......\n");

    //库打桩
    void *(* fclosep)(FILE *__stream);
    char * error;
    fclosep = dlsym(RTLD_NEXT, "fclose");
    if ((error = dlerror()) != NULL) {
        fputs(error, stderr);
        exit(1);
    }

    int back = close_fd_list(__stream,fclosep);
    // if(back ==-1)
    // {
    //     printf("fclose failed! check null ==============================\n");
    // }else
    // {
    //     printf("fclose successs!====================================\n");
    // }
    printf("fclose successs %X!====================================\n",__stream);
    return back;
}

#endif