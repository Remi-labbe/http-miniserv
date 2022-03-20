iadir=internetaddr/
tcpsdir=tcpsockets/
formatdir=format/
headdir=header/
docdir=doc/

CC = gcc

CPPFLAGS = -D_XOPEN_SOURCE=700

CFLAGS = -std=c18 -Wpedantic -Wall -Wextra -Wconversion -Wwrite-strings \
				 -Werror -fstack-protector-all -fpie -O2 -g \
				 -I$(iadir) -I$(tcpsdir) -I$(formatdir) -I$(headdir)

LDFLAGS = -pthread -Wl,-z,relro,-z,now -pie

VPATH = $(iadir) $(tcpsdir) $(formatdir) $(headdir)

OBJS = $(formatdir)format.o $(headdir)header.o \
			 $(iadir)adresse_internet.o $(tcpsdir)tcp_socket.o server.o

EXEC = server

DOCS = $(docdir)Manuel_Technique.pdf $(docdir)Manuel_Utilisateur.pdf

all: $(EXEC)

$(EXEC): $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

adresse_internet.o: adresse_internet.h adresse_internet_type.h adresse_internet.c

format.o: format.h format.c

header.o: header.h header.c config.h tcp_socket.h tcp_socket_type.h

tcp_socket.o: tcp_socket.h tcp_socket_type.h tcp_socket.c

server.o: server.c $(iadir)adresse_internet.o $(tcpsdir)tcp_socket.o \
	$(formatdir)format.o $(headdir)header.o

$(docdir)Manuel_Technique.pdf:
	pandoc --pdf-engine=pdflatex -o $@ $(docdir)tech_manual.md

$(docdir)Manuel_Utilisateur.pdf:
	pandoc --pdf-engine=pdflatex -o $@ $(docdir)user_manual.md

doc: $(DOCS)

clean:
	$(RM) $(EXEC) $(OBJS) $(DOCS)

tar:
	$(MAKE) clean
	$(MAKE) doc
	tar -zcf "$(CURDIR).tar.gz" server.c config.h internetaddr/* tcpsockets/* format/* header/* content/* doc/* Makefile
