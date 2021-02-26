package com.chenshuo.muduo.example.multiplexer;

import java.net.InetSocketAddress;
import java.util.concurrent.Executor;
import java.util.concurrent.TimeUnit;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelFuture;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.ChannelStateEvent;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.ExceptionEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelHandler;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;
import org.jboss.netty.util.HashedWheelTimer;
import org.jboss.netty.util.Timeout;
import org.jboss.netty.util.Timer;
import org.jboss.netty.util.TimerTask;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MockClient {
    private static final Logger logger = LoggerFactory.getLogger("MockClient");

    private class Handler extends SimpleChannelHandler {

        @Override
        public void channelConnected(ChannelHandlerContext ctx, ChannelStateEvent e)
                throws Exception {
            logger.debug("channelConnected {},, {}", ctx, e);
            assert connection == null;
            connection = e.getChannel();
            if (latch != null)
                latch.countDown();
        }

        @Override
        public void channelDisconnected(ChannelHandlerContext ctx, ChannelStateEvent e)
                throws Exception {
            logger.debug("channelDisconnected {},, {}", ctx, e);
            assert connection == e.getChannel();
            connection = null;
        }

        @Override
        public void messageReceived(ChannelHandlerContext ctx, MessageEvent e)
                throws Exception {
            logger.debug("messageReceived {},, {}", ctx, e);
            assert connection == e.getChannel();
            queue.put(new DataEvent(EventSource.kClient, connId, (ChannelBuffer) e.getMessage()));
        }

        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, ExceptionEvent e)
                throws Exception {
            logger.error("exceptionCaught {},, {}", ctx, e);
            // reconnect();
        }

        @SuppressWarnings("unused")
        private void reconnect() {
            timer.newTimeout(new TimerTask() {
                @Override
                public void run(Timeout timeout) throws Exception {
                    logger.info("Reconnecting");
                    bootstrap.connect();

                }
            }, 5, TimeUnit.SECONDS);
        }
    }

    private final EventQueue queue;
    private final InetSocketAddress remoteAddress;
    private final Executor boss;
    private final Executor worker;
    private final Timer timer;
    private volatile Channel connection;
    private ClientBootstrap bootstrap;
    private int connId;
    private MyCountDownLatch latch;

    public MockClient(EventQueue queue, InetSocketAddress remoteAddress, Executor boss,
            Executor worker) {
        this.queue = queue;
        this.remoteAddress = remoteAddress;
        this.boss = boss;
        this.worker = worker;
        this.timer = new HashedWheelTimer();
        connId = -1;
    }

    public ChannelFuture connect() {
        assert bootstrap == null;

        ChannelFactory factory = new NioClientSocketChannelFactory(boss, worker);
        bootstrap = new ClientBootstrap(factory);

        bootstrap.setPipelineFactory(new ChannelPipelineFactory() {
            public ChannelPipeline getPipeline() {
                return Channels.pipeline(new Handler());
            }
        });

        bootstrap.setOption("tcpNoDelay", true);
        bootstrap.setOption("remoteAddress", remoteAddress);

        return bootstrap.connect();
    }

    public void connectAndWait() {
        latch = new MyCountDownLatch(1);
        connect();
        latch.awaitUninterruptibly(500);
        assert connection != null;
    }

    public void send(ChannelBuffer buf) {
        connection.write(buf);
    }
    
    public ChannelBuffer send(String str) {
        byte[] bytes = str.getBytes();
        ChannelBuffer buf = MultiplexerTest.bufferFactory.getBuffer(bytes, 0, bytes.length);
        connection.write(buf);
        return buf;
    }
    
    public void disconnect() {
        connection.close();
    }

    public void setId(int connId) {
        assert this.connId == -1;
        this.connId = connId;
    }
}
