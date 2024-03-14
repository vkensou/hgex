add_repositories("hge-xrepo xrepo", {rootdir = os.scriptdir()})

add_rules("mode.debug", "mode.release", "mode.releasedbg")
set_exceptions("cxx")
set_languages("cxx20")
if (is_os("windows")) then
    add_defines("NOMINMAX")
end

add_requires("minizip", "libpng")
add_requires("cgpu")
add_requires("freeimage")
add_requires("glm")
add_requires("wasm-micro-runtime")

target("bass")
    set_kind("headeronly")

    add_includedirs("deps/bass/include", {public = true})
    add_headerfiles("deps/bass/include/*.h")

    after_build("windows", function(target)
        if (is_arch("x86")) then
            os.cp("deps/bass/bin/x86/bass.dll", target:targetdir())
        end
    end)

target("jobsystem")
    set_kind("static")
    add_includedirs("src/jobsystem", {public = true})
    add_files("src/jobsystem/utils/*.cpp")

target("hgex")
    set_kind("shared")

    add_deps("bass")
    add_deps("jobsystem")
    add_packages("minizip")
    add_packages("cgpu")
    add_packages("freeimage")
    add_packages("glm")
    add_rules("utils.hlsl2spv", {bin2c = true})

    add_defines("HGEDLL")

    if (is_os("windows")) then
        add_syslinks("user32", "shell32", "gdi32", "winmm", "advapi32")
    end

    if (is_os("windows")) then
        add_includedirs("deps/dx9sdkmini/include", {public = false})
        add_headerfiles("deps/dx9sdkmini/include/*.h", {install = false})

        if (is_arch("x86")) then
            add_linkdirs("deps/dx9sdkmini/lib/x86")
            add_links("d3d9.lib", "d3dx9.lib")
        elseif (is_arch("x64")) then
            add_linkdirs("deps/dx9sdkmini/lib/x64")
            add_links("d3d9.lib", "d3dx9.lib")
        end
    end

    add_includedirs("include", {public = true})
    add_headerfiles("include/hge.h")
    add_files("src/core/*.cpp")
    add_files("src/core/*.hlsl")

target("hgexhelpers")
    set_kind("static")

    add_includedirs("include", {public = true})
    add_headerfiles("include/hge*.h")
    add_files("src/helpers/*.cpp")

target("particleed")
    set_kind("binary")
    set_group("tools")
    set_rundir("$(projectdir)/tools/particleed")

    add_rules("win.sdk.application")
    add_deps("hgex", "hgexhelpers")

    add_files("src/particleed/*.cpp")

    after_build(function(target)
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/particleed")
        os.cp(path.join(target:targetdir(), "particleed.exe"), "tools/particleed")
    end)

target("fonted")
    set_kind("binary")
    set_group("tools")
    set_rundir("$(projectdir)/tools/fonted")

    add_rules("win.sdk.application")
    add_deps("hgex", "hgexhelpers")
    add_packages("minizip", "libpng")

    add_files("src/fonted/*.cpp")
    
    after_build(function(target)
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/fonted")
        os.cp(path.join(target:targetdir(), "fonted.exe"), "tools/fonted")
    end)

target("fontconv")
    set_kind("binary")
    set_group("tools")
    set_rundir("$(projectdir)/tools/fonted")

    add_deps("hgex", "hgexhelpers")

    add_files("src/fontconv/*.cpp")
    
    after_build(function(target)
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/fonted")
        os.cp(path.join(target:targetdir(), "fontconv.exe"), "tools/fonted")
    end)

target("pngopt")
    set_kind("binary")
    set_group("tools")
    set_rundir("$(projectdir)/tools/fonted")

    add_deps("hgex", "hgexhelpers")
    add_packages("minizip", "libpng")

    add_files("src/pngopt/*.cpp")
    
    after_build(function(target)
        os.mkdir("tools/texasm")
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/texasm")
        os.cp(path.join(target:targetdir(), "pngopt.exe"), "tools/texasm")
    end)

target("texasm")
    set_kind("binary")
    set_group("tools")
    set_rundir("$(projectdir)/tools/fonted")

    add_deps("hgex", "hgexhelpers")
    add_packages("minizip", "libpng")

    add_files("src/texasm/*.cpp")
    
    after_build(function(target)
        os.mkdir("tools/texasm")
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/texasm")
        os.cp(path.join(target:targetdir(), "texasm.exe"), "tools/texasm")
    end)

includes("wasm/xmake.lua")

includes("tutorials/xmake.lua")