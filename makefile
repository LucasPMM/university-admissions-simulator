all: main.o lista.o
	gcc lista.o main.o -o sisu
	make clean
	clear

lista.o:
	gcc -c lista.c

main.o: main.c
	gcc -c main.c

clean:
	rm *.o