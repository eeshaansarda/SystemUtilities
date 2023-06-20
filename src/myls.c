//  https://blog.packagecloud.io/eng/2016/04/05/the-definitive-guide-to-linux-system-calls/
//  https://en.wikibooks.org/wiki/X86_Assembly/Interfacing_with_Linux
//  man pages!!

#include <time.h>
#include <fcntl.h> // open, constants
#include <unistd.h> // _exit

#define BUF_SIZE 1024

#define WRITE_SYSCALL 1
#define OPEN_SYSCALL 2
#define STAT_SYSCALL 4
#define GETDENTS_SYSCALL 78

const char* MONTH[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
int tsize = 0;

struct linux_dirent {
    long           d_ino;
    off_t          d_off;
    unsigned short d_reclen;
    char           d_name[];
};

// https://stackoverflow.com/questions/14863465/using-write-to-print-to-console
int _strlen(const char *s);

void print(const char *s);
void printInt(int num);
void printTime(const struct tm time);
void printDetails(char* fdname, char* name);
void printRelativeDir(char* dir, char* name);

int myopen(char* fdname);
void mystat(char* fdname, struct stat *finfo);
int mygetdents(int fd, char* buff, int count);
int ls(char* fdname);

int main(int argc, char** argv) {
    // File / dir name
    char* fdname = argc > 1 ? argv[1]: ".";

    return ls(fdname);
}

int ls(char* fdname) {
    struct stat finfo;
    mystat(fdname, &finfo);

    if((finfo.st_mode & S_IFMT) == S_IFDIR) {
        int fd = myopen(fdname);

        char buff[BUF_SIZE];
        int nread = mygetdents(fd, buff, BUF_SIZE);
        struct linux_dirent *d;
        int bpos = 0;

        while(bpos < nread) {
            d = (struct linux_dirent*) (buff + bpos);
            // to pass ., .. and hidden stuff
            if(d->d_name[0] != '.') {
                printRelativeDir(fdname, d->d_name);
                print("\n");
            }
            bpos += d->d_reclen;
        }
        print("total ");
        printInt(tsize/2 + ((tsize % 2 == 0) ? 0 : 1));
        print("\n");
    }
    else if((finfo.st_mode & S_IFMT) == S_IFREG) {
        printDetails(fdname, fdname);
        print("\n");
    }
    else {
        print("No such regular file or directory\n");
        return -1;
    }
    return 0;
}

int _strlen(const char *s) {
    const char *end = s;
    while (*end++) {};
    return end-s-1;
}

// only to be used, when using the ret value immediately
char* intToString(int num) {
    if(num == 0) return "0";

    int number = num;
    static char numString[50];
    char *ptr = &numString[49];
    *ptr = '\0';
    while(number != 0) {
        *--ptr = '0' + (number % 10);
        number /= 10;
    }
    return ptr;
}

void printInt(int num) {
    print(intToString(num));
}

void printIntWithSpaces(int num, int spaces) {
    char* str = intToString(num);
    int len = _strlen(str);
    while(len < spaces) {
        print(" ");
        len++;
    }
    print(str);
}

void printTime(const struct tm time) {
    print(MONTH[time.tm_mon]);
    print(" ");
    printIntWithSpaces(time.tm_mday, 2);
    print(" ");
    if(time.tm_hour < 10) print("0");
    printInt(time.tm_hour);
    print(":");
    if(time.tm_min < 10) print("0");
    printInt(time.tm_min);
}

void printDetails(char* fdname, char* name) {
    struct stat finfo;
    mystat(fdname, &finfo);

    int perm[3] = {S_IRUSR, S_IWUSR, S_IXUSR};
    char* out[3] = {"r", "w", "x"};

    tsize += finfo.st_blocks;

    if((finfo.st_mode & S_IFMT) == S_IFDIR) print("d");
    else print("-");

    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < 3; j++) {
            if(finfo.st_mode & perm[j]) print(out[j]);
            else print("-");
            perm[j] = perm[j] >> 3;
        }
    }

    print(" ");
    printIntWithSpaces(finfo.st_nlink, 4);
    print(" ");
    printInt(finfo.st_uid);
    print(" ");
    printInt(finfo.st_gid);
    print(" ");
    printIntWithSpaces(finfo.st_size, 6);
    print(" ");
    printTime(*localtime(&finfo.st_mtime));
    print(" ");
    // https://www.theurbanpenguin.com/4184-2/
    // Set color
    if((finfo.st_mode & S_IFMT) == S_IFDIR) print("\033[0;32m");
    print(name);
    if((finfo.st_mode & S_IFMT) == S_IFDIR) print("\033[0m");
}

void printRelativeDir(char* dir, char* name) {
    char relativeDir[50];
    char *ptr = relativeDir;
    int dirSize = _strlen(dir), nameSize = _strlen(name);

    for(int i = 0; i<dirSize; i++) {*ptr++ = dir[i];}
    if(dir[dirSize-1] != '/') *ptr++ = '/';
    for(int i = 0; i<nameSize; i++) {*ptr++ = name[i];}

    *ptr = '\0';

    printDetails(relativeDir, name);
}

void print(const char *s) {
    // 1 for stdout, 2 for stderr, file handle from open() for files
    int ret = -1;
    asm("syscall" : "=a" (ret) : "0"(WRITE_SYSCALL), "D"(1), "S"(s), "d"(_strlen(s)) : "cc", "rcx", "r11", "memory");
    if(ret == -1) {
        print("Error print");
        _exit(-1);
    }
}

int myopen(char* fdname) {
    int fd;
    // O_RDWR = read/write, O_RDONLY = read only
    asm("syscall" : "=a" (fd) : "0"(OPEN_SYSCALL), "D"(fdname), "S"(O_RDONLY | O_DIRECTORY) : "cc", "rcx", "r11", "memory");
    return fd;
}


void mystat(char* fdname, struct stat *finfo) {
    int ret = -1;
    asm("syscall" : "=a" (ret) : "0"(STAT_SYSCALL), "D"(fdname), "S"(finfo) : "cc", "rcx", "r11", "memory");
    if(ret == -1) {
        print("Error mystat");
        _exit(-1);
    }
}

int mygetdents(int fd, char* buff, int count) {
    int nread;
    asm("syscall" : "=a" (nread) : "0"(GETDENTS_SYSCALL), "D"(fd), "S"(buff), "d"(count) : "cc", "rcx", "r11", "memory");
    return nread;
}
