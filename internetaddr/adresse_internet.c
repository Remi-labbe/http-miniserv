#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include "adresse_internet.h"

adresse_internet *adresse_internet_new(const char *nom, uint16_t port) {
  char buf[_SERVICE_NAME_MAX_SIZE] = {0};
  snprintf(buf, _SERVICE_NAME_MAX_SIZE, "%hd", port);
  adresse_internet *adin = malloc(sizeof(adresse_internet));
  if (adin == NULL) {
    fprintf(stderr, "Error allocating.\n");
    return NULL;
  }

  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  struct addrinfo *result;
  int r;
  if ((r = getaddrinfo(nom, buf, &hints, &result)) != 0) {
    freeaddrinfo(result);
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    return NULL;
  }

  sprintf(adin->nom, "%s", nom);
  sprintf(adin->service, "%s", buf);
  memcpy(&(adin->sock_addr), result->ai_addr, result->ai_addrlen);

  freeaddrinfo(result);
  return adin;
}

adresse_internet *adresse_internet_any(uint16_t port) {
  char addr[INET_ADDRSTRLEN];
  struct in_addr inaddr;
  inaddr.s_addr = htonl(INADDR_ANY);
  inet_ntop(AF_INET, &inaddr, addr, INET_ADDRSTRLEN);
  return adresse_internet_new(addr, port);
}

adresse_internet *adresse_internet_loopback(uint16_t port) {
  char addr[INET_ADDRSTRLEN];
  struct in_addr inaddr;
  inaddr.s_addr = htonl(INADDR_LOOPBACK);
  inet_ntop(AF_INET, &inaddr, addr, INET_ADDRSTRLEN);
  return adresse_internet_new(addr, port);
}

void adresse_internet_free(adresse_internet *adresse) {
  if (adresse == NULL) {
    return;
  }
  free(adresse);
}

int adresse_internet_get_info(adresse_internet *adresse, char *nom_dns,
                              int taille_dns, char *nom_port, int taille_port) {
  if (nom_dns == NULL && nom_port == NULL) {
    return -1;
  }
  int r;
  if ((r = getnameinfo((struct sockaddr *)&(adresse->sock_addr),
                       sizeof(adresse->sock_addr), nom_dns,
                       (socklen_t)taille_dns, nom_port, (socklen_t)taille_port,
                       NI_NAMEREQD)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
    return -1;
  }
  return 0;
}

int adresse_internet_get_ip(const adresse_internet *adresse, char *ip,
                            int taille_ip) {
  if ((adresse->sock_addr).ss_family == AF_INET) {
    if (inet_ntop(AF_INET,
                  &(((struct sockaddr_in *)&(adresse->sock_addr))->sin_addr),
                  ip, (socklen_t)taille_ip) == NULL) {
      return -1;
    }
  } else {
    if (inet_ntop(AF_INET6,
                  &(((struct sockaddr_in6 *)&(adresse->sock_addr))->sin6_addr),
                  ip, (socklen_t)taille_ip) == NULL) {
      return -1;
    }
  }
  return 0;
}

uint16_t adresse_internet_get_port(const adresse_internet *adresse) {
  if (adresse == NULL) {
    return 0;
  }
  char *endptr;
  errno = 0;
  uint16_t port = (uint16_t)strtoul(adresse->service, &endptr, 10);
  if (errno != 0) {
    perror("strtoul");
    return 0;
  }
  if (endptr == adresse->service) {
    fprintf(stderr, "invalid service: %s\n", adresse->service);
    return 0;
  }

  return port;
}

int adresse_internet_get_domain(const adresse_internet *adresse) {
  if (adresse == NULL) {
    return -1;
  } else {
    return (adresse->sock_addr).ss_family;
  }
}

int sockaddr_to_adresse_internet(const struct sockaddr *addr,
                                 adresse_internet *adresse) {
  if (adresse == NULL) {
    return -1;
  }

  char nom[INET6_ADDRSTRLEN > INET_ADDRSTRLEN ? INET6_ADDRSTRLEN
                                              : INET_ADDRSTRLEN] = {0};

  if (addr->sa_family == AF_INET6) {
    struct sockaddr_in6 *tmp = (struct sockaddr_in6 *)addr;
    inet_ntop(AF_INET6, &(tmp->sin6_addr), nom, INET6_ADDRSTRLEN);
    snprintf(adresse->service, _SERVICE_NAME_MAX_SIZE, "%hd", tmp->sin6_port);
    memcpy(&(adresse->sock_addr), tmp, sizeof(*tmp));
    memcpy(&(adresse->nom), nom, sizeof(*nom));

  } else {
    struct sockaddr_in *tmp = (struct sockaddr_in *)addr;
    inet_ntop(AF_INET, &(tmp->sin_addr), nom, INET_ADDRSTRLEN);
    snprintf(adresse->service, _SERVICE_NAME_MAX_SIZE, "%hd", tmp->sin_port);
    memcpy(&(adresse->sock_addr), tmp, sizeof(*tmp));
    memcpy(&(adresse->nom), nom, sizeof(*nom));
  }
  return 0;
}

int adresse_internet_to_sockaddr(const adresse_internet *adresse,
                                 struct sockaddr *addr) {
  if (adresse == NULL || addr == NULL) {
    return -1;
  }
  if (sizeof(*addr) < sizeof(adresse->sock_addr)) {
    return -1;
  }
  memcpy(addr, &(adresse->sock_addr), sizeof(adresse->sock_addr));
  return 0;
}

int adresse_internet_compare(const adresse_internet *adresse1,
                             const adresse_internet *adresse2) {
  char ip1[INET6_ADDRSTRLEN];
  if (adresse_internet_get_ip(adresse1, ip1, INET6_ADDRSTRLEN) == -1) {
    return -1;
  }
  char ip2[INET6_ADDRSTRLEN];
  if (adresse_internet_get_ip(adresse2, ip2, INET6_ADDRSTRLEN) == -1) {
    return -1;
  }
  uint16_t port1 = adresse_internet_get_port(adresse1);
  if (port1 == 0) {
    return -1;
  }
  uint16_t port2 = adresse_internet_get_port(adresse2);
  if (port2 == 0) {
    return -1;
  }
  return ((strcmp(ip1, ip2) == 0) && (port1 == port2)) ? 1 : 0;
}

int adresse_internet_copy(adresse_internet *adrdst,
                          const adresse_internet *adrsrc) {
  if (adrdst == NULL || adrsrc == NULL || sizeof(*adrdst) < sizeof(*adrsrc)) {
    return -1;
  }
  memcpy(adrdst, adrsrc, sizeof(*adrsrc));
  return 0;
}
