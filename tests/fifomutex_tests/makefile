# define the C++ compiler to use
CC = gcc

# define any compile-time flags
CFLAGS = -Wall -g

# define any directories containing header files other than /usr/include
# INCLUDES = -I/home/newhall/include  -I../include #tutaj wrzucamy headery po spacji
# define library paths in addition to /usr/lib
#   if I wanted to include libraries not in /usr/lib I'd specify
#   their path using -Lpath, something like:
#LFLAGS = -L/home/newhall/lib  -L../lib

# define any libraries to link into executable:
#   if I want to link in libraries (libx.so or libx.a) I use the -llibname 
#   option, something like (this will link in libmylib.so and libm.so:
#LIBS = -lmylib -lm

# define the C source files
# tutaj leca pliki .c, .cc, .cpp
##SRCS = desc_manager.c fifo_mutex.c main.c sfs_ops.c sfs_vd.c simplefs.c simplefs_aux.c sf_test.c

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = main.o ../../src/fifo_mutex.o ../../src/simplefs_init.o

# define the executable file 
MAIN = ./

INIT = ../../bin/simplefs_init


# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'

.PHONY: clean

all:    process init closeinit
	@echo  Testing module has been compiled
	
#refresh:
#	rm -f process
#	rm -f closeinit
 #  	rm -f main.o
#	process init closeinit
#	@echo  Testing module has been compiled

$(MAIN): main.c ../../src/fifo_mutex.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)  
# $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
#.c.o:
#	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o

process: main.o ../../src/fifo_mutex.o
	$(CC) -o process main.o ../../src/fifo_mutex.o -I ../../src/	

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c	

init: ../../src/simplefs_init.o 
	$(CC) $(CFLAGS) -o $(INIT) ../../src/simplefs_init.o	

closeinit: closeinit.c
	$(CC) $(CFLAGS) closeinit.c -o closeinit ../../src/fifo_mutex.o -I ../../src/	









