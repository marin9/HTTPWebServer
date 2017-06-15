CC=gcc
FLAGS=-Wall -Wextra -c
OBJ=main.o net.o file.o web.o mimetype.o


all: mweb scontrol

mweb : $(OBJ)
	$(CC) $(OBJ) -o mweb
	
scontrol : scon.o net.o 
	$(CC) scon.o net.o -o scontrol

	
scon.o : scon.c
		$(CC) $(FLAGS) scon.c


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


clean:
	rm *.o
	rm scontrol
	rm mweb

