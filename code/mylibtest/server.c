#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include <sys/stat.h>

#include <openssl/err.h>
#include <openssl/ssl.h>

// #define port 3333
#define TEXT_SZ 1024   //最大路径长度
#define MAX_HARD_LINK 5  //最多5个硬链接
#define LIST_LENGTH 1024

int sockfd, newfd;
struct sockaddr_in sockaddr;
struct sockaddr_in client_addr;
int sin_size;
SSL_CTX *ctx;
SSL *ssl;

//配置文件加载
int port;
char DIR_LOAD[100];
char DATA_IP[100];

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






//根据inode号获取文件路径,返回应链接个数
int GetPathByInode(const ino_t inode,char pathbuffer[][TEXT_SZ]) //文件最长路径1024字节
{
    char shl[200]="find ";
    //char shl[100]="find /home/liwei/桌面/code/Nodes/node1 -inum ";
    strcat(shl,DIR_LOAD); 
    strcat(shl," -inum ");
    char buffer[15];
    sprintf(buffer, "%d", inode);
    strcat(shl,buffer);  //指令拼接
    //执行指令
    char buf[1024]={0};
    ExecCmd(shl,buf);

    //printf("%s\n",buf);
    
    //执行切分存二维数组
    int i = 0;
    int index_space=0;
    char *token;
    token = strtok(buf, "\n");
    while(token)
	{
        //找到一个空位置存
        int index = get_old_path_index(pathbuffer);
	    strcpy(pathbuffer[index],token);
		++i;
		// 再次调用分割时指针要变为NULL, 也就是这里的第一个参数，分割的字符串还是str
		// 第二个参数要和第一次调用时的分割符保持一致
		token = strtok(NULL, "\n") ;  
	}             
    return i;
}

int get_old_path_index(char pathbuffer[][TEXT_SZ])
{
    static int i =0;
    int back=0;
    if( strcmp(pathbuffer[i],"") ==0 )
    {
        back =i;
        i =  (i+1)%MAX_HARD_LINK;
        return back; 
    } 
    i =  (i+1)%MAX_HARD_LINK;

    for ( i; i < MAX_HARD_LINK; i++)
    {
        if(strcmp(pathbuffer[i],"")==0)
        {
            back = i;
            i = (i+1)%MAX_HARD_LINK;
            break;
        }
    }
    return back;
}

//系统调用管道返回文件名字
void ExecCmd(char * in,char out[])
{
    FILE *fp = NULL;
    
    fp = popen(in, "r");
    if(fp)
    {
        //int ret = fread(out,1,sizeof(out)-1,fp);
        int ret = fread(out,sizeof(char),300,fp);
        if(ret > 0) {
            printf("系统调用执行成功\n");
        }
        pclose(fp);
        //printf("\n");
    }
    
}

