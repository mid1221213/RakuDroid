#include <jni.h>
#include <cstring>
#include <stdlib.h>

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

jvalue *args2jargs(rakujvalue_t **args)
{
    jvalue *jargs;
    int i;

    for (i = 0; args[i]->type != ';' || args[i]->val->L; i++)
        printf("i=%i, type=%c, val=%p\n", i, args[i]->type, args[i]->val->L);

    jargs = (jvalue *) calloc(i + 1, sizeof(jvalue));

    while (i--) {
        printf("i2=%i\n", i);
        switch (args[i]->type) {
        case 's':
            jargs[i].l = env->NewStringUTF(args[i]->val->s);
            break;
        case 'Z':
            jargs[i].z = args[i]->val->Z;
            break;
        case 'B':
            jargs[i].b = args[i]->val->B;
            break;
        case 'C':
            jargs[i].c = args[i]->val->C;
            break;
        case 'S':
            jargs[i].s = args[i]->val->S;
            break;
        case 'I':
            jargs[i].i = args[i]->val->I;
            break;
        case 'J':
            jargs[i].j = args[i]->val->J;
            break;
        case 'F':
            jargs[i].f = args[i]->val->F;
            break;
        case 'D':
            jargs[i].d = args[i]->val->D;
            break;
        default:
            jargs[i].l = (jobject) args[i]->val->L;
            break;
        }
    };

    return jargs;
}

void release_jargs(rakujvalue_t **args, jvalue *jargs)
{
    // for (; *args; args++, jargs++) {
    //     switch (args[i]->type) {
    //     case 's':
    //         env->NewStringUTF(args[i]->val->s);
    //         break;
    // }

    free((void *)jargs);
}

extern "C" char *jni_ctor_invoke(char *class_name, char *sig, rakujvalue_t **args, rakujvalue_t *ret)
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

    jvalue *jargs = args2jargs(args);

    ret->val->L = static_cast<void *>(env->NewObjectA(clazz, mID, jargs));

    if (env->ExceptionOccurred()) {
        printf("jni_ctor_invoke(): NewObjectA raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_ctor_invoke(): NewObjectA raised exception!");
    }

    release_jargs(args, jargs);

    return strdup("OK");
}

extern "C" char *jni_method_invoke(char *class_name, jobject obj, char *name, char *sig, rakujvalue_t **args, char ret_type, rakujvalue_t *ret)
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

    jvalue *jargs = args2jargs(args);

    switch(ret_type) {
    case ';':
        ret->val->L = static_cast<void *>(env->CallObjectMethodA(obj, mID, jargs));
        break;
    case 'V':
        env->CallVoidMethodA(obj, mID, jargs);
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

    release_jargs(args, jargs);

    return strdup("OK");
}

extern "C" char *jni_static_method_invoke(char *class_name, char *name, char *sig, rakujvalue_t **args, char ret_type, rakujvalue_t *ret)
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

    jvalue *jargs = args2jargs(args);

    switch(ret_type) {
    case ';':
        ret->val->L = static_cast<void *>(env->CallStaticObjectMethodA(clazz, mID, jargs));
        break;
    case 'V':
        env->CallStaticObjectMethodA(clazz, mID, jargs);
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

    release_jargs(args, jargs);

    return strdup("OK");
}
