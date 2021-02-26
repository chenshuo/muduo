import java.math.BigInteger;
import java.security.MessageDigest;
import java.util.concurrent.CountDownLatch;

import org.jboss.netty.buffer.BigEndianHeapChannelBuffer;
import org.jboss.netty.channel.Channel;
import org.jboss.netty.channel.ChannelHandlerContext;
import org.jboss.netty.channel.ChannelStateEvent;
import org.jboss.netty.channel.ExceptionEvent;
import org.jboss.netty.channel.MessageEvent;
import org.jboss.netty.channel.SimpleChannelUpstreamHandler;

public class Handler extends SimpleChannelUpstreamHandler {

    private static int created = 0;
    private int received = 0;
    private final int maxLength;
    private int id;
    private CountDownLatch latch;
    private MessageDigest digest;

    public Handler(int maxLength, CountDownLatch latch) throws Exception {
        this.id = created++;
        this.maxLength = maxLength;
        this.latch = latch;
        this.digest = MessageDigest.getInstance("MD5");
        System.out.println("Handler tid=" + Thread.currentThread().getId() + " " + id);
    }

    @Override
    public void channelConnected(ChannelHandlerContext ctx, ChannelStateEvent e)
            throws Exception {
        System.out.println("channelConnected tid=" + Thread.currentThread().getId() + " " + id);
    }

    @Override
    public void channelDisconnected(ChannelHandlerContext ctx, ChannelStateEvent e)
            throws Exception {
        byte[] md5 = digest.digest();
        BigInteger bigInt = new BigInteger(1, md5);
        System.out.println("channelDisconnected tid=" + Thread.currentThread().getId() + " " + id
                + " got "
                + received + " " + bigInt.toString(16));
        latch.countDown();
    }

    @Override
    public void messageReceived(ChannelHandlerContext ctx, MessageEvent e) throws Exception {
        BigEndianHeapChannelBuffer message = (BigEndianHeapChannelBuffer) e.getMessage();
        // System.out.println("messageReceived " + ctx.getChannel() + message.readableBytes());
        received += message.readableBytes();
        digest.update(message.array(), message.readerIndex(), message.readableBytes());
        if (received > maxLength) {
            System.out.println("messageReceived tid=" + Thread.currentThread().getId()
                    + " " + id + " got " + received);
            ctx.getChannel().close();
        }
    }

    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, ExceptionEvent e) throws Exception {
        e.getCause().printStackTrace();

        Channel ch = e.getChannel();
        ch.close();
        latch.countDown();
    }
}
