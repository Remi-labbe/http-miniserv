#ifndef HEADER_H
#define HEADER_H

#include "../tcpsockets/tcp_socket_type.h"

/**
 * @define STATUS_*  Status codes the server can return as defined in RFC1945
 */
#define STATUS_OK 200
#define STATUS_NOT_MODIFIED 304
#define STATUS_BAD_REQUEST 400
#define STATUS_FORBIDDEN 403
#define STATUS_NOT_FOUND 404
#define STATUS_INTERNAL_SERVER_ERROR 500
#define STATUS_NOT_IMPLEMENTED 501

/**
 * @function  header
 * @abstract  build and send the header responding to a request
 * @param   client      a tcp_socket storing information about the client
 *                        who sent the request
 * @param   scode       status code sent defining the result of the request
 * @param   ext         file extension of the requested file, unused if scode
 *                        is an error code
 * @param   fsize       The size in Bytes of the content to send
 */
extern int header(tcp_socket *client, int scode, const char *ext, off_t fsize);

#endif // !HEADER_H
