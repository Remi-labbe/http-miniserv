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

#include "internetaddr/adresse_internet.h"
#include "internetaddr/adresse_internet_type.h"
#include "tcpsockets/tcp_socket.h"
#include "tcpsockets/tcp_socket_type.h"

#define BUF_SIZE 512

#define STATUS_OK 200
#define STATUS_NOT_MODIFIED 304
#define STATUS_BAD_REQUEST 400
#define STATUS_NOT_FOUND 404
#define STATUS_NOT_IMPLEMENTED 501

#define SERVER_NAME "http_miniserv"
#define PORT 8080

tcp_socket *local;
adresse_internet *addr;

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
    return;
  }
  if ((r = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED)) != 0) {
    fprintf(stderr, "pthread_attr_setdetachstate: %s\n", strerror(r));
    return;
  }
  if ((r = pthread_create(&th, &attr, (void *(*)(void *))th_routine, c)) != 0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(r));
    return;
  }
}

void *th_routine(tcp_socket *client) {
  char req[128] = {0};
  printf("\n---------\n");
  recv(client->fd, req, 128, 0);
  printf("%s", req);
  printf("\n---------\n");
  int status = STATUS_OK;
  char buf[BUF_SIZE] = {0};
  char now[128] = {0};
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (tm == NULL) {
    close_socket_tcp(client);
    return NULL;
  }
  strftime(now, 128, "%a, %d %b %Y %H:%M:%S GMT", tm);
  if (status == STATUS_OK) {
    sprintf(buf,
            "HTTP/1.1 200 OK\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            // "Content-length: %zu\r\n"
            // "\r\n",
            ,
            now);
  }
  if (write_socket_tcp(client, buf, strlen(buf)) == -1) {
    fprintf(stderr, "Couldn't send response\n");
    close_socket_tcp(client);
    return NULL;
  }
  int file = open("index.html", O_RDONLY, S_IRUSR);
  if (file == -1) {
    fprintf(stderr, "Couldn't open file\n");
    close_socket_tcp(client);
    return NULL;
  }
  struct stat s;
  if (fstat(client->fd, &s) == -1) {
    fprintf(stderr, "Error on file\n");
    close_socket_tcp(client);
    return NULL;
  }
  ssize_t blk_client = s.st_blksize;
  if (fstat(file, &s) == -1) {
    fprintf(stderr, "Error on file\n");
    close_socket_tcp(client);
    return NULL;
  }
  memset(buf, 0, BUF_SIZE);
  sprintf(buf,
          "Content-length: %zu\r\n"
          "\r\n",
          s.st_size);
  if (write_socket_tcp(client, buf, strlen(buf)) == -1) {
    fprintf(stderr, "Couldn't send response\n");
    close_socket_tcp(client);
    return NULL;
  }
  ssize_t size = s.st_blksize > blk_client ? blk_client : s.st_blksize;
  char msg[size];
  memset(msg, 0, (size_t)size);
  while (read(file, msg, (size_t)size) > 0) {
    if (write_socket_tcp(client, msg, strlen(msg)) == -1) {
      fprintf(stderr, "Couldn't send response\n");
      close_socket_tcp(client);
      return NULL;
    }
  }
  close_socket_tcp(client);
  return NULL;
}
