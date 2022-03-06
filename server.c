#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "internetaddr/adresse_internet.h"
#include "internetaddr/adresse_internet_type.h"
#include "tcpsockets/tcp_socket.h"
#include "tcpsockets/tcp_socket_type.h"

#define BUF_SIZE 512

#define STATUS_OK 0
#define STATUS_FORBIDDEN 1
#define STATUS_NOT_FOUND 2

#define SERVER_NAME "http_miniserv"
#define PORT 8080

tcp_socket *local;
adresse_internet *addr;

// Threads related
/**
 * @function  start_th
 * @abstract  Start the ith thread in the runner_pool binding it to the client c
 * @param     i      index if the runner to start
 * @param     c      client to bind to the runner
 */
void start_th(tcp_socket *c);
/**
 * @function  runner_routine
 * @abstract  Routine ran by a thread when it is listening to a client
 * @param     r       the runner associated to the thread
 */
void *runner_routine(tcp_socket *c);

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

void header(const tcp_socket *soc, int status) {
  char req[128] = {0};
  printf("\n---------\n");
  recv(soc->fd, req, 128, 0);
  printf("%s", req);
  printf("\n---------\n");
  char header[BUF_SIZE] = {0};
  char now[128] = {0};
  time_t t = time(NULL);
  struct tm *tm = localtime(&t);
  if (tm == NULL) {
    status = STATUS_NOT_FOUND;
  }
  strftime(now, 128, "%a, %d %b %Y %H:%M:%S GMT", tm);
  if (status == STATUS_OK) {
    sprintf(header,
            "HTTP/1.1 200 OK\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/plain\r\n"
            "Content-length: %zu\r\n"
            "\r\n",
            now, strlen("Bonjour\n"));
  } else if (status == STATUS_FORBIDDEN) {
    sprintf(header,
            "HTTP/1.1 403 Forbidden\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n",
            now);
  } else {
    sprintf(header,
            "HTTP/1.1 404 Not Found\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/plain\r\n"
            "Connection: close\r\n"
            "\r\n",
            now);
  }
  write_socket_tcp(soc, header, strlen(header));
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
  if ((r = pthread_create(&th, &attr, (void *(*)(void *))runner_routine, c)) !=
      0) {
    fprintf(stderr, "pthread_create: %s\n", strerror(r));
    return;
  }
}

void *runner_routine(tcp_socket *client) {
  char buf[512] = {0};
  sprintf(buf, "Bonjour\n");
  header(client, STATUS_OK);
  if (write_socket_tcp(client, buf, strlen(buf)) == -1) {
    fprintf(stderr, "error sending message\n");
    close_socket_tcp(client);
    cleanup();
    return NULL;
  }
  close_socket_tcp(client);
  return NULL;
}
