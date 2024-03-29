#include <moar.h>

#include <libgen.h>
#include <dlfcn.h>
#include <unistd.h>
#include <link.h>

#include <android/log.h>

#include "rakudroid_jni.h"

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKUUUUUUUUUUUUUUUUU", __VA_ARGS__);

#define STRINGIFY1(x) #x
#define STRINGIFY(x) STRINGIFY1(x)

const char my_interp[] __attribute__((section(".interp"))) = "/system/bin/linker64";

static int argc = 0;
static char **argv;

typedef char *(*eval)(char *);
typedef char *(*init_activity)(void *);

eval eval_p6;
init_activity init_activity_p6;

void rakudo_p6_init(eval e, init_activity i)
{
    eval_p6 = e;
    init_activity_p6 = i;
}

static int64_t *ok;

void rakudo_p6_set_ok(int64_t p6ok)
{
//    printf("IN rakudo_p6_set_ok(%ld)\n", p6ok);
    *ok = p6ok;
}

static char        *lib_path[3];
static char        *perl6_file;
static char        *perl6_lib_path;
static char        *helper_file;
static char        *exec_dir_path;
static char        *rdlib_path = NULL;
static char        *exec_path = NULL;
static char        *home;
static MVMInstance *instance;
static MVMCompUnit *cu;

/* Points to the current opcode. */
static MVMuint8 *cur_op = NULL;

/* The current frame's bytecode start. */
static MVMuint8 *bytecode_start = NULL;

/* Points to the base of the current register set for the frame we
 * are presently in. */
static MVMRegister *reg_base = NULL;

/* This callback is passed to the interpreter code. It takes care of making
 * the initial invocation. */
static void toplevel_initial_invoke(MVMThreadContext *tc, void *data) {
    /* Create initial frame, which sets up all of the interpreter state also. */
    MVM_frame_invoke(tc, (MVMStaticFrame *)data, MVM_callsite_get_common(tc, MVM_CALLSITE_ID_NULL_ARGS), NULL, NULL, NULL, -1);
}

static int callback(struct dl_phdr_info *info, size_t size, void *data)
{
    if (!info->dlpi_name)
        return 0;

    const char *last_slash = strrchr(info->dlpi_name, '/');

    if (!strncmp(last_slash ? last_slash + 1 : info->dlpi_name, STRINGIFY(LIBFILENAME), strlen(STRINGIFY(LIBFILENAME))))
        rdlib_path = strdup(info->dlpi_name);

    return 0;
}

static int pfd[2];
static pthread_t thr;
static const char *tag = "RAKUUUUUUUUUUUUUUUUUUU";

static void *thread_func(void *dum)
{
    ssize_t rdsz;
    char buf[10240];

    for (;;)
        while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
//            if(buf[rdsz - 1] == '\n') --rdsz;
            buf[rdsz] = 0;  /* add null-terminator */
            __android_log_write(ANDROID_LOG_DEBUG, tag, buf);
        }

    return 0;
}

int start_logger()
{
    /* make stdout line-buffered and stderr unbuffered */
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    /* create the pipe and redirect stdout and stderr */
    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    /* spawn the logging thread */
    if(pthread_create(&thr, 0, thread_func, 0) == -1)
        return -1;
    pthread_detach(thr);
    return 0;
}

