#ifndef TCPSOCKETTYPE_H_
#define TCPSOCKETTYPE_H_

#include "../internetaddr/adresse_internet.h"
#include <stdbool.h>

/**
* @typedef _tcp_socket_struct
*           struct to handle a TCP socket
* @field    fd              fd of the socket
* @field    local           local address initialised if the socket is listenning
* @field    dist            dist address initialiased when connected to another socket
* @field    is_connected    flag set if the socket is connected
* @field    is_listenning   flag set if the socket is listenning
* @field    is_bound        flag set if the socket is bound
*/
typedef struct {
  int fd;
  adresse_internet *local;
  adresse_internet *dist;
  bool is_connected;
  bool is_listenning;
  bool is_bound;
} _tcp_socket_struct;

/**
* @typedef  tcp_socket
*             exporting _tcp_socket_struct as tcp_socket
*/
typedef _tcp_socket_struct tcp_socket;

#endif // TCPSOCKETTYPE_H_
