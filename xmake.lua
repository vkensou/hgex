add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_cxflags("/EHsc")
add_cxflags("/permissive")
set_languages("cxx20")

target("bass")
    set_kind("headeronly")

    add_includedirs("deps/bass/include", {public = true})
    add_headerfiles("deps/bass/include/*.h")

    after_build("windows", function(target)
        print("bassinstall")
        os.cp("deps/bass/bin/bass.dll", target:targetdir())
    end)

target("hgex")
    set_kind("shared")

    add_deps("bass")

    add_defines("HGEDLL") 

    if (is_os("windows")) then
        add_syslinks("user32", "shell32", "gdi32", "winmm")
    end

    if (is_os("windows")) then
        add_includedirs("deps/dx9sdkmini/include", {public = false})
        add_headerfiles("deps/dx9sdkmini/include/*.h", {install = false})

        if (is_arch("x86")) then
            add_linkdirs("deps/dx9sdkmini/lib/x86")
            add_links("d3d9.lib", "d3dx9.lib")
        end
    end

    add_includedirs("include", {public = true})
    add_headerfiles("include/hge.h")
    add_files("src/core/*.cpp")

    add_files("src/core/ZLIB/*.c")

rule("tutorial_base")
    add_deps("win.sdk.application")

    after_load(function(target)
        target:add("deps", "hgex")
        target:set("rundir", "$(projectdir)/tutorials/precompiled")
    end)

target("tutorial01")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial01/*.cpp")
    
target("tutorial02")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial02/*.cpp")