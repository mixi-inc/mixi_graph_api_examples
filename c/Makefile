CC = gcc
OBJ = utils.o http.o mixi.o test.o

all: test

test: $(OBJ)
	$(CC) $(OBJ) -o test -lssl -lcrypto

utils.o: utils.c
	gcc -c utils.c

utils.o: utils.h

http.o: http.c
	gcc -c http.c

http.o: http.h

mixi.o: mixi.c
	gcc -c mixi.c

mixi.o: mixi.h

test.o: test.c
	gcc -c test.c

clean:
	rm -f test utils.o http.o test.o