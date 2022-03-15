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
#include <sys/socket.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

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

#define REQ_REGEX                                                              \
  "^(GET|POST|HEAD) /(.*\\.([a-zA-Z0-9]{1,4}))? HTTP/[0-9]\\.[0-9]$"

#define SERVER_NAME "http_miniserv"
#define PORT 8080

tcp_socket *local;
adresse_internet *addr;

int header(tcp_socket *client, int scode, const char *ext, off_t fsize);

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

#define REG_ERROR -1
#define REG_MATCH 0
#define REG_NO_MATCH 1

int redFormat(char *s, const char *pattern, regmatch_t matches[],
              size_t nmatches) {
  regex_t reg;
  if (regcomp(&reg, pattern, REG_EXTENDED)) {
    return REG_ERROR;
  }
  int res = regexec(&reg, s, nmatches, matches, 0);
  regfree(&reg);
  if (res == 0) {
    return REG_MATCH;
  } else if (res == REG_NOMATCH) {
    return REG_NO_MATCH;
  }
  return REG_ERROR;
}

void getGrp(const char *str, char *buf, regmatch_t *matches, size_t n) {
  const char *ptr = str + matches[n].rm_so;
  int len = matches[n].rm_eo - matches[n].rm_so;
  strncpy(buf, ptr, (size_t)len);
  buf[strlen(buf)] = 0;
}

#define MATCH_NB 3

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
  switch (redFormat(fstline, REQ_REGEX, matches, MATCH_NB + 1)) {
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
  getGrp(fstline, method, matches, 1);
  getGrp(fstline, path, matches, 2);
  getGrp(fstline, ext, matches, 3);
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

void status(const int code, char *buf) {
  switch (code) {
  case STATUS_OK:
    strcpy(buf, "Ok");
    break;
  case STATUS_NOT_MODIFIED:
    strcpy(buf, "Not Modified");
    break;
  case STATUS_BAD_REQUEST:
    strcpy(buf, "Bad Request");
    break;
  case STATUS_FORBIDDEN:
    strcpy(buf, "Forbidden");
    break;
  case STATUS_NOT_FOUND:
    strcpy(buf, "Not Found");
    break;
  case STATUS_INTERNAL_SERVER_ERROR:
    strcpy(buf, "Internal Server Error");
    break;
  case STATUS_NOT_IMPLEMENTED:
    strcpy(buf, "Not Implemented");
    break;
  default:
    strcpy(buf, "Internal Server Error");
  }
}

void content_type(const char *ext, char *buf) {
  if (strcmp(ext, "html") == 0) {
    strcpy(buf, "text/html");
  } else if (strcmp(ext, "png") == 0) {
    strcpy(buf, "image/png");
  } else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
    strcpy(buf, "image/jpeg");
  } else if (strcmp(ext, "mp4") == 0) {
    strcpy(buf, "video/mp4");
  } else if (strcmp(ext, "js") == 0) {
    strcpy(buf, "text/javascript");
  } else {
    strcpy(buf, "text/plain");
  }
}

int header(tcp_socket *client, int scode, const char *ext, off_t fsize) {
  char status_str[128] = {0};
  char cont_type[128] = {0};
  char buf[BUF_SIZE] = {0};
  status(scode, status_str);
  content_type(ext, cont_type);
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
            "Content-Type: %s\r\n"
            "Content-Length: %zu\r\n"
            "Connection: keep-alive\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "\r\n",
            scode, status_str, cont_type, fsize, now);
    break;
  case STATUS_NOT_MODIFIED:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "\r\n",
            scode, status_str, now);
    break;
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
            "Connection: close\r\n"
            "\r\n",
            scode, status_str, now);
    break;
  default:
    sprintf(buf,
            "HTTP/1.0 %d %s\r\n"
            "Date: %s\r\n"
            "Server: " SERVER_NAME "\r\n"
            "Content-type: text/html\r\n"
            "Connection: close\r\n"
            "\r\n",
            STATUS_INTERNAL_SERVER_ERROR, status_str, now);
  }
  if (write_socket_tcp(client, buf, strlen(buf)) == -1) {
    fprintf(stderr, "Couldn't send response\n");
    return -1;
  }
  return 0;
}
