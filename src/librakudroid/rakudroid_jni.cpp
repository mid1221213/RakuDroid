#include <jni.h>
#include <cstring>

extern "C" {
#include "rakudroid_jni.h"
}

#include <android/log.h>
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKUUUUUUUUUUUUUUUUU", __VA_ARGS__);

static JNIEnv *env;

extern "C" void jni_init_env(JNIEnv *envParam)
{
    env = envParam;
}

extern "C" void *jni_method_invoke(char *class_name, void *obj, char *name, char *sig, void *args[], char ret_type,
				   uint8_t *Z,
				   uint8_t *B,
				   int8_t  *C,
				   int16_t *S,
				   int     *I,
				   int64_t *J,
				   float   *F,
				   double  *D
    )
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return nullptr;
    }

    jmethodID mID = env->GetMethodID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_method_invoke(): GetMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return nullptr;
    }

    void *ret;
    jvalue *jargs = reinterpret_cast<jvalue *>(args);

    switch(ret_type) {
    case ';':
        ret = static_cast<void *>(env->CallObjectMethodA(reinterpret_cast<jobject>(obj), mID, jargs));
        break;
    default:
        printf("jni_static_method_invoke(): don't know what to call (yet) for '%s'!\n", sig);
        return nullptr;
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): GetStaticMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return nullptr;
    }

    return ret;
}

extern "C" void *jni_static_method_invoke(char *class_name, char *name, char *sig, void *args[], char ret_type,
					  uint8_t *Z,
					  uint8_t *B,
					  int8_t  *C,
					  int16_t *S,
					  int     *I,
					  int64_t *J,
					  float   *F,
					  double  *D
    )
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return nullptr;
    }

    jmethodID mID = env->GetStaticMethodID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): GetStaticMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return nullptr;
    }

    void *ret;
    jvalue *jargs = reinterpret_cast<jvalue *>(args);

    switch(ret_type) {
    case ';':
        ret = static_cast<void *>(env->CallStaticObjectMethodA(clazz, mID, jargs));
        break;
    default:
        printf("jni_static_method_invoke(): don't know what to call (yet) for '%s'!\n", sig);
        return nullptr;
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): GetStaticMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return nullptr;
    }

    return ret;
}
