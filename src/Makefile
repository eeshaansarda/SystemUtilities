##
# ls and cat implementation
##


CC = gcc
RM = rm -f
DFLAG = -g
GFLAGS = -Wall -Wextra
CFLAGS = $(DFLAG) $(GFLAGS) -c
LFLAGS = $(DFLAG) $(GFLAGS)

all: ls cat

ls: myls.o
	$(CC) $(LFLAGS) myls.o -o ls

cat: mycat.o
	$(CC) $(LFLAGS) mycat.o -o cat

myls.o: myls.c
	$(CC) $(CFLAGS) myls.c

mycat.o: mycat.c
	$(CC) $(CFLAGS) mycat.c

clean:
	$(RM) *.o

# end
