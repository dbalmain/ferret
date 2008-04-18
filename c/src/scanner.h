#ifndef FRT_SCANNER
#define FRT_SCANNER

/*
 * Scan +buf+ and return the position of the next token in +start+,
 * and its length in +len.
 */
void frt_scan(const char *buf, char **start, int *len);

#endif /* FRT_SCANNER */
