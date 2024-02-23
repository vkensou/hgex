add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_cxflags("/EHsc")
add_cxflags("/permissive")
set_languages("cxx20")

target("hgex")
    set_kind("shared")

    add_defines("HGEDLL") 

    if (is_os("windows")) then
        add_syslinks("user32", "shell32", "gdi32", "winmm")
    end

    if (is_os("windows")) then
        add_includedirs("deps/dx9sdkmini/include", {public = false})
        add_headerfiles("deps/dx9sdkmini/include/*.h")

        if (is_arch("x86")) then
            add_linkdirs("deps/dx9sdkmini/lib/x86")
            add_links("d3d9.lib", "d3dx9.lib")
        end
    end

    add_includedirs("deps/bass/include", {public = false})
    add_headerfiles("deps/bass/include/*.h")

    add_includedirs("include", {public = true})
    add_headerfiles("include/hge.h")
    add_files("src/core/*.cpp")

    add_files("src/core/ZLIB/*.c")

target("tutorial01")
    add_rules("win.sdk.application")
    
    add_deps("hgex")

    add_files("tutorials/tutorial01/*.cpp")
    
target("tutorial02")
    add_rules("win.sdk.application")
    
    add_deps("hgex")

    add_files("tutorials/tutorial02/*.cpp")