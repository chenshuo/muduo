#!/bin/sh

CLASSPATH=lib/netty-3.2.4.Final.jar:lib/slf4j-api-1.6.1.jar:lib/slf4j-simple-1.6.1.jar:./bin

export CLASSPATH
mkdir -p bin
javac -d bin ./src/com/chenshuo/muduo/example/multiplexer/*.java ./src/com/chenshuo/muduo/example/multiplexer/testcase/*.java
java -ea -Djava.net.preferIPv4Stack=true com.chenshuo.muduo.example.multiplexer.MultiplexerTest localhost
