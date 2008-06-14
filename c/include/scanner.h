#ifndef FRT_SCANNER_H
#define FRT_SCANNER_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Scan +in+ and copy the token into +out+, up until +out_size+ bytes.
 * The +start+ and +end+ are pointers to the original untouched token
 * somewhere inside +in+.  This token may not always be copied
 * verbatim into +out+.  For example, the http://google.com token will
 * be truncated down to just google.com during the copy.
 * +token_length+ is the size of the resulting token.
 */
void frt_std_scan(const char *in,
                  char *out, size_t out_size,
                  const char **start, const char **end,
                  int *token_length);


void frt_std_scan_mb(const char *in,
                     char *out, size_t out_size,
                     const char **start, const char **end,
                     int *token_length);

void frt_std_scan_utf8(const char *in,
                       char *out, size_t out_size,
                       const char **start, const char **end,
                       int *token_length);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* FRT_SCANNER */
