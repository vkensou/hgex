package("cgpu")
    add_urls("https://github.com/vkensou/cgpu.git")
    add_versions("latest", "develop")

    add_deps("spirv-reflect")

    on_install(function(package)
        import("package.tools.xmake").install(package, {}, {target="cgpu"})
    end)
