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

extern "C" char *jni_ctor_invoke(char *class_name, char *sig, jvalue jargs[], rakujvalue_t *ret)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_ctor_invoke(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_ctor_invoke(): FindClass raised exception!");
    }

    jmethodID mID = env->GetMethodID(clazz, "<init>", sig);
    if (env->ExceptionOccurred()) {
        printf("jni_ctor_invoke(): GetMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_ctor_invoke(): GetMethodID raised exception!");
    }

    ret->val->L = static_cast<void *>(env->NewObjectA(clazz, mID, jargs));

    if (env->ExceptionOccurred()) {
        printf("jni_ctor_invoke(): NewObjectA raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_ctor_invoke(): NewObjectA raised exception!");
    }

    return strdup("OK");
}

extern "C" char *jni_method_invoke(char *class_name, jobject obj, char *name, char *sig, jvalue jargs[], char ret_type, rakujvalue_t *ret)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_method_invoke(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_method_invoke(): FindClass raised exception!");
    }

    jmethodID mID = env->GetMethodID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_method_invoke(): GetMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_method_invoke(): GetMethodID raised exception!");
    }

    switch(ret_type) {
    case ';':
        ret->val->L = static_cast<void *>(env->CallObjectMethodA(obj, mID, jargs));
        break;
    default:
        printf("jni_method_invoke(): don't know what to call (yet) for '%s'!\n", sig);
        return strdup("jni_method_invoke(): don't know what to call (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_method_invoke(): CallObjectMethodA raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_method_invoke(): CallObjectMethodA raised exception!");
    }

    return strdup("OK");
}

extern "C" char *jni_static_method_invoke(char *class_name, char *name, char *sig, jvalue jargs[], char ret_type, rakujvalue_t *ret)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_method_invoke(): FindClass raised exception!\n");
    }

    jmethodID mID = env->GetStaticMethodID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): GetStaticMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_method_invoke(): GetStaticMethodID raised exception!");
    }

    switch(ret_type) {
    case ';':
        ret->val->L = static_cast<void *>(env->CallStaticObjectMethodA(clazz, mID, jargs));
        break;
    default:
        printf("jni_static_method_invoke(): don't know what to call (yet) for '%s'!\n", sig);
        return strdup("jni_static_method_invoke(): don't know what to call (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): GetStaticMethodID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_method_invoke(): GetStaticMethodID raised exception!");
    }

    return strdup("OK");
}