void rakudo_init(int from_main, int argc, char *argv[], int64_t *main_ok)
{
    MVMThreadContext *tc;

    size_t  exec_dir_path_size;
    char   *slash_pos;
    size_t  home_size;

    char   *nqp_home;
    size_t  nqp_home_size;

    char   *perl6_home;
    size_t  perl6_home_size;

    char   *perl6_lib;
    size_t  perl6_lib_size;

    char   *helper_path;
    size_t  helper_path_size;

    /* Retrieve the executable directory path. */

    dl_iterate_phdr(callback, NULL);
    if (!rdlib_path) {
        printf("cannot find rdlib_path\n");
        exit(EXIT_FAILURE);
    }

//    printf("from_main=%d, lpath=%s, argc = %d\n", from_main, exec_path, argc);

    exec_dir_path = strdup(rdlib_path);

    slash_pos = strrchr(exec_dir_path, '/');
    if (slash_pos)
        *(slash_pos + 1) = 0;
    else
        *exec_dir_path = 0;

    exec_dir_path_size = strlen(exec_dir_path);

    /* Retrieve RAKUDO_HOME and NQP_HOME. */

    nqp_home = STRINGIFY(STATIC_NQP_HOME);
    nqp_home_size = strlen(nqp_home);

    perl6_home = STRINGIFY(STATIC_RAKUDO_HOME);
    perl6_home_size = strlen(perl6_home);

    perl6_lib = STRINGIFY(STATIC_PERL6_LIB);
    perl6_lib_size = strlen(perl6_lib);

    helper_path = STRINGIFY(STATIC_HELPER_FILE);
    helper_path_size = strlen(helper_path);

    /* Put together the lib paths and perl6_file path. */

    char *home = getenv("HOME");
    home_size = strlen(home);

    char *ldlibpath = (char*)malloc(exec_dir_path_size + 1 + home_size + perl6_home_size + 50);
    strcpy(ldlibpath, exec_dir_path);
    strcat(ldlibpath, ":");
    strcat(ldlibpath, home);
    strcat(ldlibpath, perl6_home);
    strcat(ldlibpath, "/runtime");

    setenv("LD_LIBRARY_PATH", ldlibpath, 1);

    free(ldlibpath);
    free(exec_dir_path);

    lib_path[0]    = (char*)malloc(home_size + nqp_home_size    + 50);
    lib_path[1]    = (char*)malloc(home_size + perl6_home_size  + 50);
    lib_path[2]    = (char*)malloc(home_size + perl6_home_size  + 50);
    perl6_file     = (char*)malloc(home_size + perl6_home_size  + 50);
    perl6_lib_path = (char*)malloc(home_size + perl6_lib_size   +  1);
    helper_file    = (char*)malloc(home_size + helper_path_size +  1);

    memcpy(lib_path[0],    home, home_size);
    memcpy(lib_path[1],    home, home_size);
    memcpy(lib_path[2],    home, home_size);
    memcpy(perl6_file,     home, home_size);
    memcpy(perl6_lib_path, home, home_size);
    memcpy(helper_file,    home, home_size);

    memcpy(lib_path[0]    + home_size, nqp_home,   nqp_home_size);
    memcpy(lib_path[1]    + home_size, perl6_home, perl6_home_size);
    memcpy(lib_path[2]    + home_size, perl6_home, perl6_home_size);
    memcpy(perl6_file     + home_size, perl6_home, perl6_home_size);

    if (from_main) {
        exec_path = strdup(argv[0]);
    } else {
        strcpy(perl6_lib_path + home_size, "/rakudroid/bin");
        mkdir(perl6_lib_path, 0700);
        strcpy(perl6_lib_path + home_size + 14, "/rakudroid");
        symlink(rdlib_path, perl6_lib_path);
        exec_path = strdup(perl6_lib_path);
    }

    strcpy(perl6_lib_path + home_size, perl6_lib);
    setenv("RAKUDOLIB", perl6_lib_path, 1);
    free(perl6_lib_path);

    strcpy(lib_path[0] + home_size +   nqp_home_size, "/lib");
    strcpy(lib_path[1] + home_size + perl6_home_size, "/lib");
    strcpy(lib_path[2] + home_size + perl6_home_size, "/runtime");
    strcpy(perl6_file  + home_size + perl6_home_size, "/runtime/perl6.moarvm");
    strcpy(helper_file + home_size                  , helper_path);

    /* Start up the VM. */

    instance = MVM_vm_create_instance();

    MVM_vm_set_prog_name(instance, perl6_file);
    MVM_vm_set_exec_name(instance, exec_path);
    MVM_vm_set_lib_path(instance, 3, (const char **)lib_path);

    if (from_main) {
        MVM_vm_set_clargs(instance, argc - 1, argv + 1);
        MVM_vm_run_file(instance, perl6_file);
        MVM_vm_exit(instance); // does not return
    }

    /* start_logger(); */

    close(2);
    open("/data/data/com.example.myapplication/files/stderr", O_APPEND | O_CREAT | O_WRONLY);
//    setenv("RAKUDO_MAX_THREADS", "2", 1);
    setenv("RAKUDO_MODULE_DEBUG", "1", 1);

    ok = main_ok;
    MVM_vm_set_clargs(instance, 0, NULL);
    tc = instance->main_thread;
    cu = MVM_cu_map_from_file(tc, perl6_file);
    MVMROOT(tc, cu, {
            /* The call to MVM_string_utf8_decode() may allocate, invalidating the
               location cu->body.filename */
            MVMString *const str = MVM_string_utf8_c8_decode(tc, instance->VMString, perl6_file, strlen(perl6_file));
            cu->body.filename = str;

            /* Run deserialization frame, if there is one. Disable specialization
             * during this time, so we don't waste time logging one-shot setup
             * code. */
            if (cu->body.deserialize_frame) {
                MVMint8 spesh_enabled_orig = tc->instance->spesh_enabled;
                tc->instance->spesh_enabled = 0;
                MVM_interp_run(tc, toplevel_initial_invoke, cu->body.deserialize_frame);
                tc->instance->spesh_enabled = spesh_enabled_orig;
            }
        });

    instance->num_clargs = 1;
    instance->raw_clargs = (char **) &helper_file;
    instance->clargs = NULL; /* clear cache */

    MVM_interp_run(tc, toplevel_initial_invoke, cu->body.main_frame);

    /* Stash addresses of current op, register base and SC deref base
     * in the TC; this will be used by anything that needs to switch
     * the current place we're interpreting. */
    tc->interp_cur_op         = &cur_op;
    tc->interp_bytecode_start = &bytecode_start;
    tc->interp_reg_base       = &reg_base;
    tc->interp_cu             = &cu;

    toplevel_initial_invoke(tc, cu->body.main_frame);

    MVM_gc_mark_thread_blocked(tc);
}

