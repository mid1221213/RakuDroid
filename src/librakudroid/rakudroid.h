#ifndef _RAKUDROID_H_
#define _RAKUDROID_H_

extern void rakudo_init(int from_lib, int argc, char *argv[], int64_t *ok);
extern void rakudo_fini();
extern char *rakudo_eval(char *perl6);
extern char *rakudo_init_activity(void *activity_ptr);

#endif // _RAKUDROID_H_
