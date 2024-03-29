target("bootstrapofwasm")
    set_kind("binary")
    set_rundir("$(projectdir)/wasm/tutorials/precompiled")
    add_packages("wamr-hge")
    add_deps("hgex", "hgexhelpers")
    add_files("src/bootstrapofwasm/*.cpp")

rule("wasm_tutorial_base")
    after_load(function(target)
        target:set("plat", "wasm")
        target:set("toolchains", "wasi")
        target:set("kind", "binary")
        target:set("group", "wasm_tutorials")
        target:add("deps", "bootstrapofwasm", {inherit = false})
        target:set("exceptions", "no-cxx")
        target:add("cxxflags", "-pthread")
        target:add("cxxflags", " -z stack-size=32768")
        target:add("ldflags", "--verbose", {force=true})
        target:add("ldflags", "-Wl,--shared-memory,--max-memory=262144")
        target:add("ldflags", "-Wl,--no-check-features")
        target:add("ldflags", "-Wl,--export=__heap_base,--export=__data_end")
        target:add("ldflags", "-Wl,--export=__wasm_call_ctors")
        target:add("includedirs", "wasm/include", {public = true})
        target:add("headerfiles", "wasm/include/hge.h")
    end)

    after_build(function(target)
        local dir = target:targetdir()
        local filename = target:basename() .. ".html"
        local filepath = path.join(dir, filename)
        local outdir = "wasm/tutorials/precompiled"
        os.mkdir(outdir)
        os.cp(filepath, outdir)
        os.mv(path.join(outdir, filename), path.join(outdir, target:basename() .. ".wasm"))
    end)

    on_run(function(target)
        local rundir = target:dep("bootstrapofwasm"):rundir()
        local bootstrap = target:dep("bootstrapofwasm"):targetfile()
        local args = {target:basename() .. ".wasm"}
        os.execv(bootstrap, args, {curdir = rundir})
    end)
rule_end()

for _, dir in ipairs(os.filedirs("tutorials/tutorial*")) do
    target("wasm_" .. path.basename(dir))
        add_rules("wasm_tutorial_base")
        add_files(path.join(dir, "*.cpp"))
end
