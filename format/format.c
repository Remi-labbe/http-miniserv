#include "format.h"

#include <string.h>

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
