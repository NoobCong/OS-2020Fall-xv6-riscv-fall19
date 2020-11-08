#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

int checkMatch(char *name, char *pattern)
{
    int len1 = strlen(name);
    int len2 = strlen(pattern);
    int match = 1;
    for (int i = 0; i <= len1 - len2; ++i)
    {
        match = 1;
        for (int j = 0; j < len2; ++j)
        {
            if (name[i + j] != pattern[j])
            {
                match = 0;
                break;
            }
        }
        if (match)
            return 1;
    }
    return 0;
}

void find(char *path, char *pattern)
{
    int fd;
    struct stat st;
    struct dirent de;
    char buf[512];

    if ((fd = open(path, 0)) < 0)
    {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0)
    {
        fprintf(2, "find: cannot fstat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type)
    {
    case T_DIR:
        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0 || !strcmp(de.name, ".") || !strcmp(de.name, ".."))
                continue;
            strcpy(buf, path);
            int len = strlen(buf);
            buf[len] = '/';
            strcpy(buf + len + 1, de.name);
            find(buf, pattern);
        }
        break;
    case T_FILE:
        if (checkMatch(path, pattern))
        {
            fprintf(1, "%s\n", path);
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(2, "%s u: not enough varguments!\n", argv[0]);
        fprintf(2, "usage: %s path pattern\n", argv[0]);
        exit();
    }
    find(argv[1], argv[2]);
    exit();
}
