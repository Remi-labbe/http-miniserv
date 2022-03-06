iadir=internetaddr/
tcpsdir=tcpsockets/

CC = gcc

CPPFLAGS = -D_XOPEN_SOURCE=700

CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings \
				 -Werror -fstack-protector-all -fpie -O2 -g -I$(iadir) -I$(tcpsdir)

LDFLAGS = -pthread -Wl,-z,relro,-z,now -pie

VPATH = $(iadir) $(tcpsdir)

OBJS = $(iadir)adresse_internet.o $(tcpsdir)tcp_socket.o server.o

EXEC = server

# DOCS = $(doc_dir)Manuel_Technique.pdf $(doc_dir)Manuel_Utilisateur.pdf

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

adresse_internet.o: adresse_internet.h adresse_internet_type.h adresse_internet.c

tcp_socket.o: tcp_socket.h tcp_socket_type.h tcp_socket.c

server.o: server.c $(iadir)adresse_internet.o $(tcpsdir)tcp_socket.o

# $(doc_dir)Manuel_Technique.pdf:
# 	pandoc --pdf-engine=pdflatex -o $@ $(doc_dir)Manuel_Technique.md
#
# $(doc_dir)Manuel_Utilisateur.pdf:
# 	pandoc --pdf-engine=pdflatex -o $@ $(doc_dir)Manuel_Utilisateur.md
#
# doc: $(DOCS)

clean:
	$(RM) $(EXEC) $(OBJS)

tar:
	$(MAKE) clean
	# $(MAKE) doc
	tar -zcf "http_miniserv.tar.gz" server.c internetaddr/* tcp_sockets/* Makefile
