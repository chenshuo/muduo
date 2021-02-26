package com.chenshuo.muduo.example.multiplexer;

import static org.jboss.netty.buffer.ChannelBuffers.wrappedBuffer;

import java.net.InetSocketAddress;
import java.nio.charset.Charset;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executor;

import org.jboss.netty.bootstrap.ServerBootstrap;
import org.jboss.netty.buffer.ChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.ChannelStateEvent;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.ExceptionEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelHandler;
import org.jboss.netty.channel.socket.nio.NioServerSocketChannelFactory;
import org.jboss.netty.handler.codec.frame.LengthFieldBasedFrameDecoder;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class MockBackendServer {
    private static final Logger logger = LoggerFactory.getLogger("MockBackendServer");

    private class Handler extends SimpleChannelHandler {

        @Override
        public void channelConnected(ChannelHandlerContext ctx, ChannelStateEvent e)
                throws Exception {
            logger.debug("channelConnected {},, {}", ctx, e);
            assert connection == null;
            connection = e.getChannel();
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
            ChannelBuffer input = (ChannelBuffer) e.getMessage();
            int len = input.readUnsignedByte();
            int whichClient = input.readUnsignedShort();
            assert len == input.readableBytes();
            logger.debug("From {}, '{}'", whichClient, input.toString(Charset.defaultCharset()));
            queue.put(new DataEvent(EventSource.kBackend, whichClient, input));
        }

        @Override
        public void exceptionCaught(ChannelHandlerContext ctx, ExceptionEvent e)
                throws Exception {
            logger.error("exceptionCaught {},, {}", ctx, e);
        }
    }

    private final EventQueue queue;
    private final int port;
    private final Executor boss;
    private final Executor worker;
    private final CountDownLatch latch;
    private Channel listener;
    private volatile Channel connection;

    public MockBackendServer(EventQueue queue, int listeningPort, Executor boss, Executor worker,
            CountDownLatch latch) {
        this.queue = queue;
        port = listeningPort;
        this.boss = boss;
        this.worker = worker;
        this.latch = latch;
    }

    public void start() {
        ServerBootstrap bootstrap = getBootstrap();
        listener = bootstrap.bind(new InetSocketAddress(port));
        logger.debug("started");
    }

    public void sendToClient(int whichClient, ChannelBuffer data) {
        ChannelBuffer output = data.factory().getBuffer(3);
        output.writeByte(data.readableBytes());
        output.writeShort(whichClient);
        connection.write(wrappedBuffer(output, data));
    }

    public ChannelBuffer sendToClient(int whichClient, String str) {
        byte[] bytes = str.getBytes();
        ChannelBuffer data = MultiplexerTest.bufferFactory.getBuffer(bytes, 0, bytes.length);
        sendToClient(whichClient, data);
        return data;
    }

    public void stop() {
        listener.close();
    }

    private ServerBootstrap getBootstrap() {
        ChannelFactory factory = new NioServerSocketChannelFactory(boss, worker);
        ServerBootstrap bootstrap = new ServerBootstrap(factory);
        bootstrap.setPipelineFactory(new ChannelPipelineFactory() {
            @Override
            public ChannelPipeline getPipeline() throws Exception {
                return Channels.pipeline(
                        new LengthFieldBasedFrameDecoder(255 + 3, 0, 1, 2, 0),
                        new Handler());
            }
        });
        bootstrap.setOption("reuseAddress", true);
        bootstrap.setOption("child.tcpNoDelay", true);
        bootstrap.setOption("child.bufferFactory", MultiplexerTest.bufferFactory);
        return bootstrap;
    }
}
