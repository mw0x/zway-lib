# zway-lib

This library provides network, cryptography and storage facilities for the Zway messenger app.

## Network

The library provides the client interface for server connection.

All server communication is done over TLS (version 1.2) using the [GnuTLS](http://www.gnutls.org/) library.

The protocol is based on UBJ (Univeral Binary JSON) for exchange of dynamic data structures and therefore very flexible.

## Cryptography

For cryptography we rely on the [Nettle](https://www.lysator.liu.se/~nisse/nettle/) library, which is also used by GnuTLS internally.

Hybrid encryption is applied using AES-256 (counter mode) for symmetric encryption of the message contents and RSA-2048 for end-to-end encrpytion of a random message key. I'm aware of perfect forward secrecy, which will be implemented soon.

## Storage

The library provides password-protected, encrypted storage based on [SQLite](https://www.sqlite.org/), which enables us to insert, update, delete and query encrypted records.

## How to build

The library depends, as already mentioned, on other open-source libraries, which are:

* libgmp (Multi precision arithmetic, needed by libhogweed)
* libnettle (Symmetric ciphers)
* libhogweed (Asymmetric ciphers)
* libgnutls (Transport layer security)
* libsqlite3 (Storage)
* libexif (Exif data processing)

You can build these libraries yourself or install them via package manager if available.

For building libzway itself, CMake is required.

### Build under Linux or Mac

In case you want to build the dependencies yourself, there is a [helper script](https://github.com/mw0x/zway-lib/blob/master/deps/build.sh) in the deps directory that will build static libraries for you, but you will have to download the source packages yourself and place them inside the deps directory. The script will create an install directory inside the deps folder and install the compiled libraries into there.

Switch to the deps sub directory:

```
$ cd zway-lib/deps/ 
```

Fire build script:

```
$ ./build.sh 
```

If everything went well, switch back to the root directory:

```
$ cd .. 
```

Create a build directory and switch to it:

```
$ mkdir build && cd build 
```

Generate Makefile via CMake:

```
$ cmake -DCMAKE_BUILD_TYPE=Release .. 
```

Fire build command:

```
$ make
```

### Build under Linux or Mac for Android

Building for Android requires the [Android NDK](https://developer.android.com/ndk/index.html).

The helper script can also be used to build the dependencies for Android.

Switch to the deps sub directory:

```
$ cd zway-lib/deps/ 
```

Fire build script:

```
$ ./build.sh android_arm7 PATH_TO_YOUR_ANDROID_NDK 19 linux_64
```

If everything went well, switch back to the root directory:

```
$ cd .. 
```

Create a build directory and switch to it:

```
$ mkdir build && cd build 
```

Unfortunately CMake is making trouble delivering the configuration switch ANDROID_NDK_PATH to compiler tests,
so I had to hardcode it, please adjust it in the [android_arm7_toolchain.cmake](https://github.com/mw0x/zway-lib/blob/master/android_arm7_toolchain.cmake) file.

If anyone has any clue about that issue, please let me know.

Generate Makefile via CMake:

```
$ cmake -DCMAKE_BUILD_TYPE=Release -DANDROID_NDK_PATH=PATH_TO_ANDROID_NDK -DANDROID_API_LEVEL=19 -DCMAKE_TOOLCHAIN_FILE=android_arm7_toolchain.cmake .. 
```

Fire build command:

```
$ make
```

### Build under Linux for Windows

Details to follow ...
