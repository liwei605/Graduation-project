#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

int main() {
    int res;
    char filepath[100] = {0};

    // 检查并创建节点文件夹
    //filepath为绝对路径
    //如果文件夹不存在
    if(access("/home/liwei/桌面/code/Nodes/node1",0)!=0)
    {
        //创建文件夹
        res = mkdir("/home/liwei/桌面/code/Nodes/node1",0777);// 返回 0 表示创建成功，-1 表示失败
        if(res != 0)
        {
            perror("Creat node1 :");
        }
    }
    else
    {
        system("rm -rf /home/liwei/桌面/code/Nodes/node1"); //删除文件夹
        //创建文件夹
        res = mkdir("/home/liwei/桌面/code/Nodes/node1",0777);// 返回 0 表示创建成功，-1 表示失败
        if(res != 0)
        {
            perror("=> Creat node1 :");
        }
    }
    
    if(access("/home/liwei/桌面/code/Nodes/node2",0)!=0)
    {
        //创建文件夹
        res = mkdir("/home/liwei/桌面/code/Nodes/node2",0777);// 返回 0 表示创建成功，-1 表示失败
        if(res != 0)
        {
            perror("Creat node2 :");
        }
    }else
    {
        system("rm -rf /home/liwei/桌面/code/Nodes/node2"); //删除文件夹
        //创建文件夹
        res = mkdir("/home/liwei/桌面/code/Nodes/node2",0777);// 返回 0 表示创建成功，-1 表示失败
        if(res != 0)
        {
            perror("=> Creat node2 :");
        }
    }
    
    if(access("/home/liwei/桌面/code/Nodes/node3",0)!=0)
    {
        //创建文件夹
        res = mkdir("/home/liwei/桌面/code/Nodes/node3",0777);// 返回 0 表示创建成功，-1 表示失败
        if(res != 0)
        {
            perror("Creat node3 :");
        }
    }else
    {
        system("rm -rf /home/liwei/桌面/code/Nodes/node3"); //删除文件夹
        //创建文件夹
        res = mkdir("/home/liwei/桌面/code/Nodes/node3",0777);// 返回 0 表示创建成功，-1 表示失败
        if(res != 0)
        {
            perror("=> Creat node3 :");
        }
    }


    srand(time(NULL));
    FILE *fp;

    // 生成100个文件并写入随机数
    for (int node = 1; node <= 3; node++) {
        for (int i = 0; i < 100; i++) {
            sprintf(filepath, "/home/liwei/桌面/code/Nodes/node%d/test%d.txt", node, i);
            fp = fopen(filepath, "w+");
            if (fp == NULL) {
                perror("Open file:");
                exit(1);
            }

            fprintf(fp, "%d", rand() % 51);
            fclose(fp);
        }
        sleep(0.5);
    }

    // 生成20个bin文件并写入随机数
    for (int node = 1; node <= 3; node++) {
        for (int i = 0; i < 20; i++) {
            sprintf(filepath, "/home/liwei/桌面/code/Nodes/node%d/binfile%d.bin", node, i);
            fp = fopen(filepath, "wb");
            if (fp == NULL) {
                perror("Open file:");
                exit(1);
            }

            fprintf(fp, "%d", rand() % 11);
            fclose(fp);
        }
        sleep(0.5);
    }

    // 复制随机的图片文件
    for (int node = 1; node <= 3; node++) {
        for (int i = 0; i < 15; i++) {
            int ra =rand() % 21;
            sprintf(filepath, "cp /home/liwei/桌面/code/Nodes/image/im%d.png /home/liwei/桌面/code/Nodes/node%d/im%d.png", ra, node, ra);
            system(filepath);
        }
        sleep(0.5);
    }

    // 复制随机的压缩文件
    for (int node = 1; node <= 3; node++) {
        for (int i = 0; i < 15; i++) {
            int ra =rand() % 21;
            sprintf(filepath, "cp /home/liwei/桌面/code/Nodes/zipdir/zi%d.zip /home/liwei/桌面/code/Nodes/node%d/zi%d.zip", ra, node, ra);
            system(filepath);
        }
        sleep(0.5);
    }

    // 复制随机的视频文件
    for (int node = 1; node <= 3; node++) {
        for (int i = 0; i < 10; i++) {
            int ra =rand() % 21;
            sprintf(filepath, "cp /home/liwei/桌面/code/Nodes/video/vid%d.mp4 /home/liwei/桌面/code/Nodes/node%d/vid%d.mp4", ra, node, ra);
            system(filepath);
        }
        sleep(0.5);
    }

    return 0;
}