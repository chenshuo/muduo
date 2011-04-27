package com.chenshuo.muduo.example.multiplexer;

import java.net.InetSocketAddress;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.regex.Pattern;

import org.jboss.netty.buffer.ChannelBufferFactory;
import org.jboss.netty.buffer.HeapChannelBufferFactory;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.chenshuo.muduo.example.multiplexer.testcase.TestOneClientBothSend;
import com.chenshuo.muduo.example.multiplexer.testcase.TestOneClientNoData;
import com.chenshuo.muduo.example.multiplexer.testcase.TestOneClientSend;
import com.chenshuo.muduo.example.multiplexer.testcase.TestOneClientBackendSend;
import com.chenshuo.muduo.example.multiplexer.testcase.TestTwoClients;

public class MultiplexerTest {
    private static final Logger logger = LoggerFactory.getLogger("MultiplexerTest");
    public static final ChannelBufferFactory bufferFactory =
            HeapChannelBufferFactory.getInstance(ByteOrder.LITTLE_ENDIAN);
    
    public final Pattern commandChannel = Pattern.compile("CONN (\\d+) FROM [0-9.:]+ IS ([A-Z]+)\r\n");
    
    private static final int kMultiplexerServerPort = 3333;
    private static final int kLogicalServerPort = 9999;
    private final InetSocketAddress multiplexerAddress;
    private final ExecutorService boss;
    private final ExecutorService worker;
    private EventQueue queue;
    private MyCountDownLatch latch;
    private MockBackendServer backend;
    private ArrayList<TestCase> testCases;

    public MultiplexerTest(String multiplexerHost) {
        multiplexerAddress = new InetSocketAddress(multiplexerHost, kMultiplexerServerPort);
        boss = Executors.newCachedThreadPool();
        worker = Executors.newCachedThreadPool();
        queue = new EventQueue();
        latch = new MyCountDownLatch(1);
        backend = new MockBackendServer(queue, kLogicalServerPort, boss, worker, latch);
        testCases = new ArrayList<TestCase>();
    }

    public static void main(String[] args) {
        if (args.length >= 1) {
            String multiplexerHost = args[0];
            MultiplexerTest test = new MultiplexerTest(multiplexerHost);
            test.addTestCase(new TestOneClientNoData());
            test.addTestCase(new TestOneClientSend());
            test.addTestCase(new TestOneClientBackendSend());
            test.addTestCase(new TestOneClientBothSend());
            test.addTestCase(new TestTwoClients());
            test.run();
        } else {
            System.out.println("Usage: ./run.sh path_to_test_data multiplexer_host");
            System.out.println("Example: ./run.sh localhost");
        }
    }

    private void addTestCase(TestCase testCase) {
        testCases.add(testCase);
        testCase.setOwner(this);
    }

    private void run() {
        logger.info("Waiting for connection");
        backend.start();
        latch.awaitUninterruptibly();

        logger.info("Ready");
        for (TestCase testCase : testCases) {
            testCase.test();
        }
        System.out.flush();
        sleep(500);
        logger.info("Finished");
        System.exit(0);
    }

    public MockClient newClient() {
        MockClient client = new MockClient(queue, multiplexerAddress, boss, worker);
        client.connectAndWait();
        return client;
    }

    public EventQueue getEventQueue() {
        return queue;
    }
    
    public MockBackendServer getBackend() {
        return backend;
    }

    public void sleep(int millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException e) {
        }
    }
}
