CC=gcc
EXEC=battleShip

all: $(EXEC)

battleShip: battle.o main.o
	$(CC) -o $(EXEC) battle.o main.o

bataille.o: battle.c
	$(CC) -o battle.o -c battle.c -Wall -O
	
main.o: main.c battle.h
	$(CC) -o main.o -c main.c -Wall -O

clean:
	rm -f *.o
