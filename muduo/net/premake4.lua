project "net"
    kind "StaticLib"
    language "C++"
    links('base')
    targetdir(libdir)
    targetname('muduo_net')
    includedirs('../..')
    headersdir('muduo/net')
    headers {
        'Buffer.h',
        'Callbacks.h',
        'Channel.h',
        'Endian.h',
        'EventLoop.h',
        'EventLoopThread.h',
        'EventLoopThreadPool.h',
        'InetAddress.h',
        'TcpClient.h',
        'TcpConnection.h',
        'TcpServer.h',
        'TimerId.h',
    }

    files {
        'Acceptor.cc',
        'Buffer.cc',
        'Channel.cc',
        'Connector.cc',
        'EventLoop.cc',
        'EventLoopThread.cc',
        'EventLoopThreadPool.cc',
        'InetAddress.cc',
        'Poller.cc',
        'poller/DefaultPoller.cc',
        'poller/EPollPoller.cc',
        'poller/PollPoller.cc',
        'Socket.cc',
        'SocketsOps.cc',
        'TcpClient.cc',
        'TcpConnection.cc',
        'TcpServer.cc',
        'Timer.cc',
        'TimerQueue.cc',
     }

