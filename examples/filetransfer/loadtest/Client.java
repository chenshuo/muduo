import java.net.InetSocketAddress;
import java.util.Random;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.Executors;

import org.jboss.netty.bootstrap.ClientBootstrap;
import org.jboss.netty.channel.ChannelFactory;
import org.jboss.netty.channel.ChannelPipeline;
import org.jboss.netty.channel.ChannelPipelineFactory;
import org.jboss.netty.channel.Channels;
import org.jboss.netty.channel.socket.nio.NioClientSocketChannelFactory;

public class Client {

    private static final class PipelineFactory implements ChannelPipelineFactory {
        private final int kMinLength;
        private final int kMaxLength;
        private final CountDownLatch latch;
        Random random = new Random();

        private PipelineFactory(int kMinLength, int kMaxLength, CountDownLatch latch) {
            this.kMinLength = kMinLength;
            this.kMaxLength = kMaxLength;
            this.latch = latch;
            assert kMinLength <= kMaxLength;
        }

        @Override
        public ChannelPipeline getPipeline() throws Exception {
            int variance = random.nextInt(kMaxLength - kMinLength + 1);
            int maxLength = kMinLength + variance;
            return Channels.pipeline(new Handler(maxLength, latch));
        }
    }

    static final int kClients = 500;
    static final int kMB = 1024 * 1024;
    static final int kMinLength = 1 * kMB;
    static final int kMaxLength = 6 * kMB;

    public static void main(String[] args) throws Exception {
        ChannelFactory channelFactory = new NioClientSocketChannelFactory(
                Executors.newCachedThreadPool(),
                Executors.newCachedThreadPool());
        long start = System.currentTimeMillis();

        final CountDownLatch latch = new CountDownLatch(kClients);
        ChannelPipelineFactory pipelineFactory = new PipelineFactory(kMinLength, kMaxLength, latch);
        for (int i = 0; i < kClients; ++i) {
            ClientBootstrap bootstrap = new ClientBootstrap(channelFactory);
            bootstrap.setPipelineFactory(pipelineFactory);
            bootstrap.connect(new InetSocketAddress(args[0], 2021));
        }

        latch.await();

        System.out.println(Thread.currentThread().getId() + " All done. "
                + (System.currentTimeMillis() - start));
        System.exit(0);
    }

}
