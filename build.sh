#!/bin/sh

base=muduo/base/tests/lib
net=muduo/net/tests/lib
build=build

# clean
if [ "$1" = "clean" ]; then
    echo "clean"

    cd muduo/base/tests  && make -f makefile clean && cd -
    cd muduo/net/tests  && make -f makefile clean && cd -
    rm -rf $build
    return
fi

# build
cd muduo/base/tests  && make -f makefile && cd -
cd muduo/net/tests  && make -f makefile && cd -


# make build dir.
if [ ! -d "build" ]; then
    mkdir -p build/lib
fi


# copy the lib to build dir
cp $base/libmuduo_base.a  $build/lib
cp $net/libmuduo.a  $build/lib
