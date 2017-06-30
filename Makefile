CC=gcc
FLAGS=-Wall -Wextra -c
OBJ=main.o net.o file.o web.o mimetype.o mft.o


all: mweb mft

mweb : $(OBJ)
	$(CC) $(OBJ) -o mweb

mft : client.o net.o mft.o file.o
	$(CC) client.o net.o mft.o file.o -o mft


mimetype.o : mimetype.c mimetype.h
	$(CC) $(FLAGS) mimetype.c

web.o : web.c web.h
	$(CC) $(FLAGS) web.c

file.o : file.c file.h
	$(CC) $(FLAGS) file.c

net.o : net.c net.h
	$(CC) $(FLAGS) net.c

mft.o : mft.c mft.h
	$(CC) $(FLAGS) mft.c

main.o : main.c
	$(CC) $(FLAGS) main.c

client.o : client.c
	$(CC) $(FLAGS) client.c


clean:
	rm *.o
	rm mweb
	rm mft

