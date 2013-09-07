project "base"
    kind "StaticLib"
    language "C++"
    links{'pthread', 'rt'}
    targetdir(libdir)
    targetname('muduo_base')
    headersdir('muduo/base')
    headers('*.h')
    files {
            'AsyncLogging.cc',
            'Condition.cc',
            'CountDownLatch.cc',
            'Date.cc',
            'Exception.cc',
            'FileUtil.cc',
            'LogFile.cc',
            'Logging.cc',
            'LogStream.cc',
            'ProcessInfo.cc',
            'Timestamp.cc',
            'TimeZone.cc',
            'Thread.cc',
            'ThreadPool.cc',
     }
