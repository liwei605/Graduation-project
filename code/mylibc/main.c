#include<stdio.h>
#include<string.h>
#include <sys/stat.h>
#include<unistd.h>
int main()
{
    //传入绝对路径
    printf("\n开始\n");
    FILE* f1 = fopen("/home/liwei/桌面/code/Nodes/node1/test2.txt","r+");
    printf("\n结束\n");
    return 0;
}   

// // fprintf(f1, "%s %s %s %d", "We", "are", "in", 2014);
    // // FILE* f2 = fopen("/home/liwei/桌面/code/Nodes/node2/test2.txt","r+");
    // // fprintf(f2, "%s %s %s %d", "We", "are", "in", 2015);
    
    // sleep(4);
    // fclose(f1);
    // fclose(f2);