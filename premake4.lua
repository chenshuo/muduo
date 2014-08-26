solution "muduo"
    projectdir = basedir()
    builddir = path.join(projectdir, '../build')
    installdir = path.join(projectdir, '../install')
    bindir = path.join(installdir, 'bin')
    libdir = path.join(installdir, 'lib')
    includedir = path.join(installdir, 'include')
    curheadersdir = includedir

    -- set the sub header dir to install files
    function headersdir(dir)
        curheadersdir = dir
    end

    -- install header files
    function headers(files)
        if type(files) == 'string' then
            files = { files }
        end
        local finalfiles = {}
        for _, file in ipairs(files) do
            if file:find('*') then
                file = os.matchfiles(file)
            end 
            finalfiles = table.join(finalfiles, file)
        end
        local source = table.concat(finalfiles, ' ')
        local target = path.join(includedir, curheadersdir)
        assert(os.mkdir(target))
        assert(os.execute('cp ' .. source .. ' ' .. target))
    end

    -- solution level settings
    configurations { "Debug", "Release" }
    location(builddir)
    includedirs(includedir)

    defines '_FILE_OFFSET_BITS=64'
    flags {"ExtraWarnings", "FatalWarnings", "Symbols"}
    buildoptions { '-g', 
                   '-march=native', 
                   '-rdynamic',
                   '-Wconversion',
                   '-Wno-unused-parameter',
                   '-Wold-style-cast',
                   '-Woverloaded-virtual',
                   '-Wpointer-arith',
                   '-Wshadow',
                   '-Wwrite-strings',
    } 

    configuration "Debug"
        buildoptions {'-O0'} 
        targetsuffix('-g')
    configuration "Release"
        buildoptions {'-finline-limit=1000'}
        defines { "NDEBUG" }
        flags { "Optimize" }  

--   filter by kind not support in premake4.4 .4 .4 .4 beta
--    configuration {"*App"}
--        targetdir(bindir)
--    configuration { "*Lib" }
--        targetdir(libdir)
--    configuration {}

    include 'muduo/base'
    include 'muduo/net'
    include 'examples/simple'
