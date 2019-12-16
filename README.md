# RakuDroid

RakuDroid is a PoC.

RakuDroid is a WIP. Please read this file entirely before doing anything.

RakuDroid's goal is to port Raku (a.k.a. Perl6) on Android platform.

The first work has been to cross compile Rakudo / MoarVM for Android. It uses the latest release of both at the time of writing, i.e. `2019.11`. This has been possible with just 2 patches to MoarVM. The first makes the cross-compilation possible with Android SDK/NDK (see `src/librakudroid/0001-Make-MoarVM-cross-compile-nicely-for-Android.patch`), the second allows jit to work but, of course, only on x86_64 (see `src/librakudroid/0001-Make-MoarVM-cross-compile-w-jit-nicely-for-Android.patch`).

Next, work has been done to write JNI stuff, Raku helpers and to automatically generate bindings to interact with the Java side of the Android system.

## Supported platforms

* Build (cross compilation host): Linux x86_64 (I personally use Ubuntu, not tested on other distros but should work)
* Targets:
  * Architectures:
	* `x86_64` (typically the Android emulator for testing)
	* `aarch64` (arm 64-bit, most new smartphones)
  * Android versions: 8.0 (Oreo) and above (API 26+)

## Requirements for building

* Internet access (to download Rakudo and MoarVM)
* `git` and standard build tools (`sudo apt install build-essential`)
* An Android device **connected** (can be the emulator) and reachable through `adb` (only one at a time), because cross-compilation tries to guess things by executing code on the device
* Android Studio + Android SDK / NDK (default Makefile config is for installation of these in default paths, install SDK & NDK from Android Studio) - please refer to Android documentation

## Building

To first check if everything seems to be OK, and look at customizable variables:

	make check

To build:

	make install

To specify a different arch than the default (which is x86_64):

	make ARCH=aarch64 check

### Customizable variables

The configuration variables can be passed as in the example above to the make command (no need to hack the `Makefile`). The list of these variables and their current default value is:

```make
ARCH           ?= x86_64
API_VERSION    ?= 26
ANDROID_SDK    ?= ${HOME}/Android/Sdk
RELEASE        ?= 2019.11
DBG_CFLAGS     ?= -g -O0
PROJ_JAVA_PATH ?= com/example/myapplication
```

### Makefile targets

These targets usable with `make`:
* `help` (default): displays the file you're currently reading (`README.md`)
* `check`: makes somes sanity checks about configuration and variables. Tests `adb` availability, NDK's cross compiler availability, presence and arch of connected device, etc… It also prints the variables actual values (so you can customize them when calling `make check`)
* `clean`: cleans up RakuDroid target directory (`app`)
* `clean-all`: cleans up everything and the directory final structure should become the one you freshly installed
* `clean-arch`: cleans up what needs to be rebuilt for a different arch (to switch between them)
* `all`: builds everything
* `install`: make a `gzip`'ed tarball of the complete `app` directory with all needed files in it. You can actually use this target directly after `make check` to build everything

## Building process explained

MoarVM is arch dependant.

Rakudo is not completely arch independant. There is (for what we need) one shared library that is built with it: `dynext/libperl6_ops_moar.so`. That `dynext/` part is a bit annoying because it prevents the library to be installed in `jniLibs/$(JNI_ARCH)/` like the others because the Android system does not support subdirectories in that directory. We are then obliged to install it elsewhere.

When building the following occurs:
* Rakudo is "`git clone`'ed" from github at the release specified in the `RELEASE` variable. It is then built with no special processing, as it would be built for the host (it is)
* MoarVM is "`git-clon`'ed" from github at the release specified in the same `RELEASE` variable. It is then patched to allow cross compiling for Android. It is at this very moment that the device (be it a real device or an emulator) is used. Only **one** device should be connected and reachable with `adb`
* `libperl6_ops_moar.so` is then cross compiled using the source in Rakudo
* Finally, `librakudroid.so` is cross compiled from source in `src/librakudroid/`

## Installation and test

As said before, this is a PoC. It is actually the second one.

The first goal was to have an Android application that, when launched, displays a kind or REPL. You entered the expression to eval, then clicked on "Eval", and the result (returned value) was displayed. The button "Extract Assets" was here in case the asset extraction has been interrupted at the beginning (should not happen, and this button is actually useless). Be careful, the evaluation step of REPL is done independantly each time. That means that a "my" variable is lost between evaluations. An "our" variable is kept, but you need to use it with the special package name `RakuDroidHelper`, e.g. `$RakuDroidHelper::my-var` at 2nd and forth times.

