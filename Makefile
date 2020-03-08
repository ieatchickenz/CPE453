CC = gcc -g -std=c99

CC_FLAGS = -c -Wall -pedantic -Wextra

# Target specific variable values
# https://www.gnu.org/software/make/manual/make.html#Target_002dspecific
all: clean non_diag
diag: clean minls minget
non_diag: CC_FLAGS += -DNDEBUG
non_diag: minls minget

minls: minls.o min_funcs.o utilities.o
	$(CC) -o minls minls.o min_funcs.o utilities.o
minls.o: minls.c min_funcs.c
	$(CC) $(CC_FLAGS) -c -o minls.o minls.c

minget: utilities.o minget.o min_funcs.o
	$(CC) -o minget minget.o utilities.o min_funcs.o
minget.o: minget.c min_funcs.c utilities.c
	$(CC) $(CC_FLAGS) -c -o minget.o minget.c

min_funcs.o: min_funcs.c min_funcs.h
	$(CC) $(CC_FLAGS) -c -o min_funcs.o min_funcs.c

utilities.o: utilities.c utilities.h
	$(CC) $(CC_FLAGS) -c -o utilities.o utilities.c
#<<<<<<< HEAD=======>>>>>>> fd8c888f5d6ba47164d5379e968114b24cf99640

clean:
	@rm -f *.o minls minget
