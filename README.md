# Build on Debian or Ubuntu

## Install required packages

```shell
# Required
$ sudo apt-get install g++ libboost-dev cmake
```

If you don't want `boost`, check out the `cpp17` branch and build with g++ 7 or above.

## Install optional packages

See `.travis.yml` for optional packages.

## Get source code and compile

```shell
$ git clone https://github.com/chenshuo/muduo.git
$ cd muduo
$ ./build.sh
```

Check output files in ../build/release-cpp11/bin

To build debug version:

```shell
$ BUILD_TYPE=debug ./build.sh
```

