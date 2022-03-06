#include "tcp_socket_type.h"
#include <stdlib.h>

#define SIZE_QUEUE 10

extern tcp_socket *init_socket_tcp(void);

extern int connect_socket_tcp(tcp_socket *s, const char *adresse,
                              uint16_t port);

extern int ajoute_ecoute_socket_tcp(tcp_socket *s, const char *adresse,
                                    uint16_t port);

extern int accept_socket_tcp(const tcp_socket s, tcp_socket *service);

extern int write_socket_tcp(const tcp_socket *s, const void *buffer,
                            size_t length);

extern int read_socket_tcp(const tcp_socket *s, void *buffer, size_t length);

extern int close_socket_tcp(tcp_socket *s);
