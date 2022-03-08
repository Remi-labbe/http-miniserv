#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
// #include <regex.h>

#include "internetaddr/adresse_internet.h"
#include "internetaddr/adresse_internet_type.h"
#include "tcpsockets/tcp_socket.h"
#include "tcpsockets/tcp_socket_type.h"

#define BUF_SIZE 1024

#define STATUS_OK 200
#define STATUS_NOT_MODIFIED 304
#define STATUS_BAD_REQUEST 400
#define STATUS_FORBIDDEN 403
#define STATUS_NOT_FOUND 404
#define STATUS_INTERNAL_SERVER_ERROR 500
#define STATUS_NOT_IMPLEMENTED 501

#define TYPE_GET "GET"

#define SERVER_NAME "http_miniserv"
#define PORT 8080

tcp_socket *local;
adresse_internet *addr;

int header(tcp_socket *client, int scode, off_t fsize);

// Threads related
/**
 * @function  start_th
 * @abstract  Start the ith thread in the runner_pool binding it to the client c
 * @param     c      client to bind to the runner
 */
void start_th(tcp_socket *c);
/**
 * @function  th_routine
 * @abstract  Routine ran by a thread when it isanswering a client
 * @param     c       client connected to the server on this thread
 */
void *th_routine(tcp_socket *c);

void cleanup(void) {
  if (local != NULL) {
    close_socket_tcp(local);
  }
  if (addr != NULL) {
    adresse_internet_free(addr);
  }
}

void handler(int signum) {
  cleanup();
  if (signum == SIGINT || signum == SIGQUIT) {
    printf("Disconnecting...\n");
    exit(EXIT_SUCCESS);
  }
  fprintf(stderr, "Wrong signal received [%d].\n", signum);
  exit(EXIT_FAILURE);
}

