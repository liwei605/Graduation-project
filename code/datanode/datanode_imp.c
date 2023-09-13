#include "datanode.h"
void getHostIP(char ipbuf[20])
{
    int i=0;
	int sockfd;
	struct ifconf ifc;
	char buf[1024]={0};
	//char ipbuf[20]={0};
	struct ifreq *ifr;
 
	ifc.ifc_len = 1024;
	ifc.ifc_buf = buf;
 
	if((sockfd = socket(AF_INET, SOCK_DGRAM,0))<0)
	{
	    printf("socket error\n");
		return -1;
	}
	ioctl(sockfd,SIOCGIFCONF, &ifc);
	ifr = (struct ifreq*)buf;
 
	for(i=(ifc.ifc_len/sizeof(struct ifreq)); i > 0; i--)
	{
		//printf("net name: %s\n",ifr->ifr_name);
		inet_ntop(AF_INET,&((struct sockaddr_in *)&ifr->ifr_addr)->sin_addr,ipbuf,20);
		//printf("ip: %s \n",ipbuf);
        if(strcmp(ipbuf,"127.0.0.1") != 0) return;
           //printf("非回环ip: %s \n",ipbuf);
		ifr = ifr +1;
	}
}


//周期文件检测线程函数
void * clock_filesearch(void * argv)
{
    void *shm = NULL;
    struct redirect_list *shared = NULL;
    int shmid;

    // 创建共享内存
    shmid = shmget((key_t)MEM_KEY, sizeof(struct redirect_list), 0666|IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "首次创建共享内存失败!\n");
        exit(EXIT_FAILURE);
    }
   
    // 删除共享内存
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("首次启动，共享内存清除！");
    }

    // 创建共享内存
    shmid = shmget((key_t)MEM_KEY, sizeof(struct redirect_list), 0666|IPC_CREAT);
    if (shmid == -1)
    {
        fprintf(stderr, "创建共享内存失败!\n");
        exit(EXIT_FAILURE);
    }

 // 将共享内存连接到当前的进程地址空间
    shm = shmat(shmid, (void *)0, 0);
    if (shm == (void *)-1)
    {
        fprintf(stderr, "共享内存挂载进程失败！\n");
        exit(EXIT_FAILURE);
    }
    printf("共享内存地址 : %X\n", (int)shm);
    // 设置共享内存
    shared = (struct redirect_list *)shm;
    shared->count=0;
    shared->mutex=1;
    

    printf("这是我创建的一个文件查找线程tid = %ld\n",pthread_self());
    int times=1;
    //设定扫描间隔
    int k= 30;
    while(true){

        printf("开始第%d扫描新增文件.....共享内存地址 : %X\\n",times++,(int)shm);

        //开销测试
        printf("开销测试\n");
         // Start measuring time
        struct timeval begin, end;
        gettimeofday(&begin, 0);
        
        //初始化
        creat_htab();

        //周期性扫描node1  ，15s一次
        FindFiles(DIR_LOAD,k,times-1);//    /home/liwei/桌面/code/datanode/node1
        testhush();
        
        //如果存在新增文件则向Mate Data node发送数据
        if(myNode->count_ino!=0)
        {
            Socdket_Send(shared,shm);
        }
        
        //销毁各种表
        destroy_htab();
        time_t timep;
        time(&timep);
    
        printf(".........本次扫描结束,距离下一次扫描剩余:%ds.............\n\n\n",k-timep%k);
        
        printf("开销测试结束返回结果：\n");
        // Stop measuring time and calculate the elapsed time
        gettimeofday(&end, 0);
        long seconds = end.tv_sec - begin.tv_sec;
        long microseconds = end.tv_usec - begin.tv_usec;
        double elapsed = seconds + microseconds*1e-6;
    
        printf("Time measured: %.3f seconds.\n", elapsed);
        //获取当前系统时间
        
        sleep(k-timep%k);   //每隔15s检测一次新增或修改的文件，返回其指纹和inode号。
    }
    
    // 把共享内存从当前进程中分离
    if (shmdt(shm) == -1)
    {
        fprintf(stderr, "shmdt failed\n");
        exit(EXIT_FAILURE);
    }
    
    // 删除共享内存
    if (shmctl(shmid, IPC_RMID, 0) == -1)
    {
        fprintf(stderr, "shmctl(IPC_RMID) failed\n");
        exit(EXIT_FAILURE);
    }else
    {
        printf("内存删除成功！");
    }

    return NULL;
}


