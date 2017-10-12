#!/bin/sh
#!/usr/bin/env bash
# ******************************************************
# DESC    : zookeeper devops script
# AUTHOR  : Alex Stocks
# VERSION : 1.0
# LICENCE : LGPL V3
# EMAIL   : alexstocks@foxmail.com
# MOD     : 2016-05-13 02:01
# FILE    : load.sh
# ******************************************************

name="zookeeper"

base=muduo/base/tests/lib
net=muduo/net/tests/lib
build=build

usage() {
    echo "Usage: $0 build [base | net | all] # in default, build all."
    echo "       $0 clean [base | net | all] # in default, clean all."
}

build() {
    target=$1
    echo "target:" $target
    # make build dir.
    if [ ! -d "build" ]; then
        mkdir -p build/lib
    fi

    case C"$target" in
        Cbase)
            cd muduo/base/tests  && make -f makefile && cd -
            cp $base/libmuduo_base.a  $build/lib
            ;;
        Cnet)
            cd muduo/net/tests  && make -f makefile && cd -
            cp $net/libmuduo.a  $build/lib
            ;;
        C*)
            cd muduo/base/tests  && make -f makefile && cd -
            cp $base/libmuduo_base.a  $build/lib
            cd muduo/net/tests  && make -f makefile && cd -
            cp $net/libmuduo.a  $build/lib
            ;;
    esac
}

clean() {
    target=$1
    echo "target:" $target

    case C"$target" in
        Cbase)
            cd muduo/base/tests  && make -f makefile clean && cd -
            rm -rf $build/lib/libmuduo_base.a
            ;;
        Cnet)
            cd muduo/net/tests  && make -f makefile clean && cd -
            rm -rf $build/lib/libmuduo.a
            ;;
        C*)
            cd muduo/base/tests  && make -f makefile clean && cd -
            cd muduo/net/tests  && make -f makefile clean && cd -
            rm -rf $build
            ;;
    esac
}

opt=$1
target="all"
if [ $# == 2 ]; then
	target=$2
fi

case C"$opt" in
    Cbuild)
        build $target
        ;;
    Cclean)
        clean $target
        ;;
    C*)
        usage
        ;;
esac

