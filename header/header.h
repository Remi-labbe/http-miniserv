#ifndef HEADER_H
#define HEADER_H

#include "../tcpsockets/tcp_socket_type.h"

#define STATUS_OK 200
#define STATUS_NOT_MODIFIED 304
#define STATUS_BAD_REQUEST 400
#define STATUS_FORBIDDEN 403
#define STATUS_NOT_FOUND 404
#define STATUS_INTERNAL_SERVER_ERROR 500
#define STATUS_NOT_IMPLEMENTED 501

extern int header(tcp_socket *client, int scode, const char *ext, off_t fsize);

#endif // !HEADER_H
