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
jobject myApp = nullptr;
jobject myActivity = nullptr;

JNIEnv *env;

extern "C" JNIEXPORT void JNICALL
Java_com_example_myapplication_MyApplication_rakuInit(
        JNIEnv* envParam,
        jobject zis,
        jstring appDir) {

    env = envParam;
    myApp = zis;

    const char *c_dir = env->GetStringUTFChars(appDir, nullptr);
    chdir(c_dir);
    setenv("HOME", c_dir, 1);
    env->ReleaseStringUTFChars(appDir, c_dir);

    rakudo_init(0, 0, nullptr, &ok);
}

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_rakuEval(
        JNIEnv* /* env */,
        jobject zis,
        jstring toEval) {

    char *ret_str;
    const char *evalMe;

    ok = 0;

    if (!myActivity) {
        myActivity = zis;

        ret_str = rakudo_init_activity(static_cast<void *>(zis));
        if (!ok) {
            printf("Activity setup failed: %s\n", ret_str);
            goto end;
        }
        free(ret_str);
    }

    evalMe = env->GetStringUTFChars(toEval, nullptr);

    ret_str = rakudo_eval(const_cast<char *>(evalMe));

    env->ReleaseStringUTFChars(toEval, evalMe);

end:
//    auto ret = env->NewStringUTF(ok ? ret_str : "NOK");
    auto ret = env->NewStringUTF(ret_str);

    free(ret_str);

    return ret;
}
