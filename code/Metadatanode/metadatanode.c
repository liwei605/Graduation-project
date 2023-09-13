#include "metadatanode.h"

void Socket_rev_meta()
{
    // set socket's address information
    struct sockaddr_in   server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    //server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.5");
    server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);//6666 的十六进制为 2586

    printf("Began create Socket!.......................... !\n");
    // create a stream socket
    int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
    {
        printf("Socket create failed!\n");
        exit(1);
    }
    printf("Mapping port!.............................. !\n");
    //bind
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    {
        printf("Port: %d failed!\n", HELLO_WORLD_SERVER_PORT);
        exit(1);
    }
    printf("server Lisening ........................!\n");
    // listen
    if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    {
        printf("Lisening failed!\n");
        exit(1);
    }
    //打印服务器地址
    char IP_server[50]="";
    strcpy( IP_server,inet_ntoa(server_addr.sin_addr));
    printf("Server ip address ======> %s:%d\n\n\n",IP_server,server_addr.sin_port);


    while(1)
    {
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);

        int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
        if (new_server_socket < 0)
        {
            printf("Server recieve failed!\n");
            break;
        }else
        {
            printf("Socket file number %d\n",new_server_socket);
        }

        
        //获取客户端地址和端口
        Datasend *myNode=(Datasend*)malloc(sizeof(Datasend));
        char IP_client[50]="";
        strcpy( IP_client,inet_ntoa(client_addr.sin_addr));
        in_port_t client_port = client_addr.sin_port;

        //接受元数据
        int needRecv=sizeof(Datasend);
        char *recv_buffer=(char*)malloc(needRecv);
        int recv_pos=0;
        int recv_len=0;
        while(recv_pos < needRecv)
        {
            recv_len = recv(new_server_socket, recv_buffer+recv_pos, BUFFER_SIZE, 0);
            if (recv_len < 0)
            {
                printf("Data length problem!\n");
                break;
            }
            recv_pos+=recv_len;

        }
       // close(new_server_socket);
        memcpy(myNode,recv_buffer,needRecv);

        //测试元数据
        printf( "IP_client =%s:%d  |  count_ino=%d  \n",IP_client,client_addr.sin_port,myNode->count_ino);
        
        for (int i = 0; i < myNode->count_ino; i++)
        {
            printf("%d : %s \n",myNode->ino_arr[i],myNode->file_ID[i]);
        }
        //printf("\n");


        //查表
        printf(".............checking list\n");
         
        pthread_mutex_lock(&mutexglobel); // 获取互斥锁

        Instruction *instr=(Instruction*)malloc(sizeof(Instruction));

        instr->count_ino=0;
        memset(instr->ino_arr_del, 0, sizeof(instr->ino_arr_del)); 
        memset(instr->ino_arr_redir, 0, sizeof(instr->ino_arr_redir)); 
        memset(instr->IP_datanode, 0, sizeof(instr->IP_datanode)); 
        instr->MSG = 0;
        bool flag = false; //是否需要重定向标志

        for (int i = 0; i < myNode->count_ino; i++)
        {
            //printf("%d : %s \n",myNode->ino_arr[i],myNode->file_ID[i]);
            //struct MetaData* MD=NULL;
            int index_i;
            backindex a = is_in_List(myNode->file_ID[i],IP_client,myNode->ino_arr[i]);

            if(a.type == 'A')
            {
                printf(".............file not change,finger ID no change............\n");
            }
            else if(a.type == 'B')
            {
                bool kk =false ;
                //遍历有可能修改后指纹与其他文件相同
                for (int j = 0; j < MAX_MDList_SIZE ; j++)
                {
                    if(index_MD_List[j]==0)
                    {
                        continue;
                    }

                    if((strcmp(myNode->file_ID[i] ,MDList[j].ID_file)==0))  //只会检查有效位置,判断指纹
                    {
                        //重删
                        flag=true;
                        kk = true;
                        printf(".............find address,same as other files after change,redirect and delete............\n");
                        instr->ino_arr_del[instr->count_ino] = myNode->ino_arr[i];
                        instr->ino_arr_redir[instr->count_ino] = MDList[j].ino;
                        strcpy(instr->IP_datanode[instr->count_ino],MDList[j].IP_client);
                        instr->count_ino++;
                        MDList[j].sharenum++;  //共项数量加1
                        break;
                    }
                }
                if(kk ==false )
                {
                    //修改指纹
                    index_i = a.index;
                    printf(".............find address,finger id only , change finger id ............\n");
                    strcpy(MDList[index_i].ID_file,myNode->file_ID[i]);
                }
            }
            else if(a.type == 'C')
            {
                flag=true;
                //新增文件地址不存在，但是指纹内容重复了
                index_i = a.index;
                printf(".............new address but finger ID same,redirect and delete............\n");
                instr->ino_arr_del[instr->count_ino] = myNode->ino_arr[i];
                instr->ino_arr_redir[instr->count_ino] = MDList[index_i].ino;
                strcpy(instr->IP_datanode[instr->count_ino],MDList[index_i].IP_client);
                instr->count_ino++;
                MDList[index_i].sharenum++;  //共项数量加1
            }
            else if(a.type == 'D')
            {
                printf("only finger id ,store!\n");
                add_list(myNode->file_ID[i],myNode->ino_arr[i],IP_client);
            }
        }

       // 测试是否存表
        print_MD_list();

        pthread_mutex_unlock(&mutexglobel); // 释放互斥锁

        //发送指令
        if(flag == true)   instr->MSG = META_REDIRECT;
        else                instr->MSG = META_RECV;

        int needSend=sizeof(Instruction);
        int send_len = send(new_server_socket,  (void *)instr, needSend,0);
        
        //关闭链接释放缓存
        //free(send_buffer);
        free(recv_buffer);
        free(instr);
        free(myNode);
        instr =NULL;
        myNode =NULL;
        printf("S-C epoch end!\n\n\n");
        close(new_server_socket);
    }
    printf("close Server Socket!\n");
    close(server_socket);
}

