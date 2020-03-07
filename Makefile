CC = gcc -Wall -pedantic -Wextra -g

CC_FLAGS = -c

all: minls
#minget

#diag: minls_diag minget_diag #for now these arent different
# minls_diag: minls.o minls.c #for now these arent different
# 	$(CC) -o minls minls.o
# minget_diag: minget.o minget.c #for now these arent different
# 	$(CC) -o minget minget.o

minls: utilities.o utilities.c minls.o minls.c min_funcs.o min_funcs.c
	$(CC) -o minls minls.o utilities.o min_funcs.o
minls.o: minls.c utilities.c utilities.h min_funcs.c min_funcs.h
	$(CC) $(CC_FLAGS) -o minls.o minls.c

minget: minget.o minget.c
	$(CC) -o minget minget.o
minget.o: minget.c
	$(CC) $(CC_FLAGS) -o minget minget.c

utilities: utilities.o utilities.c utilities.h
	$(CC) $(CC_FLAGS) -c utilities.o
utilities.o: utilities.c utilities.h
	$(CC) $(CC_FLAGS) -c utilities.c

min_funcs: min_funcs.o min_funcs.c
	$(CC) $(CC_FLAGS) -c min_funcs.o
min_funcs.o: min_funcs.o min_funcs.c
	$(CC) $(CC_FLAGS) -c min_funcs.c


clean:
	rm -f *.o
