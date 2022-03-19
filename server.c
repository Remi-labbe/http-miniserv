#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE
#endif /* !_XOPEN_SOURCE */
#include <errno.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "format/format.h"
#include "header/header.h"
#include "internetaddr/adresse_internet.h"
#include "internetaddr/adresse_internet_type.h"
#include "tcpsockets/tcp_socket.h"
#include "tcpsockets/tcp_socket_type.h"

#define BUF_SIZE 1024

#define TYPE_GET "GET"

tcp_socket *local;
adresse_internet *addr;

void start_th(tcp_socket *c);

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
      cleanup();
      return -1;
    }
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
  if ((r = pthread_attr_destroy(&attr)) != 0) {
    fprintf(stderr, "pthread_attr_destroy: %s\n", strerror(r));
    exit(EXIT_FAILURE);
  }
}

// do {
// readline
// } while (line != "")
void *th_routine(tcp_socket *client) {
  char req[BUF_SIZE] = {0};
  printf("---------\n");
  read_socket_tcp(client, req, BUF_SIZE);
  printf("%s", req);
  printf("---------\n");
  char *fstline, *saveptr;
  fstline = strtok_r(req, "\r\n", &saveptr);
  regmatch_t matches[MATCH_NB + 1];
  switch (reg_test(fstline, REQ_REGEX, matches, MATCH_NB + 1)) {
  case REG_ERROR:
    header(client, STATUS_INTERNAL_SERVER_ERROR, "", 0);
    close_socket_tcp(client);
    return NULL;
  case REG_NO_MATCH:
    header(client, STATUS_BAD_REQUEST, "", 0);
    close_socket_tcp(client);
    return NULL;
  default:;
  }
  char method[64] = {0}, path[128] = {0}, ext[8] = {0};
  req_get_group(fstline, method, matches, 1);
  req_get_group(fstline, path, matches, 2);
  req_get_group(fstline, ext, matches, 3);
  if (strcmp(method, TYPE_GET) != 0) {
    header(client, STATUS_NOT_IMPLEMENTED, "", 0);
    close_socket_tcp(client);
    return NULL;
  }
  if (access(path, F_OK) != 0) {
    header(client, STATUS_NOT_FOUND, "", 0);
    close_socket_tcp(client);
    return NULL;
  } else if (access(path, R_OK) != 0) {
    header(client, STATUS_FORBIDDEN, "", 0);
    close_socket_tcp(client);
    return NULL;
  }
  int file = open(path, O_RDONLY, S_IRUSR);
  if (file == -1) {
    header(client, STATUS_INTERNAL_SERVER_ERROR, "", 0);
    close_socket_tcp(client);
    return NULL;
  }
  struct stat s;
  if (fstat(file, &s) == -1) {
    header(client, STATUS_INTERNAL_SERVER_ERROR, "", 0);
    close_socket_tcp(client);
    return NULL;
  }
  char *line = NULL, *date;
  while ((line = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
    const char *pattern = "If-Modified-Since: ";
    if (strncmp(line, pattern, strlen(pattern)) == 0) {
      strtok_r(line, ":", &saveptr);
      date = strtok_r(NULL, "\r\n", &saveptr);
      ++date;
      break;
    }
  }
  if (line != NULL) {
    struct tm tm;
    if (strptime(date, "%a, %d %b %Y %H:%M:%S GMT", &tm) == NULL) {
      header(client, STATUS_INTERNAL_SERVER_ERROR, "", 0);
      close_socket_tcp(client);
      return NULL;
    }
    time_t r_time;
    if ((r_time = mktime(&tm)) == -1) { // request time as time_t
      header(client, STATUS_INTERNAL_SERVER_ERROR, "", 0);
      close_socket_tcp(client);
      return NULL;
    }
    time_t f_time = s.st_mtime; // last file modification
    double diff = difftime(f_time, r_time);
    if (diff < 0) {
      header(client, STATUS_NOT_MODIFIED, ext, 0);
      close_socket_tcp(client);
      return NULL;
    }
  }
  if ((s.st_mode & S_IFMT) != S_IFREG) {
    header(client, STATUS_FORBIDDEN, "", 0);
    close_socket_tcp(client);
    return NULL;
  }
  off_t fsize = s.st_size;
  ssize_t fblk = s.st_blksize;
  if (fstat(client->fd, &s) == -1) {
    header(client, STATUS_INTERNAL_SERVER_ERROR, "", 0);
    close_socket_tcp(client);
    return NULL;
  }
  header(client, STATUS_OK, ext, fsize);
  ssize_t size = s.st_blksize > fblk ? fblk : s.st_blksize;
  char msg[size];
  ssize_t n;
  while ((n = read(file, msg, (size_t)size)) != 0) {
    if (write_socket_tcp(client, msg, (size_t)n) == -1) {
      perror("write");
      break;
    }
  }
  if (n == -1) {
    perror("read");
  }
  close_socket_tcp(client);
  return NULL;
}
