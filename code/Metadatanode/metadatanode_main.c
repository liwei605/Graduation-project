#include "metadatanode.h"


struct ThreadArgs {
    int client_socket;
    struct sockaddr_in client_addr;
};

void Socket_rev_meta_high_model(int client_socket,struct sockaddr_in client_addr) {
    // 你的代码逻辑
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
            recv_len = recv(client_socket, recv_buffer+recv_pos, BUFFER_SIZE, 0);
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
        // 加锁
        pthread_mutex_lock(&mutex);
        printf(".............checking list\n");
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
                //新增文件地址不存在，但是重复了
                index_i = a.index;
                printf(".............new address but finger ID same,redirect and delete............\n");
                instr->ino_arr_del[instr->count_ino] = myNode->ino_arr[i];
                instr->ino_arr_redir[instr->count_ino] = MDList[index_i].ino;
                strcpy(instr->IP_datanode[instr->count_ino],MDList[index_i].IP_client);
                instr->count_ino++;
            }
            else if(a.type == 'D')
            {
                printf("only finger id ,store!\n");
                add_list(myNode->file_ID[i],myNode->ino_arr[i],IP_client);
            }
        }

       // 测试是否存表
        print_MD_list();

        // 解锁
        pthread_mutex_unlock(&mutex);

        //发送指令
        if(flag == true)   instr->MSG = META_REDIRECT;
        else                instr->MSG = META_RECV;

        int needSend=sizeof(Instruction);
        int send_len = send(client_socket,  (void *)instr, needSend,0);
        
        //关闭链接释放缓存
        //free(send_buffer);
        free(recv_buffer);
        free(instr);
        free(myNode);
        instr =NULL;
        myNode =NULL;
        printf("S-C epoch end!\n\n\n");
        // 关闭客户端socket
        close(client_socket);
}

void *handle_client(void *arg) {
    struct ThreadArgs *args = (struct ThreadArgs *)arg;
    int client_socket = args->client_socket;
    struct sockaddr_in client_addr = args->client_addr;

    Socket_rev_meta_high_model(client_socket,client_addr);

    free(args); // 释放分配的内存

    return NULL;
}


// 监控线程函数
void * monitorThread(void* arg) {
    printf("Monitor begin! ................................\n");
    while (1) {
        // 等待 20 秒
        sleep(20);
        printf("Monitor start! ................................");
        // 遍历 MDList 数组
        for (int i = 0; i < total_number; i++) {
            pthread_mutex_lock(&mutexglobel); // 获取互斥锁

            // 如果 sharenum 为0，将索引置为0
            if (MDList[i].sharenum == 0) {
                index_MD_List[i] = 0;
            }

            pthread_mutex_unlock(&mutexglobel); // 释放互斥锁
        }
        printf("Monitor end! ..................................");
    }
    return NULL;
}


