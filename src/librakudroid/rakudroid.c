#include <moar.h>

#include <libgen.h>
#include <dlfcn.h>
#include <link.h>

#include <android/log.h>

#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKU", __VA_ARGS__);
#define puts(...) __android_log_print(ANDROID_LOG_DEBUG, "RAKU", __VA_ARGS__);

#define STRINGIFY1(x) #x
#define STRINGIFY(x) STRINGIFY1(x)

const char my_interp[] __attribute__((section(".interp"))) = "/system/bin/linker64";

static int argc = 0;
static char **argv;

typedef char *(*eval)(char *);

eval eval_p6;

void rakudo_p6_init(eval e)
{
    eval_p6 = e;
}

static int64_t *ok;

void rakudo_p6_set_ok(int64_t p6ok)
{
//    printf("IN rakudo_p6_set_ok(%ld)\n", p6ok);
    *ok = p6ok;
}

static char        *lib_path[4];
static char        *perl6_file;
static char        *perl6_lib_path;
static char        *helper_file;
static char        *exec_dir_path;
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
        exec_path = strdup(info->dlpi_name);

    return 0;
}

void rakudo_init(int from_lib, int argc, char *argv[], int64_t *main_ok)
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

    if (from_lib) {
        exec_path = strdup(argv[0]);
    } else {
        dl_iterate_phdr(callback, NULL);

        if (!exec_path) {
            puts("cannot find exec_path");
            exit(EXIT_FAILURE);
        }
    }

    /* Retrieve the executable directory path. */

//    printf("from_lib=%d, lpath=%s, argc = %d\n", from_lib, exec_path, argc);

    /* exec_dir_path = strdup(argv[0]); */
    exec_dir_path = strdup(exec_path);

    slash_pos = strrchr(exec_dir_path, '/');
    if (slash_pos)
        *(slash_pos + 1) = 0;
    else
        *exec_dir_path = 0;

    exec_dir_path_size = strlen(exec_dir_path);

    /* Retrieve PERL6_HOME and NQP_HOME. */

    nqp_home = STRINGIFY(STATIC_NQP_HOME);
    nqp_home_size = strlen(nqp_home);

    perl6_home = STRINGIFY(STATIC_PERL6_HOME);
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
    memcpy(perl6_lib_path + home_size, perl6_lib,  perl6_lib_size);

    lib_path[1][home_size + perl6_home_size] = 0;
    char *rakudo_prefix = strdup(lib_path[1]);
    setenv("RAKUDO_PREFIX", rakudo_prefix, 1);
    free(rakudo_prefix);

    perl6_lib_path[home_size + perl6_lib_size] = 0;
    setenv("PERL6LIB", perl6_lib_path, 1);
    free(perl6_lib_path);

    strcpy(lib_path[0] + home_size +   nqp_home_size, "/lib");
    strcpy(lib_path[1] + home_size + perl6_home_size, "/lib");
    strcpy(lib_path[2] + home_size + perl6_home_size, "/runtime");
    strcpy(perl6_file  + home_size + perl6_home_size, "/runtime/perl6.moarvm");
    strcpy(helper_file + home_size                  , helper_path);

    lib_path[3] = NULL;

    /* Start up the VM. */

    instance = MVM_vm_create_instance();

    MVM_vm_set_prog_name(instance, perl6_file);
    MVM_vm_set_exec_name(instance, exec_path);
    MVM_vm_set_lib_path(instance, 4, (const char **)lib_path);
//    signal(SIGPIPE, SIG_IGN);

    if (from_lib) {
        /* setenv("RAKUDO_MODULE_DEBUG", "1", 1); */
        MVM_vm_set_clargs(instance, argc - 1, argv + 1);
        MVM_vm_run_file(instance, perl6_file);
        exit(EXIT_SUCCESS);
    }

    /* close(2); */
    /* open("/data/data/com.example.myapplication/files/stderr", O_APPEND | O_CREAT | O_WRONLY); */

    ok = main_ok;
    MVM_vm_set_clargs(instance, 0, NULL);
    // remove later?
    /* Ignore SIGPIPE by default, since we error-check reads/writes. This does
     * not prevent users from setting up their own signal handler for SIGPIPE,
     * which will take precedence over this ignore. */
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

    /* Points to the current opcode. */
    MVMuint8 *cur_op = NULL;

    /* The current frame's bytecode start. */
    MVMuint8 *bytecode_start = NULL;

    /* Points to the base of the current register set for the frame we
     * are presently in. */
    MVMRegister *reg_base = NULL;

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

    /* MVM_vm_destroy_instance(instance); */
}

char *rakudo_eval(char *perl6)
{
    MVM_gc_mark_thread_unblocked(instance->main_thread);

    char *ret = strdup(eval_p6(perl6));

    MVM_gc_mark_thread_blocked(instance->main_thread);

    return ret;
}

void *method_invoke(char *name, char *sig, void *args[], int argNb)
{
    printf("method_invoke(%s, %s, %p, %d\n", name, sig, args, argNb);
    return NULL;
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
    PRINT_EVAL("RakuDroidRun::add(3, 4)");

    rakudo_fini();

    exit(EXIT_SUCCESS);
}

void lib_init(int s_argc, char **s_argv)
{
    argc = s_argc;
    argv = s_argv;
}

__attribute__((section(".init_array"))) void *lib_init_constructor = lib_init;
