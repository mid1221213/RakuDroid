# RakuDroid

RakuDroid is a PoC.

RakuDroid is a WIP.

RakuDroid's goal is to port Raku (a.k.a. Perl6) on Android platform.

The first work has been to cross compile Rakudo / MoarVM for Android. It uses the last release of both at the time of writing, i.e. `2019.07.1`. This has been possible with just a patch to MoarVM. You can look at the patch in the file `src/librakudroid/0001-Make-MoarVM-cross-compile-nicely-for-Android.patch`.

## Supported platforms

* Build (cross compilation host): Linux x86_64
* Targets:
  * Architecture:
	* `x86_64` (typically the Android emulator for testing)
	* `aarch64` (arm 64-bit, most new smartphones)
  * Android versions: 8.0 (Oreo) and above (API 26+)

## Requirements for building

* Internet access (to download Rakudo and MoarVM)
* `git` & `wget`
* An Android device **connected** (can be the emulator) and accessible through `adb` (only one at a time), because cross-compilation tries to guess things by executing code on the device
* Android SDK / NDK (default config is for Android Studio's installation of these in default paths)

## Building

To first check if everything seems to be OK, and look at customizable variables:

	make check

To build:

	make all

To specify a different arch than the default (which is x86_64):

	make all ARCH=aarch64

To make a gzip'ed tarball:

	make install

### Customizable variables

The configuration variables can be passed as in the example above to the make command (no need to hack the `Makefile`. The list of these variables and their current default value is:

```make
ARCH           ?= x86_64
API_VERSION    ?= 26
ANDROID_SDK    ?= ${HOME}/Android/Sdk
RELEASE        ?= 2019.07.1
DBG_CFLAGS     ?= -g -O0
```

### Makefile targets

These targets are publicly exported targets usable with `make`:
- `help` (default): displays the file you're currently reading (`README.md`)
- `check`: makes somes sanity checks about configuration and variables. Tests `adb` availability, NDK's cross compiler availability, presence and arch of connected device, etc… It also prints the variables actual values (so you can customize them when calling `make check`)
- `all`: builds everything
- `clean`: cleans up RakuDroid target directory (`app`)
- `clean-all`: cleans up everything and the directory final structure should become the one you installed first
- `install`: make a gzip'ed tarball of the complete `app` directory with all needed files in it. You can actually use this target directly after `make check` to build everything

## Building process explained

MoarVM is arch dependant.

Rakudo should be arch independant but is not completely. There is one shared library that is built with it: `dynext/libperl6_ops_moar.so`. That `dynext/` part is a bit annoying because it prevents the library to be installed in `jniLibs/$(JNI_ARCH)/` like the others because the Android system does not support subdirectories in this directory. We are then obliged to install it elsewhere.

To build for, say, `X86_64`, the following occurs:
- Rakudo is "git-cloned" from github at the release specified in the `RELEASE` variable. It is then built with no special processing, as it would be built for the host
- MoarVM is "git-cloned" from github at the release specified in the same `RELEASE` variable. It is then patched (see `src/librakudroid/0001-Make-MoarVM-cross-compile-nicely-for-Android.patch`) to allow cross compiling for Android. It is at this very moment that the device (be it a real device or an emulator) is used. At the moment only **one** device should be connected, RakuDroid does not allow for specifying the target device. This is possible and easy to do, but too early for these specialized customizations in this project
- `libperl6_ops_moar.so` is then cross compiled using the source in Rakudo
- Finally, `librakudroid.so` is cross compiled from source in `src/librakudroid/`

## Installation and test

As said before, this is a PoC. The goal at the moment is to have an Android application that, when launched, displays **exactly** « 5 + 4² = 9 », where the source of this string, which is eval'ed by Rakudo, is « `'5 + 4² = ' ~ 5 + 4²` » (`src/AndroidStudio/native-lib.cpp` line 30 at the moment of writing).

One you have successfully `make install`'ed, you should end up with a file `MyApplication.tgz`. Launch Android Studio, select to make a new project and use the `C++` activity template. Keep the default settings (especially the name of the application `com.example.myapplication`).

Once this is done, in a shell, `cd` to the new application directory (which should be, using all defaults, `${HOME}/AndroidStudioProjects/MyApplication/`), then issue a `tar -xzvf ${PATH_TO_RAKUDROID}/MyApplication.tgz`.

Last step, in Android Studio, click on "run" (the green triangle) and voilà…