This is the second version of the PoC. It brings the Android system bindings. The variable `$RakuDroidHelper::main-activity` holds the activity object displayed and you can use the pre-filled text just by clicking "Eval" to display an Android Toast. More info will be given later in a separate file on how bindings are done and their use.

Once you have successfully `make install`'ed (that takes about 5mn on my host), you should end up with a file `MyApplication.tgz`. Launch Android Studio, select « New Project » and use the `Native C++` activity template. Keep the default settings (especially the ID of the application `com.example.myapplication`, or else overwrite the `PROJ_JAVA_PATH` variable), except for Android version to use, it should be **at least** Oreo (8.0).

Once this is done, in a shell, `cd` to the new application directory (which should be, using all defaults, `${HOME}/AndroidStudioProjects/MyApplication/`), then issue a `tar -xzvf /path/to/RakuDroid/MyApplication.tgz`.

Last step, in Android Studio (which should have detected and refreshed all changed files), click on "run" (the green triangle) and voilà…

The application startup time is, on the emulator on my PC, about 10 seconds. On my smartphone it is about 7 seconds. This may seem long, but in my first tests it was about 13 minutes (yes, minutes !) on the emulator. Maybe this can be improved more.

## Additional details / FAQ

### What does the patch do?

The current MoarVM cross-compilation system:
* makes assumtions on the target instead of trying to compile / execute tests executables
* does not support the Android architecture at all
  * Android uses "bionic" as libc
  * first tests using gcc cross compiler on command line failed. Using Android Studio-installed SDK / NDK was the solution. These could have been installed from the command line too, although I haven't tested because I needed Android Studio anyway.

The 1st patch modifies the following 4 files in the MoarVM source tree:
* `build/probe.pm`:
  * add a `sub run` which will be used instead of `system()` (or the backtick operator) calls when trying to run a test result. The sub replaces seamlessly (I hope so, not tested at the moment) the normal test runs when no `try_run` is defined. When there is one, it replaces it with a `try_cp` (to copy the executable to test on the target tempdir) followed by an effective call to `try_run` to run the test, and captures and returns the result back to the caller
  * replace all calls to `system()` and backticks with the appropriate call to `run()`
* `build/setup.pm`:
  * add a `%TC_ANDROID` into `%TOOLCHAINS` list. It declares the compiler family as 'android' (actually that means `clang` with "bionic", that is set elsewhere) and adds a rpath for finding libs in relocatable versions
  * add a `android` key in `%COMPILERS`, leading to parameters suited for it, including the `try_cp` and `try_run` mentioned above
  * add a `android` key in `%SYSTEMS`, leading to an `%OS_ANDROID` containing a `syslibs` overwrite and that changes a `-thirdparty`/`uv` variable (`UV_ANDROID`, added in `Makefile.in`, see below, instead of `UV_LINUX`)
* `build/Makefile.in`: adds the `UV_ANDROID` variable, adding `3rdparty/libuv/src/unix/pthread-fixes@obj@` to the list of objects needed by the build. It helps with some missing thread functions in bionic (no `libpthread.so` on Android)
* `Configure.pl`:
  * bypass the "guessed" configuration based on crossconf detection if `try_run` is set
  * patch the `sub setup_cross` (this is where I don't know if the patch is good in all situations) to:
	* detect system triplets ending with `-android(\d+)`, and remove the API version (the number) from it. That number is not used elsewhere, it is just here to detect and set the Android SDK compilation environment
	* do some trickery to get the configuration work correctly (need to test that on another cross-compiled thing to check if nothing is broken)

The 2nd patch modifies the following 3 files in the MoarVM source tree:
* `Configure.pl`: add a variable `cc_host` (containing the cross-compilation build system compiler)
* `build/Makefile.in`: set a Make variable from the above, and use that for (not cross) compiling `3rdparty/dynasm/minilua.c`
* `build/setup.pm`: set the above variable to `gcc` when compiling for Android

### Why Oreo?

During my tests I've been stuck a while trying to generate a shared library you can also call as an (almost) regular executable. It seems that before Oreo, the linker was not sending `argc` and `argv` to the functions declared in the section `.init_array`, which I use. I could have used some nasty trickery with the stack there, but I prefered not to. So the dependancy to at least Android API 26 a.k.a. Oreo a.k.a 8.0.
