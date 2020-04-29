cc=gcc
flag=-Wall -std=gnu99


default :
	${cc} ${flag} -c main.c 
	${cc} ${flag} -c task.c
	${CC} ${flag} -c Queue.c
	${cc} ${flag} main.o task.o Queue.o
clean :
	rm a.out task.o main.o Queue.o