void Socdket_Send(struct redirect_list *shared,void *shm)
{
    ///sockfd
    int sock_cli = socket(AF_INET,SOCK_STREAM, 0);
 
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(MYPORT);//6666：2586
    //servaddr.sin_addr.s_addr = inet_addr("127.0.0.5");
    servaddr.sin_addr.s_addr = inet_addr(MEtE_IP);
    

    //sockhost

    struct sockaddr_in hostaddr;
    memset(&hostaddr, 0, sizeof(hostaddr));
    hostaddr.sin_family = AF_INET;
    //客户端绑定本机非回环地址,可以读取配置文件
    // char ipbuf[20];
    // getHostIP(ipbuf);
    // hostaddr.sin_addr.s_addr = inet_addr(ipbuf);
    hostaddr.sin_addr.s_addr = inet_addr(DATA_IP);


    //客户端bind
    if (bind(sock_cli, (struct sockaddr*)&hostaddr, sizeof(hostaddr)))
    {
        char IP_client[50]="";
        strcpy( IP_client,inet_ntoa(hostaddr.sin_addr));
        printf("客户端绑定本机地址失败: %s\n",IP_client);
        exit(1);
    }

    //TODO检测主机地址
    printf("..........正在连接 %s\n",MEtE_IP);


    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("connect");
        exit(1);
    }else
    {
        printf("连接成功\n");
    }

    printf("%d\n",servaddr.sin_port);
    int needSend=sizeof(Datasend);
    char *send_buffer=(char*)malloc(needSend);
    memcpy(send_buffer,myNode,needSend);

    int pos=0;
    int len=0;
    
    printf("发送指纹数据\n");

    while(pos < needSend)
    {
        len=send(sock_cli, send_buffer+pos, BUFFER_SIZE,0);
        if(len <= 0)
        {
            perror("ERRPR");
            break;
        }
        pos+=len;
    }
    
    
    Instruction *instr=(Instruction*)malloc(sizeof(Instruction));
    int needRecv=sizeof(Instruction);
    char *recv_buffer=(char*)malloc(needRecv);
    int recv_pos=0;
    int recv_len=0;

    while(recv_pos < needRecv)
    {
        recv_len = recv(sock_cli, recv_buffer+recv_pos, BUFFER_SIZE, 0);
        if (recv_len < 0)
        {
            printf("服务器接收数据长度存在问题!\n");
            break;
        }
        recv_pos+=recv_len;

    }
        
    memcpy(instr,recv_buffer,needRecv);
    if(instr->MSG == META_RECV)
    {
        printf("服务器已接收！\n");
    }else
    {
        for (int i = 0; i < instr->count_ino; i++)
        {
            if(strcmp(DATA_IP,instr->IP_datanode[i])==0)
            {
                printf("本地重复，不允删除！\n");
                continue;
            }
            printf("重定向：  删除inode: %d  重定向地址：%s:%d\n",instr->ino_arr_del[i],instr->IP_datanode[i],instr->ino_arr_redir[i]);
        
            //内存共享存入重定向表
            store_memory(instr,shared,shm,i);
            //删除节点
            del_inode(instr->ino_arr_del[i]);
        }
    }
    

    free(instr);
    free(recv_buffer);
    free(send_buffer);
    printf("关闭套接子\n");
    close(sock_cli);
}


void del_inode(ino_t inode)
{
    char buffer[15];
    sprintf(buffer, "%d", inode);
    char rminstr_2[20]=" -exec rm -r {} \\;";
    //命令拼接
    char rminstr_1[200]="find "; //find_
    strcat(rminstr_1,DIR_LOAD); //find_DIR_LOAD   //挂载路径
    strcat(rminstr_1," -inum ") ; //find_DIR_LOAD_-inum
    strcat(rminstr_1,buffer);  //find_DIR_LOAD_-inum_12309812
    strcat(rminstr_1,rminstr_2); //find_DIR_LOAD_-inum_12309812_-exec rm -r {} \\;
    
    //系统调用删除
    system(rminstr_1);       
}

//内存共享函数
void store_memory(const Instruction *instr,struct redirect_list * shared,void *shm,int i)
{ 
    while (shared->mutex == 0)
    {
        sleep(1);
        printf("Waiting to write...\n");
    }
    //获取锁写入数据
    shared->mutex=0; //获取锁

    if(shared->count == LIST_LENGTH)
    {
        printf("内存共享已满，文件被永久删除,无法恢复！\n");
        // 把共享内存从当前进程中分离
        if (shmdt(shm) == -1)
        {
            fprintf(stderr, "共享内存分离失败,程序终止！\n");
            exit(EXIT_FAILURE);
        }
        shared->mutex=1; //释放锁
        //exit(EXIT_SUCCESS);
    }
   
    //存内存
    int index_space = get_index(shared->index_map);
    strcpy(shared->list[index_space].IP_addr,instr->IP_datanode[i]); //存储目标IP地址
    shared->list[index_space].inonumber = instr->ino_arr_redir[i];  //存储目标inode号
    printf("%s : %d \n",shared->list[index_space].IP_addr,shared->list[index_space].inonumber);
    //获取重删inode号的文件路径，可能包含多个硬链接
    for (int i = 0; i < MAX_HARD_LINK; i++)
    {
        //初始化赋值为空串
        strcpy(shared->list[index_space].old_path[i],"");
    }

    shared->list[index_space].old_path_number = GetPathByInode(instr->ino_arr_del[i],shared->list[index_space].old_path);

    // printf("打印硬链接：\n");
    // for (int i = 0; i < 5; i++)
    // {
    //     printf("第 %d 硬链接 ：%s\n",i+1,shared->list[index_space].old_path[i]);
    // }
    // printf("打印数量 %d\n",shared->list[index_space].old_path_number);

    shared->index_map[index_space] =1; //标记位图表示存储
    shared->count++; //数量加1
    shared->mutex=1; //释放锁
}

//获取共享表的空位置索引
int get_index(int index_map[])
{   
    static int index =0;
    int back;
    if( index_map[index] == 0 )
    {
        back =index;
        index =  (index+1)%LIST_LENGTH;
        return back; 
    } 

    index =  (index+1)%LIST_LENGTH;

    while (index_map[index] != 0)
    {
        index =  (index+1)%LIST_LENGTH;
    }
    back =index;
    index = (index+1)%LIST_LENGTH;

    return back;
}

//根据inode号获取文件路径,返回应链接个数
int GetPathByInode(const ino_t inode,char pathbuffer[][TEXT_SZ]) //文件最长路径1024字节
{
    char shl[200]="find ";
    //char shl[100]="find /home/liwei/桌面/code/Nodes/node3 -inum ";
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

    while (strcmp(pathbuffer[i],"")!=0)
    {
        i =  (i+1)%MAX_HARD_LINK;
    }
    back = i;
    i = (i+1)%MAX_HARD_LINK;

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