void setup_signals(void) {
  struct sigaction action;
  action.sa_handler = handler;
  action.sa_flags = 0;

  if (sigfillset(&action.sa_mask) == -1) {
    perror("sigfillset");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGINT, &action, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
  if (sigaction(SIGQUIT, &action, NULL) == -1) {
    perror("sigaction");
    exit(EXIT_FAILURE);
  }
}

int main(void) {
  setup_signals();
  local = init_socket_tcp();
  addr = adresse_internet_any(PORT);
  char ip[_DNS_NAME_MAX_SIZE] = {0};
  if (adresse_internet_get_ip(addr, ip, _DNS_NAME_MAX_SIZE) == -1) {
    fprintf(stderr, "couldn't add listening socket\n");
    cleanup();
    return -1;
  }
  if (ajoute_ecoute_socket_tcp(local, ip, adresse_internet_get_port(addr)) ==
      -1) {
    fprintf(stderr, "couldn't add listening socket\n");
    cleanup();
    return -1;
  }
  while (true) {
    tcp_socket *client = init_socket_tcp();
    if (client == NULL) {
      close_socket_tcp(client);
      cleanup();
      return -1;
    }
    printf("Waiting for request...\n");
    if (accept_socket_tcp(*local, client) == -1) {
      close_socket_tcp(client);
      cleanup();
      return -1;
    }
    start_th(client);
  }
  cleanup();
  return EXIT_SUCCESS;
}

void start_th(tcp_socket *c) {
  pthread_t th;
  pthread_attr_t attr;
  int r;
  if ((r = pthread_attr_init(&attr)) != 0) {
    fprintf(stderr, "pthread_attr_init: %s\n", strerror(r));
    close_socket_tcp(c);
    return;
  }
  if ((r = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
    fprintf(stderr, "pthread_attr_setdetachstate: %s\n", strerror(r));
    close_socket_tcp(c);
    return;
  }
  if ((r = pthread_create(&th, &attr, (void *(*)(void *))th_routine, c)) != 0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(r));
    close_socket_tcp(c);
    return;
  }
}

void *th_routine(tcp_socket *client) {
  char req[BUF_SIZE] = {0};
  printf("\n---------\n");
  recv(client->fd, req, BUF_SIZE, 0);
  printf("%s", req);
  printf("---------\n");
  //
  // TODO: Check request format
  //
  char *method, *path, *saveptr;
  method = strtok_r(req, " ", &saveptr);
  if (strcmp(method, TYPE_GET) != 0) {
    header(client, STATUS_NOT_IMPLEMENTED, 0);
    close_socket_tcp(client);
    return NULL;
  }
  path = strtok_r(NULL, " ", &saveptr);
  if (path[0] == '/') {
    ++path;
  }
  if (access(path, F_OK) != 0) {
    header(client, STATUS_NOT_FOUND, 0);
    close_socket_tcp(client);
    return NULL;
  } else if (access(path, R_OK) != 0) {
    header(client, STATUS_FORBIDDEN, 0);
    close_socket_tcp(client);
    return NULL;
  }
  //
  int file = open(path, O_RDONLY, S_IRUSR);
  if (file == -1) {
    header(client, STATUS_INTERNAL_SERVER_ERROR, 0);
    close_socket_tcp(client);
    return NULL;
  }
  struct stat s;
  if (fstat(file, &s) == -1) {
    header(client, STATUS_INTERNAL_SERVER_ERROR, 0);
    close_socket_tcp(client);
    return NULL;
  }
  if ((s.st_mode & S_IFMT) != S_IFREG) {
    header(client, STATUS_BAD_REQUEST, 0);
    close_socket_tcp(client);
    return NULL;
  }
  off_t fsize = s.st_size;
  ssize_t fblk = s.st_blksize;
  if (fstat(client->fd, &s) == -1) {
    header(client, STATUS_INTERNAL_SERVER_ERROR, 0);
    close_socket_tcp(client);
    return NULL;
  }
  header(client, STATUS_OK, fsize);
  ssize_t size = s.st_blksize > fblk ? fblk : s.st_blksize;
  // printf("fsize: %zu\nfblk: %zu\nsize: %zu\n", fsize, fblk, size);
  char msg[size];
  memset(msg, 0, (size_t)size);
  while (read(file, msg, (size_t)size) > 0) {
    if (write_socket_tcp(client, msg, strlen(msg)) == -1) {
      break;
    }
    printf(">WRITE\n");
    memset(msg, 0, (size_t)size);
  }
  printf(">END\n");
  close_socket_tcp(client);
  return NULL;
}

// int redFormat(char *s) {
//   regex_t reg;
//   regcomp(&reg, "", REG_EXTENDED);
//   return 0;
// }

int status(int code, char *buf) {
  switch (code) {
  case STATUS_OK:
    strcpy(buf, "Ok");
    return 0;
  // case STATUS_NOT_MODIFIED:
  //   strcpy(buf, "Not Modified");
  //   return 0;
  case STATUS_BAD_REQUEST:
    strcpy(buf, "Bad Request");
    return 0;
  case STATUS_FORBIDDEN:
    strcpy(buf, "Forbidden");
    return 0;
  case STATUS_NOT_FOUND:
    strcpy(buf, "Not Found");
    return 0;
  case STATUS_INTERNAL_SERVER_ERROR:
    strcpy(buf, "Internal Server Error");
    return 0;
  case STATUS_NOT_IMPLEMENTED:
    strcpy(buf, "Not Implemented");
    return 0;
  default:
    strcpy(buf, "Internal Server Error");
    return -1;
  }
}

int header(tcp_socket *client, int scode, off_t fsize) {
  char buf[BUF_SIZE] = {0};
  char status_str[128] = {0};
  if (status(scode, status_str) == -1) {
    return -1;
  }
  char now[128] = {0};
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (tm == NULL) {
    return -1;
  }
  strftime(now, 128, "%a, %d %b %Y %H:%M:%S GMT", tm);
  switch (scode) {
  case STATUS_OK:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "Content-length: %zu\r\n"
            "\r\n",
            scode, status_str, now, fsize);
    break;
  // case STATUS_NOT_MODIFIED:
  //   sprintf(buf,
  //           "HTTP/1.0 %d %s\r\n"
  //           "Date: %s\r\n"
  //           "Server: " SERVER_NAME "\r\n"
  //           "Content-type: text/html\r\n",
  //           scode, status_str, now);
  //   break;
  case STATUS_BAD_REQUEST:
  case STATUS_FORBIDDEN:
  case STATUS_NOT_FOUND:
  case STATUS_INTERNAL_SERVER_ERROR:
  case STATUS_NOT_IMPLEMENTED:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "Connection: close\r\n",
            scode, status_str, now);
    break;
  default:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "Connection: close\r\n",
            STATUS_INTERNAL_SERVER_ERROR, status_str, now);
  }
  if (write_socket_tcp(client, buf, strlen(buf)) == -1) {
    fprintf(stderr, "Couldn't send response\n");
    return -1;
  }
  return 0;
}
