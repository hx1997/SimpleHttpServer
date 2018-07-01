CFLAGS=-std=c11 -Wall -O3
OBJS=config.o http.o main.o parse.o platform.o socket.o

simplehttpserver: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -f *.o simplehttpserver