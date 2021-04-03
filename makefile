CFLAGS = -Wpedantic -Wall -Wextra -Werror -Wconversion -pthread -fstack-protector-all -std=c11 -D_XOPEN_SOURCE=700 -I.
LDFLAGS = -pthread -lrt -L.
ARFLAGS = rs

all: demon client affichage


affichage: affichage.o
	$(CC) $(CFLAGS) affichage.o $(LDFLAGS) -o affichage 
affichage.o: affichage.c
	$(CC) $(CFLAGS) -c affichage.c
demon: demon.o
	$(CC) $(CFLAGS) demon.o $(LDFLAGS) -o demon
client: client.o 
	$(CC) $(CFLAGS) client.o $(LDFLAGS) -o client 
demon.o: demon.c
	$(CC) $(CFLAGS) -c demon.c util.h
client.o: client.c
	$(CC) $(CFLAGS) -c client.c util.h

clean:
	$(RM) affichage demon client *.o *.gch
	$(RM) PROJET_SE_2016*
clear:
	$(RM) *.o *.gch
	$(RM) PROJET_SE_2016*