project "simple_echo"
    kind "ConsoleApp"
    language "C++"
    targetdir(bindir)
    links{'net', 'base', 'pthread', 'rt'}
    files {
        'echo/echo.cc',
        'echo/main.cc',
    }

