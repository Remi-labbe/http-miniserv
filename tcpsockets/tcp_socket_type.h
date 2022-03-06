#ifndef TCPSOCKETTYPE_H_
#define TCPSOCKETTYPE_H_

#include "../tp4/adresse_internet_type.h"
#include <stdbool.h>

typedef struct {
  int fd;
  adresse_internet *local;
  adresse_internet *dist;
  bool is_connected;
  bool is_listenning;
  bool is_bound;
} _tcp_socket_struct;

typedef _tcp_socket_struct tcp_socket;

#endif // TCPSOCKETTYPE_H_
