CPPFLAGS = -D_XOPEN_SOURCE=700
CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings -Werror -fstack-protector-all -fpie -ld -O2 -g
#LDLIBS = -pthread -Wl,-z,relro,-z,now -fpie -lm
LDFLAGS =

objects = tcp_socket.o test.o
executable = test

all: $(executable)

$(executable): $(objects)
	$(CC) $(objects) $(LDFLAGS) -o $(executable)


tcp_socket.o: tcp_socket.c tcp_socket.h tcp_socket_type.h

test.o: test.c tcp_socket.h tcp_socket_type.h


clean:
	$(RM) -r *~ *.o $(executable) *.aux *.log *.synctex.gz



