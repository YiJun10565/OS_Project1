cc=gcc
flag=-Wall -std=gnu99


default :
	${cc} ${flag} -c main.c 
	${cc} ${flag} -c task.c
	${cc} ${flag} main.o task.o

clean :
	rm a.out task.o main.o
