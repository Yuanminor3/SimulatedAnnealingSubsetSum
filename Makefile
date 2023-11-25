CFLAGS=-O3
#CFLAGS=-g

all: SimAnneal.o main.o
	g++ $(CFLAGS) -o subsum SimAnneal.o main.o

SimAnneal.o: SimAnneal.cpp SimAnneal.h
	g++ $(CFLAGS) -c SimAnneal.cpp

main.o: main.cpp SimAnneal.h
	g++ $(CFLAGS) -c main.cpp

clean: 
	rm -f anneal.log
	rm -f output.txt
	rm *.o subsum

test1:
	./subsum list_100.txt

test2:
	./subsum list_1K.txt

test3:
	./subsum list_10K.txt 1818841



