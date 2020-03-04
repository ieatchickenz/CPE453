CC = gcc -Wall -pedantic -Wextra -g

CC_FLAGS = -c

all:minls minget

minls: minls.o minls.c
	$(CC) -o minls minls.o

minls.o: minls.c
	$(CC) $(CC_FLAGS) -o minls.o minls.c

minget: minget.o minget.c
	$(CC) -o minget minget.o
	
minget: minget.c
	$(CC) $(CC_FLAGS) -o minget minget.c 

clean:
	rm -f *.o