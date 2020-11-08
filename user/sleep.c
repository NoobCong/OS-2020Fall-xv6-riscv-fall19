#include "kernel/types.h"
#include "user/user.h"

//argn是命令行的参数个数 argv[]用来存放指向的字符串参数的指针数组，每一个元素指向一个参数
int main(int argc, char *argv[]){
    if(argc != 2){
        fprintf(2,"error\n");
        exit();
    }
    //C 库函数 int atoi(const char *str) 把参数 str 所指向的字符串转换为一个整数（类型为 int 型）。
    int number = atoi(argv[1]);
    printf("(nothing happens for a little while)\n");
    sleep(number);
    exit();
}