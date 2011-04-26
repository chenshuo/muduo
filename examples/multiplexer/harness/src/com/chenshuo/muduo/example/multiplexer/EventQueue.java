package com.chenshuo.muduo.example.multiplexer;

import java.util.concurrent.BlockingDeque;
import java.util.concurrent.LinkedBlockingDeque;
import java.util.concurrent.TimeUnit;

public class EventQueue {
    private BlockingDeque<Event> queue = new LinkedBlockingDeque<Event>();
    
    public void put(Event e) {
        queue.add(e);
    }
    
    public Event take() {
        try {
            return queue.poll(5, TimeUnit.SECONDS);
        } catch (InterruptedException e) {
            return null;
        }
    }
    
    public boolean isEmpty() {
        return queue.isEmpty();
    }
}
