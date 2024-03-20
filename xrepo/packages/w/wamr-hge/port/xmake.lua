target("wamr")
    set_kind("shared")
    set_languages("c17", "cxx20")
    add_rules("mode.debug", "mode.release")

    add_includedirs("core/shared/platform/include")
    add_headerfiles("core/shared/platform/include/*.h", {install = false})

    add_defines("WASM_ENABLE_BULK_MEMORY=1")
    add_defines("WASM_ENABLE_SHARED_MEMORY=1")
    add_defines("WASM_ENABLE_MODULE_INST_CONTEXT=1")

    add_includedirs("core/shared/platform/common/libc-util")
    add_headerfiles("core/shared/platform/common/libc-util/*.h", {install = false})
    add_files("core/shared/platform/common/libc-util/*.c")

    if is_os("windows") then
        add_defines("BH_PLATFORM_WINDOWS")
        add_defines("HAVE_STRUCT_TIMESPEC")
        add_defines("_WINSOCK_DEPRECATED_NO_WARNINGS")

        add_includedirs("core/shared/platform/windows")
        add_headerfiles("core/shared/platform/windows/*.h", {install = false})
        add_files("core/shared/platform/windows/*.c")
        add_files("core/shared/platform/windows/*.cpp")

        add_links("ntdll")
    end

    if is_arch("x64") then
        add_defines("BUILD_TARGET_X86_64")
    elseif is_arch("x86") then
        add_defines("BUILD_TARGET_X86_32")
    end

    add_defines("COMPILING_WASM_RUNTIME_API")

    add_includedirs("core/shared/utils")
    add_headerfiles("core/shared/utils/*.h", {install = false})
    add_files("core/shared/utils/*.c")

    add_includedirs("core/shared/mem-alloc")
    add_headerfiles("core/shared/mem-alloc/*.h", {install = false})
    add_files("core/shared/mem-alloc/*.c")
    add_files("core/shared/mem-alloc/ems/*.c")


    add_defines("BH_MALLOC=wasm_runtime_malloc")
    add_defines("BH_FREE=wasm_runtime_free")
    add_includedirs("core/iwasm/common")
    add_files("core/iwasm/common/*.c")

    if is_arch("x64") then
        if is_os("windows") then 
            add_files("core/iwasm/common/arch/invokeNative_em64.asm")
        end
    end 

    add_includedirs("core/iwasm/include")
    add_headerfiles("core/iwasm/include/*.h")

    add_defines("WASM_ENABLE_INTERP=1")
    add_includedirs("core/iwasm/interpreter")
    add_files("core/iwasm/interpreter/wasm_loader.c")
    add_files("core/iwasm/interpreter/wasm_interp_classic.c")
    add_files("core/iwasm/interpreter/wasm_runtime.c")
    
    add_defines("WASM_ENABLE_LIBC_WASI=1")
    add_includedirs("core/iwasm/libraries/libc-wasi/sandboxed-system-primitives/include")
    add_includedirs("core/iwasm/libraries/libc-wasi/sandboxed-system-primitives/src")
    add_files("core/iwasm/libraries/libc-wasi/*.c")
    add_files("core/iwasm/libraries/libc-wasi/sandboxed-system-primitives/src/*.c")

    add_defines("WASM_ENABLE_THREAD_MGR=1")
    add_includedirs("core/iwasm/libraries/thread-mgr")
    add_files("core/iwasm/libraries/thread-mgr/*.c")

    add_defines("WASM_ENABLE_LIB_WASI_THREADS=1")
    add_defines("WASM_ENABLE_HEAP_AUX_STACK_ALLOCATION=1")
    add_includedirs("core/iwasm/libraries/lib-wasi-threads")
    add_files("core/iwasm/libraries/lib-wasi-threads/*.c")

    on_config(function (target)
        if target:has_tool("cxx", "cl") then 
            target:add("cflags", "/experimental:c11atomics")
        end 
    end)