#include "tcp_socket.h"
#include "tcp_socket_type.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

tcp_socket *init_socket_tcp(void) {
  tcp_socket *s = malloc(sizeof(tcp_socket));
  if (s == NULL) {
    return NULL;
  }

  s->fd = -1;
  s->local = NULL;
  s->dist = NULL;
  s->is_connected = false;
  s->is_listenning = false;
  s->is_bound = false;

  return s;
}

int connect_socket_tcp(tcp_socket *s, const char *adresse, uint16_t port) {
  if (s == NULL || adresse == NULL) {
    fprintf(stderr, "connect_socket_tcp: socket or adresse = NULL\n");
    return -1;
  }

  int soc = socket(AF_INET, SOCK_STREAM, 0);
  if (soc == -1) {
    perror("socket");
    return -1;
  }

  s->fd = soc;

  adresse_internet *addr = adresse_internet_new(adresse, port);
  if (addr == NULL) {
    fprintf(stderr, "adresse_internet_new\n");
    return -1;
  }

  s->dist = addr;
  if (connect(s->fd, (const struct sockaddr *)&s->dist->sock_addr,
              sizeof s->dist->sock_addr) == -1) {
    perror("connect");
    adresse_internet_free(addr);
    return -1;
  }

  s->is_connected = true;

  return 0;
}

int ajoute_ecoute_socket_tcp(tcp_socket *s, const char *adresse,
                             uint16_t port) {
  if (s == NULL) {
    fprintf(stderr, "ajoute_ecoute_socket_tcp: tcp_socket = NULL\n");
    return -1;
  }

  int soc = socket(AF_INET, SOCK_STREAM, 0);
  if (soc == -1) {
    perror("socket");
    return -1;
  }

  s->fd = soc;

  adresse_internet *addr;
  if (adresse == NULL) {
    addr = adresse_internet_any(port);
  } else {
    addr = adresse_internet_new(adresse, port);
  }

  if (addr == NULL) {
    fprintf(stderr, "adresse_internet_new\n");
    return -1;
  }

  s->local = addr;

	int reuse = 1;
	if (setsockopt(s->fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		perror("setsockopt");
		return EXIT_FAILURE;
	}

  if (bind(s->fd, (const struct sockaddr *)&s->local->sock_addr,
           sizeof s->local->sock_addr) == -1) {
    perror("bind");
    return -1;
  }

  s->is_bound = true;

  if (listen(s->fd, SIZE_QUEUE) == -1) {
    perror("listen");
    return -1;
  }

  s->is_listenning = true;

  return 0;
}

int accept_socket_tcp(const tcp_socket s, tcp_socket *service) {

  struct sockaddr_storage addr;
  socklen_t addr_len = sizeof(addr);

  if ((service->fd = accept(s.fd, (struct sockaddr *)&addr, &addr_len)) == -1) {
    perror("accept");
    return -1;
  }

	int keepalive = 1;
	if (setsockopt(service->fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int)) == -1) {
		perror("setsockopt");
		return EXIT_FAILURE;
	}

  adresse_internet *adin = malloc(sizeof(adresse_internet));
  if (sockaddr_to_adresse_internet((const struct sockaddr *)&addr, adin) ==
      -1) {
    free(adin);
    fprintf(stderr, "accept : sock to ad_in\n");
    return -1;
  }

  service->dist = adin;
  service->is_connected = true;

  return 0;
}

int write_socket_tcp(const tcp_socket *s, const void *buffer, size_t length) {
  return (int)send(s->fd, buffer, length, 0);
}

int read_socket_tcp(const tcp_socket *s, void *buffer, size_t length) {
  return (int)recv(s->fd, buffer, length, 0);
}

int close_socket_tcp(tcp_socket *s) {
  if (close(s->fd) == -1) {
    perror("close");
    return -1;
  }
  adresse_internet_free(s->local);
  adresse_internet_free(s->dist);
  free(s);
  return 0;
}
