#include <jni.h>
#include <string>

extern "C" {
#include <android/log.h>
#include "rakudroid.h"
}

#include <cstdlib>
#include <unistd.h>

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKU", __VA_ARGS__);

int64_t ok = 0;

JNIEnv *env;

extern "C" JNIEXPORT void JNICALL
Java_com_example_myapplication_MyApplication_rakuInit(
        JNIEnv* envParam,
        jobject /* this */,
        jstring appDir) {

    env = envParam;

    const char *c_dir = env->GetStringUTFChars(appDir, nullptr);
    chdir(c_dir);
    setenv("HOME", c_dir, 1);
    env->ReleaseStringUTFChars(appDir, c_dir);

    rakudo_init(0, 0, nullptr, &ok);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_rakuEval(
        JNIEnv* /* env */,
        jobject /* this */,
        jstring toEval) {

    const char *evalMe = env->GetStringUTFChars(toEval, nullptr);

    char *eval = rakudo_eval(const_cast<char *>(evalMe));

    env->ReleaseStringUTFChars(toEval, evalMe);

//    auto ret = env->NewStringUTF(ok ? eval : "NOK");
    auto ret = env->NewStringUTF(eval);

    free(eval);

    return ret;
}
