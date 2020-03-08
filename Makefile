CC = gcc -g -std=c99

CC_FLAGS = -c -Wall -pedantic -Wextra
#-Wall -pedantic -Wextra
all: minls
#minget

diag: minls_diag minget_diag #for diagnostic print
# minls_diag: minls.o minls.c #for now these arent different
# 	$(CC) -o minls minls.o
# minget_diag: minget.o minget.c #for now these arent different
# 	$(CC) -o minget minget.o

minls: utilities.o minls.o minls.c min_funcs.o
	$(CC) -o minls minls.o utilities.o min_funcs.o
minls.o: minls.c min_funcs.c
	$(CC) $(CC_FLAGS) -o minls.o minls.c

minget: minget.o minget.c
	$(CC) -o minget minget.o
minget.o: minget.c
	$(CC) $(CC_FLAGS) -o minget minget.c

utilities.o: utilities.c utilities.h
	$(CC) $(CC_FLAGS) -c -o utilities.o utilities.c

min_funcs.o: min_funcs.o min_funcs.c
	$(CC) $(CC_FLAGS) -o min_funcs.o min_funcs.c


clean:
	rm -f *.o