void handle(char cmd)
{
	char filename[1024] = { 0 };
	int FileNameSize = 0;
	int fd;
	ino_t inode;
	int filesize = 0;
	int count = 0, totalrecv = 0;
	char buf[1024];
	struct stat fstat;
	switch (cmd)
	{
	case 'U':
	{
		//接收文件名
		SSL_read(ssl,&inode,sizeof(ino_t));
		char pathbuffer[5][TEXT_SZ]={0};
		GetPathByInode(inode,pathbuffer);
		// SSL_read(ssl, &FileNameSize, 4);
		// SSL_read(ssl, (void *)filename, FileNameSize);
		//filename[FileNameSize] = '\0';
		strcpy(filename,pathbuffer[0]);
		//创建文件
		if ((fd = open(filename, O_RDWR | O_CREAT)) == -1)
		{
			perror("U creat:");
			_exit(0);
		}
		//接收文件长度
		SSL_read(ssl, &filesize, 4);
 
		//接收文件
		while ((count = SSL_read(ssl, (void *)buf, 1024)) > 0)
		{
			write(fd, &buf, count);
			totalrecv += count;
			if (totalrecv == filesize)
				break;
		}
		//关闭文件
		close(fd);
	}
	break;
 
	case 'D':
	{
		//接收文件名
		SSL_read(ssl,&inode,sizeof(ino_t));
		char pathbuffer[5][TEXT_SZ]={0};
        int index =0;
        printf("inode : %d\n",inode);
		GetPathByInode(inode,pathbuffer);
        for (int i = 0; i < 5; i++)
        {
            if(strcmp(pathbuffer[i],"") !=0 )
            {
                index =i;
                break;
            }
        }
        
		strcpy(filename,pathbuffer[index]);
        printf("要发送的文件名称: %s\n",filename);
		//打开文件
		if ((fd = open(filename, O_RDONLY)) == -1)
		{
			perror("D creat:");
			_exit(0);
		}
		//发送文件包括文件长度
		if ((stat(filename, &fstat)) == -1)
			return;
		SSL_write(ssl, &fstat.st_size, 4);
 
		while ((count = read(fd, (void *)buf, 1024)) > 0)
		{
			SSL_write(ssl, &buf, count);
		}
		close(fd);
	}
	break;
	}
}

int main()
{

	//加载配置

    printf("数据节点服务器数据初始化加载中！........................................................\n");
    char buff[100];
	
    int ret;
	int go;  

	char laod_path[100];
	// strcpy(laod_path,"/home/liwei/桌面/code/ini/datanodeconfig_1.ini");
	printf("输入配置文件序号：");
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

	ret = GetIniKeyString("S_PORT","SERVER_PORT",laod_path,buff);
    port = atoi(buff);
    printf("ret:%d Datanode server port:%d\n",ret,port);

	ret = GetIniKeyString("LAOD_DIR","LAOD_DIR",laod_path,DIR_LOAD);
    printf("ret:%d Laod dir:%s\n",ret,DIR_LOAD);
	printf("数据节点服务器数据初始化加载完毕！........................................................\n");


    char cmd;

	//建立连接
 
	// SSL初始化
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();
	ctx = SSL_CTX_new(SSLv23_server_method());
	
	// 载入数字证书
	SSL_CTX_use_certificate_file(ctx, "./cacert.pem", SSL_FILETYPE_PEM);
	// 载入并检查私钥
	SSL_CTX_use_PrivateKey_file(ctx, "./privkey.pem", SSL_FILETYPE_PEM);
	// 检查用户私钥
	SSL_CTX_check_private_key(ctx);
	// 创建socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket:");
		_exit(0);
	}
 
	memset(&sockaddr, 0, sizeof(sockaddr));
	sockaddr.sin_family = AF_INET;
	sockaddr.sin_port = htons(port);
	//sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    sockaddr.sin_addr.s_addr = inet_addr(DATA_IP);

	// 绑定地址
	if (bind(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)) == -1)
	{
		perror("bind:");
		_exit(0);
	}
	// 监听
	if (listen(sockfd, 10) == -1)
	{
		perror("listen");
	}
	printf("文件发送服务端启动成功！...........\n");
	while (1)
	{
		// 连接
		if ((newfd = accept(sockfd, (struct sockaddr *)(&client_addr), &sin_size)) == -1)
		{
			perror("accept:");
			_exit(0);
		}
        printf("产生新的SSL\n");
		ssl = SSL_new(ctx);// 产生新的SSL
		SSL_set_fd(ssl, newfd);
		SSL_accept(ssl);
		// 处理事件
        printf("处理事件\n");
		while (1)
		{
			SSL_read(ssl, &cmd, 1);
 
			if (cmd == 'Q')
			{
                printf("事件结束\n");
				break;
			}
			else
			{
				handle(cmd);
			}
		}
		SSL_shutdown(ssl);
		SSL_free(ssl);
		close(newfd);
        
        printf("关闭SSL\n");
	}
	
	close(sockfd);
	SSL_CTX_free(ctx);


    return (0);
}