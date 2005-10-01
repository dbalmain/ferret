int
frt_hash(register char *p, register int len)
{
  register int key = 0;

  while (len--) {
    key = key*65599 + *p;
    p++;
  }
  key = key + (key>>5);
  return key;
}
