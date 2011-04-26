package com.chenshuo.muduo.example.multiplexer;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

public class MyCountDownLatch extends CountDownLatch {

    public MyCountDownLatch(int count) {
        super(count);
    }

    public void awaitUninterruptibly() {
        try {
            await();
        } catch (InterruptedException e) {
        }
    }

    public void awaitUninterruptibly(int millis) {
        try {
            await(millis, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
        }
    }

}
