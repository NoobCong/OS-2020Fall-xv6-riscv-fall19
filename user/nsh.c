#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXARGS 10
#define MAXWORD 30

char whitespace[] = " \t\r\n\v";
char args[MAXARGS];

void nshPipe(char *argv[], int argc);
int getcmd(char *buf, int nbuf);
void setargs(char *cmd, char *argv[], int *argc);
void runcmd(char *argv[], int argc);

int getcmd(char *buf, int nbuf)
{
    fprintf(2, "@ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) // EOF
        return -1;
    return 0;
}

void setargs(char *cmd, char *argv[], int *argc)
{
    // 让argv的每一个元素都指向args的每一行
    for (int i = 0; i < MAXARGS; i++)
    {
        argv[i] = &args[i];
    }
    int i = 0, j = 0;
    for (; cmd[j] != '\n' && cmd[j] != '\0'; j++)
    {
        // 跳过之前的空格
        while (strchr(whitespace, cmd[j]))
        {
            j++;
        }
        argv[i++] = cmd + j;
        // 如果不是空格，则找到下一个空格为止
        while (strchr(whitespace, cmd[j]) == 0)
        {
            j++;
        }
        cmd[j] = '\0';
    }
    argv[i] = 0;
    *argc = i;
}

void runcmd(char *argv[], int argc)
{
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "|"))
        {
            // 如果遇到 | ，说明后面有命令要执行
            nshPipe(argv, argc);
        }
    }
    for (int i = 1; i < argc; i++)
    {
        // > -> 输出重定向，关闭stdout
        if (!strcmp(argv[i], ">"))
        {
            close(1);
            //此时需要把输出重定向到后面给出的文件名对应的文件里
            open(argv[i + 1], O_CREATE | O_WRONLY);
            argv[i] = 0;
        }
        if (!strcmp(argv[i], "<"))
        {
            // < -> 输入重定向，关闭stdin
            close(0);
            open(argv[i + 1], O_RDONLY);
            argv[i] = 0;
        }
    }
    exec(argv[0], argv);
}

void nshPipe(char *argv[], int argc)
{
    int i = 0;
    // "|" -> '\0'
    for (; i < argc; i++)
    {
        if (!strcmp(argv[i], "|"))
        {
            argv[i] = 0;
            break;
        }
    }
    int fd[2];
    pipe(fd);
    if (fork() == 0)
    {
        //标准输出关闭
        close(1);
        dup(fd[1]);
        close(fd[0]);
        close(fd[1]);
        runcmd(argv, i);
    }
    else
    {
        //标准输入关闭
        close(0);
        dup(fd[0]);
        close(fd[0]);
        close(fd[1]);
        runcmd(argv + i + 1, argc - i - 1);
    }
}
int main()
{
    char buf[100];
    while (getcmd(buf, sizeof(buf)) >= 0)
    {

        if (fork() == 0)
        {
            char *argv[MAXARGS];
            int argc = -1;
            setargs(buf, argv, &argc);
            runcmd(argv, argc);
        }
        wait(0);
    }

    exit(0);
}