#!/bin/sh

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-../build}
cd $BUILD_DIR
cmake $SOURCE_DIR
make

