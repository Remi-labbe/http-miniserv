#include "tcp_socket_type.h"
#include <stdlib.h>

#define SIZE_QUEUE 10

/**
 * @function  init_socket_tcp
 * @abstract  initialise une nouvelle socket
 * @return    tcp_socket    a new socket
 */
extern tcp_socket *init_socket_tcp(void);

/**
 * @function  connect_socket_tcp
 * @abstract  connecte la socket s a une adresse distante
 * @return    int     renvoie -1 en cas d'erreur, 0 sinon
 */
extern int connect_socket_tcp(tcp_socket *s, const char *adresse,
                              uint16_t port);

/**
 * @function  ajoute_ecoute_socket_tcp
 * @abstract  defini adresse et port comme l'adresse locale
 * @return    int     renvoie -1 en cas d'erreur, 0 sinon
 */
extern int ajoute_ecoute_socket_tcp(tcp_socket *s, const char *adresse,
                                    uint16_t port);

/**
 * @function  accept_socket_tcp
 * @abstract  attend et recupere une nouvelle connection sur la socket s et la stock dans service
 * @return    int     renvoie -1 en cas d'erreur, 0 sinon
 */
extern int accept_socket_tcp(const tcp_socket s, tcp_socket *service);

/**
 * @function  write_socket_tcp
 * @abstract  envoie le message contenu dans buffer de taille length sur la socket s
 * @return    int     renvoie la taille du message ecrit
 */
extern int write_socket_tcp(const tcp_socket *s, const void *buffer,
                            size_t length);

/**
 * @function  read_socket_tcp
 * @abstract  lit un message sur s de taille au plus length et le stock dans buffer
 * @return    int     renvoie la taille du message lu
 */
extern int read_socket_tcp(const tcp_socket *s, void *buffer, size_t length);

/**
 * @function  close_socket_tcp
 * @abstract  ferme les connection sur la socket s
 * @return    int     renvoie -1 en cas d'erreur, 0 sinon
 */
extern int close_socket_tcp(tcp_socket *s);
