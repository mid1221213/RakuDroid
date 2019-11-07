#include <jni.h>
#include <string>

extern "C" {
#include <android/log.h>
#include "rakudroid.h"
}

#include <cstdlib>
#include <unistd.h>

//#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKU", __VA_ARGS__);

extern "C" JNIEXPORT jstring JNICALL
Java_com_example_myapplication_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */,
        jstring appDir) {
    int64_t ok = 0;

    const char *c_dir = env->GetStringUTFChars(appDir, 0);
    chdir(c_dir);
    env->ReleaseStringUTFChars(appDir, c_dir);

    char buf[256];

    setenv("HOME", getcwd(buf, sizeof(buf)), 1);
    rakudo_init(0, 0, nullptr, &ok);

    char *eval = rakudo_eval(const_cast<char *>("'5 + 4² = ' ~ 5 + 4²"));

    if (ok) {
        auto ret = env->NewStringUTF(eval);
        free(eval);
        return ret;
    }

    free(eval);

    return env->NewStringUTF("NOK");
}
