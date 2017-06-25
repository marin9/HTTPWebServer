CC=gcc
FLAGS=-Wall -Wextra -c
OBJ=main.o net.o file.o web.o mimetype.o mft.o


all: mweb mft

mweb : $(OBJ)
	$(CC) $(OBJ) -o mweb

mft : client.o net.o mft.o file.o
	$(CC) client.o net.o mft.o file.o -o mft


mimetype.o : mimetype.c
	$(CC) $(FLAGS) mimetype.c

web.o : web.c
	$(CC) $(FLAGS) web.c

file.o : file.c
	$(CC) $(FLAGS) file.c

net.o : net.c
	$(CC) $(FLAGS) net.c

main.o : main.c
	$(CC) $(FLAGS) main.c

mft.o : mft.c
	$(CC) $(FLAGS) mft.c

client.o : client.c
	$(CC) $(FLAGS) client.c


clean:
	rm *.o
	rm mweb
	rm mft

