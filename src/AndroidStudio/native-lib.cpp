#include <jni.h>
#include <string>

extern "C" {
#include <android/log.h>
#include "rakudroid.h"
}

#include <cstdlib>
#include <unistd.h>

//#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKU", __VA_ARGS__);

int64_t ok = 0;

extern "C" JNIEXPORT void JNICALL
Java_com_example_myapplication_MainActivity_rakuInit(
        JNIEnv* env,
        jobject /* this */,
        jstring appDir) {

    const char *c_dir = env->GetStringUTFChars(appDir, nullptr);
    chdir(c_dir);
    env->ReleaseStringUTFChars(appDir, c_dir);

    char buf[256];

    setenv("HOME", getcwd(buf, sizeof(buf)), 1);
    rakudo_init(0, 0, nullptr, &ok);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_rakuEval(
        JNIEnv* env,
        jobject /* this */,
        jstring toEval) {

    const char *evalMe = env->GetStringUTFChars(toEval, nullptr);

    char *eval = rakudo_eval(const_cast<char *>(evalMe));

    env->ReleaseStringUTFChars(toEval, evalMe);

    auto ret = env->NewStringUTF(ok ? eval : "NOK");

    free(eval);

    return ret;
}
