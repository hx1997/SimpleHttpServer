CFLAGS=-std=c11 -Wall -O3
OBJS=config.o http.o main.o parse.o platform.o socket.o

SimpleHttpServer: $(OBJS)
	$(CC) $(CFLAGS) $^ -o ../Debug/SimpleHttpServer

clean:
	rm -f *.o