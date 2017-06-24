CC=gcc
FLAGS=-Wall -Wextra -c
OBJ=main.o net.o file.o web.o mimetype.o mft.o


all: mweb client

mweb : $(OBJ)
	$(CC) $(OBJ) -o mweb
	
client : client.o net.o
	$(CC) client.o net.o -o client
	

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


clean:
	rm *.o
	rm mweb
	rm client

