#ifndef _RAKUDROID_H_

extern void rakudo_init(int from_lib, int argc, char *argv[], int64_t *ok);
extern void rakudo_fini();
extern char *rakudo_eval(char *perl6);

#endif // _RAKUDROID_H_
