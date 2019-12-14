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
    }

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

    return strdup("");
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
    case 'V':
        env->CallVoidMethodA(obj, mID, jargs);
        break;
    case 'Z':
        ret->val->Z = env->CallBooleanMethodA(obj, mID, jargs) ? 1 : 0;
        break;
    case 'B':
        ret->val->B = env->CallByteMethodA(obj, mID, jargs);
        break;
    case 'C':
        ret->val->C = env->CallCharMethodA(obj, mID, jargs);
        break;
    case 'S':
        ret->val->S = env->CallShortMethodA(obj, mID, jargs);
        break;
    case 'I':
        ret->val->I = env->CallIntMethodA(obj, mID, jargs);
        break;
    case 'J':
        ret->val->J = env->CallLongMethodA(obj, mID, jargs);
        break;
    case 'F':
        ret->val->F = env->CallFloatMethodA(obj, mID, jargs);
        break;
    case 'D':
        ret->val->D = env->CallDoubleMethodA(obj, mID, jargs);
        break;
    case ';':
        ret->val->L = static_cast<void *>(env->CallObjectMethodA(obj, mID, jargs));
        break;
    default:
        printf("jni_method_invoke(): don't know what to call (yet) for '%s'!\n", sig);
        return strdup("jni_method_invoke(): don't know what to call (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_method_invoke(): Call*MethodA raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_method_invoke(): Call*MethodA raised exception!");
    }

    release_jargs(args, jargs);

    return strdup("");
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
    case 'V':
        env->CallStaticVoidMethodA(clazz, mID, jargs);
        break;
    case 'Z':
        ret->val->Z = env->CallStaticBooleanMethodA(clazz, mID, jargs) ? 1 : 0;
        break;
    case 'B':
        ret->val->B = env->CallStaticByteMethodA(clazz, mID, jargs);
        break;
    case 'C':
        ret->val->C = env->CallStaticCharMethodA(clazz, mID, jargs);
        break;
    case 'S':
        ret->val->S = env->CallStaticShortMethodA(clazz, mID, jargs);
        break;
    case 'I':
        ret->val->I = env->CallStaticIntMethodA(clazz, mID, jargs);
        break;
    case 'J':
        ret->val->J = env->CallStaticLongMethodA(clazz, mID, jargs);
        break;
    case 'F':
        ret->val->F = env->CallStaticFloatMethodA(clazz, mID, jargs);
        break;
    case 'D':
        ret->val->D = env->CallStaticDoubleMethodA(clazz, mID, jargs);
        break;
    case ';':
        ret->val->L = static_cast<void *>(env->CallStaticObjectMethodA(clazz, mID, jargs));
        break;
    default:
        printf("jni_static_method_invoke(): don't know what to call (yet) for '%s'!\n", sig);
        return strdup("jni_static_method_invoke(): don't know what to call (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_static_method_invoke(): GetStatic*MethodA raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_method_invoke(): GetStatic*MethodA raised exception!");
    }

    release_jargs(args, jargs);

    return strdup("");
}

extern "C" char *jni_field_get(char *class_name, jobject obj, char *name, char *sig, char ret_type, rakujvalue_t *ret)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_field_get(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_field_get(): FindClass raised exception!\n");
    }

    jfieldID fID = env->GetFieldID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_field_get(): GetFieldID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_field_get(): GetFieldID raised exception!");
    }

    switch(ret_type) {
    case 'Z':
        ret->val->Z = env->GetBooleanField(obj, fID) ? 1 : 0;
        break;
    case 'B':
        ret->val->B = env->GetByteField(obj, fID);
        break;
    case 'C':
        ret->val->C = env->GetCharField(obj, fID);
        break;
    case 'S':
        ret->val->S = env->GetShortField(obj, fID);
        break;
    case 'I':
        ret->val->I = env->GetIntField(obj, fID);
        break;
    case 'J':
        ret->val->J = env->GetLongField(obj, fID);
        break;
    case 'F':
        ret->val->F = env->GetFloatField(obj, fID);
        break;
    case 'D':
        ret->val->D = env->GetDoubleField(obj, fID);
        break;
    case ';':
        ret->val->L = static_cast<void *>(env->GetObjectField(obj, fID));
        break;
    default:
        printf("jni_field_get(): don't know what to get (yet) for '%s'!\n", sig);
        return strdup("jni_field_get(): don't know what to get (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_field_get(): Get*Field raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_field_get(): Get*Field raised exception!");
    }

    return strdup("");
}

