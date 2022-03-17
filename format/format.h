#ifndef FORMAT_H
#define FORMAT_H

#include <regex.h>

#define REG_ERROR -1
#define REG_MATCH 0
#define REG_NO_MATCH 1

#define MATCH_NB 3

#define REQ_REGEX                                                              \
  "^(GET|POST|HEAD) /(.*\\.([a-zA-Z0-9]{1,4}))? HTTP/[0-9]\\.[0-9]$"

extern int redFormat(char *s, const char *pattern, regmatch_t matches[],
                     size_t nmatches);

extern void getGrp(const char *str, char *buf, regmatch_t *matches, size_t n);

#endif // !FORMAT_H