int main()
{
    //清理监视线程
    pthread_t monitor;
    pthread_create(&monitor, NULL, monitorThread, NULL);
  
    //元数据主线程
    pid_t process_id;
    process_id = getpid();
    printf("Process id : %d\n",process_id);
    pthread_t tid;
    if((pthread_create(&tid,NULL,listening_client_meta,NULL))!=0){
        fprintf(stderr,"pthread create error\n");
        exit(EXIT_FAILURE);
    }
    // 主线程等待一段时间，确保子线程有机会启动

    sleep(100);  // 5 秒，根据需要调整等待时间
    pthread_join(tid, NULL);
      // 等待监控线程结束
    pthread_join(monitor, NULL);

//=========================================================================================================================================================
   
    // // 初始化互斥锁
    // pthread_mutex_init(&mutex, NULL);

    //   // set socket's address information
    // struct sockaddr_in   server_addr;
    // bzero(&server_addr, sizeof(server_addr));
    // server_addr.sin_family = AF_INET;
    // //server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    // server_addr.sin_addr.s_addr = inet_addr("127.0.0.5");
    // server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);//6666 的十六进制为 2586

    // printf("Began create Socket!.......................... !\n");
    // // create a stream socket
    // int server_socket = socket(PF_INET, SOCK_STREAM, 0);
    // if (server_socket < 0)
    // {
    //     printf("Socket create failed!\n");
    //     exit(1);
    // }
    // printf("Mapping port!.............................. !\n");
    // //bind
    // if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
    // {
    //     printf("Port: %d failed!\n", HELLO_WORLD_SERVER_PORT);
    //     exit(1);
    // }
    // printf("server Lisening ........................!\n");
    // // listen
    // if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
    // {
    //     printf("Lisening failed!\n");
    //     exit(1);
    // }
    // //打印服务器地址
    // char IP_server[50]="";
    // strcpy( IP_server,inet_ntoa(server_addr.sin_addr));
    // printf("Server ip address ======> %s:%d\n\n\n",IP_server,server_addr.sin_port);

    // pthread_t thread_ids[MAX_CLIENTS];
    // int client_sockets[MAX_CLIENTS] = {0};
    // int client_count = 0;
    // struct ThreadArgs *args;

    // while(1)
    // {
    //     struct sockaddr_in client_addr;
    //     socklen_t length = sizeof(client_addr);

    //     int new_server_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
    //     if (new_server_socket < 0)
    //     {
    //         printf("Server recieve failed!\n");
    //         break;
    //     }else
    //     {
    //         printf("Socket file number %d\n",new_server_socket);
    //     }

    //     // 创建线程处理客户端请求
    //     args = (struct ThreadArgs *)malloc(sizeof(struct ThreadArgs));
    //     args->client_socket = new_server_socket;
    //     args->client_addr = client_addr;

    //     pthread_t thread_id;
    //     // 创建线程处理客户端请求
    //     if (pthread_create(&thread_id, NULL, handle_client, args) != 0) {
    //         printf("Failed to create thread for client!\n");
    //         break;
    //     }

    //      // 存储客户端socket，以便在线程中使用
    //     client_sockets[client_count++] = new_server_socket;
    //     // 存储线程ID，以便后续清理
    //     thread_ids[client_count - 1] = thread_id;

    //       // 打印 client_sockets 和 thread_ids 的内容
    //     printf("client_sockets: ");
    //     for (int i = 0; i < client_count; i++) {
    //         printf("%d ", client_sockets[i]);
    //     }
    //     printf("\n");

    //     printf("thread_ids: ");
    //     for (int i = 0; i < client_count; i++) {
    //         printf("%lu ", (unsigned long)thread_ids[i]);
    //     }
    //     printf("\n");
        
    //     // 检查并清理已完成的线程
    //     for (int i = 0; i < client_count; i++) {
    //         void *thread_result;
    //         if (pthread_tryjoin_np(thread_ids[i], &thread_result) == 0) {
    //             // 线程已完成，从数组中移除对应的元素
    //             client_sockets[i] = 0;
    //             thread_ids[i] = 0;
    //         }
    //     }

    //      // 更新 client_count 的值
    //     int updated_client_count = 0;
    //     for (int i = 0; i < client_count; i++) {
    //         if (client_sockets[i] != 0) {
    //             client_sockets[updated_client_count] = client_sockets[i];
    //             thread_ids[updated_client_count] = thread_ids[i];
    //             updated_client_count++;
    //         }
    //     }
    //     client_count = updated_client_count;
    // }
    // // 检查并清理已完成的线程
    // printf("清理线程，释放空间\n");
    // for (int i = 0; i < client_count; i++) {
    //     if (pthread_join(thread_ids[i], NULL) != 0) {
    //         printf("Failed to join thread for client!\n");
    //         //break;
    //     }else
    //     {
    //             printf("Success to join thread for client!\n");
    //     }
    //     client_sockets[i] = 0;
    // }
    // // 清理互斥锁
    // pthread_mutex_destroy(&mutex);

    // printf("close Server Socket!\n");
    // close(server_socket);
    // printf("end main\n");
    return 0;
}