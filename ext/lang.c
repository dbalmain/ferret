#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "global.h"

void ft_raise(char *file, int line_num, VALUE etype, const char *fmt, ...)
{
  va_list args;
  char buf[MAX_ERROR_LEN];
  char *buf_ptr = buf;

  if (progname() != NULL) {
    sprintf(buf_ptr, "%s: ", progname());
    buf_ptr += strlen(buf_ptr);
  }

  sprintf(buf_ptr, "Error occured at <%s>:%d\n", file, line_num);
  buf_ptr += strlen(buf_ptr);
  va_start(args, fmt);
  vsprintf(buf_ptr, fmt, args);
  buf_ptr += strlen(buf_ptr);
  va_end(args);

  if (fmt[0] != '\0' && fmt[strlen(fmt)-1] == ':') {
    sprintf(buf_ptr, " %s", strerror(errno));
    buf_ptr += strlen(buf_ptr);
  }
  sprintf(buf_ptr, "\n");
  rb_raise(etype, buf); /* conventional value for failed execution */
}

#ifdef WIN32
void eprintf(VALUE etype, const char *fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  ft_raise("Windows", -1, etype, fmt, args);
  va_end(args);
}
#endif