extern "C" char *jni_static_field_get(char *class_name, char *name, char *sig, char ret_type, rakujvalue_t *ret)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_static_field_get(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_field_get(): FindClass raised exception!\n");
    }

    jfieldID fID = env->GetStaticFieldID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_static_field_get(): GetStaticFieldID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_field_get(): GetStaticFieldID raised exception!");
    }

    switch(ret_type) {
    case 'Z':
        ret->val->Z = env->GetStaticBooleanField(clazz, fID) ? 1 : 0;
        break;
    case 'B':
        ret->val->B = env->GetStaticByteField(clazz, fID);
        break;
    case 'C':
        ret->val->C = env->GetStaticCharField(clazz, fID);
        break;
    case 'S':
        ret->val->S = env->GetStaticShortField(clazz, fID);
        break;
    case 'I':
        ret->val->I = env->GetStaticIntField(clazz, fID);
        break;
    case 'J':
        ret->val->J = env->GetStaticLongField(clazz, fID);
        break;
    case 'F':
        ret->val->F = env->GetStaticFloatField(clazz, fID);
        break;
    case 'D':
        ret->val->D = env->GetStaticDoubleField(clazz, fID);
        break;
    case ';':
        ret->val->L = static_cast<void *>(env->GetStaticObjectField(clazz, fID));
        break;
    default:
        printf("jni_static_field_get(): don't know what to get (yet) for '%s'!\n", sig);
        return strdup("jni_static_field_get(): don't know what to get (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_static_field_get(): GetStatic*Field raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_field_get(): GetStatic*Field raised exception!");
    }

    return strdup("");
}

extern "C" char *jni_field_set(char *class_name, jobject obj, char *name, char *sig, char val_type, rakujvalue_t *val)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_field_set(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_field_set(): FindClass raised exception!\n");
    }

    jfieldID fID = env->GetFieldID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_field_set(): GetFieldID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_field_set(): GetFieldID raised exception!");
    }

    switch(val_type) {
    case 'Z':
        env->SetBooleanField(obj, fID, val->val->Z ? JNI_TRUE : JNI_FALSE);
        break;
    case 'B':
        env->SetByteField(obj, fID, val->val->B);
        break;
    case 'C':
        env->SetCharField(obj, fID, val->val->C);
        break;
    case 'S':
        env->SetShortField(obj, fID, val->val->S);
        break;
    case 'I':
        env->SetIntField(obj, fID, val->val->I);
        break;
    case 'J':
        env->SetLongField(obj, fID, val->val->J);
        break;
    case 'F':
        env->SetFloatField(obj, fID, val->val->F);
        break;
    case 'D':
        env->SetDoubleField(obj, fID, val->val->D);
        break;
    case ';':
        env->SetObjectField(obj, fID, static_cast<jobject>(val->val->L));
        break;
    default:
        printf("jni_field_set(): don't know what to set (yet) for '%s'!\n", sig);
        return strdup("jni_field_set(): don't know what to set (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_field_set(): Set*Field raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_field_set(): Set*Field raised exception!");
    }

    return strdup("");
}

extern "C" char *jni_static_field_set(char *class_name, char *name, char *sig, char val_type, rakujvalue_t *val)
{
    jclass clazz = env->FindClass(class_name);
    if (env->ExceptionOccurred()) {
        printf("jni_static_field_set(): FindClass raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_field_set(): FindClass raised exception!\n");
    }

    jfieldID fID = env->GetFieldID(clazz, name, sig);
    if (env->ExceptionOccurred()) {
        printf("jni_static_field_set(): GetFieldID raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_field_set(): GetFieldID raised exception!");
    }

    switch(val_type) {
    case 'Z':
        env->SetStaticBooleanField(clazz, fID, val->val->Z ? JNI_TRUE : JNI_FALSE);
        break;
    case 'B':
        env->SetStaticByteField(clazz, fID, val->val->B);
        break;
    case 'C':
        env->SetStaticCharField(clazz, fID, val->val->C);
        break;
    case 'S':
        env->SetStaticShortField(clazz, fID, val->val->S);
        break;
    case 'I':
        env->SetStaticIntField(clazz, fID, val->val->I);
        break;
    case 'J':
        env->SetStaticLongField(clazz, fID, val->val->J);
        break;
    case 'F':
        env->SetStaticFloatField(clazz, fID, val->val->F);
        break;
    case 'D':
        env->SetStaticDoubleField(clazz, fID, val->val->D);
        break;
    case ';':
        env->SetStaticObjectField(clazz, fID, static_cast<jobject>(val->val->L));
        break;
    default:
        printf("jni_static_field_set(): don't know what to set (yet) for '%s'!\n", sig);
        return strdup("jni_static_field_set(): don't know what to set (yet) for '%s'!");
        break;
    }

    if (env->ExceptionOccurred()) {
        printf("jni_static_field_set(): SetStatic*Field raised exception!\n");
        env->ExceptionDescribe();
        env->ExceptionClear();

        return strdup("jni_static_field_set(): SetStatic*Field raised exception!");
    }

    return strdup("");
}