void rakudo_fini()
{
    free(lib_path[0]);
    free(lib_path[1]);
    free(lib_path[2]);
    free(helper_file);
    free(perl6_file);
    free(home);
    free(exec_path);
    free(rdlib_path);

    /* MVM_vm_destroy_instance(instance); */
}

char *rakudo_eval(char *perl6)
{
//    MVM_gc_mark_thread_unblocked(instance->main_thread);

    char *ret = strdup(eval_p6(perl6));

//    MVM_gc_mark_thread_blocked(instance->main_thread);

    return ret;
}

char *rakudo_init_activity(void *activity_ptr)
{
    return strdup(init_activity_p6(activity_ptr));
}

char *ctor_invoke(char *class_name, char *sig, rakujvalue_t **args, rakujvalue_t *ret)
{
    printf("ctor_invoke(class_name='%s', sig='%s', args='%p', ret=%p\n", class_name, sig, args, ret);
    return jni_ctor_invoke(class_name, sig, args, ret);
}

char *method_invoke(char *class_name, jobject obj, char *name, char *sig, rakujvalue_t **args, char *ret_type, rakujvalue_t *ret)
{
    printf("method_invoke(class_name='%s', obj='%p', method='%s', sig='%s', args='%p', ret_type=%c, ret=%p\n", class_name, obj, name, sig, args, ret_type[0], ret);
    return jni_method_invoke(class_name, obj, name, sig, args, ret_type[0], ret);
}

char *static_method_invoke(char *class_name, char *name, char *sig, rakujvalue_t **args, char *ret_type, rakujvalue_t *ret)
{
    printf("static_method_invoke(class_name='%s', method='%s', sig='%s', args='%p', ret_type=%c, ret=%p\n", class_name, name, sig, args, ret_type[0], ret);
    return jni_static_method_invoke(class_name, name, sig, args, ret_type[0], ret);
}

char *field_get(char *class_name, jobject obj, char *name, char *sig, char *ret_type, rakujvalue_t *ret)
{
    printf("field_get(class_name='%s', obj='%p', name='%s', sig='%s', ret_type=%c, ret=%p\n", class_name, obj, name, sig, ret_type[0], ret);
    return jni_field_get(class_name, obj, name, sig, ret_type[0], ret);
}

char *static_field_get(char *class_name, char *name, char *sig, char *ret_type, rakujvalue_t *ret)
{
    printf("static_field_get(class_name='%s', name='%s', sig='%s', ret_type=%c, ret=%p\n", class_name, name, sig, ret_type[0], ret);
    return jni_static_field_get(class_name, name, sig, ret_type[0], ret);
}

char *field_set(char *class_name, jobject obj, char *name, char *sig, char *val_type, rakujvalue_t *val)
{
    printf("field_set(class_name='%s', obj='%p', name='%s', sig='%s', val_type=%c, val=%p\n", class_name, obj, name, sig, val_type[0], val);
    return jni_field_set(class_name, obj, name, sig, val_type[0], val);
}

char *static_field_set(char *class_name, char *name, char *sig, char *val_type, rakujvalue_t *val)
{
    printf("static_field_set(class_name='%s', name='%s', sig='%s', val_type=%c, val=%p\n", class_name, name, sig, val_type[0], val);
    return jni_static_field_set(class_name, name, sig, val_type[0], val);
}

#define PRINT_EVAL(str) { char *ret = rakudo_eval(str); printf("« " str " » = %s%s\n", ok ? "" : "[NOK] → ", ret); free(ret); }

void start(void)
{
    int64_t ok;

    rakudo_init(1, argc, argv, &ok);

    PRINT_EVAL("5 + 4");
    PRINT_EVAL("2 * 3 + 9");
    PRINT_EVAL("warn 1/3; 1/3");
    PRINT_EVAL("our sub add ($a, $b) { $a + $b }; add(1, 2)");
    PRINT_EVAL("add(3, 4)");
    PRINT_EVAL("'============================================='");
    PRINT_EVAL("RakuDroid::add(3, 4)");

    rakudo_fini();

    exit(EXIT_SUCCESS);
}

void lib_init(int s_argc, char **s_argv)
{
    argc = s_argc;
    argv = s_argv;
}

__attribute__((section(".init_array"))) void *lib_init_constructor = lib_init;
