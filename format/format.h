#ifndef FORMAT_H
#define FORMAT_H

#include <regex.h>

/**
* @define REG_*  redFormat possible return values.
*/
#define REG_ERROR -1
#define REG_MATCH 0
#define REG_NO_MATCH 1

/**
* @define MATCH_NB  Number of groups in the header regex
*/
#define MATCH_NB 3

/**
* @define REQ_REGEX  defines the format of the first line of a request
*/
#define REQ_REGEX                                                              \
  "^(GET|POST|HEAD) /(.*\\.([a-zA-Z0-9]{1,4}))? HTTP/[0-9]\\.[0-9]$"

/**
 * @function  reg_test
 * @abstract  test if a string matches a regex
 * @param   s           the string to test
 * @param   pattern     Regex pattern the string should match
 * @param   matches     array storing the groups in the regex
 * @param   nmatches    The number of groups in the regex
 */
extern int reg_test(char *s, const char *pattern, regmatch_t *matches,
                     size_t nmatches);

/**
 * @function  reg_get_group
 * @abstract  store the string corresponding to the nth match in the string str in buf
 * @param   str         the string previously matched
 * @param   buf         a buffer to store the matched group
 * @param   matches     array storing the groups in the regex
 * @param   n           the number of the group to get
 */
extern void reg_get_group(const char *str, char *buf, regmatch_t *matches, size_t n);

#endif // !FORMAT_H
