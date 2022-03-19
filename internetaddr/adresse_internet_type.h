#ifndef ADRESSEINTERNETTYPE_H_
#define ADRESSEINTERNETTYPE_H_

#include <netinet/ip.h>

/**
* @define _DNS_NAME_MAX_SIZE    size allocated for the dns string
*/
#define _DNS_NAME_MAX_SIZE 256

/**
* @define _SERVICE_NAME_MAX_SIZE    size allocated for the service string
*/
#define _SERVICE_NAME_MAX_SIZE 20

/**
* @typedef _adresse_internet_struct
*         struct storing infos used for a TCP connection
* @field    sock_addr   struct giving infos like ip version...
* @field    nom         the ip
* @field    service     the port
*/
typedef struct {
  struct sockaddr_storage sock_addr;
  char nom[_DNS_NAME_MAX_SIZE];
  char service[_SERVICE_NAME_MAX_SIZE];
} _adresse_internet_struct;

/**
* @typedef adresse_internet
*           exporting _adresse_internet_struct as adresse_internet
*/
typedef _adresse_internet_struct adresse_internet;

#endif // ADRESSEINTERNETTYPE_H_
