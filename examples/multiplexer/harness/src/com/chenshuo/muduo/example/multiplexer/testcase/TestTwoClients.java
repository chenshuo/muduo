package com.chenshuo.muduo.example.multiplexer.testcase;

import java.nio.charset.Charset;
import java.util.regex.Matcher;

import org.jboss.netty.buffer.ChannelBuffer;

import com.chenshuo.muduo.example.multiplexer.DataEvent;
import com.chenshuo.muduo.example.multiplexer.Event;
import com.chenshuo.muduo.example.multiplexer.EventSource;
import com.chenshuo.muduo.example.multiplexer.MockClient;
import com.chenshuo.muduo.example.multiplexer.TestCase;

public class TestTwoClients extends TestCase {

    @Override
    public void run() {
        if (!queue.isEmpty())
            fail("EventQueue is not empty");

        // step 1
        final MockClient client1 = god.newClient();
        Event ev = queue.take();
        DataEvent de = (DataEvent) ev;
        assertEquals(EventSource.kBackend, de.source);

        Matcher m = god.commandChannel.matcher(de.getString());
        if (!m.matches())
            fail("command channel message doesn't match.");

        final int connId1 = Integer.parseInt(m.group(1));
        assertTrue(connId1 > 0);
        client1.setId(connId1);
        assertEquals("UP", m.group(2));

        // step 2
        final MockClient client2 = god.newClient();
        de = (DataEvent) queue.take();
        assertEquals(EventSource.kBackend, de.source);

        m = god.commandChannel.matcher(de.getString());
        if (!m.matches())
            fail("command channel message doesn't match.");

        final int connId2 = Integer.parseInt(m.group(1));
        assertTrue(connId2 > 0);
        client2.setId(connId2);
        assertEquals("UP", m.group(2));

        ChannelBuffer buf = client1.send("hello");
        de = (DataEvent) queue.take();
        assertEquals(EventSource.kBackend, de.source);
        assertEquals(connId1, de.whichClient);
        assertEquals(buf, de.data);
        System.out.println(de.data.toString(Charset.defaultCharset()));

        // step 3
        buf = backend.sendToClient(connId2, "World!");
        de = (DataEvent) queue.take();
        assertEquals(EventSource.kClient, de.source);
        assertEquals(connId2, de.whichClient);
        assertEquals(buf, de.data);
        System.out.println(de.data.toString(Charset.defaultCharset()));

        // step 4
        client1.disconnect();
        de = (DataEvent) queue.take();
        assertEquals(EventSource.kBackend, de.source);
        m = god.commandChannel.matcher(de.getString());
        if (!m.matches())
            fail("command channel message doesn't match.");
        assertEquals(connId1, Integer.parseInt(m.group(1)));
        assertEquals("DOWN", m.group(2));

        client2.disconnect();
        de = (DataEvent) queue.take();
        assertEquals(EventSource.kBackend, de.source);
        m = god.commandChannel.matcher(de.getString());
        if (!m.matches())
            fail("command channel message doesn't match.");
        assertEquals(connId2, Integer.parseInt(m.group(1)));
        assertEquals("DOWN", m.group(2));

    }
}
