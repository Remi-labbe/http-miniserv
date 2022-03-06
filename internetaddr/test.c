#include "adresse_internet.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void print_adresse(adresse_internet *adresse) {
  printf("\tnom: %s\n\tservice: %s\n", adresse->nom, adresse->service);
  uint16_t port = adresse_internet_get_port(adresse);
  if (port == 0) {
    printf("\tProblème port\n");
  } else {
    printf("\tPort: %d\n", port);
  }
  int domain = adresse_internet_get_domain(adresse);
  printf("\tDomaine: ");
  switch (domain) {
    case AF_INET:
      printf("AF_INET\n");
      break;
    case AF_INET6:
      printf("AF_INET6\n");
      break;
    case -1:
      printf("\tproblème domaine\n");
      break;
  }
  char ip[INET6_ADDRSTRLEN];
  int r = adresse_internet_get_ip(adresse, ip, INET6_ADDRSTRLEN);
  if (r == 0) {
    printf("\tIP: %s\n", ip);
  } else {
    printf("\tproblème IP\n");
  }
}

int main(void) {
  printf("Test de la bibliothèque d'adressage\n");
  adresse_internet *adresse;
  adresse = adresse_internet_new("www.google.com", 80);
  if (adresse != NULL) {
    printf("Adresse www.google.com créée\n");
  } else {
    printf("Problème création d'adresse\n");
  }
  print_adresse(adresse);
  // la même
  adresse_internet *adresse1;
  adresse1 = adresse_internet_new("wailly.univ-rouen.fr", 80);
  if (adresse1 != NULL) {
    printf("Adresse wailly.univ-rouen.fr créée\n");
  } else {
    printf("Problème création d'adresse\n");
  }
  print_adresse(adresse1);
  int n = adresse_internet_compare(adresse, adresse1);
  printf(
      "les adresses www.univ-rouen.fr et wailly.univ-rouen.fr sont: ");
  switch (n) {
    case 0:
      printf("différentes\n");
      break;
    case 1:
      printf("identiques\n");
      break;
    default:
      printf("ERREUR!\n");
  }
  // une any
  adresse_internet_free(adresse);
  adresse = adresse_internet_any(80);
  if (adresse != 0) {
    printf("Adresse ANY créée\n");
  } else {
    printf("Problème création d'adresse\n");
  }
  print_adresse(adresse);
  adresse_internet_free(adresse);
  adresse_internet_free(adresse1);
  adresse1 = adresse_internet_loopback(80);
  if (adresse1 != 0) {
    printf("Adresse loopback créée\n");
  } else {
    printf("Problème création d'adresse\n");
  }
  print_adresse(adresse1);
  adresse_internet_free(adresse1);
}
