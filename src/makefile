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
SRCS = desc_manager.c fifo_mutex.c main.c sfs_ops.c sfs_vd.c simplefs.c simplefs_aux.c sf_test.c

# define the C object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = ../bin/main

# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'

.PHONY: clean

all:    $(MAIN)
	@echo  Project has been compiled
	
build: $(MAIN)
	@echo Project has been built
	
rebuild: clean $(MAIN)
	@echo Project has been rebuilt

test: 
	$(MAIN)

$(MAIN): $(OBJS) 
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
	
delbin:
	$(RM) *~ $(MAIN)
	
cleanall: clean delbin

main.o: main.c
	$(CC) $(CFLAGS) -c -o main.o main.c
	
desc_manager.o: desc_manager.c desc_manager.h
	$(CC) $(CFLAGS) -c -o desc_manager.o desc_manager.c

fifo_mutex.o: fifo_mutex.c fifo_mutex.h
	$(CC) $(CFLAGS) -c -o fifo_mutex.o fifo_mutex.c

sfs_ops.o: sfs_ops.c sfs_ops.h
	$(CC) $(CFLAGS) -c -o sfs_ops.o sfs_ops.c
	
sfs_vd.o: sfs_vd.c sfs_vd.h
	$(CC) $(CFLAGS) -c -o sfs_vd.o sfs_vd.c
	
simplefs.o: simplefs.c simplefs.h
	$(CC) $(CFLAGS) -c -o simplefs.o simplefs.c
	
simplefs_aux.o: simplefs_aux.c simplefs_aux.h
	$(CC) $(CFLAGS) -c -o simplefs_aux.o simplefs_aux.c
	
sf_test.o: sf_test.c sf_test.h
	$(CC) $(CFLAGS) -c -o sf_test.o sf_test.c
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	
	