void * listening_client_meta(void * argv)
{
    printf("This is a Server thread Lisening Client  = %ld\n",pthread_self());
    Socket_rev_meta();
    return NULL;
}

backindex is_in_List(const char ID_file[],const char IP_client[],const ino_t ino) //返回下标
{
    for (int i = 0; i < MAX_MDList_SIZE ; i++)
    {
        if(index_MD_List[i]==0)
        {
            continue;
        }
        //返回有四种情况
        //地址相同，指纹相同    原文件上传，不做任何改变
        //地址不同，指纹相同    
        //地址相同，指纹不同
        //地址不同，指纹不同

        if((strcmp(IP_client,MDList[i].IP_client) ==0 && ino == MDList[i].ino))
        {
            printf("find address,");
            if(strcmp(ID_file ,MDList[i].ID_file)==0)
            {
                printf("same finger id\n");
                backindex a = {i,'A'};
                return a ;
            }
            else
            {
                printf("different finger id\n");
                backindex a = {i,'B'};
                return a ;
            }
        }
    }

    for (int i = 0; i < MAX_MDList_SIZE ; i++)
    {
        if(index_MD_List[i]==0)
        {
            continue;
        }

        if((strcmp(ID_file ,MDList[i].ID_file)==0))  //只会检查有效位置,判断指纹
        {
            printf("address not exist,finger same\n");
            backindex a = {i,'C'}; //新增文件重删
            return a ;
        }
    }
    
    
    printf("A new finger\n");
    backindex a = {-1,'D'}; //新增文件,入表
    return a;
}

void add_list(const char ID_file[],const ino_t ino,const char IP_client[])
{
    if(total_number>=MAX_MDList_SIZE)
    {
        printf("The list is overload , process stop!\n");
        exit(0);
        return;
    }
    for (int i = 0; i < MAX_MDList_SIZE; i++)
    {
        if(index_MD_List[i] == 0)
        {
            //分配内存
            MDList[i].ino=ino;
            strcpy(MDList[i].ID_file,ID_file);
            strcpy(MDList[i].IP_client,IP_client);
            MDList[i].sharenum =0;
            index_MD_List[i]=1;
            total_number++;
            break;
        }
    }
}

void print_MD_list()
{
    printf("==================gloable list================\n");
    for (int i = 0; i < MAX_MDList_SIZE; i++)
    {
        if(index_MD_List[i]!=0)
        {
            printf("i:%d  IP_client:%s | ID_file:%s | ino_num:%d | sharenum:%d \n",i,MDList[i].IP_client,MDList[i].ID_file,MDList[i].ino,MDList[i].sharenum);

        }
    }
    printf("==============================================\n");
}

bool is_in_save_table(const char IPClient[50])
{
    for (int i = 0; i <total_client ; i++) //判断该IP地址是否已经加入
    {
        if(strcmp(save_IP_client[i],IPClient) == 0)  //已经存在
        {
            return false;
            break;
        }
    }
    return true;
}