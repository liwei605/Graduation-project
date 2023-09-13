#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string.h>


int main()
{
    int ko =100;
    FILE *fp;
    char filepath[100]={0};
    //node1;
    strcpy(filepath,"");
    //创建50个文件
    for (int i = ko; i < ko+50; i++)
    {
        strcpy(filepath,"/home/liwei/桌面/code/Nodes/node1/test");
        char A[10]={0};
        sprintf(A,"%d",i);
        strcat(filepath,A);
        strcat(filepath,".txt");
        printf("%s\n",filepath);
	    fp = fopen(filepath,"w+"); // 创建名为test.c文件,"w+"该文件可读可写
        fprintf(fp,"%d",rand()%51);//将0-50的随机数写入文件
        fclose(fp);
    }

     //node2;
    strcpy(filepath,"");
    //创建50个文件
    for (int i = ko; i < ko+50; i++)
    {
        strcpy(filepath,"/home/liwei/桌面/code/Nodes/node2/test");
        char A[10]={0};
        sprintf(A,"%d",i);
        strcat(filepath,A);
        strcat(filepath,".txt");
        printf("%s\n",filepath);
	    fp = fopen(filepath,"w+"); // 创建名为test.c文件,"w+"该文件可读可写
        fprintf(fp,"%d",rand()%51);//将0-50的随机数写入文件
        fclose(fp);
    }
    
    //node3;
    strcpy(filepath,"");
    //创建50个文件
    for (int i = ko; i < ko+50; i++)
    {
        strcpy(filepath,"/home/liwei/桌面/code/Nodes/node3/test");
        char A[10]={0};
        sprintf(A,"%d",i);
        strcat(filepath,A);
        strcat(filepath,".txt");
        printf("%s\n",filepath);
	    fp = fopen(filepath,"w+"); // 创建名为test.c文件,"w+"该文件可读可写
        fprintf(fp,"%d",rand()%51);//将0-50的随机数写入文件
        fclose(fp);
    }
    return 0;
}