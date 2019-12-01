#ifndef _RAKUDROID_JNI_H_
#define _RAKUDROID_JNI_H_

#include <jni.h>

extern void jni_init_env(JNIEnv *envParam);

extern void *jni_method_invoke(char *_name, void *obj, char *name, char *sig, void *args[], char ret_type,
			       uint8_t *Z,
			       uint8_t *B,
			       int8_t  *C,
			       int16_t *S,
			       int     *I,
			       int64_t *J,
			       float   *F,
			       double  *D
	);
extern void *jni_static_method_invoke(char *class_name, char *name, char *sig, void *args[], char ret_type,
				      uint8_t *Z,
				      uint8_t *B,
				      int8_t  *C,
				      int16_t *S,
				      int     *I,
				      int64_t *J,
				      float   *F,
				      double  *D
	);

#endif // _RAKUDROID_JNI_H_
