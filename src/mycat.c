// created using open, read and write system calls

#include <fcntl.h>
#include <unistd.h>

#define BUF_SIZE 1024*4

#define READ_SYSCALL 0
#define WRITE_SYSCALL 1
#define OPEN_SYSCALL 2

int _strlen(const char *s);
int readandwrite(int fd);

int print(const char *s);
int myopen(char* fdname);
int myread(int fd, char* buf, int sizeBuf);

int main(int argc, char **argv) {

    int fd = argc == 1 ? STDIN_FILENO : myopen(argv[1]);
    if (readandwrite(fd) != 0) print("Read or write error!");

    return 0;
}

int _strlen(const char *s) {
    const char *end = s;
    while (*end++) {};
    return end-s-1;
}

int readandwrite(int fd) {
    char buf[BUF_SIZE];
    int nread;

    while ((nread = myread(fd, buf, BUF_SIZE)) > 0) {
        int twritten = 0;
        while (twritten < nread) {
            int nwritten = print(buf + twritten);
            if (nwritten < 1)
                return -1;
            twritten += nwritten;
        }
    }
    return nread == 0 ? 0 : -1;
}

int print(const char *s) {
    // 1 for stdout, 2 for stderr, file handle from open() for files
    int ret = -1;
    asm("syscall" : "=a" (ret) : "0"(WRITE_SYSCALL), "D"(1), "S"(s), "d"(_strlen(s)) : "cc", "rcx", "r11", "memory");
    return ret;
}

int myopen(char* fdname) {
    int fd;
    // O_RDWR = read/write, O_RDONLY = read only
    asm("syscall" : "=a" (fd) : "0"(OPEN_SYSCALL), "D"(fdname), "S"(O_RDONLY) : "cc", "rcx", "r11", "memory");
    return fd;
}

int myread(int fd, char* buf, int sizeBuf) {
    int nread;
    asm("syscall" : "=a" (nread) : "0"(READ_SYSCALL), "D"(fd), "S"(buf), "d"(sizeBuf) : "cc", "rcx", "r11", "memory");
    return nread;
}
