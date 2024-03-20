package("wamr-hge")
    set_homepage("https://github.com/bytecodealliance/wasm-micro-runtime")
    set_description("WebAssembly Micro Runtime (WAMR)")
    set_license("Apache-2.0")

    add_urls("https://github.com/bytecodealliance/wasm-micro-runtime/archive/refs/tags/WAMR-$(version).tar.gz", {alias = "archive", excludes = {"*/language-bindings/python/LICENSE"}})
    add_urls("https://github.com/bytecodealliance/wasm-micro-runtime.git", {alias = "github"})

    add_versions("archive:1.3.2", "58961ba387ed66ace2dd903597f1670a42b8154a409757ae6f06f43fe867a98c")

    add_versions("archive:1.2.3", "85057f788630dc1b8c371f5443cc192627175003a8ea63c491beaff29a338346")
    add_versions("github:1.2.3", "fa2f29fd8ab3f72e3efb551d19cb6b214ead91f8")
    
    add_configs("interp", {description = "Enable interpreter", default = true, type = "boolean"})
    add_configs("fast_interp", {description = "Enable fast interpreter", default = false, type = "boolean"})
    add_configs("aot", {description = "Enable AOT", default = false, type = "boolean"})
    -- TODO: improve llvm
    add_configs("jit", {description = "Enable JIT", default = false, type = "boolean", readonly = true})
    add_configs("fast_jit", {description = "Enable Fast JIT", default = false, type = "boolean", readonly = true})
    add_configs("libc", {description = "Choose libc", default = "builtin", type = "string", values = {"builtin", "wasi", "uvwasi"}})
    add_configs("libc_builtin", {description = "Enable builtin libc", default = true, type = "boolean"})
    add_configs("libc_wasi", {description = "Enable wasi libc", default = true, type = "boolean"})
    add_configs("libc_uvwasi", {description = "Enable uvwasi libc", default = true, type = "boolean"})
    add_configs("multi_module", {description = "Enable multiple modules", default = false, type = "boolean"})
    add_configs("mini_loader", {description = "Enable wasm mini loader", default = false, type = "boolean"})
    add_configs("wasi_threads", {description = "Enable wasi threads library", default = false, type = "boolean"})
    add_configs("simd", {description = "Enable SIMD", default = false, type = "boolean"})
    add_configs("ref_types", {description = "Enable reference types", default = false, type = "boolean"})

    on_install("windows", function (package)
        os.cp(path.join(package:scriptdir(), "port", "xmake.lua"), "xmake.lua")
        import("package.tools.xmake").install(package, configs)
    end)

    on_test(function (package)
        assert(package:has_cfuncs("wasm_engine_new", {includes = "wasm_c_api.h", {configs = {languages = "c99"}}}))
    end)