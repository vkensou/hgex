add_rules("mode.debug", "mode.release", "mode.releasedbg")
add_cxflags("/EHsc")
add_cxflags("/permissive")
set_languages("cxx20")

package("cgpu")
    add_urls("https://github.com/vkensou/cgpu.git")
    add_versions("latest", "develop")

    add_deps("spirv-reflect")

    on_install(function(package)
        import("package.tools.xmake").install(package, {}, {target="cgpu"})
    end)
package_end()

add_requires("minizip", "libpng")
add_requires("cgpu")

target("bass")
    set_kind("headeronly")

    add_includedirs("deps/bass/include", {public = true})
    add_headerfiles("deps/bass/include/*.h")

    after_build("windows", function(target)
        if (is_arch("x86")) then
            os.cp("deps/bass/bin/x86/bass.dll", target:targetdir())
        end
    end)

target("hgex")
    set_kind("shared")

    add_deps("bass")
    add_packages("minizip")
    add_packages("cgpu")

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
        elseif (is_arch("x64")) then
            add_linkdirs("deps/dx9sdkmini/lib/x64")
            add_links("d3d9.lib", "d3dx9.lib")
        end
    end

    add_includedirs("include", {public = true})
    add_headerfiles("include/hge.h")
    add_files("src/core/*.cpp")

target("hgexhelpers")
    set_kind("static")

    add_includedirs("include", {public = true})
    add_headerfiles("include/hge*.h")
    add_files("src/helpers/*.cpp")

target("particleed")
    set_kind("binary")
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
    set_rundir("$(projectdir)/tools/fonted")

    add_deps("hgex", "hgexhelpers")

    add_files("src/fontconv/*.cpp")
    
    after_build(function(target)
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/fonted")
        os.cp(path.join(target:targetdir(), "fontconv.exe"), "tools/fonted")
    end)

target("pngopt")
    set_kind("binary")
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
    set_rundir("$(projectdir)/tools/fonted")

    add_deps("hgex", "hgexhelpers")
    add_packages("minizip", "libpng")

    add_files("src/texasm/*.cpp")
    
    after_build(function(target)
        os.mkdir("tools/texasm")
        os.cp(path.join(target:targetdir(), "hgex.dll"), "tools/texasm")
        os.cp(path.join(target:targetdir(), "texasm.exe"), "tools/texasm")
    end)

rule("tutorial_base")
    add_deps("win.sdk.application")

    after_load(function(target)
        target:add("deps", "hgex", "hgexhelpers")
        target:set("rundir", "$(projectdir)/tutorials/precompiled")
    end)

target("tutorial01")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial01/*.cpp")

target("tutorial02")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial02/*.cpp")

target("tutorial03")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial03/*.cpp")

target("tutorial04")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial04/*.cpp")

target("tutorial05")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial05/*.cpp")

target("tutorial06")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial06/*.cpp")

target("tutorial07")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial07/*.cpp")

target("tutorial08")
    add_rules("tutorial_base")

    add_files("tutorials/tutorial08/*.cpp")