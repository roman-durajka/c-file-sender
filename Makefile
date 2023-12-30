LIBS = -pthread
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

.PHONY: clean all

all: client server

client: $(filter-out server.o, $(OBJS))
	gcc $(filter-out server.o, $(OBJS)) $(LIBS) -o client

server: $(filter-out client.o, $(OBJS))
	gcc $(filter-out client.o, $(OBJS)) $(LIBS) -o server

%.o: %.c
	gcc -c $(CFLAGS) $< -o $@

clean:
	@rm -rf $(OBJS) client server
