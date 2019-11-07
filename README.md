# RakuDroid

RakuDroid is a PoC.

RakuDroid is a WIP.

RakuDroid's goal is to port Raku (A.K.A. Perl6) on Android platform.

The first work has been to cross compile Rakudo / MoarVM for Android. It uses the last release of both at the time of writing, i.e. 2019.07.1. This has been possible with just a patch to MoarVM. You can look at the patch in the file `src/rakudroid/0001-Make-MoarVM-cross-compile-nicely-for-Android.patch`.

## Supported platforms

* Build (cross compilation host): Linux x86_64
* Targets:
  * Architecture:
	* x86_64 (typically the Android emulator for testing)
	* aarch64 (arm 64-bit, most new smartphones)
  * Android versions: 8.0 and above (API 26+)

## Requirements for building

* Internet access (to download Rakudo and MoarVM)
* An Android device **connected** (can be the emulator) and accessible through `adb` (only one at a time), because cross-compilation tries to guess things by executing code on the device
* Android SDK / NDK (default config is for Android Studio's installation of these in default paths)

## Building

To first check if everything seems to be OK, and look at configurable variables:

	make check

To build:

	make all

To specify a different arch than the default (which is x86_64):

	make all ARCH=aarch64

To make a gzip'ed tarball:

	make install
