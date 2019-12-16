#ifndef _RAKUDROID_JNI_H_
#define _RAKUDROID_JNI_H_

#include <jni.h>

extern void jni_init_env(JNIEnv *envParam);

typedef union {
  char     *s;
  uint8_t   Z;
  int8_t    B;
  uint16_t  C;
  int16_t   S;
  int32_t   I;
  int64_t   J;
  float     F;
  double    D;
  void     *L;
} rakujunion_t;

typedef struct {
  uint8_t type;
  rakujunion_t *val;
} rakujvalue_t;

extern char *jni_ctor_invoke(         char *class_name,                          char *sig, rakujvalue_t **args,                rakujvalue_t *ret);
extern char *jni_method_invoke(       char *class_name, jobject obj, char *name, char *sig, rakujvalue_t **args, char ret_type, rakujvalue_t *ret);
extern char *jni_static_method_invoke(char *class_name,              char *name, char *sig, rakujvalue_t **args, char ret_type, rakujvalue_t *ret);
extern char *jni_field_get(           char *class_name, jobject obj, char *name, char *sig,                      char ret_type, rakujvalue_t *ret);
extern char *jni_static_field_get(    char *class_name,              char *name, char *sig,                      char ret_type, rakujvalue_t *ret);
extern char *jni_field_set(           char *class_name, jobject obj, char *name, char *sig,                      char val_type, rakujvalue_t *val);
extern char *jni_static_field_set(    char *class_name,              char *name, char *sig,                      char val_type, rakujvalue_t *val);

#endif // _RAKUDROID_JNI_H_
