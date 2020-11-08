#include "kernel/types.h"
#include "user/user.h"

int main(){
    int parentfd[2], childfd[2];
    
    char buf[64];
    
    pipe(parentfd); //创建管道函数
    pipe(childfd); //创建管道函数

    if(fork() == 0){
        //对于子进程，返回0;对于父进程，返回子进程ID，所以下面一段是子进程执行的代码段，else后是父进程执行的代码段
        read(parentfd[0], buf, sizeof(buf)); //parentfd[0]读端
        printf("%d: received %s\n", getpid(), buf);
        write(childfd[1],"pong",sizeof(buf)); //childfd[1]写端
    }else
    {
        write(parentfd[1],"ping",sizeof(buf)); //parentfd[1]写端
        read(childfd[0], buf, sizeof(buf)); //childfd[0]读端
        printf("%d: received %s\n", getpid(), buf);
    }
    exit();
}